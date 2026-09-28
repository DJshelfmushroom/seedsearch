#pragma once
#include "cubiomes/finders.h"
#include "cubiomes/generator.h"
#include "cubiomes/util.h"
#include "seedsearch.h"
#include <pthread.h>


#ifndef SEEDSEARCH_UTIL_H
#define SEEDSEARCH_UTIL_H

#define MAX_TERMS 8
#define FROM_SPAWN -1

typedef struct { int type; int dist; int from; } Term;        // one structure. from is the index of an earlier term to measure dist from, or FROM_SPAWN
typedef struct { Term terms[MAX_TERMS]; int count; } Group;   // all must match
typedef struct { Group groups[MAX_TERMS]; int count; } Filter;

typedef struct {
    int64_t start;
    int64_t end;
    Generator* go;
    Generator* gn;
    int *regionSizesO;
    int *regionSizesN;
    int *wantO;
    int *wantN;
    Filter filter;
} CheckSeedsParams;

int str2struct(const char *name);
bool parse_i64(const char *s, int64_t *out);
void seed_search_threaded(int nthreads, int *wantO, int *wantN, int *regionSizesO, int *regionSizesN, Filter filter);
#endif //SEEDSEARCH_UTIL_H
