#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include "cubiomes/generator.h"
#include "cubiomes/finders.h"
#include "cubiomes/util.h"
#include "cubiomes/rng.h"
#include "seedsearch.h"
#include "util.h"

#define MC MC_1_16_1

StructureConfig configs[FEATURE_NUM];
static bool configValid[FEATURE_NUM];

void initConfigs(int mc) {
    for (int i = 0; i < FEATURE_NUM; i++)
        configValid[i] = getStructureConfig(i, mc, &configs[i]);

}

void *checkSeeds(void *arg) {
    // don't touch it it works
    CheckSeedsParams params = *(CheckSeedsParams *)arg;
    for (int64_t seed = params.start; seed <= params.end; seed++) {
        bool success = true;
        Pos locs[params.filter.groups[0].count];
        Group sGroup;
        for (int i = 0; i < params.filter.count; i++) {
            success = true; // if this group fails, loop repeats
            Group group = params.filter.groups[i];
            for (int j = 0; j < params.filter.groups[i].count; j++) {
                Term term = group.terms[j];
                int checkDist = term.dist + 524; // don't ask why 524
                CheckParams check_params = {term.type, NULL, seed, {0,0}, checkDist};
                Pos structPos;
                if (!structurePosCheck(check_params, &structPos)) { success = false; break; } // a required structure is not in this seed
                locs[j] = structPos;
            }
            if (success) { sGroup = group; break; } //GOOD GROUP
        }
        if (!success) {
            goto end;
            /* hit logic if i need it, deletable
            printf("HIT %lld ", seed);
            for (int i = 0; i < sGroup.count; i++) {
                printf("%s %i,%i ", struct2str(sGroup.terms[i].type), locs[i].x, locs[i].z);
            }
            printf("\n");
            */
        }
        // applySeed(params.go, DIM_OVERWORLD, seed);
        applySeed(params.gn, DIM_NETHER, seed);
        for (int i = 0; i < sGroup.count; i++) {
            
        }
        /*
        Pos spawn = getSpawn(params.go);
        int r0x, r1x, r0z, r1z;
        int px = spawn.x, pz = spawn.z;
        int minX = px - 96, maxX = px + 96;
        int minZ = pz - 96, maxZ = pz + 96;
        for (int i = 0; i < sizeof(params.groups)/sizeof(params.groups[0]); i++) {
            for (int j = 0; j < params.groups[i].count; j++) {
                //TODO filter by position
            }
        }
        /*
        calcRegionBounds(params.regionSizesO[0], minX, maxX, minZ, maxZ, &r0x, &r1x, &r0z, &r1z);
        Pos structPos;
        checkParams checkparams = {params.wantO[0], params.go, seed, spawn, r0x, r1x, r0z, r1z};
        bool portal = false;
        if (structureCheck(checkparams, &structPos)) {
            // printf("HIT %" PRIi64 " %s %i,%i\n" , seed, struct2str(11), structPos.x, structPos.z);
            portal = true;
            if (ferror(stdout)) return NULL;
        }
        Pos portalPos = structPos;
        minX -= 32, maxX += 32, minZ -= 32, maxZ += 32;
        calcRegionBounds(params.regionSizesN[0], minX, maxX, minZ, maxZ, &r0x, &r1x, &r0z, &r1z);
        checkparams.structureType = params.wantN[0];
        checkparams.g = params.gn;
        checkparams.spawn = structPos;
        checkparams.spawn.x = floor(portalPos.x / 8.0);
        checkparams.spawn.z = floor(portalPos.z / 8.0);
        bool bastion = false;
        if (structureCheck(checkparams, &structPos)) {
            bastion = true;
            // printf("HIT %" PRIi64 " %s %i,%i\t%s %i,%i\n" , seed, struct2str(wantO[0]), portalPos.x, portalPos.z, struct2str(wantN[0]), structPos.x, structPos.z);
        }
        Pos bastionPos = structPos;
        calcRegionBounds(params.regionSizesN[1], minX, maxX, minZ, maxZ, &r0x, &r1x, &r0z, &r1z);
        checkparams.structureType = params.wantN[1];
        checkparams.g = params.gn;
        checkparams.spawn = structPos;
        checkparams.spawn.x = floor(bastionPos.x);
        checkparams.spawn.z = floor(bastionPos.z);
        if (structureCheck(checkparams, &structPos) && portal && bastion) {
            printf("HIT %" PRIi64 " %s %i,%i\t%s %i,%i\t %s %i,%i\n" , seed, struct2str(params.wantO[0]), portalPos.x, portalPos.z, struct2str(params.wantN[0]), bastionPos.x, bastionPos.z, struct2str(params.wantN[1]), structPos.x, structPos.z);
        }
        */
        if (seed % 1000 == 0) {
            fflush(stdout);
        }
    }
    end:
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
