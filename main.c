#include "seedsearch.h"
#include "cpucount.h"
#include "parseargs.h"
#include "util.h"
#include <string.h>

int main(int argc, char **argv) {
    setvbuf(stdout, NULL, _IOFBF, 0);
    int nthreads = cpuCount();
    char* patterns[argc];
    int64_t endseed = INT64_MAX;
    int64_t seed = parse_args(argc, argv, patterns, &endseed);
    initConfigs(MC);
    Filter filter;
    int ngroups = 0;
    for (int i = 0; patterns[i]; i++, ngroups++) {
        if (ngroups >= MAX_TERMS) {
            fprintf(stderr, "too many filters (max %d)\n", MAX_TERMS);
            return 1;
        }
        char original[strlen(patterns[i]) + 1];
        strcpy(original, patterns[i]); // parse_group writes over its input, save this in case error
        int parse_out = parse_group(patterns[i], &filter.groups[i]);
        char ftext[sizeof(original) + 5];
        if (parse_out != -1) {
            for (int j = 0; j <= parse_out; j++) {
                if (j < parse_out) {
                    ftext[j] = ' ';
                } else {
                    ftext[j] = '^';
                }
            }
            ftext[parse_out+1] = '\0';
            strcat(ftext, "HERE");
            fprintf(stderr, "invalid filter:\n%s\n%s\n", original, ftext);
            return 1;
        }
    }
    filter.count = ngroups;
    seed_search_threaded(nthreads, seed, endseed, filter);
    return 0;
}
