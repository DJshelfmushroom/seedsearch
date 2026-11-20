Finds Minecraft Java Edition 1.16.1 seeds that have the structures you choose near spawn. It uses every CPU core and is built on [cubiomes](https://github.com/Cubitect/cubiomes).

## Downloads

| Platform | File |
| --- | --- |
| Windows (x86_64) | `seedsearch-{{TAG}}-windows-x86_64.zip` |
| Linux (x86_64) | `seedsearch-{{TAG}}-linux-x86_64.tar.gz` |
| macOS (Apple Silicon and Intel) | `seedsearch-{{TAG}}-macos-universal.tar.gz` |

## Usage

```sh
tar xzf seedsearch-{{TAG}}-*.tar.gz
./seedsearch [-s seed] [-e end seed] <filter>...
```

### macOS

The binary is not signed, so macOS may refuse to open it. To allow it, run this once:

```sh
xattr -d com.apple.quarantine seedsearch
```
