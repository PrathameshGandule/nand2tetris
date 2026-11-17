# Hack Computer Emulator

A custom Hack Computer emulator written in C++ using SDL2, built as part of the [nand2tetris](https://www.nand2tetris.org/) project.

The emulator loads Hack machine code (`.hack` files), executes instructions, manages RAM and displays the memory-mapped screen in an SDL2 window.

## Features

- Executes Hack A-instructions and C-instructions.
- Implements CPU registers (`A`, `D`) and RAM.
- Supports Hack computation and jump instructions.
- Memory-mapped screen (512 × 256 pixels).
- Keyboard input through SDL2.
- SDL streaming texture for efficient screen rendering.
- Batched CPU instruction execution.
- Screen refresh rate of approximately 60 FPS.

## Requirements

- C++ compiler supporting C++17 or later.
- SDL2 development libraries.
- Linux (or another SDL2-supported platform).

## Compile
- make sure to install sdl2 on your system
```bash
g++ cpu.cpp -o cpu $(sdl2-config --cflags --libs)
```

## Usage

```bash
./cpu <filename> <start_address> <end_address>
```

Example:

```bash
./cpu test.hack 16380 16400
```

Arguments:
- `filename`: Path to the Hack machine-code file.
- `start_address`: Starting RAM address to inspect after execution.
- `end_address`: Ending RAM address to inspect after execution.

## Memory Map

| Address Range | Description |
|---|---|
| 0–16383 | General-purpose RAM |
| 16384–24575 | Screen memory |
| 24576 | Keyboard memory-mapped register |

The screen uses 8192 16-bit words to represent 512 × 256 pixels.

## Project

This project is an implementation of the Hack computer architecture from the nand2tetris course, with a custom execution engine and SDL2-based graphical display.