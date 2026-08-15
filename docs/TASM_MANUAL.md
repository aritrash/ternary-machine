# TASM Programmer's Manual
**Version:** 0.1  **Target:** Ternary Virtual Machine (TVM)

---

## 1. Introduction

TASM (Ternary Assembly) is the assembly language of the Ternary Virtual Machine.

TASM follows conventional assembly-language syntax and programming conventions, while defining its own instruction set and machine semantics according to the TVM architecture.

TASM is not intended to emulate or reproduce the instruction set or semantics of NASM/x86.

---

## 2. Source Files

TASM source files use the `.tasm` extension.

A source file consists of:
1. Directives
2. Section declarations
3. Labels
4. Instructions
5. Data declarations
6. Comments

Example:
```asm
section .text
global _start

_start:
    LDI R1, 10
    LDI R2, 20
    ADD R3, R1, R2
    HLT
```

## 3. Lexical Rules

### 3.1 Whitespace
Spaces, tabs, and line breaks separate tokens and have no semantic meaning except where required to separate adjacent tokens.

### 3.2 Comments
A semicolon begins a comment.

```asm
LDI R1, 10    ; Load ten into R1
```
The remainder of the line is ignored.

### 3.3 Case
Instruction mnemonics, register names, and directives are case-insensitive.
Symbol names are case-sensitive.
Therefore:

```asm
add R1, R2, R3
ADD r1, r2, r3
```
are equivalent, while:

```asm
loop:
LOOP:
```
represent different symbols.

### 3.4 Identifiers
Identifiers consist of letters, digits, underscores, and periods where permitted by the relevant syntactic context.
A label is terminated by `:`.

```asm
_start:
loop:
exit:
```

## 4. Registers
TVM provides nine general-purpose registers:
`R0 R1 R2 R3 R4 R5 R6 R7 R8`

The special architectural registers are:
*   `PC`
*   `SP`
*   `STATUS`

General-purpose registers may be used as instruction operands according to the TVM ISA.
Special registers are not implicitly interchangeable with general-purpose registers.

## 5. Literals

### 5.1 Decimal
Decimal integer literals are supported.

```asm
LDI R1, 42
LDI R2, -17
```

### 5.2 Ternary
TASM supports explicit balanced-ternary literals.
The canonical ternary digits are:

```text
n = -1
0 =  0
1 = +1
```
Example:

```asm
; Example only
; ternary literal syntax is defined by the assembler grammar
```
The exact literal prefix is reserved for the TASM lexer specification.

### 5.3 Symbolic Values
Symbols may be used wherever the instruction or directive permits an address or immediate expression.

```asm
JMP loop

loop:
    HLT
```

## 6. Sections
TASM recognizes the following sections:

```asm
section .text
section .data
section .rodata
section .bss
```
*   `.text`: Contains executable TVM instructions.
*   `.data`: Contains initialized writable data.
*   `.rodata`: Contains initialized read-only data.
*   `.bss`: Reserves zero-initialized writable storage.

## 7. Symbols and Labels
A label associates a symbolic name with the current assembly location.

```asm
_start:
    HLT

loop:
    JMP loop
```
Symbols may be referenced before their definitions.

```asm
JMP loop

loop:
    HLT
```
The assembler shall resolve all symbols before producing a runnable `.trn` executable.
Unresolved symbols are assembly errors in TASM v0.1.

## 8. Global Symbols
The `global` directive marks a symbol as externally visible.

```asm
global _start
```
The conventional executable entry symbol is: `_start`
A runnable executable shall have a valid entry point.

## 9. Instructions
TASM exposes only instructions defined by the TVM ISA.
The instruction syntax is:

```asm
MNEMONIC operand[, operand ...]
```
Instruction mnemonics are case-insensitive.

### 9.1 Arithmetic
```asm
ADD Rd, Rs1, Rs2
SUB Rd, Rs1, Rs2
MUL Rd, Rs1, Rs2
```

### 9.2 Comparison
```asm
CMP Rs1, Rs2
```

### 9.3 Ternary Logic
```asm
TAND Rd, Rs1, Rs2
TOR  Rd, Rs1, Rs2
TXOR Rd, Rs1, Rs2
TNOT Rd, Rs1
```

### 9.4 Shift
```asm
SHF Rd, Rs1, amount
```
The shift amount is expressed in trits.
Incoming trits are zero-filled.

### 9.5 Data Transfer
```asm
LDI Rd, immediate
MOV Rd, Rs1
LD  Rd, [base + offset]
ST  [base + offset], Rs1
LEA Rd, [base + offset]
```
Memory operand syntax uses square brackets.

### 9.6 Control Flow
```asm
JMP target
BEQ target
BGT target
BLT target
CALL target
RET
```

### 9.7 System and I/O
```asm
IN Rd, device
OUT Rs1, device
SYS
IRET
SWAP
HLT
```
`SWAP` is reserved according to the TVM architecture and shall produce an implementation fault when encountered.

## 10. Memory Operands
Memory operands use bracket notation:

```asm
LD R1, [R2]
LD R1, [R2 + 10]
ST [R2], R1
ST [R2 + 10], R1
LEA R3, [R2 + 10]
```
The assembler shall translate the source-level memory operand into the corresponding TVM base-register and offset fields.
Only addressing forms defined by the TVM architecture are valid.
TASM shall not introduce x86 addressing modes that have no corresponding TVM architectural semantics.

## 11. Directives
TASM v0.1 defines:

```asm
section
global
```
Additional data directives shall be introduced as the data representation is finalized.

## 12. Assembly Model
Assembly proceeds in the following conceptual stages:

```text
TASM source
    ↓
Lexical analysis
    ↓
Parsing
    ↓
Symbol collection
    ↓
Symbol resolution
    ↓
TVM instruction encoding
    ↓
Section construction
    ↓
TRN executable generation
```
The assembler shall reject malformed source, invalid operands, invalid instructions, undefined symbols, and values that cannot be represented by the corresponding TVM field.

## 13. TASM and TVM
TASM is a source representation of TVM programs.
The TVM architecture defines:
*   instruction semantics,
*   register semantics,
*   memory semantics,
*   execution state,
*   privilege behavior,
*   interrupt behavior,
*   executable format.

TASM defines how those architectural facilities are expressed by programmers.
TASM shall not introduce machine behavior that is absent from the TVM architecture.

## 14. Version 0.1 Scope
TASM v0.1 provides the minimum language required to assemble and execute basic TVM programs.
The initial implementation shall prioritize:
*   labels,
*   symbols,
*   `.text`,
*   `.data`,
*   `.rodata`,
*   `.bss`,
*   `global`,
*   TVM instruction syntax,
*   register operands,
*   immediate operands,
*   memory operands,
*   symbol references,
*   `.trn` generation.

More advanced facilities such as macros, conditional assembly, includes, external linking, relocation, debugging directives, and optimization are outside the scope of TASM v0.1.
