#include <stdio.h>
#include "cubiomes/generator.h"
#include "cubiomes/finders.h"

int main(void) {
    Generator g;
    setupGenerator(&g, MC_1_16_1, 0);
    uint64_t seed;
    for (seed = 0; ; seed++) {
        applySeed(&g, DIM_OVERWORLD, seed);

        getStructurePos(Village, MC_1_16_1, seed, );
    }
    return 0;
}
