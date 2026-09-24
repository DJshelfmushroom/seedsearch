#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "parseargs.h"
#include "cubiomes/generator.h"
#include "cubiomes/finders.h"
#include "seedsearch.h"

#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define RESET   "\x1b[0m"

static void test_regionBounds(void) {
    //check proper bounds for village
    Generator g;
    setupGenerator(&g, MC, 0);
    int r0x, r1x, r0z, r1z;
    uint64_t seed = 0;
    applySeed(&g, DIM_OVERWORLD, seed);
    calcRegionBounds(512, -80, 112, -64, 128, &r0x, &r1x, &r0z, &r1z);
    assert(r0x == -1);
    assert(r1x == 0);
    assert(r0z == -1);
    assert(r1z == 0);
    // most cases are like this ^, seed 25 is not.
    seed = 25;
    applySeed(&g, DIM_OVERWORLD, seed);
    calcRegionBounds(512, 80, 272, -112, 80, &r0x, &r1x, &r0z, &r1z);
    assert(r0x == 0);
    assert(r1x == 0);
    assert(r0z == -1);
    assert(r1z == 0);
}

static void test_structureCheck(void) {
    Generator g;
    setupGenerator(&g, MC, 0);
    uint64_t seed = 1;
    applySeed(&g, DIM_OVERWORLD, seed);
    Pos spawn = getSpawn(&g);
    struct checkParams params = {Village, &g, seed, spawn, -1, 0, -1, 0};
    Pos structPos;
    assert(structureCheck(params, &structPos) == false);
    seed = 21;
    applySeed(&g, DIM_OVERWORLD, seed);
    spawn = getSpawn(&g);
    params.seed = seed;
    params.spawn = spawn;
    assert(structureCheck(params, &structPos) == true);
    assert(structPos.x == -160);
    assert(structPos.z == 80);
    seed = 31;
    applySeed(&g, DIM_OVERWORLD, seed);
    spawn = getSpawn(&g);
    params.seed = seed;
    params.spawn = spawn;
    assert(structureCheck(params, &structPos) == true);
    assert(structPos.x == 160);
    assert(structPos.z == 0);
    seed = 32;
    applySeed(&g, DIM_OVERWORLD, seed);
    spawn = getSpawn(&g);
    params.seed = seed;
    params.spawn = spawn;
    params.r0x = 0;
    params.r1z = -1;
    assert(structureCheck(params, &structPos) == true);
    assert(structPos.x == 240);
    assert(structPos.z == -160);
}

static void test_getConfig(void) {
    initConfigs(MC);
    StructureConfig config;
    assert(!getConfig(99, NULL));
    assert(getConfig(Village /* 5 */, &config));
    assert(config.regionSize == 32);
    assert(config.chunkRange == 24);
    assert(config.structType == Village); // 5
    assert(!config.rarity); // 0
}

static void test_parse_group(void) {
    struct {
        const char *input;
        bool ok;          // should it parse?
        int count;        // how many terms
        Term terms[3];    // expected terms, if ok
    } cases[] = {
        { "",                                             false, 0, {}},
        { "fortres",                                      false, 0, {}},
        { "fortress:abc",                                 false, 0, {}},
        { "fortress",                                     true, 1, {{Fortress, 96}} },
        { "fortress+bastion_remnant:128",                 true, 2, {{Fortress, 96}, {Bastion, 128}} },
        {"ruined_portal+fortress:64+bastion_remnant:124", true, 3, {{Ruined_Portal, 96}, {Fortress, 64}, {Bastion, 124}} },
    };

    for (size_t i = 0; i < sizeof cases / sizeof cases[0]; i++) {
        char str[64];
        strcpy(str, cases[i].input);
        Group g;
        bool ok = parse_group(str, &g);

        assert(ok == cases[i].ok);
        if (!ok) continue;
        assert(g.count == cases[i].count);
        for (int t = 0; t < g.count; t++) {
            assert(g.terms[t].type == cases[i].terms[t].type);
            assert(g.terms[t].dist == cases[i].terms[t].dist);
        }
    }
}

int main(void) {
    test_getConfig();
    test_regionBounds();
    test_structureCheck();
    test_parse_group();
    printf(GREEN "all tests pass\n" RESET);
    return 0;
}