#pragma once
#include "cubiomes/finders.h"
#include "cubiomes/generator.h"
#include "cubiomes/util.h"
#include "seedsearch.h"
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>


#ifndef SEEDSEARCH_UTIL_H
#define SEEDSEARCH_UTIL_H

#define MAX_TERMS 8
#define FROM_SPAWN -1

typedef struct { int type; int dist; int from; } Term;        // one structure. from is the index of an earlier term to measure dist from, or FROM_SPAWN
typedef struct { Term terms[MAX_TERMS]; int count; } Group;   // all must match
typedef struct { Group groups[MAX_TERMS]; int count; } Filter;

typedef struct {
    int64_t* start;
    int64_t offset;
    Generator* go;
    Generator* gn;
    Filter filter;
    atomic_bool* stop; // atomic so the ctrl-c handler can set it (it can't lock a mutex)
    pthread_mutex_t* mtx; // guards start
} CheckSeedsParams;

typedef struct {
    int minX;
    int minZ;
    int maxX;
    int maxZ;
} PosBox;

typedef struct {
    Pos* locs[MAX_TERMS]; // one Pos array for each term in the group
    // locs[j][k] = k'th position found for term index j
    int count[MAX_TERMS]; // how many positions found
} GroupCandidates;

int str2struct(const char *name);
bool parse_i64(const char *s, int64_t *out);
void seed_search_threaded(int nthreads, int64_t startSeed, Filter filter);
/**
 * checks all possible positions for a given Term within an area, with reference to previous positions for anchored terms
 * @param group the group to check
 * @param term the index of the term to start the search; almost always 0
 * @param area a PosBox of where spawn (or the search center) should be
 * @param picked previous positions found indexed at group.terms[]
 * @param candidates a GroupCandidates with the pos array for this term (and the anchor if needed)
 * @return bool, every term from `term` on has a candidate that fits, with spawn somewhere in area (fills picked)
 */
bool searchForStructures(Group* group, int term, PosBox area, Pos picked[], GroupCandidates* candidates);
#endif //SEEDSEARCH_UTIL_H
