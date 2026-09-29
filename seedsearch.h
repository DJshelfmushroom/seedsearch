#pragma once
#include <stdbool.h>
#include "cubiomes/generator.h"
#include "cubiomes/finders.h"
#include "cubiomes/util.h"
#define MC MC_1_16_1

#ifndef SEEDSEARCH_SEEDSEARCH_H
#define SEEDSEARCH_SEEDSEARCH_H
typedef struct {
    int structureType;
    Generator *g;
    uint64_t seed;
    Pos center;
    int dist;
} CheckParams;

extern StructureConfig configs[FEATURE_NUM];

bool structurePosCheck(CheckParams params, Pos *outPos);
bool structureCheck(CheckParams params, Pos *outPos);
void initConfigs(int mc);
bool getConfig(int structureType, StructureConfig *out);
void *checkSeeds(void *arg);

#endif //SEEDSEARCH_SEEDSEARCH_H
