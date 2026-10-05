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
static void prefilterRanges(const Group *group, int out[]) {
    for (int j = 0; j < group->count; j++) {
        Term term = group->terms[j];
        int dim = (int)configs[term.type].dim;
        int range = SPAWN_RANGE;
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
        prefilterRanges(&params.filter.groups[i], ranges[i]);
    for (int64_t seed = params.start; seed <= params.end; seed++) {
        Group groups[MAX_TERMS];
        int successfulCount = 0;
        // prefilter
        for (int i = 0; i < params.filter.count; i++) {
            bool success = true;
            Group group = params.filter.groups[i];
            for (int j = 0; j < group.count; j++) {
                CheckParams check_params = {group.terms[j].type, NULL, seed, {0,0}, ranges[i][j]};
                Pos structPos;
                if (!structurePosCheck(check_params, &structPos)) { success = false; break; } // a required structure is not in this seed
            }
            if (success) { groups[successfulCount] = group; successfulCount++; } //GOOD GROUP
        }
        if (successfulCount == 0) goto end;

        applySeed(params.go, DIM_OVERWORLD, seed);
        applySeed(params.gn, DIM_NETHER, seed);
        Pos spawn = getSpawn(params.go);
        Pos found[MAX_TERMS][MAX_TERMS];
        int successIndex = -1;
        for (int i = 0; i < successfulCount; i++) {
            bool ok = true;
            for (int j = 0; j < groups[i].count; j++) {
                Term term = groups[i].terms[j];
                int dim = (int)configs[term.type].dim;
                // spawn is an overworld position, so treat it like an overworld anchor
                Pos center = spawn;
                int centerDim = DIM_OVERWORLD;
                if (term.from != FROM_SPAWN) { // we're looking from another structure
                    center = found[i][term.from];
                    centerDim = (int)configs[groups[i].terms[term.from].type].dim;
                }
                // convert the center into the term's dimension
                if (centerDim == DIM_OVERWORLD && dim == DIM_NETHER) center = (Pos){floordiv(center.x, 8), floordiv(center.z, 8)};
                if (centerDim == DIM_NETHER && dim == DIM_OVERWORLD) center = (Pos){center.x * 8, center.z * 8};
                Generator *g = dim == DIM_NETHER ? params.gn : params.go;
                CheckParams check_params = {term.type, g, seed, center, term.dist};
                // this assumes 1 succesful group per seed and may lose some, but the chances are astronomically small that two groups would be correct, and it doesn't really matter for this use case.
                if (!structureCheck(check_params, &found[i][j])) { ok = false; break; }
            }
            if (ok) { successIndex = i; break; } // group successful, one group per seed.
        }
        if (successIndex == -1) goto end;
        flockfile(stdout); // keep other threads from writing into the middle of this line
        printf("HIT %" PRId64 " ", seed);
        for (int i = 0; i < groups[successIndex].count; i++) {
            printf("%s %i,%i ", struct2str(groups[successIndex].terms[i].type), found[successIndex][i].x, found[successIndex][i].z);
        }
        printf("\n");
        funlockfile(stdout);
        end: ;
        if (seed % 1000 == 0) {
            fflush(stdout);
        }
    }
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
