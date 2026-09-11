#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include "cubiomes/generator.h"
#include "cubiomes/finders.h"
#include "cubiomes/util.h"
#include "cubiomes/rng.h"

struct checkParams{
    int structureType;
    Generator *g;
    uint64_t seed;
    Pos spawn;
    int r0x;
    int r1x;
    int r0z;
    int r1z;
};

static StructureConfig configs[FEATURE_NUM];
static bool configValid[FEATURE_NUM];

void initConfigs(int mc) {
    for (int i = 0; i < FEATURE_NUM; i++)
        configValid[i] = getStructureConfig(i, mc, &configs[i]);

}

bool structureCheck(struct checkParams params, Pos *outPos) {
    for (int rx = params.r0x; rx <= params.r1x; rx++){
        for (int rz = params.r0z; rz <= params.r1z; rz++){
            Pos pos;
            if (!getStructurePos(params.structureType, MC_1_16_1, params.seed, rx, rz, &pos)) continue;

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

int main(int argc, char **argv) {
    initConfigs(MC_1_16_1);
    Generator g;
    setupGenerator(&g, MC_1_16_1, 0);
    uint64_t seed;
    bool want[FEATURE_NUM] = {0};
    // parse args, in order: [prog name, starting seed (-1 is current time), comma separated list (string) of structures to search for]
    for (int i = 1; i < argc; i++) {
        if (i == 1) {
            uint64_t rng;
            setSeed(&rng, time(NULL));
            seed = strtoll(argv[1], NULL, 10)==-1?(uint64_t)nextLong(&rng):strtoll(argv[1], NULL, 10);
        } else if (i == 2) {
            //TODO
        }
    }
    for (;;seed++) {
        applySeed(&g, DIM_OVERWORLD, seed);
        Pos spawn = getSpawn(&g);
        int r0x, r1x, r0z, r1z;
        int px = spawn.x, pz = spawn.z;
        int minX = px - 96, maxX = px + 96;
        int minZ = pz - 96, maxZ = pz + 96;
        // Once per structure, get the region size (in blocks)
        //TODO: use argv[2] to select things to check
        int toCheck[] = {Village, Ruined_Portal, Desert_Pyramid, Jungle_Pyramid};
        for (int i = 0; i < sizeof(toCheck)/sizeof(toCheck[0]); i++) {
            int regionSize = configs[toCheck[i]].regionSize << 4;
            calcRegionBounds(regionSize, minX, maxX, minZ, maxZ, &r0x, &r1x, &r0z, &r1z);
            struct checkParams params = {toCheck[i], &g, seed, spawn, r0x, r1x, r0z, r1z};
            Pos structPos;
            if (structureCheck(params, &structPos)) {
                printf("HIT %" PRIu64 " %s %i,%i\n" , seed, struct2str(toCheck[i]), structPos.x, structPos.z);
                fflush(stdout);
            }
        }
    }
    return 0;
}
