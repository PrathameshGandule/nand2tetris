# VM Translator

A C++ implementation of the **VM Translator** from the [Nand2Tetris](https://www.nand2tetris.org/) course.

The translator converts **Virtual Machine (VM) commands** into **Hack assembly (`.asm`) code**.

## Features

* Arithmetic and logical commands

  * `add`, `sub`, `neg`
  * `and`, `or`, `not`
  * `eq`, `gt`, `lt`
* Memory access

  * `push`
  * `pop`
  * `local`, `argument`, `this`, `that`
  * `constant`, `static`, `temp`, `pointer`
* Program flow

  * `label`
  * `goto`
  * `if-goto`
* Functions

  * `function`
  * `call`
  * `return`
* Bootstrap code for multi-file VM programs
* Function-scoped labels
* File-scoped static variables
* Input validation and error reporting
* Supports both single `.vm` files and directories containing multiple `.vm` files

> Note: programmer or compiler is supposed to handle uniqueness of labels and function names within files

## Requirements

* C++17 or later
* A C++ compiler such as `g++`

## Compilation

```bash
g++ -std=c++17 vm.cpp -o vm
```

## Usage

### Single VM file

```bash
./vm SimpleAdd.vm
```

This produces:

```text
SimpleAdd.asm
```

Bootstrap code is **not** generated for a single VM file.

### Directory

```bash
./vm Program/
```

For a directory such as:

```text
Program/
├── Main.vm
├── Math.vm
└── Sys.vm
```

the translator produces:

```text
Program/
├── Main.vm
├── Math.vm
├── Sys.vm
└── Program.asm
```

For directory input, bootstrap code is generated once at the beginning of the output and execution starts at `Sys.init`.

## VM → Assembly

For example:

```vm
push constant 7
push constant 8
add
```

is translated into Hack assembly that pushes `7`, pushes `8`, and adds the two values on the stack.

## Project Structure

```text
.
├── vm.cpp
└── README.md
```

## About

This project is part of my implementation of the **Nand2Tetris** computer system, specifically the VM Translator stage.

The goal is to understand how a stack-based virtual machine is translated into low-level Hack assembly.
