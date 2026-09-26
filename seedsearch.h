#pragma once
#include <stdbool.h>
#include "cubiomes/generator.h"
#include "cubiomes/finders.h"
#include "cubiomes/util.h"
#define MC MC_1_16_1

#ifndef SEEDSEARCH_SEEDSEARCH_H
#define SEEDSEARCH_SEEDSEARCH_H
struct checkParams{
    int structureType;
    Generator *g;
    uint64_t seed;
    Pos spawn;
    int r0x;
    int r1x;
    int r0z;
    int r1z;
} typedef checkParams;

bool structureCheck(struct checkParams params, Pos *outPos);
void calcRegionBounds(int regionSize, int minX, int maxX, int minZ, int maxZ, int *r0x, int *r1x, int *r0z, int *r1z);
void initConfigs(int mc);
bool getConfig(int structureType, StructureConfig *out);
void *checkSeeds(void *arg);

#endif //SEEDSEARCH_SEEDSEARCH_H
