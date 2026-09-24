#include "parseargs.h"
#include "util.h"
#include <stdio.h>
#include <unistd.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

bool parse_group(char *text, Group *g) {
    g->count = 0;
    // loop through the text, splitting by "+". when strtok returns NULL, the loop exits.
    for (char *term = strtok(text, "+"); term; term = strtok(NULL, "+"), g->count++) {
        if (g->count >= MAX_TERMS) return false; // improper group, too many terms.
        Term* t = &(g->terms[g->count]);
        char* colon = strchr(term, ':');
        t->dist = 96;
        if (colon != NULL){
            // there is a colon, but we can't use strtok again. split the string at the colon by replacing the colon with a null terminator, splitting it into two strings.
            *colon = '\0'; // structure:123 becomes "structure" and "123"
            int64_t dist;
            if (!parse_i64(colon + 1, &dist) || dist <= 0 || dist > 2048) {
                return false; // not a number, or not a sensible distance
            }
            t->dist = (int)dist;
        }
        t->type = str2struct(term);
        StructureConfig sc;
        if (t->type < 0 || !getConfig(t->type, &sc)) return false; // not a structure
    }
    return g->count > 0; // false for an empty group
}

static void usage(FILE *out, const char *prog) {
    fprintf(out, "usage: %1$s [-s seed] <filter>\n\n"
                 "A filter is defined by a list of structure names combined with spaces or + signs, with an optional distance to check from spawn.\n"
                 "For example, ruined_portal+fortress:128\n", prog);
}

int64_t parse_args(int argc, char *argv[], char* patterns[]) {
    int opt;
    uint64_t rng;
    setSeed(&rng, time(NULL));
    int64_t seed = (int64_t) nextLong(&rng);
    while ((opt = getopt(argc, argv, ":s:h")) != -1) {
        switch (opt) {
            case 's':
                seed = strtoll(optarg, NULL, 10);
                break;
            case 'h':
                usage(stderr, argv[0]);
                exit(0);
                break;
        }
    }
    if (optind < argc)
    {
        // while (optind < argc)
        int c = 0;
        for (int i = optind; i < argc; i++, c++) {
            patterns[c] = argv[i];
        }
        patterns[c] = NULL;
    } else {
        patterns[0] = NULL;
    }
    return seed;
}