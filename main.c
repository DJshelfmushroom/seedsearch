#include "seedsearch.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


int main(int argc, char **argv) {
    setvbuf(stdout, NULL, _IOFBF, 0);
    if (argc < 3) {
        fprintf(stderr, "usage: %s <seed|-1> <struct> [struct,struct,...]\n", argv[0]);
        return 1;
    }
    initConfigs(MC);
    Generator go, gn;
    setupGenerator(&go, MC, 0);
    setupGenerator(&gn, MC, 0);
    int64_t seed;
    int nwant = argc - 2;
    int want[nwant];
    int regionSizes[nwant];
    // parse args, in order: [prog name, starting seed (-1 is random), feature numbers sequentially]
    uint64_t rng;
    setSeed(&rng, time(NULL));
    seed = strcmp(argv[1], "-1") == 0 ? (int64_t)nextLong(&rng) : strtoll(argv[1], NULL, 10);
    for (int i = 0; i < nwant; i++) {
        want[i] = (int)strtol(argv[i + 2], NULL, 10);
        StructureConfig sc;
        if (!getConfig(want[i], &sc)) {
            fprintf(stderr, "structure %d is not available in this version\n", want[i]);
            return 1;
        }
        // region size in blocks
        regionSizes[i] = sc.regionSize << 4;
    }
    for (;;seed++) {
        applySeed(&go, DIM_OVERWORLD, seed);
        applySeed(&gn, DIM_NETHER, seed);
        Pos spawn = getSpawn(&go);
        int r0x, r1x, r0z, r1z;
        int px = spawn.x, pz = spawn.z;
        int minX = px - 96, maxX = px + 96;
        int minZ = pz - 96, maxZ = pz + 96;
        for (int i = 0; i < nwant; i++) {
            calcRegionBounds(regionSizes[i], minX, maxX, minZ, maxZ, &r0x, &r1x, &r0z, &r1z);
            struct checkParams params = {want[i], &go, seed, spawn, r0x, r1x, r0z, r1z};
            Pos structPos;
            if (structureCheck(params, &structPos)) {
                printf("HIT %" PRIi64 " %s %i,%i\n" , seed, struct2str(want[i]), structPos.x, structPos.z);
                if (ferror(stdout)) return 1;
            }
        }
        if (seed % 1000 == 0) {
            fflush(stdout);
        }
    }
    return 0;
}
