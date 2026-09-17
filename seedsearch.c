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

/*
int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "usage: %s <seed|-1> <struct> [struct,struct,...]\n", argv[0]);
        return 1;
    }
    initConfigs(MC);
    Generator g;
    setupGenerator(&g, MC, 0);
    int64_t seed;
    int want[argc - 2];
    // parse args, in order: [prog name, starting seed (-1 is random), feature numbers sequentially]
    for (int i = 1; i < argc; i++) {
        if (i == 1) {
            uint64_t rng;
            setSeed(&rng, time(NULL));
            seed = strcmp(argv[1], "-1") == 0 ? (int64_t)nextLong(&rng) : strtoll(argv[1], NULL, 10);
            continue;
        }
        want[i-2] = (int)strtol(argv[i], NULL, 10);
    }
    for (;;seed++) {
        applySeed(&g, DIM_OVERWORLD, seed);
        Pos spawn = getSpawn(&g);
        int r0x, r1x, r0z, r1z;
        int px = spawn.x, pz = spawn.z;
        int minX = px - 96, maxX = px + 96;
        int minZ = pz - 96, maxZ = pz + 96;
        // Once per structure, get the region size (in blocks)
        for (int i = 0; i < sizeof(want)/sizeof(want[0]); i++) {
            int regionSize = configs[want[i]].regionSize << 4;
            calcRegionBounds(regionSize, minX, maxX, minZ, maxZ, &r0x, &r1x, &r0z, &r1z);
            struct checkParams params = {want[i], &g, seed, spawn, r0x, r1x, r0z, r1z};
            Pos structPos;
            if (structureCheck(params, &structPos)) {
                printf("HIT %" PRIi64 " %s %i,%i\n" , seed, struct2str(want[i]), structPos.x, structPos.z);
                fflush(stdout);
            }
        }
    }
    return 0;
}
*/