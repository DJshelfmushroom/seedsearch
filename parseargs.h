#ifndef SEEDSEARCH_PARSEARGS_H
#define SEEDSEARCH_PARSEARGS_H
#include <stdbool.h>
#define MAX_TERMS 8

typedef struct { int type; int dist; } Term;                  // one structure
typedef struct { Term terms[MAX_TERMS]; int count; } Group;   // all must match

bool parse_group(char *text, Group *g);
int64_t parse_args(int argc, char **argv, char** patterns);
#endif //SEEDSEARCH_PARSEARGS_H
