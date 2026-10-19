#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include "cubiomes/generator.h"
#include "cubiomes/finders.h"
#include "cubiomes/util.h"
#include "cubiomes/rng.h"
#include "seedsearch.h"
#include "util.h"

// how far spawn can be from 0,0 on either axis
#define SPAWN_RANGE 524

StructureConfig configs[FEATURE_NUM];
static bool configValid[FEATURE_NUM];

void initConfigs(int mc) {
    for (int i = 0; i < FEATURE_NUM; i++)
        configValid[i] = getStructureConfig(i, mc, &configs[i]);

}

// How far each term's structure can be from 0,0 in its own dimension. The prefilter searches
// this far so it never throws out a seed the full check would accept: an anchored term's range
// is its anchor's range, converted between dimensions, plus its own dist.
static void prefilterRanges(const Group *group, int out[], int spawnRange) {
    for (int j = 0; j < group->count; j++) {
        Term term = group->terms[j];
        int dim = (int)configs[term.type].dim;
        int range = spawnRange; // fresh each term, so an anchored term can't leak its range into later ones
        int centerDim = DIM_OVERWORLD;
        if (term.from != FROM_SPAWN) {
            range = out[term.from];
            centerDim = (int)configs[group->terms[term.from].type].dim;
        }
        if (centerDim == DIM_OVERWORLD && dim == DIM_NETHER) range = range / 8 + 1;
        if (centerDim == DIM_NETHER && dim == DIM_OVERWORLD) range *= 8;
        out[j] = range + term.dist;
    }
}

