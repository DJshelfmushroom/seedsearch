#include <stdio.h>
#include <stdbool.h>
#include "cubiomes/generator.h"
#include "cubiomes/finders.h"

struct checkParams{
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
/**
 * Checks whether a viable village generates within 96 blocks of the given
 * point in the configured region range.
 *
 * @param params Generator, seed, center point, and region bounds to check.
 * @return true if a viable village is found; otherwise false.
 * @note The generator must already be seeded for the overworld.
 */
bool villageCheck(struct checkParams params){
    for (int rx = params.r0x; rx <= params.r1x; rx++){
        for (int rz = params.r0z; rz <= params.r1z; rz++){
            Pos pos;
            if (!getStructurePos(Village, MC_1_16_1, params.seed, rx, rz, &pos)) continue; // no village in this region

            int dx = pos.x - params.spawn.x, dz = pos.z - params.spawn.z;
            if(abs(dx) > 96 || abs(dz) > 96) continue;
            if (!isViableStructurePos(Village, params.g, pos.x, pos.z, 0)) continue;
            return true;
        }
    }
    return false;
}

bool portalCheck(struct checkParams params){
    for (int rx = params.r0x; rx <= params.r1x; rx++){
        for (int rz = params.r0z; rz <= params.r1z; rz++){
            Pos pos;
            if (!getStructurePos(Ruined_Portal, MC_1_16_1, params.seed, rx, rz, &pos)) continue;

            int dx = pos.x - params.spawn.x, dz = pos.z - params.spawn.z;
            if(abs(dx) > 96 || abs(dz) > 96) continue;
            if (!isViableStructurePos(Ruined_Portal, params.g, pos.x, pos.z, 0)) continue;
            return true;
        }
    }
    return false;
}
//TODO: Make a singular structure check

void calcRegionBounds(int regionSize, int minX, int maxX, int minZ, int maxZ, int *r0x, int *r1x, int *r0z, int *r1z) {
    *r0x = floordiv(minX,regionSize);
    *r1x = floordiv(maxX,regionSize);
    *r0z = floordiv(minZ,regionSize);
    *r1z = floordiv(maxZ,regionSize);
}

int main(void) {
    initConfigs(MC_1_16_1);
    Generator g;
    setupGenerator(&g, MC_1_16_1, 0);
    uint64_t seed;
    for (seed = 0; ; seed++) {
        applySeed(&g, DIM_OVERWORLD, seed);
        Pos spawn = getSpawn(&g);
        int r0x, r1x, r0z, r1z;
        int px = spawn.x, pz = spawn.z;
        int minX = px - 96, maxX = px + 96;
        int minZ = pz - 96, maxZ = pz + 96;
        // Once per structure, get the region size (in blocks)
        int regionSize = configs[Village].regionSize << 4;
        calcRegionBounds(regionSize, minX, maxX, minZ, maxZ, &r0x, &r1x, &r0z, &r1z);
        struct checkParams params = {&g, seed, spawn, r0x, r1x, r0z, r1z};
        if (villageCheck(params)) printf("seed: %" PRIu64 "\n", seed);
    }
    return 0;
}
