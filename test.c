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

static void test_structureCheck(void) {
    Generator g;
    setupGenerator(&g, MC, 0);
    uint64_t seed = 1;
    applySeed(&g, DIM_OVERWORLD, seed);
    Pos spawn = getSpawn(&g);
    CheckParams params = {Village, &g, seed, spawn, 96};
    Pos structPos;
    assert(structureCheck(params, &structPos) == false);
    seed = 21;
    applySeed(&g, DIM_OVERWORLD, seed);
    spawn = getSpawn(&g);
    params.seed = seed;
    params.center = spawn;
    assert(structureCheck(params, &structPos) == true);
    assert(structPos.x == -160);
    assert(structPos.z == 80);
    seed = 31;
    applySeed(&g, DIM_OVERWORLD, seed);
    spawn = getSpawn(&g);
    params.seed = seed;
    params.center = spawn;
    assert(structureCheck(params, &structPos) == true);
    assert(structPos.x == 160);
    assert(structPos.z == 0);
    seed = 32;
    applySeed(&g, DIM_OVERWORLD, seed);
    spawn = getSpawn(&g);
    params.seed = seed;
    params.center = spawn;
    assert(structureCheck(params, &structPos) == true);
    assert(structPos.x == 240);
    assert(structPos.z == -160);
}

static void test_structurePosCheck(void) {
    Generator g;
    setupGenerator(&g, MC, 0);
    uint64_t seed = 2;
    applySeed(&g, DIM_OVERWORLD, seed);
    Pos spawn = getSpawn(&g);
    CheckParams params = {Village, &g, seed, spawn, 96};
    Pos structPos;
    // a village position is in range, but its biome can't hold a village
    assert(structurePosCheck(params, &structPos) == true);
    assert(structPos.x == 0);
    assert(structPos.z == 64);
    assert(structureCheck(params, &structPos) == false);
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
        { "fortress",                                     true, 1, {{Fortress, 96, FROM_SPAWN}} },
        { "fortress+bastion_remnant:128",                 true, 2, {{Fortress, 96, FROM_SPAWN}, {Bastion, 128, FROM_SPAWN}} },
        {"ruined_portal+fortress:64+bastion_remnant:124", true, 3, {{Ruined_Portal, 96, FROM_SPAWN}, {Fortress, 64, FROM_SPAWN}, {Bastion, 124, FROM_SPAWN}} },
        // anchors: @name measures from an earlier term instead of spawn
        { "ruined_portal+bastion_remnant:64@ruined_portal+fortress:128@bastion_remnant",
                                                          true, 3, {{Ruined_Portal, 96, FROM_SPAWN}, {Bastion, 64, 0}, {Fortress, 128, 1}} },
        { "ruined_portal+bastion_remnant@ruined_portal",  true, 2, {{Ruined_Portal, 96, FROM_SPAWN}, {Bastion, 96, 0}} },
        // duplicate anchor name picks the closest earlier one
        { "ruined_portal+ruined_portal+fortress@ruined_portal",
                                                          true, 3, {{Ruined_Portal, 96, FROM_SPAWN}, {Ruined_Portal, 96, FROM_SPAWN}, {Fortress, 96, 1}} },
        { "fortress@ruined_portal",                       false, 0, {}}, // anchor isn't earlier
        { "fortress@fortress",                            false, 0, {}}, // anchored to itself
        { "ruined_portal+fortress@fortres",               false, 0, {}}, // unknown anchor
        { "ruined_portal+fortress@",                      false, 0, {}}, // empty anchor
        { "ruined_portal+fortress@ruined_portal:64",      false, 0, {}}, // distance has to come before the anchor
    };

    for (size_t i = 0; i < sizeof cases / sizeof cases[0]; i++) {
        char str[128];
        strcpy(str, cases[i].input);
        Group g;
        bool ok = parse_group(str, &g);

        assert(ok == cases[i].ok);
        if (!ok) continue;
        assert(g.count == cases[i].count);
        for (int t = 0; t < g.count; t++) {
            assert(g.terms[t].type == cases[i].terms[t].type);
            assert(g.terms[t].dist == cases[i].terms[t].dist);
            assert(g.terms[t].from == cases[i].terms[t].from);
        }
    }
}

int main(void) {
    test_getConfig();
    test_structureCheck();
    test_structurePosCheck();
    test_parse_group();
    printf(GREEN "all tests pass\n" RESET);
    return 0;
}