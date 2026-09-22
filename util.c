#include "util.h"
#include <string.h>

int str2struct(const char *name) {
    for (int i = 0; i < FEATURE_NUM; i++) {
        const char *s = struct2str(i);
        if (s && strcmp(s, name) == 0) return i;
    }
    return -1;
}

