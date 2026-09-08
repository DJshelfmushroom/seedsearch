#include <stdio.h>
// #include <time.h>
#include <stdbool.h>
#include "cubiomes/generator.h"
#include "cubiomes/finders.h"

struct checkParams{
    Generator g;
    uint64_t seed;
    Pos spawn;
    int r0x;
    int r1x;
    int r0z;
    int r1z;
};

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
            if (!isViableStructurePos(Village, &params.g, pos.x, pos.z, 0)) continue;
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
            if (!isViableStructurePos(Ruined_Portal, &params.g, pos.x, pos.z, 0)) continue;
            return true;
        }
    }
    return false;
}

int main(void) {
    Generator g;
    setupGenerator(&g, MC_1_16_1, 0);
    uint64_t seed;
    for (seed = 0; ; seed++) {
        applySeed(&g, DIM_OVERWORLD, seed);
        Pos spawn = getSpawn(&g);
        // Region grid for this seed
        int px = spawn.x, pz = spawn.z;
        int minX = px - 96, maxX = px + 96;
        int minZ = pz - 96, maxZ = pz + 96;
        int r0x = minX >> 4 >> 5;
        int r1x = maxX >> 4 >> 5;
        int r0z = minZ >> 4 >> 5;
        int r1z = maxZ >> 4 >> 5;
        struct checkParams params = {g, seed, spawn, r0x, r1x, r0z, r1z};
        if (portalCheck(params)) printf("seed: %llu\n", seed);
    }
    return 0;
}
