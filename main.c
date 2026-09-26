#include "seedsearch.h"
#include "cpucount.h"
#include "parseargs.h"
#include "util.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

int main(int argc, char **argv) {
    setvbuf(stdout, NULL, _IOFBF, 0);
    int nthreads = cpuCount();
    char* patterns[argc];
    int64_t seed = parse_args(argc, argv, patterns);
    initConfigs(MC);
    Group groups[argc];
    int ngroups = 0;
    for (int i = 0; patterns[i]; i++, ngroups++) {
        parse_group(patterns[i], &groups[i]);
    }
    Generator go, gn;
    setupGenerator(&go, MC, 0);
    setupGenerator(&gn, MC, 0);
    /* (depracated arg parsing)
    int nwant = argc - 2;
    int wantO[nwant];
    int wantN[nwant];
    int regionSizesO[nwant];
    int regionSizesN[nwant];
    int wanto_count = 0;
    int wantn_count = 0;
    for (int i = 0; i < nwant; i++) {
        // wanto[i] = (int)strtol(argv[i + 2], NULL, 10);
        StructureConfig sc;
        int structureType = (int)strtol(argv[i + 2], NULL, 10);
        if (!getConfig((int)strtol(argv[i + 2], NULL, 10), &sc)) {
            fprintf(stderr, "structure %d is not available in this version\n", structureType);
            return 1;
        }
        // region size in blocks
        if (sc.dim == DIM_OVERWORLD) {
            regionSizesO[wanto_count] = sc.regionSize << 4;
            wantO[wanto_count] = structureType;
            wanto_count++;
        } else if (sc.dim == DIM_NETHER) {
            regionSizesN[wantn_count] = sc.regionSize << 4;
            wantN[wantn_count] = structureType;
            wantn_count++;
        }
    }
    */

    // seed_search_threaded(nthreads, wantO, wantN, regionSizesO, regionSizesN);
    return 0;
}