void *checkSeeds(void *arg) {
    CheckSeedsParams params = *(CheckSeedsParams *)arg;
    int ranges[MAX_TERMS][MAX_TERMS];
    for (int i = 0; i < params.filter.count; i++)
        prefilterRanges(&params.filter.groups[i], ranges[i], SPAWN_RANGE);
    GroupCandidates candidates[params.filter.count]; // one GroupCandidates per group
    for (int i = 0; i < params.filter.count; i++) { // groups
        Group group = params.filter.groups[i];
        for (int j = 0; j < group.count; j++) { // terms in group i
            int regionSize = configs[group.terms[j].type].regionSize << 4; // NOLINT(*-str34-c)
            int maxLocs = (2 * ranges[i][j] / regionSize + 2); // one structure per region
            maxLocs = maxLocs * maxLocs;
            candidates[i].locs[j] = malloc(maxLocs * sizeof(Pos)); // allocate max structure positions for this term
        }
    }

    PosBox anywhere = { -SPAWN_RANGE, -SPAWN_RANGE, SPAWN_RANGE, SPAWN_RANGE }; // generic PosBox for our max spawn range
    while (!atomic_load(params.stop)) {
        pthread_mutex_lock(params.mtx);
            int64_t batch = *params.start;
            *params.start += params.offset;
        pthread_mutex_unlock(params.mtx);
        // checked every seed so ctrl-c doesn't wait for the batch to finish; relaxed is enough for a flag
        for (int64_t seed = batch; seed < batch + params.offset && !atomic_load_explicit(params.stop, memory_order_relaxed); seed++) {
            int successful[MAX_TERMS]; // indicies of successful groups
            int successfulCount = 0;
            Pos picked[MAX_TERMS];

            // 1. check all locations against max spawn range. cheap, no spawn check needed
            for (int i = 0; i < params.filter.count; i++) { // all groups in filter
                Group group = params.filter.groups[i];
                bool ok = true;
                for (int j = 0; j < group.count; j++) {
                    // all terms in group
                    Term term = group.terms[j];
                    candidates[i].count[j] = 0;
                    int regionSize = configs[term.type].regionSize << 4;
                    int r0x = floordiv(-ranges[i][j], regionSize), r1x = floordiv(ranges[i][j], regionSize);
                    int r0z = r0x, r1z = r1x;
                    for (int rx = r0x; rx <= r1x; rx++){
                        for (int rz = r0z; rz <= r1z; rz++){
                            Pos pos;
                            if (getStructurePos(term.type, MC, seed, rx, rz, &pos)
                                && abs(pos.x) <= ranges[i][j]
                                && abs(pos.z) <= ranges[i][j])
                            {
                                candidates[i].locs[j][candidates[i].count[j]++] = pos; //structure found; in bounds; set the position and increment the count
                            }
                        }
                    }
                    if (candidates[i].count[j] == 0) { ok = false; break; }
                }
                if (ok && searchForStructures(&group, 0, anywhere, picked, &candidates[i])) successful[successfulCount++] = i; // found a solution, this group could work.
            }
            if (successfulCount == 0) goto end;

            // 2. make sure found structures are in viable biomes
            applySeed(params.go, DIM_OVERWORLD, seed);
            // check if any nether terms
            bool needNether = false;
            for (int i = 0; i < successfulCount && !needNether; i++) { // groups in filter
                for (int j = 0; j < params.filter.groups[successful[i]].count; j++) { // terms j in group i
                    if (configs[params.filter.groups[successful[i]].terms[j].type].dim == DIM_NETHER) { needNether = true; break; }
                }
            }
            if (needNether) applySeed(params.gn, DIM_NETHER, seed);
            int kept = 0;
            for (int s = 0; s < successfulCount; s++) { // all sucessful groups i
                int i = successful[s];
                Group group = params.filter.groups[i];
                bool ok = true;
                for (int j = 0; j < group.count; j++) { // terms j in group i
                    Term term = group.terms[j];
                    Generator* g = configs[term.type].dim == DIM_OVERWORLD ? params.go : params.gn;
                    // keep successful positions, moving them to the front.
                    int pos_kept_count = 0;
                    for (int k = 0; k < candidates[i].count[j]; k++) {
                        Pos pos = candidates[i].locs[j][k];
                        if (isViableStructurePos(term.type, g, pos.x, pos.z, 0)) candidates[i].locs[j][pos_kept_count++] = pos; // if it is viable, put it into the next candidate slot
                    }
                    candidates[i].count[j] = pos_kept_count;
                    if (pos_kept_count == 0) { ok = false; break; }
                }
                if (ok && searchForStructures(&group, 0, anywhere, picked, &candidates[i])) successful[kept++] = i;
            }
            successfulCount = kept;
            if (successfulCount == 0) goto end;

            // 3. check near estimated spawn
            // intentional: getSpawn can wander ~256 blocks from the estimate, so this ±128 box drops some valid seeds. that's an accepted tradeoff for skipping most getSpawn calls.
            Pos spawnEst = estimateSpawn(params.go, NULL);
            PosBox nearSpawn = {spawnEst.x - 128, spawnEst.z - 128, spawnEst.x + 128, spawnEst.z + 128 };
            kept = 0;
            for (int s = 0; s < successfulCount; s++) {
                int i = successful[s];
                Group group = params.filter.groups[i];
                if (searchForStructures(&group, 0, nearSpawn, picked, &candidates[i])) successful[kept++] = i;
            }
            successfulCount = kept;
            if (successfulCount == 0) goto end;

            // 4. check against actual spawn
            Pos spawn = getSpawnEstimated(params.go, spawnEst, 0);
            PosBox exactSpawn = { spawn.x, spawn.z, spawn.x, spawn.z };
            for (int s = 0; s < successfulCount; s++) {
                int i = successful[s];
                Group group = params.filter.groups[i];
                if (searchForStructures(&group, 0, exactSpawn, picked, &candidates[i])) { // success!
                    flockfile(stdout); // keep other threads from writing into the middle of this line
                    printf("HIT %" PRId64 " spawn %d,%d ", seed, spawn.x, spawn.z);
                    for (int j = 0; j < group.count; j++) {
                        printf("%s %i,%i ", struct2str(group.terms[j].type), picked[j].x, picked[j].z);
                    }
                    printf("\n");
                    funlockfile(stdout);
                    break; //TODO remove this to print all hits for a seed
                }
            }
            end:;
            if (seed % 1000 == 0) {
                fflush(stdout);
                if (ferror(stdout)) atomic_store(params.stop, true); // reader went away (e.g. `| head`); SIGPIPE is ignored
            }
        }
    }
    for (int i = 0; i < params.filter.count; i++)
        for (int j = 0; j < params.filter.groups[i].count; j++)
            free(candidates[i].locs[j]);
    return NULL;
}

bool getConfig(int structureType, StructureConfig *out) {
    if (structureType < 0 || structureType >= FEATURE_NUM || !configValid[structureType])
        return false;
    *out = configs[structureType];
    return true;
}

static bool findStructure(CheckParams params, bool viable, Pos *outPos) {
    int regionSize = configs[params.structureType].regionSize << 4;
    int r0x = floordiv(params.center.x - params.dist, regionSize), r1x = floordiv(params.center.x + params.dist, regionSize);
    int r0z = floordiv(params.center.z - params.dist, regionSize), r1z = floordiv(params.center.z + params.dist, regionSize);
    for (int rx = r0x; rx <= r1x; rx++){
        for (int rz = r0z; rz <= r1z; rz++){
            Pos pos;
            if (!getStructurePos(params.structureType, MC, params.seed, rx, rz, &pos)) continue;

            int dx = pos.x - params.center.x, dz = pos.z - params.center.z;
            if(abs(dx) > params.dist || abs(dz) > params.dist) continue;
            if (viable && !isViableStructurePos(params.structureType, params.g, pos.x, pos.z, 0)) continue;
            *outPos = pos;
            return true;
        }
    }
    return false;
}

// position only: depends on the seed alone, does not need a generator
bool structurePosCheck(CheckParams params, Pos *outPos) {
    return findStructure(params, false, outPos);
}

// position plus biome viability: params.g must have the seed applied in the structure's dimension
bool structureCheck(CheckParams params, Pos *outPos) {
    return findStructure(params, true, outPos);
}
