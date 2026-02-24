# fat12-parser

Tool for FAT12 loop devices, capable of outputting files and printing directory content.

> **Platform:** Designed for **Linux** and uses **Linux system APIs** (tested on Linux).

## What it does

`fat12-parser` is a small C utility for working with **FAT12 disk images / loop devices**. It focuses on two common tasks:

- **Print directory contents** (list files/directories from the FAT12 filesystem)
- **Extract/output files** from the FAT12 filesystem to stdout or a destination


## Requirements

- `gcc`
- `make`

## Build

```sh
make build
```

This should produce the executable ( `bin/FAT12Parser`).

## Usage

### List directory contents

```sh
./fat12-parser <image> ls <path>
```

Examples:

```sh
./fat12-parser floppy.img /directory
./fat12-parser floppy.img /
```

### Output text files:

```sh
./fat12-parser <image> cat <file-path>
```

Examples:

```sh
./fat12-parser floppy.img cat /subdir/subdir2/file.txt
```
