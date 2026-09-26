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

static StructureConfig configs[FEATURE_NUM];
static bool configValid[FEATURE_NUM];

void initConfigs(int mc) {
    for (int i = 0; i < FEATURE_NUM; i++)
        configValid[i] = getStructureConfig(i, mc, &configs[i]);

}

void *checkSeeds(void *arg) {
    struct checkSeedsParams params = *(struct checkSeedsParams *)arg;
    int64_t start = params.start;
    int64_t end = params.end;
    for (int64_t seed = start; seed <= end; seed++) {
        applySeed(params.go, DIM_OVERWORLD, seed);
        applySeed(params.gn, DIM_NETHER, seed);
        Pos spawn = getSpawn(params.go);
        int r0x, r1x, r0z, r1z;
        int px = spawn.x, pz = spawn.z;
        int minX = px - 96, maxX = px + 96;
        int minZ = pz - 96, maxZ = pz + 96;
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
        if (seed % 1000 == 0) {
            // fflush(stdout);
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

bool structureCheck(struct checkParams params, Pos *outPos) {
    for (int rx = params.r0x; rx <= params.r1x; rx++){
        for (int rz = params.r0z; rz <= params.r1z; rz++){
            Pos pos;
            if (!getStructurePos(params.structureType, MC, params.seed, rx, rz, &pos)) continue;

            int dx = pos.x - params.spawn.x, dz = pos.z - params.spawn.z;
            if(abs(dx) > 96 || abs(dz) > 96) continue;
            if (!isViableStructurePos(params.structureType, params.g, pos.x, pos.z, 0)) continue;
            *outPos = pos;
            return true;
        }
    }
    return false;
}


void calcRegionBounds(int regionSize, int minX, int maxX, int minZ, int maxZ, int *r0x, int *r1x, int *r0z, int *r1z) {
    *r0x = floordiv(minX,regionSize);
    *r1x = floordiv(maxX,regionSize);
    *r0z = floordiv(minZ,regionSize);
    *r1z = floordiv(maxZ,regionSize);
}
