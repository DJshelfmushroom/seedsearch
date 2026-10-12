#include "util.h"
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

int str2struct(const char *name) {
    for (int i = 0; i < FEATURE_NUM; i++) {
        const char *s = struct2str(i);
        if (s && strcmp(s, name) == 0) return i;
    }
    return -1;
}

// fuck ass parser with error handling
bool parse_i64(const char *s, int64_t *out) {
    char *end;
    errno = 0;
    long long v = strtoll(s, &end, 10);
    if (end == s) return false;          // no digits at all: "abc"
    if (*end != '\0') return false;      // junk after the number: "128x"
    if (errno == ERANGE) return false;   // too big to fit in 64 bits
    *out = v;
    return true;
}

void seed_search_threaded(int nthreads, int64_t startSeed, Filter filter) {
    Generator genOverworld[nthreads];
    Generator genNether[nthreads];
    pthread_t threads[nthreads];
    CheckSeedsParams params[nthreads];
    // TODO counter with mutex for batching; modify checkSeeds as well
    while (true) {
        for (int t = 0; t < nthreads; t++) {
            setupGenerator(&genOverworld[t], MC, 0);
            setupGenerator(&genNether[t], MC, 0);
            params[t] = (CheckSeedsParams){startSeed+(t*10000), startSeed+((t+1)*10000) - 1, &genOverworld[t], &genNether[t], filter};
            pthread_create(&threads[t], NULL, checkSeeds, &params[t]);
        }
        for (int t = 0; t < nthreads; t++)
            pthread_join(threads[t], NULL);
        startSeed += 10000*nthreads;
        if (ferror(stdout)) { printf("SIGPIPE, exiting."); break; }
    }

}

static inline int max(int a, int b) { return a > b ? a : b; }
static inline int min(int a, int b) { return a < b ? a : b; }

// documented in util.h
bool searchForStructures(Group* group, int term, PosBox area, Pos picked[], GroupCandidates* candidates) {
    if (term == group->count) return true; // all terms in this group are successful
    Term t = group->terms[term];
    int dim = configs[t.type].dim;
    for (int i = 0; i < candidates->count[term]; i++) {
        Pos pos = candidates->locs[term][i];
        PosBox next = area;
        if (t.from == FROM_SPAWN) {
            PosBox need;
            if (dim == DIM_OVERWORLD) {
                need = (PosBox){ pos.x - t.dist, pos.z - t.dist, pos.x + t.dist, pos.z + t.dist };
            } else { // nether
                need = (PosBox){ 8*(pos.x - t.dist), 8*(pos.z - t.dist), 8*(pos.x + t.dist) + 7, 8*(pos.z + t.dist) + 7 };
            }
            // clamp and shrink the box as much as possible
            next.minX = max(area.minX, need.minX); next.maxX = min(area.maxX, need.maxX);
            next.minZ = max(area.minZ, need.minZ); next.maxZ = min(area.maxZ, need.maxZ);
            if (next.minX > next.maxX || next.minZ > next.maxZ) continue; // spawn doesn't work here
        } else {
            Pos center = picked[t.from];
            int centerDim = configs[group->terms[t.from].type].dim;
            if (centerDim == DIM_OVERWORLD && dim == DIM_NETHER) center = (Pos){ floordiv(center.x, 8), floordiv(center.z, 8)};
            else if (centerDim == DIM_NETHER && dim == DIM_OVERWORLD) center = (Pos){ center.x * 8, center.z * 8};
            if (abs(pos.x - center.x) > t.dist || abs(pos.z - center.z) > t.dist) continue;
        }
        picked[term] = pos;
        if (searchForStructures(group, term+1, next, picked, candidates)) return true;
    }
    return false; // no location works for group.terms[term], this group fails
}
