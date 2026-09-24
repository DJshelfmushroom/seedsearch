#pragma once
#include "cubiomes/finders.h"
#include "cubiomes/generator.h"
#include "cubiomes/util.h"
#include "seedsearch.h"
#include <pthread.h>

#ifndef SEEDSEARCH_UTIL_H
#define SEEDSEARCH_UTIL_H
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
int str2struct(const char *name);
bool parse_i64(const char *s, int64_t *out);
void seed_search_threaded(int nthreads, int *wantO, int *wantN, int *regionSizesO, int *regionSizesN);
#endif //SEEDSEARCH_UTIL_H
