#include "util.h"
#include <string.h>

int str2struct(const char *name) {
    for (int i = 0; i < FEATURE_NUM; i++) {
        const char *s = struct2str(i);
        if (s && strcmp(s, name) == 0) return i;
    }
    return -1;
}

void seed_search_threaded(int nthreads, int *wantO, int *wantN, int *regionSizesO, int *regionSizesN) {
    Generator genOverworld[nthreads];
    Generator genNether[nthreads];
    pthread_t threads[nthreads];
    struct checkSeedsParams params[nthreads];
    for (int t = 0; t < nthreads; t++) {
        setupGenerator(&genOverworld[t], MC, 0);
        setupGenerator(&genNether[t], MC, 0);
        params[t] = (struct checkSeedsParams){t*10000, (t*10000)+10000, &genOverworld[t], &genNether[t], regionSizesO, regionSizesN, wantO, wantN};
        pthread_create(&threads[t], NULL, checkSeeds, &params[t]);
    }
    for (int t = 0; t < nthreads; t++)
        pthread_join(threads[t], NULL);
}
