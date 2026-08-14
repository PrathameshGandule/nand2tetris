# VM Translator

A C++ implementation of the **Virtual Machine Translator** from the [Nand2Tetris](https://www.nand2tetris.org/) course.

The translator reads a `.vm` file containing stack-based VM commands and generates the corresponding **Hack assembly (`.asm`) code**.

## Features

Currently supports the complete **Project 7** VM command set:

### Memory Access

* `push constant i`
* `push/pop local i`
* `push/pop argument i`
* `push/pop this i`
* `push/pop that i`
* `push/pop static i`
* `push/pop temp i`
* `push/pop pointer i`

### Arithmetic & Logical Commands

* `add`
* `sub`
* `neg`
* `eq`
* `gt`
* `lt`
* `and`
* `or`
* `not`

The translator also performs input validation for invalid commands, segments, indices, and unsupported command/segment combinations.

## Architecture

The translator is divided into a few simple stages:

```text
VM source file
      │
      ▼
Remove comments & whitespace
      │
      ▼
Tokenization
      │
      ▼
Validation
      │
      ▼
Instruction representation
      │
      ▼
Assembly generation
      │
      ▼
Hack .asm file
```

VM instructions are represented internally using typed enums and an `Instruction` structure rather than being passed around as raw strings.

```cpp
struct Instruction {
    ACTION action;
    SEGMENT segment;
    int index;
    int line;
};
```

Assembly generation is handled by individual functions for different VM operations and memory segments.

## Static Variables

Static variables are represented using the VM filename:

```text
push static 0
```

from:

```text
Foo.vm
```

generates assembly referencing:

```asm
@Foo.0
```

The Hack assembler then assigns the corresponding symbol to a RAM location.

## Usage

Compile:

```bash
g++ vm.cpp -o vm
```

Run:

```bash
./vm Program.vm
```

This generates:

```text
Program.asm
```

The generated assembly can then be assembled and executed using the **Nand2Tetris Hack platform tools**.

## Project Context

This project implements the **VM Translator stage of Nand2Tetris Project 7**, bridging the gap between the stack-based Virtual Machine language and the Hack assembly language.

The implementation focuses on keeping parsing, validation, and assembly generation separate so that individual VM commands can be translated and tested independently.
