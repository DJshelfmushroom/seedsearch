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

static void test_searchForStructures(void) {
    initConfigs(MC);
    Pos picked[MAX_TERMS];
    Pos ow[2], ne[2];
    GroupCandidates c = {{ow, ne}, {1, 1}};

    // overworld from spawn: spawn has to be within dist of the structure
    Group g = {{{Village, 96, FROM_SPAWN}}, 1};
    ow[0] = (Pos){100, 0};
    assert(!searchForStructures(&g, 0, (PosBox){0, 0, 0, 0}, picked, &c));
    assert(searchForStructures(&g, 0, (PosBox){4, 0, 4, 0}, picked, &c));
    assert(picked[0].x == 100 && picked[0].z == 0);

    // nether from spawn: spawn/8 within dist, so a nether block covers 8 overworld blocks (the +7)
    g = (Group){{{Fortress, 0, FROM_SPAWN}}, 1};
    c.locs[0] = ne;
    ne[0] = (Pos){10, 0};
    assert(searchForStructures(&g, 0, (PosBox){80, 0, 80, 0}, picked, &c));
    assert(searchForStructures(&g, 0, (PosBox){87, 0, 87, 0}, picked, &c));
    assert(!searchForStructures(&g, 0, (PosBox){88, 0, 88, 0}, picked, &c));
    assert(!searchForStructures(&g, 0, (PosBox){79, 0, 79, 0}, picked, &c));

    // two spawn terms whose boxes don't overlap can't both hold
    g = (Group){{{Village, 10, FROM_SPAWN}, {Village, 10, FROM_SPAWN}}, 2};
    c.locs[0] = ow; c.locs[1] = ow + 1;
    ow[0] = (Pos){0, 0}; ow[1] = (Pos){100, 0};
    assert(!searchForStructures(&g, 0, (PosBox){-524, -524, 524, 524}, picked, &c));

    // anchored overworld -> nether uses floordiv, and backtracks to the candidate that works
    g = (Group){{{Village, 1000, FROM_SPAWN}, {Fortress, 5, 0}}, 2};
    c.locs[0] = ow; c.locs[1] = ne;
    c.count[0] = 2; c.count[1] = 1;
    ow[0] = (Pos){200, 0}; ow[1] = (Pos){-1, 0}; // floordiv(-1, 8) == -1, truncation would give 0
    ne[0] = (Pos){-6, 0};
    assert(searchForStructures(&g, 0, (PosBox){0, 0, 0, 0}, picked, &c));
    assert(picked[0].x == -1 && picked[1].x == -6);
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
    test_searchForStructures();
    test_parse_group();
    printf(GREEN "all tests pass\n" RESET);
    return 0;
}