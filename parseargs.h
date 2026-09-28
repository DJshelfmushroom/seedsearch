#pragma once
#include "util.h"

#ifndef SEEDSEARCH_PARSEARGS_H
#define SEEDSEARCH_PARSEARGS_H
#include <stdbool.h>

bool parse_group(char *text, Group *g);
int64_t parse_args(int argc, char **argv, char** patterns);
#endif //SEEDSEARCH_PARSEARGS_H
