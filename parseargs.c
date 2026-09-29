#include "parseargs.h"
#include "util.h"
#include <stdio.h>
#include <unistd.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

bool parse_group(char *text, Group *g) {
    g->count = 0;
    // loop through the text, splitting by "+". when strtok returns NULL, the loop exits.
    for (char *term = strtok(text, "+"); term; term = strtok(NULL, "+"), g->count++) {
        if (g->count >= MAX_TERMS) return false; // improper group, too many terms.
        Term* t = &(g->terms[g->count]);
        t->dist = 96;
        t->from = FROM_SPAWN;
        char* at = strchr(term, '@');
        if (at != NULL) {
            *at = '\0'; // structure:123@anchor becomes "structure:123" and "anchor"
            int anchor = str2struct(at + 1);
            if (anchor < 0) return false; // anchor isn't a structure
            // get the closest anchor (not the first)
            for (int k = g->count - 1; k >= 0 && t->from == FROM_SPAWN; k--) {
                if (g->terms[k].type == anchor) t->from = k;
            }
            if (t->from == FROM_SPAWN) return false; // anchor has to come earlier in the group
        }
        char* colon = strchr(term, ':');
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
    fprintf(out, "usage: %1$s [-s seed] <filter>...\n\n"
                 "A filter is a list of structure names joined with + signs, each with an optional :distance to check from spawn (default 96, max 2048).\n"
                 "For example, ruined_portal+fortress:128\n\n"
                 "Add @name after a structure to measure its distance from an earlier structure in the group instead of from spawn.\n"
                 "For example, ruined_portal+bastion_remnant:64@ruined_portal+fortress:128@bastion_remnant\n\n"
                 "Separate filters with spaces to accept seeds that match any of them.\n", prog);
}

int64_t parse_args(int argc, char *argv[], char* patterns[]) {
    int opt;
    uint64_t rng;
    setSeed(&rng, time(NULL));
    int64_t seed = (int64_t) nextLong(&rng);
    while ((opt = getopt(argc, argv, ":s:h")) != -1) {
        switch (opt) {
            case 's':
                if (!parse_i64(optarg, &seed)) {
                    fprintf(stderr, "invalid seed: %s\n", optarg);
                    exit(1);
                }
                break;
            case 'h':
                usage(stderr, argv[0]);
                exit(0);
                break;
            case ':':
                fprintf(stderr, "option -%c needs a value\n", optopt);
                exit(1);
            default:
                fprintf(stderr, "unknown option -%c\n", optopt);
                usage(stderr, argv[0]);
                exit(1);
        }
    }
    if (optind >= argc) { // nothing to search for
        usage(stderr, argv[0]);
        exit(1);
    }
    int c = 0;
    for (int i = optind; i < argc; i++, c++) {
        patterns[c] = argv[i];
    }
    patterns[c] = NULL;
    return seed;
}