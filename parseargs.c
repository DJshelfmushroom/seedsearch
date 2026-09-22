#include <stdio.h>
#include <unistd.h>
#include <inttypes.h>
#include <stdlib.h>

static void usage(FILE *out, const char *prog) {
    fprintf(out, "usage: %1$s [-s seed] <filter>\n"
                 "       %1$s\n"
                 "A filter is defined by a list of structure names combined with spaces or + signs, with an optional distance to check from spawn.\n"
                 "For example, ruined_portal+fortress:128\n", prog);
}

void parse_args(int argc, char *argv[]) {
    int opt;

    while ((opt = getopt(argc, argv, ":s:h")) != -1) {
        switch (opt) {
            case 's':
                printf("s passed with value %" PRId64 "\n", strtoll(optarg, NULL, 10));
                break;
            case 'h':
                usage(stderr, argv[0]);
                exit(0);
                break;
        }
    }
    if (optind < argc)
    {
        printf("Non-option args: ");
        while (optind < argc)
            printf("%s ", argv[optind++]);
        printf("\n");
    }
}