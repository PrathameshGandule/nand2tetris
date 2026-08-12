# Hack Assembler

A two-pass Hack assembler that converts Hack assembly (`.asm`) code into 16-bit binary machine code (`.hack`).

## Flow

```text
.asm input
   │
   ▼
Read & clean each line
(remove comments, trim whitespace)
   │
   ▼
First pass
   ├── Find labels: (LABEL)
   ├── Validate labels
   ├── Store LABEL → ROM address
   └── Keep actual instructions
   │
   ▼
Second pass
   ├── Identify A-instruction (@...)
   │     ├── Numeric address → convert to 16-bit binary
   │     └── Symbol → resolve existing symbol
   │                  or allocate a new variable
   │
   └── Identify C-instruction
         ├── Extract dest
         ├── Extract comp
         ├── Extract jump
         ├── Validate each component
         └── Build: 111 + comp + dest + jump
   │
   ▼
Write generated 16-bit instructions
to <input-name>.hack
```

## Main Components

* **Symbol table** - contains predefined Hack symbols, labels, and variables.
* **Input cleaning** - removes comments and surrounding whitespace before processing.
* **First pass** — resolves labels to ROM addresses.
* **Second pass** — translates each actual instruction into binary.
* **A-instructions** — handle numeric addresses and symbol/variable resolution.
* **C-instructions** — parse and validate `dest=comp;jump` components and generate the corresponding machine instruction.
* **Validation & errors** — invalid symbols, instructions, addresses, and malformed C-instructions are reported with the original source line number.

For the exact parsing rules, validation logic, binary mappings, and error handling, refer to the corresponding functions and comments in the source code.

