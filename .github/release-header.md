Finds Minecraft Java Edition 1.16.1 seeds that have the structures you choose near spawn. It uses every CPU core and is built on [cubiomes](https://github.com/Cubitect/cubiomes).

## Downloads

| Platform | File |
| --- | --- |
| Linux (x86_64) | `seedsearch-{{TAG}}-linux-x86_64.tar.gz` |
| macOS (Apple Silicon and Intel) | `seedsearch-{{TAG}}-macos-universal.tar.gz` |

## Usage

```sh
tar xzf seedsearch-{{TAG}}-*.tar.gz
./seedsearch [-s seed] <filter>...
```

A filter is a list of structure names joined with `+`. Each name can take an optional `:distance` to check from spawn (default 96, max 2048). Add `@name` after a structure to measure its distance from an earlier structure in the filter instead of from spawn. To accept seeds that match any one of several filters, separate the filters with spaces.

```sh
./seedsearch ruined_portal+fortress:128
./seedsearch ruined_portal+bastion_remnant:64@ruined_portal+fortress:128@bastion_remnant
./seedsearch village:64 desert_pyramid:64
```

Each match prints as `HIT <seed> <structure> <x>,<z> ...`.

### macOS

The binary is not signed, so macOS may refuse to open it. To allow it, run this once:

```sh
xattr -d com.apple.quarantine seedsearch
```
