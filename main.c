#include "seedsearch.h"
#include "cpucount.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <math.h>

struct checkSeedsParams {
    int64_t start;
    int64_t end;
    Generator *go;
    Generator *gn;
    int *regionSizesO;
    int *regionSizesN;
    int *wantO;
    int *wantN;
};

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

int main(int argc, char **argv) {
    // setvbuf(stdout, NULL, _IOFBF, 0);
    int nthreads = cpuCount();
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
    int wantO[nwant];
    int wantN[nwant];
    int regionSizesO[nwant];
    int regionSizesN[nwant];
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
        if (sc.dim == DIM_OVERWORLD) {
            regionSizesO[wanto_count] = sc.regionSize << 4;
            wantO[wanto_count] = structureType;
            wanto_count++;
        } else if (sc.dim == DIM_NETHER) {
            regionSizesN[wantn_count] = sc.regionSize << 4;
            wantN[wantn_count] = structureType;
            wantn_count++;
        }
    }

    Generator genOverworld[nthreads];
    Generator genNether[nthreads];
    pthread_t threads[nthreads];
    struct checkSeedsParams params[nthreads];
    for (int t = 0; t < nthreads; t++) {
        setupGenerator(&genOverworld[t], MC, 0);
        setupGenerator(&genNether[t], MC, 0);
        params[t] = (struct checkSeedsParams){t*10000, (t*10000)+10000, &genOverworld[t], &genNether[t], regionSizesO, regionSizesN, wantO, wantN};
        pthread_create(&threads[t], NULL, checkSeeds, &params[t]);
    }
    for (int t = 0; t < nthreads; t++)
        pthread_join(threads[t], NULL);
    // checkSeeds(check_seeds_params);
        /*
        // overworld structure check
        for (int i = 0; i < sizeof(wantO)/sizeof(wantO[0]); i++) {
            calcRegionBounds(regionSizesO[i], minX, maxX, minZ, maxZ, &r0x, &r1x, &r0z, &r1z);
            struct checkParams params = {wantO[i], &go, seed, spawn, r0x, r1x, r0z, r1z};
            Pos structPos;
            if (structureCheck(params, &structPos)) {
                printf("HIT %" PRIi64 " %s %i,%i\n" , seed, struct2str(wantO[i]), structPos.x, structPos.z);
                if (ferror(stdout)) return 1;
            }
        }
        // switch from 6 to 8 chunk search range
        minX -= 32, maxX += 32, minZ -= 32, maxZ += 32;
        // TODO nether structure check
        // this may not be necessary, as we want to search for structures in a more specific way.
        // for (int i = 0; i < sizeof(wantN)/sizeof(wantN[0]); i++) {
        //     calcRegionBounds(regionSizesN[i], minX, maxX, minZ, maxZ, &r0x, &r1x, &r0z, &r1z);
        //     struct checkParams params = {wantN[i], &gn, seed, spawn, r0x, r1x, r0z, r1z};
        //     Pos structPos;
        //     if (structureCheck(params, &structPos)) {
        //         printf("HIT %" PRIi64 " %s %i,%i\n" , seed, struct2str(wantN[i]), structPos.x, structPos.z);
        //         if (ferror(stdout)) return 1;
        //     }
        // }

        if (seed % 1000 == 0) {
            fflush(stdout);
        }
        */
    return 0;
}
