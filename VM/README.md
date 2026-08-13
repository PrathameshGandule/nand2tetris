# VM Translator

A C++ implementation of the **VM Translator** from the [Nand2Tetris](https://www.nand2tetris.org/) project.

## Overview

The translator will read a `.vm` file, parse and validate VM commands, and generate the corresponding **Hack assembly (`.asm`) code**.

It will eventually support:

* Stack operations: `push` and `pop`
* Arithmetic and logical commands: `add`, `sub`, `neg`, `eq`, `gt`, `lt`, `and`, `or`, `not`
* Memory segments: `local`, `argument`, `static`, `constant`, `this`, `that`, `temp`, and `pointer`
* Program flow commands: `label`, `goto`, and `if-goto`
* Function commands: `function`, `call`, and `return`
* VM input validation and useful error reporting
* Generation of valid Hack assembly output

The implementation is structured around parsing VM instructions into an internal representation and then translating them into assembly instructions.

## Usage

```bash
./vm <input.vm>
```

The generated assembly will be written to the corresponding `.asm` output file.

## Project

Part of the **Nand2Tetris** course, implementing the virtual machine layer that translates the stack-based VM language into Hack assembly.
