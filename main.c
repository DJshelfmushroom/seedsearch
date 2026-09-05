#include <stdio.h>
// #include <time.h>
#include <stdbool.h>
#include "cubiomes/generator.h"
#include "cubiomes/finders.h"

/**
* Checks whether a village generates within 6 chunks of the given point.
*
* @param g      generator, already seeded for the overworld
* @param seed   world seed
* @param spawn  center point in block coordinates
* @return true if a viable village is in range
* @note assumes applySeed has already been called
*/
bool villageCheck(Generator *g, uint64_t seed, Pos spawn){
    // assumes applySeed has already been called
    int px = spawn.x, pz = spawn.z;
    int minX = px - 96, maxX = px + 96;
    int minZ = pz - 96, maxZ = pz + 96;
    // 96 is 6 chunks in blocks

    // info for looking up region bounds
    // divide by 16 to get chunk coords, then by 32 to get region coords. use shift to round down instead of towards 0 to prevent an edge case
    int r0x = minX >> 4 >> 5;
    int r1x = maxX >> 4 >> 5;
    int r0z = minZ >> 4 >> 5;
    int r1z = maxZ >> 4 >> 5;
    for (int rx = r0x; rx <= r1x; rx++){
        for (int rz = r0z; rz <= r1z; rz++){
            Pos pos;
            if (!getStructurePos(Village, MC_1_16_1, seed, rx, rz, &pos)) continue; // no village in this region

            int dx = pos.x - px, dz = pos.z - pz;
            if(abs(dx) > 96 || abs(dz) > 96) continue;
            if (!isViableStructurePos(Village, g, pos.x, pos.z, 0)) continue;
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
        if(villageCheck(&g, seed, spawn))printf("seed: %llu\n",seed);
    }
    return 0;
}
