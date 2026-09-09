#include <stdio.h>
#include <stdbool.h>
#include "cubiomes/generator.h"
#include "cubiomes/finders.h"

int main(void) {
    for (enum StructureType structure = 1; structure < FEATURE_NUM; structure++) {
        StructureConfig config;
        getStructureConfig(structure, MC_1_16_1, &config);
        printf("\"%i\":%i,", structure, config.chunkRange);
    }
}