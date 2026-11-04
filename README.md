# seedsearch

seedsearch finds Minecraft Java Edition 1.16.1 seeds that have the structures you choose near spawn.

```sh
$ ./seedsearch -s 0 village:64+ruined_portal:64
HIT 30217 spawn 176,160 village 176,176 ruined_portal 112,208
HIT 10525 spawn 256,208 village 192,160 ruined_portal 320,192
...
```

## Install

Download the file for your platform from the [latest release](https://github.com/DJshelfmushroom/seedsearch/releases/latest).

| Platform | File |
| --- | --- |
| Linux (x86_64) | `seedsearch-<version>-linux-x86_64.tar.gz` |
| macOS (Apple Silicon and Intel) | `seedsearch-<version>-macos-universal.tar.gz` |
| Windows (x86_64) | `seedsearch-<version>-windows-x86_64.zip` |

On Linux and macOS, unpack it and run it from a terminal:

```sh
tar xzf seedsearch-*.tar.gz
./seedsearch -h
```

On Windows, unzip it and run `seedsearch.exe` from a terminal.

> [!NOTE]
> The binary is not signed, so macOS may refuse to open it. Run this once to allow it:
> ```sh
> xattr -d com.apple.quarantine seedsearch
> ```

## Build from source

You need a C compiler (gcc or clang), `make`, and `git`.

```sh
git clone --recurse-submodules https://github.com/DJshelfmushroom/seedsearch.git
cd seedsearch
make
```

This makes `./seedsearch`. The default build runs on any CPU of the same type. To build for your own CPU only (a little faster, but the binary may not run on other machines), use:

```sh
make native
```

Other targets:

- `make test` builds and runs the tests.
- `make debug` builds `./seedsearch-debug` with debug symbols and no optimization.
- `make clean` removes the builds.

On Windows, build in an MSYS2 UCRT64 shell with `make CC=gcc`.

## Usage

```
./seedsearch [-s seed] [-e end seed] <filter> [filter]...
```

| Option | Meaning |
| --- | --- |
| `-s seed` | First seed to check. Without it, seedsearch starts at a random seed. |
| `-e seed` | Last seed to check (this seed is included). Without it, seedsearch runs until you press Ctrl-C. |
| `-h` | Show the help text. |

## Filters

A filter is a list of structure names joined with `+`. A seed matches a filter when it has all of those structures near spawn.

```sh
./seedsearch village+ruined_portal
```

### Distance

Add `:N` after a name to set how far the structure can be from spawn, in blocks. The default is 96 and the largest value is 2048.

```sh
./seedsearch village:64+ruined_portal:200
```

> [!WARNING]
> The distance is a square, not a circle. `village:64` means the village is at most 64 blocks from spawn on the X axis and at most 64 blocks on the Z axis. This is the same shape as render distance. So a structure at 60,60 matches `:64`, even though it is about 85 blocks away in a straight line.

### Nether structures

`fortress`, `bastion_remnant`, and `ruined_portal_nether` are in the Nether. For these, the distance and the printed coordinates are in Nether blocks. One Nether block is 8 Overworld blocks.

So `fortress:96` means the fortress is within 96 Nether blocks (on each axis) of the Nether point under spawn. That point is the spawn X and Z divided by 8. For example, with spawn at 80,192, the Nether point is 10,24.

### Measure from another structure

Add `@name` after a structure to measure its distance from an earlier structure in the same filter, instead of from spawn. The earlier structure must come first in the filter. If the name is in the filter more than once, seedsearch uses the nearest earlier one.

```sh
./seedsearch ruined_portal+bastion_remnant:64@ruined_portal+fortress:128@bastion_remnant
```

This finds a ruined portal within 96 blocks of spawn, a bastion within 64 Nether blocks of that portal, and a fortress within 128 Nether blocks of that bastion. seedsearch converts between Overworld and Nether coordinates for you.

### More than one filter

Separate filters with spaces. A seed matches when it matches any one of them.

```sh
./seedsearch village:64 desert_pyramid:64
```

A filter can have up to 8 structures, and you can give up to 8 filters.

### Structure names

| Name | Dimension |
| --- | --- |
| `village` | Overworld |
| `ruined_portal` | Overworld |
| `desert_pyramid` | Overworld |
| `jungle_pyramid` | Overworld |
| `swamp_hut` | Overworld |
| `igloo` | Overworld |
| `pillager_outpost` | Overworld |
| `shipwreck` | Overworld |
| `ocean_ruin` | Overworld |
| `monument` | Overworld |
| `mansion` | Overworld |
| `buried_treasure` | Overworld |
| `mineshaft` | Overworld |
| `desert_well` | Overworld |
| `fortress` | Nether |
| `bastion_remnant` | Nether |
| `ruined_portal_nether` | Nether |

Strongholds and End structures (`end_city`, `end_gateway`) are not supported.

### Filter errors

If seedsearch cannot read a filter, it prints the filter and points at the part it could not read:

```
$ ./seedsearch fortress+bastion_remnant@village
invalid filter:
fortress+bastion_remnant@village
                         ^HERE
```

The usual causes are a misspelled name, a distance that is not a number from 1 to 2048, or an `@name` that does not come earlier in the filter.

## Output

Each matching seed prints one line:

```
HIT <seed> spawn <x>,<z> <structure> <x>,<z> <structure> <x>,<z> ...
```

```
HIT 117 spawn 80,192 ruined_portal 144,128 fortress -80,0
```

- The structures are in the same order as in the filter.
- Overworld coordinates are in Overworld blocks. Nether coordinates are in Nether blocks.
- If a seed matches more than one filter, seedsearch prints only the first filter that matches.
- seedsearch checks seeds on many threads at once, so the lines are not in seed order.

The output is plain text, so you can save it or pass it to other tools:

```sh
./seedsearch -s 0 -e 1000000 village:64 > hits.txt
./seedsearch village:64+ruined_portal:64 | head -5
```

When the reading program closes early (like `head`), seedsearch stops and prints `output closed, exiting.` to stderr. This is normal.

## Limits

- Only Minecraft Java Edition 1.16.1 is supported.
- The spawn point is a close estimate. The real spawn depends on grass blocks and can be a few blocks off.
- To go fast, seedsearch skips seeds whose spawn is far from where it first expects. Because of this, it can miss a small share (under 1%) of seeds that match.
