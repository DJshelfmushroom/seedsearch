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

#define MC MC_1_16_1

static StructureConfig configs[FEATURE_NUM];
static bool configValid[FEATURE_NUM];

void initConfigs(int mc) {
    for (int i = 0; i < FEATURE_NUM; i++)
        configValid[i] = getStructureConfig(i, mc, &configs[i]);

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
