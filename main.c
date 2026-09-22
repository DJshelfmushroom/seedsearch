#include "seedsearch.h"
#include "parseargs.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


int main(int argc, char **argv) {
    setvbuf(stdout, NULL, _IOFBF, 0);
    parse_args(argc, argv);
    initConfigs(MC);
    Generator go, gn;
    setupGenerator(&go, MC, 0);
    setupGenerator(&gn, MC, 0);
    int64_t seed;
    int nwant = argc - 2;
    int wanto[nwant];
    int wantn[nwant];
    int regionSizes[nwant];
    // parse args, in order: [prog name, starting seed (-1 is random), feature numbers sequentially]
    uint64_t rng;
    setSeed(&rng, time(NULL));
    seed = strcmp(argv[1], "-1") == 0 ? (int64_t)nextLong(&rng) : strtoll(argv[1], NULL, 10);
    int wanto_count = 0;
    int wantn_count = 0;
    for (int i = 0; i < nwant; i++) {
        // wanto[i] = (int)strtol(argv[i + 2], NULL, 10);
        StructureConfig sc;
        int structureType = (int)strtol(argv[i + 2], NULL, 10);
        if (!getConfig((int)strtol(argv[i + 2], NULL, 10), &sc)) {
            fprintf(stderr, "structure %d is not available in this version\n", structureType);
            return 1;
        }
        // region size in blocks
        regionSizes[i] = sc.regionSize << 4;
        if (sc.dim == DIM_OVERWORLD) {
            wanto[wanto_count] = structureType;
            wanto_count++;
        } else if (sc.dim == DIM_NETHER) {
            wantn[wantn_count] = structureType;
        }
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
            struct checkParams params = {wanto[i], &go, seed, spawn, r0x, r1x, r0z, r1z};
            Pos structPos;
            if (structureCheck(params, &structPos)) {
                printf("HIT %" PRIi64 " %s %i,%i\n" , seed, struct2str(wanto[i]), structPos.x, structPos.z);
                if (ferror(stdout)) return 1;
            }
        }
        if (seed % 1000 == 0) {
            fflush(stdout);
        }
    }
    return 0;
}
