# TVM Architecture Specification

**Status:** Draft\
**Architecture:** TVM (Ternary Virtual Machine)\
**Document:** `ARCHITECTURE.md`\
**ISA Width:** 27 trits\
**Data Domain:** Balanced ternary\
**Machine Model:** Conventional von-Neumann architecture

------------------------------------------------------------------------

## 1. Architectural Philosophy

The Ternary Virtual Machine (TVM) is a software-defined processor
architecture based on the balanced-ternary numerical domain.

TVM is intentionally modeled as a conventional von-Neumann processor.
The architecture does not require a novel execution topology, spatially
distributed execution mechanism, or physically specialized memory
hierarchy. Its purpose is to define a clean ternary instruction-set
architecture and machine model that can be implemented in software and,
subsequently, mapped onto physical hardware.

The legacy TPA-1 architecture provides the foundational principles for
the TVM ISA, particularly its balanced-ternary representation,
fixed-width instruction format, register organization, instruction-field
alignment, and broad instruction categories.

The legacy architecture defines the ISA as the hardware-software
contract: software may request operations defined by the ISA, while
undefined operations are treated as illegal instructions. This principle
is retained directly in TVM.

TVM therefore separates three concerns:

1.  **Architecture** --- the abstract machine visible to software.
2.  **Implementation** --- an emulator, interpreter, JIT, FPGA
    implementation, or eventual silicon implementation.
3.  **Toolchain** --- assembler, compiler, linker, debugger, and related
    software.

An implementation is conformant only when its externally observable
behavior agrees with the architectural specification.

------------------------------------------------------------------------

## 2. Machine Model

TVM is a register-based von-Neumann machine.

The processor operates over a unified address space containing both
instructions and data. Instructions are fetched from memory using the
Program Counter (PC), decoded, executed by the arithmetic/logic
subsystem, and may access the same memory system used for data.

At the architectural level, the processor consists of:

-   a Program Counter (`PC`);
-   a Stack Pointer (`SP`);
-   a general-purpose Register File;
-   an Arithmetic and Logic Unit (ALU);
-   an instruction decoder;
-   a memory interface;
-   a control-flow subsystem;
-   an exception and interrupt mechanism;
-   an I/O interface.

The legacy TPA-1 core specifies nine general-purpose registers, `R0`
through `R8`, in addition to the PC, SP, and an accumulator. For the
initial TVM architecture, the nine general-purpose registers and PC/SP
are retained as the foundational register model. The precise
architectural status of the legacy accumulator will be resolved during
the register-model specification.

------------------------------------------------------------------------

## 3. Balanced-Ternary Domain

TVM uses balanced ternary as its native numerical representation.

A trit has exactly three possible states:

  Symbol     Mathematical value
  -------- --------------------
  `n`                        −1
  `0`                         0
  `1`                        +1

The alphanumeric representation is inherited from the legacy
architecture:

-   `n` represents −1;
-   `0` represents 0;
-   `1` represents +1.

This representation is used because it permits balanced-ternary values
to be represented as ordinary contiguous textual strings without
ambiguity between a negative trit and a subtraction operator.

For example:

``` text
1n
```

represents:

``` text
(+1 × 3¹) + (−1 × 3⁰) = 2
```

The machine representation and the mathematical interpretation are
therefore distinct concepts: `n01`, for example, is a sequence of three
trits, while its numerical interpretation depends on the context in
which the sequence is used.

------------------------------------------------------------------------

## 4. Trit and Tryte

The fundamental storage and instruction-alignment unit inherited from
the legacy architecture is the **Tryte**, consisting of nine trits.

``` text
1 Tryte = 9 trits
```

The number of distinct states represented by one Tryte is:

``` text
3⁹ = 19,683
```

TVM instructions are larger than a Tryte and occupy exactly three
Trytes:

``` text
1 instruction = 27 trits = 3 Trytes
```

The fixed 27-trit instruction width is retained from the legacy design.
The fixed width provides deterministic instruction boundaries and allows
the instruction decoder to interpret every instruction according to one
permanent structural layout.

------------------------------------------------------------------------

## 5. Architectural Word and Instruction Width

TVM defines a fixed instruction width of 27 trits.

Every instruction occupies exactly:

``` text
27 trits
```

regardless of whether the instruction actually requires an immediate
value, memory offset, branch displacement, or other payload.

The instruction is divided into two functional regions:

``` text
┌─────────────────────────────── 27 trits ───────────────────────────────┐
│                                                                       │
│  Action Block                              Cargo / Payload Block       │
│  9 trits                                   18 trits                    │
│                                                                       │
│  ┌─────┬────┬─────┬─────┐                    ┌─────────────────────┐  │
│  │ OP  │ Rd │ Rs1 │ Rs2 │                    │ immediate / offset │  │
│  │ 3   │ 2  │  2  │  2  │                    │ address / payload  │  │
│  └─────┴────┴─────┴─────┘                    └─────────────────────┘  │
│                                                                       │
└───────────────────────────────────────────────────────────────────────┘
```

### 5.1 Action Block

The first nine trits contain the fields required to identify the
operation and its register operands:

``` text
Opcode : 3 trits
Rd     : 2 trits
Rs1    : 2 trits
Rs2    : 2 trits
```

Therefore:

``` text
3 + 2 + 2 + 2 = 9 trits
```

This completely occupies the first Tryte.

### 5.2 Cargo Block

The remaining 18 trits form the Cargo Block.

Depending on the instruction, this field may represent:

-   an immediate value;
-   a memory address;
-   a memory offset;
-   a branch displacement;
-   another instruction-specific payload.

For instructions that do not require an external payload, the legacy
architecture specifies that the Cargo Block is populated with neutral
`0` trits.

TVM retains the fixed field physically and semantically, but the precise
interpretation of the 18-trit field is instruction-dependent.

------------------------------------------------------------------------

## 6. Register Architecture

The legacy architecture defines nine general-purpose registers:

``` text
R0 R1 R2 R3 R4 R5 R6 R7 R8
```

Two trits are sufficient to encode nine register identifiers because:

``` text
3² = 9
```

The register-address encoding therefore uses exactly two trits.

The legacy examples include:

``` text
R0 = nn
R1 = n0
R8 = 11
```

The Action Block consequently reserves three two-trit register fields:

``` text
Rd  = Destination Register
Rs1 = Source Register 1
Rs2 = Source Register 2
```

The fixed ordering is:

``` text
Opcode | Rd | Rs1 | Rs2
```

This alignment is retained as a central property of the TVM instruction
format.

### 6.1 Special Registers

The legacy architecture also defines:

-   `PC` --- Program Counter;
-   `SP` --- Stack Pointer;
-   `ACC` --- Accumulator.

The exact TVM treatment of `ACC`, including whether it remains
architecturally visible or becomes an implementation detail, is
intentionally left open at this stage.

------------------------------------------------------------------------

## 7. Instruction Semantics

TVM instructions operate primarily on register operands,
immediate/payload data, memory, or control-flow state.

The architectural execution abstraction is:

``` text
Fetch → Decode → Read Operands → Execute → Memory Access → Write Back → Update PC
```

This is an architectural description rather than a requirement that
every implementation use physically separate pipeline stages.

An implementation may be:

-   a simple interpreter;
-   a cycle-oriented emulator;
-   a JIT compiler;
-   an FPGA processor;
-   a CMOS processor;
-   another conformant implementation.

The ISA defines observable behavior; it does not prescribe the internal
implementation.

------------------------------------------------------------------------

## 8. Instruction Categories

The legacy ISA organizes its instructions into four broad categories.

### 8.1 Arithmetic and Logic

The legacy instruction dictionary includes:

``` text
NOP   No Operation
ADD   Ternary Addition
SUB   Ternary Subtraction
MUL   Ternary Multiplication
CMP   Compare
TAND  Ternary AND
TOR   Ternary OR
TXOR  Ternary XOR
TNOT  Ternary NOT
SHF   Shift
```

The legacy definitions describe:

-   `ADD` as ternary addition;
-   `SUB` as subtraction;
-   `MUL` as ternary multiplication;
-   `CMP` as subtraction used to establish a status result without
    overwriting the destination;
-   `TAND` as the mathematical minimum;
-   `TOR` as the mathematical maximum;
-   `TXOR` as a ternary disagreement operation;
-   `TNOT` as sign inversion;
-   `SHF` as trit-wise spatial shifting.

These operation semantics will form the basis of the TVM ALU
specification.

### 8.2 Memory and Data Transfer

The legacy ISA defines:

``` text
LDI   Load Immediate
MOV   Move Register
LD    Load from Memory
ST    Store to Memory
LEA   Load Effective Address
```

The 18-trit Cargo Block is used by immediate and memory-related
instructions where required.

### 8.3 Control Flow

The legacy ISA defines:

``` text
JMP   Unconditional Jump
BEQ   Branch if Equal
BGT   Branch if Greater Than
BLT   Branch if Less Than
CALL  Call Subroutine
RET   Return
```

Conditional branches operate using the result established by the
comparison mechanism.

### 8.4 System and Hardware Control

The legacy ISA defines:

``` text
IN    Input
OUT   Output
SYS   System Call
IRET  Interrupt Return
SWAP  Context Swap
HLT   Halt
```

TVM will retain the conceptual system-control category, while
instructions whose behavior depended specifically on TPA-1 photonic
hardware will be reviewed before becoming normative TVM instructions.

------------------------------------------------------------------------

## 9. Instruction Set Architecture

The TVM Instruction Set Architecture (ISA) defines the complete set of operations visible to software executing on the virtual processor.

The ISA is a strict architectural contract. A conforming implementation shall execute every defined instruction according to its specified semantics. An opcode that is not defined by the active ISA version shall generate an illegal-instruction exception.

TVM uses a fixed-width 27-trit instruction encoding. Three trits are allocated to the opcode, providing exactly 27 possible opcode values:

`3³ = 27`

The ISA therefore defines a maximum of 27 primary instructions in the current encoding.

---

### 9.1 Opcode Representation

The canonical textual representation of an opcode uses the characters:

* `n` = −1
* `0` = 0
* `1` = +1

For example:

```text
nn0

represents the three-trit opcode:

−1, −1, 0
```

The complete TVM opcode allocation is:

| Opcode | Mnemonic | Category |
| :--- | :--- | :--- |
| `nnn` | NOP | Arithmetic / Control |
| `nn0` | ADD | Arithmetic |
| `nn1` | SUB | Arithmetic |
| `n0n` | MUL | Arithmetic |
| `n00` | CMP | Arithmetic |
| `n01` | TAND | Logic |
| `n1n` | TOR | Logic |
| `n10` | TXOR | Logic |
| `n11` | TNOT | Logic |
| `0nn` | SHF | Shift |
| `0n0` | LDI | Memory / Data Transfer |
| `0n1` | MOV | Memory / Data Transfer |
| `00n` | LD | Memory / Data Transfer |
| `000` | ST | Memory / Data Transfer |
| `001` | LEA | Memory / Data Transfer |
| `01n` | JMP | Control Flow |
| `010` | BEQ | Control Flow |
| `011` | BGT | Control Flow |
| `1nn` | BLT | Control Flow |
| `1n0` | CALL | Control Flow |
| `1n1` | RET | Control Flow |
| `10n` | IN | System / I/O |
| `100` | OUT | System / I/O |
| `101` | SYS | System |
| `11n` | IRET | System |
| `110` | SWAP | System |
| `111` | HLT | System |

The opcode allocation is inherited from the legacy TPA-1 instruction vocabulary and shall be treated as the initial TVM ISA allocation.

## 10. Instruction Encoding

Every TVM instruction occupies exactly 27 trits.
The instruction is divided into:

```text
┌─────────────────────── 27 trits ───────────────────────┐
│                                                         │
│                 Action Block                            │
│                   9 trits                               │
│                                                         │
│  ┌─────────┬──────┬───────┬───────┐                    │
│  │ Opcode  │  Rd  │  Rs1  │  Rs2   │                    │
│  │ 3 trits │2 trits│2 trits│2 trits│                    │
│  └─────────┴──────┴───────┴───────┘                    │
│                                                         │
├─────────────────────────────────────────────────────────┤
│                                                         │
│                 Cargo Block                             │
│                   18 trits                              │
│                                                         │
│        Immediate / Address / Offset / Payload           │
│                                                         │
└─────────────────────────────────────────────────────────┘
```

The first nine trits therefore contain:

`Opcode + Rd + Rs1 + Rs2`

with:

`3 + 2 + 2 + 2 = 9 trits`

The remaining 18 trits constitute the Cargo Block.

### 10.1 Bit/Trite Ordering

TVM uses a most-significant-trit-first textual convention.
The leftmost trit of a word is the Most Significant Trit (MST).
The rightmost trit is the Least Significant Trit (LST).
For an N-trit value:

```text
MST                                      LST
 │                                        │
 ▼                                        ▼
t[N−1]  t[N−2]  ...  t[2]  t[1]  t[0]
```

The architectural significance of each trit is therefore determined by its position within the value.

## 11. Register Architecture

TVM provides nine general-purpose registers:

`R0 R1 R2 R3 R4 R5 R6 R7 R8`

Each register stores exactly one TVM Word.
A TVM Word consists of three Trytes:

```text
1 Word = 3 Trytes
       = 27 trits
```

Registers are raw ternary word registers. They do not intrinsically distinguish between integers, addresses, pointers, instruction operands, or arbitrary ternary data. The interpretation of a register value is determined by the instruction using it.

### 11.1 Register Encoding

A register identifier occupies two trits.
The nine valid register encodings are:

| Encoding | Register |
| :--- | :--- |
| `nn` | R0 |
| `n0` | R1 |
| `n1` | R2 |
| `0n` | R3 |
| `00` | R4 |
| `01` | R5 |
| `1n` | R6 |
| `10` | R7 |
| `11` | R8 |

All nine possible two-trit combinations therefore correspond to a valid register.

### 11.2 Special Registers

In addition to the general-purpose registers, TVM defines the following architectural state:

| Register | Purpose |
| :--- | :--- |
| `PC` | Program Counter |
| `SP` | Stack Pointer |
| `STATUS` | Processor status and comparison state |

PC and SP are architecturally distinct from the nine general-purpose registers.

The legacy TPA-1 architecture also defines an ACC register. TVM does not currently require an architecturally visible accumulator. The necessity of ACC shall be reconsidered only if an ISA-level requirement is identified.

## 12. Cargo Block

The Cargo Block consists of 18 trits.
It is present in every instruction regardless of whether the instruction requires a payload.
The interpretation of the Cargo Block depends on the opcode.

Possible uses include:
* immediate values;
* memory addresses;
* memory offsets;
* branch displacements;
* device identifiers;
* instruction-specific parameters.

When an instruction does not require a Cargo value, the assembler shall encode the Cargo Block as eighteen zero trits:

`000000000000000000`

### 12.1 Immediate Values

The `LDI` instruction uses the Cargo Block as an immediate value.
An 18-trit balanced-ternary value provides:

`3¹⁸ = 387,420,489`

distinct states.
When interpreted as a signed balanced-ternary integer, this provides the symmetric range:

`−193,710,244 ... +193,710,244`

## 13. Instruction Formats

Although every physical instruction has the same 27-trit structure, instructions may use their fields differently.

### 13.1 Register Format

Used primarily for operations involving multiple registers:

```text
Opcode | Rd | Rs1 | Rs2 | Cargo
   3      2     2     2     18
```

Examples:
* `ADD`
* `SUB`
* `MUL`
* `CMP`
* `TAND`
* `TOR`
* `TXOR`
* `SHF`

### 13.2 Immediate Format

Used when an instruction requires a literal value:

```text
Opcode | Rd | Rs1 | Rs2 | Immediate
   3      2     2     2       18
```

Unused register fields shall be encoded as `nn` unless a future instruction definition explicitly assigns them another meaning.
`LDI` is the initial instruction using this format.

### 13.3 Memory Format

Memory instructions may interpret the register fields as base, source, or destination registers and the Cargo Block as an address or offset.
The exact addressing semantics are defined individually by each memory instruction.

### 13.4 Control-Flow Format

Control-flow instructions use the Cargo Block to represent their target or displacement where specified.
The precise interpretation of the control-flow payload shall be defined by each control-flow instruction.

## 14. Arithmetic and Logic Instructions

### 14.1 NOP — No Operation
**Opcode:** `nnn`

NOP performs no architectural operation.
**Effects:**
* General-purpose registers: unchanged
* STATUS: unchanged
* Memory: unchanged
* PC: advances to the next instruction

### 14.2 ADD — Ternary Addition
**Opcode:** `nn0`
**Syntax:** `ADD Rd, Rs1, Rs2`

**Operation:**
```text
Rd ← Rs1 + Rs2
```
The operation is performed using balanced-ternary arithmetic.
The precise result-width and overflow semantics remain an architectural item to be finalized.

### 14.3 SUB — Ternary Subtraction
**Opcode:** `nn1`
**Syntax:** `SUB Rd, Rs1, Rs2`

**Operation:**
```text
Rd ← Rs1 − Rs2
```
The legacy architecture describes subtraction as inversion of Operand B followed by ternary addition.
The precise result-width and overflow semantics remain an architectural item to be finalized.

### 14.4 MUL — Ternary Multiplication
**Opcode:** `n0n`
**Syntax:** `MUL Rd, Rs1, Rs2`

**Operation:**
```text
Rd ← Rs1 × Rs2
```
The multiplication result shall be reduced according to the finalized TVM Word arithmetic rules.

### 14.5 CMP — Compare
**Opcode:** `n00`
**Syntax:** `CMP Rs1, Rs2`

**Operation:**
```text
compare Rs1 and Rs2
```
CMP shall not modify the destination register.
The comparison result shall be represented by the STATUS architectural state.
The legacy architecture defines the comparison result as:

```text
−1  → less than
 0  → equal
+1  → greater than
```
This result is subsequently consumed by conditional branch instructions.

### 14.6 TAND — Ternary AND
**Opcode:** `n01`
**Syntax:** `TAND Rd, Rs1, Rs2`

The operation is defined trit-wise using the mathematical minimum:
```text
Rd[i] ← min(Rs1[i], Rs2[i])
```
for every trit position `i`.

### 14.7 TOR — Ternary OR
**Opcode:** `n1n`
**Syntax:** `TOR Rd, Rs1, Rs2`

The operation is defined trit-wise using the mathematical maximum:
```text
Rd[i] ← max(Rs1[i], Rs2[i])
```
for every trit position `i`.

### 14.8 TXOR — Ternary XOR
**Opcode:** `n10`
**Syntax:** `TXOR Rd, Rs1, Rs2`

The operation evaluates ternary disagreement between corresponding trits.
The precise mathematical definition and truth table shall be frozen in the ALU specification.

### 14.9 TNOT — Ternary NOT
**Opcode:** `n11`
**Syntax:** `TNOT Rd, Rs1`

The operation inverts the sign of every trit:
```text
−1 → +1
 0 →  0
+1 → −1
```

### 14.10 SHF — Shift
**Opcode:** `0nn`
**Syntax:** `SHF Rd, Rs1, Rs2`

SHF Rd, Rs1, Rs2 performs a logical shift of the 27-trit Word in Rs1.

The signed Word value in Rs2 specifies the shift distance in trits.
A positive value shifts left; a negative value shifts right.
Vacated trits are filled with Zero trits.
Trits shifted beyond the 27-trit Word boundary are discarded.
A shift whose magnitude is greater than or equal to 27 produces a zero Word.

## 15. Memory and Data Transfer Instructions

### 15.1 LDI — Load Immediate
**Opcode:** `0n0`
**Syntax:** `LDI Rd, immediate`

The instruction extracts the 18-trit Cargo Block and places the represented value into Rd.
```text
Rd ← sign_extend_or_define(immediate)
```
The exact extension rule from 18 trits to the 27-trit Word shall be specified by the numerical representation section.

### 15.2 MOV — Move
**Opcode:** `0n1`
**Syntax:** `MOV Rd, Rs1`

**Operation:**
```text
Rd ← Rs1
```
No memory access is performed.

### 15.3 LD — Load
**Opcode:** `00n`
**Syntax:** `LD Rd, Rs1, offset`

LD reads a value from virtual memory and places it into Rd.
The effective address calculation shall be:
```text
effective_address = base + offset
```
where the identity of the base register and exact offset semantics shall be finalized by the memory model.

### 15.4 ST — Store
**Opcode:** `000`
**Syntax:** `ST Rs1, Rs2, offset`

ST writes a register value to virtual memory.
The exact assignment of source/base registers shall be finalized together with the memory-addressing model.

### 15.5 LEA — Load Effective Address
**Opcode:** `001`
**Syntax:** `LEA Rd, Rs1, offset`

LEA computes an effective address without accessing the referenced memory.
Conceptually:
```text
Rd ← address(Rs1 + offset)
```

### 15.6 Memory Addressing Convention

TVM memory instructions use a base-plus-signed-offset addressing model.

The effective address is computed as:

    effective_address = base + sign_extended(offset)

The offset is encoded in the 18-trit Cargo Block and is interpreted as a signed balanced-ternary integer.

The instruction-specific register assignments are:

| Instruction | Rd | Rs1 | Rs2 | Cargo |
|---|---|---|---|---|
| LD | Destination | Base | Unused | Signed Offset |
| ST | Unused | Source/Data | Base | Signed Offset |
| LEA | Destination | Base | Unused | Signed Offset |

Thus:

    LD Rd, Rs1, offset
        Rd ← Memory[Rs1 + offset]

    ST Rs1, Rs2, offset
        Memory[Rs2 + offset] ← Rs1

    LEA Rd, Rs1, offset
        Rd ← Rs1 + offset

All effective addresses are 27-trit Words. Memory is addressed in Word units.

## 16. Control-Flow Instructions

### 16.1 JMP — Unconditional Jump
**Opcode:** `01n`
**Syntax:** `JMP target`

JMP unconditionally changes the Program Counter to the specified target.
The exact target representation shall be finalized as part of the PC and control-flow model.

### 16.2 BEQ — Branch if Equal
**Opcode:** `010`
**Syntax:** `BEQ target`

The branch is taken when:
```text
STATUS = 0
```
Otherwise execution continues sequentially.

### 16.3 BGT — Branch if Greater Than
**Opcode:** `011`
**Syntax:** `BGT target`

The branch is taken when:
```text
STATUS = +1
```
Otherwise execution continues sequentially.

### 16.4 BLT — Branch if Less Than
**Opcode:** `1nn`
**Syntax:** `BLT target`

The branch is taken when:
```text
STATUS = −1
```
Otherwise execution continues sequentially.

### 16.5 CALL — Subroutine Call
**Opcode:** `1n0`
**Syntax:** `CALL target`

CALL transfers execution to a subroutine and preserves the return location on the stack.
Conceptually:
```text
push return_address
PC ← target
```
The exact value pushed and the precise stack update semantics shall be defined by the SP/PC architecture.

### 16.6 RET — Return
**Opcode:** `1n1`
**Syntax:** `RET`

RET restores a previously saved return address from the stack.
Conceptually:
```text
PC ← pop()
```

### 16.7 Control-Flow Target Convention

JMP and conditional branch instructions use PC-relative addressing.

The 18-trit Cargo Block is interpreted as a signed balanced-ternary displacement.

For a branch instruction at PC:

    target = PC + displacement

If a conditional branch is not taken:

    PC ← PC + 1

If a branch is taken:

    PC ← target

The displacement is interpreted as a signed value and is added to the current 27-trit PC using Word arithmetic.

JMP displacement
    PC ← PC + displacement

BEQ displacement
    if STATUS == Equal
        PC ← PC + displacement
    else
        PC ← PC + 1

BGT displacement
    if STATUS == Greater
        PC ← PC + displacement
    else
        PC ← PC + 1

BLT displacement
    if STATUS == Less
        PC ← PC + displacement
    else
        PC ← PC + 1
        
### 16.8 Stack and Subroutine Convention

The TVM stack is Word-addressed and grows toward lower memory addresses.

CALL performs:

    return_address = PC + 1
    SP = SP - 1
    Memory[SP] = return_address
    PC = PC + target

RET performs:

    PC = Memory[SP]
    SP = SP + 1

The return address is the address of the instruction immediately following CALL.

All stack addresses and stack values are 27-trit Words.

## 17. System and I/O Instructions

### 17.1 IN — Input
**Opcode:** `10n`
**Syntax:** `IN Rd, device`

IN obtains data from a virtual input interface and places the result into Rd.
The device addressing mechanism shall be defined by the virtual I/O architecture.
The physical interfaces described in the legacy TPA-1 architecture are not directly exposed by TVM.

### 17.2 OUT — Output
**Opcode:** `100`
**Syntax:** `OUT Rs1, device`

OUT sends data from a register to a virtual output interface.
The device addressing mechanism shall be defined by the virtual I/O architecture.

### 17.3 SYS — System Call
**Opcode:** `101`
**Syntax:** `SYS`

SYS requests an operating-system service.
The instruction shall transfer execution from user-level execution to the kernel's system-call entry mechanism.
The precise system-call ABI, privilege transition, argument convention, and return mechanism shall be defined by the TVM privilege and ABI specifications.

### 17.4 IRET — Interrupt Return
**Opcode:** `11n`
**Syntax:** `IRET`

IRET returns from an interrupt or exception handler and restores the interrupted execution context according to the architectural interrupt mechanism.
The exact saved state and restoration mechanism shall be defined by the privilege and interrupt specifications.

### 17.5 SWAP — Context Swap
**Opcode:** `110`
**Syntax:** `SWAP`

The legacy TPA-1 architecture defines SWAP as a hardware-specific mechanism that exchanges primary and Ghost Register state.
TVM shall not inherit the photonic Ghost Register implementation.
The opcode is retained provisionally for a future architecture-defined context-switch operation.
Until its semantics are formally specified, SWAP shall be classified as reserved / implementation fault and shall not be used by conforming TVM software.

### 17.6 HLT — Halt
**Opcode:** `111`
**Syntax:** `HLT`

HLT suspends normal instruction execution.
The processor enters the architectural halted state.
The mechanism by which an interrupt or external event exits the halted state shall be defined by the interrupt and execution-state specification.

## 18. Program Counter
The Program Counter (PC) identifies the location of the next instruction to be fetched.
Because TVM instructions have a fixed width of 27 trits, sequential instruction execution advances the PC by one instruction unit.
The precise numerical representation of the PC and its relationship to the virtual memory address space shall be defined by the memory model.

## 19. Stack Pointer
The Stack Pointer (SP) identifies the current top of the architectural stack.
The stack shall reside in the virtual memory address space.
At minimum, the stack shall support:
* subroutine return addresses;
* interrupt context;
* operating-system execution state;
* future process/thread context.

The exact stack growth direction and push/pop convention remain to be finalized.

## 20. Status State
TVM provides a STATUS architectural state.
The minimum comparison state is:

```text
−1
 0
+1
```

and is established by `CMP`.
Conditional branch instructions consume this state:

```text
STATUS =  0 → BEQ
STATUS = +1 → BGT
STATUS = −1 → BLT
```

Additional status flags, such as arithmetic overflow or interrupt state, may be introduced only if required by later architectural decisions.

## 21. Instruction Execution Model
A conforming TVM implementation shall present the following architectural execution sequence:

```text
┌──────────┐
│  Fetch   │
└────┬─────┘
     ▼
┌──────────┐
│  Decode  │
└────┬─────┘
     ▼
┌──────────────┐
│ Read Operands│
└────┬─────────┘
     ▼
┌──────────┐
│ Execute  │
└────┬─────┘
     ▼
┌──────────┐
│  Memory  │
└────┬─────┘
     ▼
┌──────────┐
│ Writeback│
└────┬─────┘
     ▼
┌──────────┐
│ Update PC│
└──────────┘
```

This is a description of architectural behavior, not a requirement for a particular microarchitectural pipeline.
An emulator may implement the entire sequence as a single function call, while a future physical implementation may divide it into multiple pipeline stages.

## 22. Exceptions
The initial TVM architecture shall recognize at least the following exceptional conditions:
* illegal instruction;
* invalid register encoding, if encountered;
* invalid memory access;
* privilege violation;
* invalid virtual device operation.

The exact exception vectors, saved state, priority, and handler entry mechanism shall be defined by the interrupt and privilege architecture.

## 23. Interrupts
TVM shall provide an architecture-independent interrupt mechanism.
The virtual machine shall expose interrupts generated by virtual devices rather than directly exposing host interrupt numbers.
Initial interrupt sources shall include:
* timer;
* virtual storage;
* virtual input/output;
* external virtual events.

Host interrupts shall be translated into the virtual interrupt model by the host backend.

## 24. Privilege Model
TVM shall provide privilege separation sufficient to support a protected operating system.
The initial model shall distinguish at minimum:
* User Mode
* Kernel Mode

User-mode software shall not directly perform privileged operations.
The following operations are candidates for privileged execution:
* interrupt configuration;
* virtual memory configuration;
* device control;
* process management;
* system control.

`SYS` shall provide the controlled transition from user-level software to the operating system.
The precise privilege-state representation shall be finalized before the first kernel implementation.

## 25. Virtual Memory Model
TVM shall provide a unified virtual address space for instructions and data.
The architecture shall eventually support:

```text
Virtual Address
      │
      ▼
Address Translation
      │
      ▼
Virtual Machine Physical Address
      │
      ▼
Virtual Memory
```

The physical host's address width shall not determine the guest-visible TVM address width.
The exact address width, page size, translation hierarchy, access permissions, and page-fault semantics remain architectural decisions to be finalized.

## 26. Virtual I/O Model
TVM shall expose devices through a virtualized interface.
The virtual machine shall not expose physical host device addresses directly to guest software.
The initial device classes shall include:
* Console
* Timer
* Storage
* Network
* Random Source
* Interrupt Controller

The device interface shall be deterministic and architecture-independent.
The exact distinction between memory-mapped I/O and explicit IN/OUT operations remains to be finalized.

## 27. Host Architecture Boundary
The TVM architecture shall be independent of the physical host processor.
The host implementation shall provide:

```text
TVM
 │
 ▼
Host Abstraction
 │
 ├── x86-64
 │
 └── AArch64
```

Host-specific mechanisms may include:
* memory allocation;
* executable memory management;
* native instruction execution;
* host timers;
* host interrupts;
* host device interfaces.

None of these mechanisms shall be directly visible to conforming TVM software.
The architectural state observed by guest software shall remain defined entirely by the TVM specification.

## 28. Native Execution Strategy
The first TVM implementation shall be a reference interpreter.
The reference interpreter shall prioritize:
* architectural correctness;
* deterministic execution;
* observability;
* ease of testing;
* portability.

Performance optimization shall be performed only after architectural conformance has been established.
Future implementations may introduce:
* threaded interpretation;
* cached decoding;
* JIT compilation;
* dynamic translation;
* native host execution.

These mechanisms shall preserve the observable semantics of the TVM ISA.

## 29. Ternary ABI
The Ternary ABI shall define the software contract between compiled/assembled programs and the operating system.
The ABI shall eventually specify:
* register usage;
* caller/callee-saved registers;
* argument passing;
* return values;
* stack layout;
* stack alignment;
* system-call convention;
* process entry state;
* executable entry point;
* memory layout.

The ABI shall be specified separately after the CPU, memory, privilege, and stack models have been finalized.

## 30. Native Executable Model
TVM shall use `.trn` as the native executable file extension.
A native executable shall identify at minimum:
* TVM architecture;
* ISA version;
* executable format version;
* entry point;
* code;
* initialized data;
* read-only data;
* required memory layout information.

The final executable format shall be defined after the ABI and virtual memory architecture are frozen.

## 31. Architectural Invariants
The following properties are mandatory for TVM v0.1:
* TVM is a conventional von-Neumann machine.
* The native numerical domain is balanced ternary.
* A trit has one of three values: −1, 0, +1.
  * `n`, `0`, and `1` are the canonical textual representations.
* A Tryte consists of exactly nine trits.
* A Word consists of exactly three Trytes.
* A Word therefore contains exactly 27 trits.
* Every instruction consists of exactly 27 trits.
* The leftmost trit is the Most Significant Trit.
* The first nine instruction trits form the Action Block.
* The remaining eighteen instruction trits form the Cargo Block.
* The Action Block contains a three-trit opcode and three two-trit register fields.
* The register fields are ordered Rd, Rs1, Rs2.
* Nine general-purpose registers are provided.
* Each general-purpose register stores one raw 27-trit Word.
* PC, SP, and STATUS are architectural state.
* Undefined instructions generate an illegal-instruction condition.
* Guest software shall not depend upon the physical host architecture.
* The reference implementation shall be behaviorally conformant with the architecture specification.
* Host-specific optimizations shall not alter guest-visible architectural semantics.

## 32. Architecture Status
The current architecture is designated:

`TVM Architecture v0.1-DRAFT`

The following portions are considered provisionally established:
* balanced-ternary data domain;
* trit notation;
* Tryte definition;
* 27-trit Word;
* fixed 27-trit instruction width;
* 9-trit Action Block;
* 18-trit Cargo Block;
* nine general-purpose registers;
* register-field encoding;
* opcode allocation;
* instruction categories;
* basic instruction vocabulary;
* conventional von-Neumann execution model;
* host-independent architectural boundary.

The following remain open:
* exact multi-word arithmetic behavior;
* overflow semantics;
* exact TXOR truth table;
* SHF encoding and semantics;
* memory address width;
* memory access granularity;
* stack direction;
* PC representation;
* branch displacement semantics;
* privilege-state encoding;
* exception vectors;
* interrupt priority;
* virtual I/O addressing;
* ABI;
* `.trn` executable structure;
* SWAP semantics.

These shall be resolved before the architecture reaches version 1.0.

## 33. Immediate Implementation Milestone
The first software implementation shall begin with the fundamental ternary data type.
The initial C++ abstraction shall represent a single TVM trit:

```cpp
enum class Trit : int8_t { Neg = -1, Zero = 0, Pos = 1 };
```

This type shall be accompanied by compile-time and runtime validation before higher-level TVM structures are implemented.
The next implementation layers shall be:

```plaintext
Trit
  ↓
Tryte
  ↓
Word
  ↓
Register File
  ↓
Instruction
  ↓
CPU State
  ↓
TVM Interpreter
```

No CPU instruction execution shall be implemented until the underlying ternary representation has automated tests.

## 34. Memory Architecture

TVM uses a Word-addressed memory architecture.

The architectural Word is the universal machine unit and consists of exactly 27 trits, or three Trytes.

The following architectural components are all 27 trits wide:

- general-purpose registers;
- PC;
- SP;
- memory locations;
- instruction words;
- memory addresses.

Therefore:

    1 memory location = 1 Word = 27 trits
    1 instruction = 1 Word = 27 trits

There is no sub-Word addressing in TVM v0.1.

A memory address identifies one complete Word.

### 34.1 Address Space

A 27-trit address provides:

    3^27 = 7,625,597,484,987

distinct Word addresses.

The theoretical virtual address range is therefore:

    0 ... 7,625,597,484,986

The implementation is not required to physically allocate the entire address space. Sparse or demand-allocated host storage may be used.

### 34.2 Instruction Fetch

Every instruction occupies exactly one Word.

Instruction fetch is therefore:

    instruction = memory[PC]

After a sequential instruction fetch:

    PC = PC + 1

unless the instruction modifies control flow.

### 34.3 Data Access

The initial TVM architecture provides Word-granular data access.

`LD` reads one Word from memory.

`ST` writes one Word to memory.

No byte-, trit-, or Tryte-level memory access is provided by the initial ISA.

### 34.4 Stack Access

The stack is located within the same unified Word-addressed virtual memory space.

`SP` identifies a Word location.

Stack operations therefore operate on complete Words.

## 35. Privilege, Interrupt, and System-Call Architecture

This section defines the architectural execution-state model required by
`SYS`, `IRET`, interrupts, and protected kernel execution.

The TVM defines two privilege levels:

- **User mode**
- **Kernel mode**

User mode is the normal execution environment for application programs.
Kernel mode is the privileged execution environment responsible for system
services, interrupt handling, and protected machine operations.

---

### 35.1 Privilege Levels

TVM defines the following privilege levels:

| Value | Level | Description |
|---|---|---|
| `0` | User | Unprivileged application execution |
| `1` | Kernel | Privileged operating-system execution |

The architectural CPU state therefore contains a **Privilege Level (PL)**
field.

After reset, the processor enters:

```text
PL = Kernel
```

This permits the initial software environment to establish the kernel state
before transferring control to user software.

Only Kernel mode may perform privileged execution mechanisms, including:

- system-call entry handling;
- interrupt and exception handling;
- modification of privileged execution state;
- return from interrupt or system-call context.

Normal arithmetic, register, memory, and control-flow instructions are not
privileged merely because they execute while the processor is in Kernel mode.

### 35.2 Execution-State Model

The complete architectural execution state consists of:

- General Registers
- Program Counter (PC)
- Stack Pointer (SP)
- Status
- Privilege Level (PL)
- Kernel Stack Pointer (KSP)

The general-purpose register file contains the nine architectural Word
registers defined elsewhere in this specification.

All state elements are represented using the TVM's native 27-trit Word
domain unless otherwise specified.

SP identifies the active stack.

KSP identifies the kernel stack used during transitions from User mode into
Kernel mode.

While executing in User mode:

```
SP  = user stack pointer
KSP = kernel stack pointer
```

While executing in Kernel mode:

```
SP  = kernel stack pointer
KSP = kernel stack pointer
```

The distinction between the user and kernel stack is preserved through the
saved execution context.

### 35.3 User-to-Kernel Transition

TVM provides two architectural mechanisms for entering Kernel mode:

Synchronous entry through `SYS`
Asynchronous entry through an interrupt or exception

Both mechanisms create a protected execution context and transfer control to
Kernel mode.

A transition from User mode to Kernel mode performs the following conceptual
operations:

- save current execution context
- switch to Kernel mode
- switch to kernel stack
- record transition cause
- load handler PC
- begin Kernel execution

The transition mechanism is responsible for preserving enough state for
IRET to restore the interrupted or calling User execution environment.

Kernel-to-User transitions are only permitted through the architectural
return mechanism defined by `IRET`.

### 35.4 System Call Entry

SYS provides synchronous entry into the Kernel.

When executed while in User mode, `SYS` performs:

- return_PC = PC + 1
- save execution context
- PL ← Kernel
- SP ← KSP
- PC ← system-call entry address

The instruction immediately following `SYS` is therefore the return point of
the system call.

The system-call entry address is an architectural constant defined by the
TVM system-call vector.

The system-call transition records:

```
cause = SystemCall
```

in the saved execution context.

SYS executed while already in Kernel mode is permitted and is treated as a
nested system-call request. The existing Kernel execution context is not
discarded; a new context frame is created on the active Kernel stack.

### 35.5 System-Call ABI

The TVM system-call ABI uses the general-purpose registers for argument and
result passing.

The convention is:

- R0 = system-call number
- R1 = argument 0
- R2 = argument 1
- R3 = argument 2
- R4 = argument 3
- R5 = argument 4
- R6 = argument 5

Additional arguments may be passed through memory using pointers supplied in
the argument registers.

The system-call return convention is:

1. R0 = return value
2. R7 = error code

R8 is reserved for future ABI extensions and shall not be assigned a
system-call meaning by the base TVM ABI.

The system-call number namespace is implementation-defined by the operating
system environment and is not encoded directly into the `SYS` instruction.

A successful system call returns:
```
R7 = 0
```
A failed system call returns:
```
R7 != 0
```
The interpretation of individual error codes is defined by the operating
system ABI rather than by the base TVM architecture.

### 35.6 Saved Execution Context

Every transition from User mode into Kernel mode creates a saved execution
context.

The context contains:

1. General Registers R0-R8
2. PC
3. SP
4. STATUS
5. PL
6. Transition Cause

The saved context is stored as a context frame on the Kernel stack.

The context frame therefore contains the complete architectural state
required to resume the interrupted or calling execution environment.

The Kernel Stack Pointer (KSP) identifies the location of the active
Kernel stack.

The exact physical layout of the context frame is an ABI concern and shall
be defined separately from the logical architectural contents.

### 35.7 Interrupt Entry

Interrupts are asynchronous transfers from normal execution into Kernel
mode.

An interrupt is recognized at an instruction boundary.

When an interrupt is accepted, the processor:

- save current execution context
- record interrupt cause
- PL ← Kernel
- SP ← KSP
- PC ← interrupt handler address

The saved PC identifies the next instruction that would have executed if
the interrupt had not occurred.

Consequently, after successful interrupt return:
```
PC ← saved PC
```
and execution resumes at the interrupted instruction boundary.

Interrupt sources are identified by an architectural interrupt cause value.

The mapping between interrupt causes and handler addresses is defined by the
TVM interrupt vector mechanism.

### 35.8 Exception Entry

Architectural exceptions use the same execution-state transition mechanism
as interrupts.

When an exception is raised:

- save current execution context
- record exception cause
- PL ← Kernel
- SP ← KSP
- PC ← exception handler address

The saved context permits the Kernel to inspect the interrupted state and
determine whether execution can safely continue.

The distinction between an interrupt and an exception is represented by the
transition cause.

### 35.9 Context Cause

Every saved Kernel context contains a transition cause.

The base TVM defines at least:

- SystemCall
- Interrupt
- Exception

The cause allows a common Kernel entry mechanism to determine why execution
entered Kernel mode.

The numerical encoding of causes is reserved for the privilege and ABI
specification.

### 35.10 IRET — Context Restoration

IRET is the architectural mechanism for returning from a Kernel execution
context.

IRET restores the most recently saved execution context:

- restore General Registers
- restore PC
- restore SP
- restore STATUS
- restore PL
- discard context frame

The restored SP is the stack pointer that was active before the Kernel
transition.

If the restored privilege level is User mode, execution resumes in User
mode using the restored User stack.

Conceptually:
```text
Kernel
  │
  │ IRET
  ▼
restored execution context
  │
  ▼
previous privilege level
```
IRET therefore serves as the return mechanism for:

- system calls;
- interrupts;
- exceptions.

### 35.11 Privilege Enforcement

The following operations are privileged:

- executing IRET;
- modifying the architectural privilege state;
- modifying KSP;
- modifying interrupt configuration;
- modifying system-call configuration;
- entering or modifying protected Kernel execution state.

Attempts by User-mode software to directly perform privileged operations
result in an architectural privilege exception.

User-mode software cannot directly transition to Kernel mode except through
an architectural entry mechanism such as SYS or an accepted interrupt.

### 35.12 Nested Kernel Execution

Kernel execution may itself be interrupted when the architectural interrupt
state permits nested interrupts.

Each accepted transition creates an independent context frame.

Therefore, contexts form a stack:

        ┌─────────────────────┐
        │ Context N           │
        ├─────────────────────┤
        │ Context N-1         │
        ├─────────────────────┤
        │ Context N-2         │
        ├─────────────────────┤
        │ ...                 │
        └─────────────────────┘
                 ▲
                 │
                KSP

IRET always restores the most recently created context.

Nested context handling therefore follows strict last-in-first-out
semantics.

### 35.13 System Call and Interrupt Return

A system call and an interrupt both return through IRET.

For a system call:
```text
User
  │
  │ SYS
  ▼
Kernel
  │
  │ IRET
  ▼
User
```

For an interrupt:

```text
User
  │
  │ interrupt
  ▼
Kernel
  │
  │ IRET
  ▼
User
```

The distinction between these transitions is retained in the saved cause,
but the architectural restoration mechanism is identical.

### 35.14 Initial Kernel State

After reset:

- PL  = Kernel
- PC  = 0
- SP  = 0
- KSP = 0
- STATUS = Equal

The reset environment is therefore a privileged execution environment.

The initial Kernel software is responsible for establishing:

- the Kernel stack;
- the system-call entry address;
- the interrupt vector configuration;
- the User execution environment;
- the initial User stack;
- any operating-system ABI state.

The mechanism used by Kernel software to transfer control into User mode is
defined by the privilege implementation and shall preserve the architectural
requirements of the User execution state.

### 35.15 Architectural Invariants

The following invariants shall hold:

- User-mode execution cannot directly modify privilege state.
- Every User-to-Kernel transition creates a saved execution context.
- Every saved context contains sufficient state for complete restoration.
- IRET restores the most recently saved context.
- Context frames are restored in last-in-first-out order.
- User execution uses the User SP; Kernel execution uses the Kernel stack.
- SYS resumes at the instruction immediately following SYS.
- An interrupt resumes at the saved instruction boundary.
- IRET is the only architectural mechanism for returning from a saved
- Kernel execution context.

System-call arguments and results follow the TVM system-call ABI defined
above.

# 36. System-Call and Interrupt Vector Architecture

This section defines how TVM resolves system-call, interrupt, and exception entry points.

All vector addresses, vector entries, handler addresses, and vector-base values are represented using the native 27-trit Word domain.

The vector mechanism is entirely virtual and is part of the TVM architectural machine model.

---

## 36.1 Vector Table

TVM maintains a fixed vector table in virtual memory.

The vector table is located at the architectural address:

`VECTOR_BASE = 1`

The reset address `0` is therefore reserved for the initial Kernel entry point and is not occupied by the vector table.
Each vector-table entry occupies exactly one Word.
A vector entry contains the 27-trit virtual address of the corresponding handler.
The vector table is divided into three regions:

```text
                    VECTOR_BASE
                         │
                         ▼
              ┌─────────────────────┐
              │ System Call Vector  │  1 Word
              ├─────────────────────┤
              │ Interrupt Vector 0  │
              │ Interrupt Vector 1  │
              │       ...           │
              │ Interrupt Vector 26 │  27 Words
              ├─────────────────────┤
              │ Exception Vector 0  │
              │ Exception Vector 1  │
              │       ...           │
              │ Exception Vector 26 │  27 Words
              └─────────────────────┘
```

Therefore:

```text
SYSTEM_CALL_VECTOR = VECTOR_BASE

INTERRUPT_VECTOR_BASE = VECTOR_BASE + 1

EXCEPTION_VECTOR_BASE = VECTOR_BASE + 28
```

The complete vector table occupies 55 Words.

## 36.2 Vector Causes

Interrupts and exceptions are identified by a cause value.
The base TVM architecture permits 27 interrupt causes and 27 exception causes.
Cause values are represented as:

`0 ≤ cause ≤ 26`

The numerical meaning of individual causes is defined by the interrupt and operating-system environment.
The base architecture does not require a particular physical interrupt source to correspond to a particular cause number.
Cause values are therefore part of the TVM execution environment rather than the physical machine implementation.

## 36.3 System-Call Vector

All system calls enter through a single architectural vector.
The system-call vector is located at:

`SYSTEM_CALL_VECTOR = VECTOR_BASE`

The Word stored at this address is the system-call handler address.
When `SYS` is executed from User mode, the processor performs the privilege transition defined in Section 18 and resolves the handler address as:

`handler = Memory[SYSTEM_CALL_VECTOR]`

The resulting Word becomes the new PC.
The system-call number contained in R0 does not select the hardware vector.
Instead, the common system-call handler receives control and dispatches the requested service according to the system-call ABI.

Conceptually:

```text
User
  │
  │ SYS
  ▼
Memory[SYSTEM_CALL_VECTOR]
  │
  ▼
Kernel system-call handler
  │
  ▼
dispatch using R0
```

This keeps the hardware entry mechanism independent of the operating system's system-call namespace.

## 36.4 Vector Architecture

TVM uses an architectural vector mechanism to determine the Kernel entry address associated with system calls, interrupts, and exceptions.

All vector entries contain one 27-trit Word representing a memory address.
Because TVM uses Word-addressed memory, each vector entry occupies exactly one memory Word.

The vector mechanism distinguishes three classes of Kernel entry:
1. System-call vector
2. Interrupt vector table
3. Exception vector table

The vector mechanism is part of the privileged execution architecture and shall not be directly modified by User-mode software.

### 36.4.1 System-Call Vector

The system-call vector contains a single architectural entry address.

The address is defined as:

`SCV = 1`

where `SCV` denotes the System-Call Vector entry address.
When `SYS` is executed from User mode, the processor loads:

`PC ← Memory[SCV]`

after creating the saved execution context and entering Kernel mode.
The system-call vector therefore provides a single common entry point for all system calls. Individual system calls are distinguished by the system-call number supplied through R0 according to the TVM system-call ABI.
The contents of the system-call vector are established by Kernel software during system initialization and shall not be modified by User-mode software.

### 36.4.2 Interrupt Vector Table

Interrupts are identified by an architectural interrupt cause value.
TVM defines a fixed interrupt vector table beginning at:

`IVB = 2`

where `IVB` denotes the Interrupt Vector Base.
For interrupt cause `n`, the corresponding vector entry is located at:

`vector_address = IVB + n`

and the handler address is obtained as:

`handler_PC = Memory[IVB + n]`

The processor therefore performs:

`PC ← Memory[IVB + interrupt_cause]`

when an interrupt is accepted.
The initial architectural interrupt sources are:

| Interrupt Cause | Source |
| :--- | :--- |
| 0 | Timer |
| 1 | Virtual Storage |
| 2 | Virtual Input/Output |
| 3 | External Virtual Event |

Cause values 4 and above are reserved for future architectural interrupt sources.
Reserved interrupt vector entries shall contain the neutral Word value `0` until assigned by a future architectural revision or implementation definition.

### 36.4.3 Exception Vector Table

Architectural exceptions use a separate exception vector table.
TVM defines a fixed exception vector table beginning at:

`EVB = 29`

where `EVB` denotes the Exception Vector Base.

For exception cause `n`, the corresponding vector entry is located at:

`vector_address = EVB + n`

and the handler address is obtained as:

`handler_PC = Memory[EVB + n]`

The processor therefore performs:

`PC ← Memory[EVB + exception_cause]`

when an exception is raised.

The initial architectural exception causes are:

| Exception Cause | Condition |
| :--- | :--- |
| 0 | Illegal Instruction |
| 1 | Invalid Register Encoding |
| 2 | Invalid Memory Access |
| 3 | Privilege Violation |
| 4 | Invalid Virtual Device Operation |

Cause values 5 and above are reserved for future architectural exception conditions.
Reserved exception vector entries shall contain the neutral Word value `0` until assigned by a future architectural revision or implementation definition.

### 36.4.4 Vector Address Reservation

The architectural vector regions are reserved from normal software use.
The reserved regions are:

```text
System-Call Vector:
    address 1

Interrupt Vector Table:
    addresses 2 ... 28

Exception Vector Table:
    addresses 29 ... 55
```

Address 0 remains the reset entry address and is therefore not part of the vector tables.

The complete vector table occupies 55 Words:
```text
Address 0       Reset Entry

Address 1       System-Call Vector

Addresses 2–28  Interrupt Vectors 0–26

Addresses 29–55 Exception Vectors 0–26
```
The vector regions shall not be used for ordinary program code or data by conforming Kernel software.

The vector regions are part of the architectural memory map and are therefore present in every conforming TVM implementation.

### 36.4.5 Vector Entry Requirements

Each vector entry shall contain a valid 27-trit Word memory address.
A vector entry containing the neutral Word value `0` represents an unassigned vector.
If an interrupt or exception is raised for an unassigned vector, the processor shall enter the Kernel using the corresponding vector mechanism with `PC = 0`. Because address 0 is the reset entry point, Kernel software shall provide an appropriate common fault or unhandled-event path at the reset entry when such vectors are expected to occur.
The architecture does not require a distinct hardware handler for every individual cause. Multiple vector entries may contain the same handler address, allowing Kernel software to implement a common dispatcher.

### 36.4.6 Vector Initialization

After reset, the processor begins execution in Kernel mode at:

`PC = 0`

The initial Kernel software is responsible for initializing the vector entries before enabling User-mode execution or accepting externally generated interrupts.
At minimum, Kernel initialization shall establish:

* the System-Call Vector;
* all interrupt vectors required by the active virtual-machine environment;
* all exception vectors required by the active implementation;
* the Kernel stack;
* the initial User execution environment.

Vector configuration is a privileged operation.
User-mode software shall not be permitted to modify vector entries or alter the architectural vector configuration.

### 36.4.7 Vector Lookup Semantics

Vector lookup is performed only after the corresponding transition has been accepted.
For a system call:

`PC ← Memory[SCV]`

For an interrupt with cause `n`:

`PC ← Memory[IVB + n]`

For an exception with cause `n`:

`PC ← Memory[EVB + n]`

The vector lookup occurs as part of the User-to-Kernel transition and does not modify the saved execution context.
The saved context records the transition cause and therefore permits common Kernel entry code to distinguish system calls, interrupts, and exceptions after control has been transferred to the selected handler.

### 36.4.8 Vector Safety

Vector entries are Kernel-controlled architectural state.
The following operations are privileged:

* modifying the System-Call Vector;
* modifying interrupt vector entries;
* modifying exception vector entries;
* enabling an interrupt source whose vector has not been initialized;
* changing any implementation-defined vector configuration.

A User-mode attempt to modify protected vector state results in a privilege exception.
The vector mechanism itself does not define interrupt priority, masking, or nesting policy. Those mechanisms are defined by the interrupt execution-state architecture.

## 36.5 Interrupt Vector Table

Interrupt handlers are selected using the interrupt cause.
The interrupt vector table begins at:

`INTERRUPT_VECTOR_BASE = VECTOR_BASE + 1`

For an interrupt with cause `c`, where:

`0 ≤ c ≤ 26`

the handler address is obtained from:

`handler = Memory[INTERRUPT_VECTOR_BASE + c]`

The resulting Word becomes the new PC after the interrupt context has been saved and Kernel mode has been entered.
Therefore:

```text
cause 0  → Memory[INTERRUPT_VECTOR_BASE + 0]
cause 1  → Memory[INTERRUPT_VECTOR_BASE + 1]
...
cause 26 → Memory[INTERRUPT_VECTOR_BASE + 26]
```

Each interrupt cause therefore has an independent architectural entry point.

## 36.6 Exception Vector Table

Exceptions use a separate vector region.
The exception vector table begins at:

`EXCEPTION_VECTOR_BASE = VECTOR_BASE + 28`

For an exception with cause `c`, where:

`0 ≤ c ≤ 26`

the handler address is obtained from:

`handler = Memory[EXCEPTION_VECTOR_BASE + c]`

The resulting Word becomes the new PC after the exception context has been saved and Kernel mode has been entered.
Therefore:

```text
cause 0  → Memory[EXCEPTION_VECTOR_BASE + 0]
cause 1  → Memory[EXCEPTION_VECTOR_BASE + 1]
...
cause 26 → Memory[EXCEPTION_VECTOR_BASE + 26]
```

Interrupt and exception causes therefore occupy independent namespaces.

## 36.7 Handler Selection

Handler selection is performed entirely through virtual memory.
The processor does not encode handler addresses directly into `SYS`, interrupt, or exception operations.
The resolution rules are:

```text
SYS:
    handler = Memory[SYSTEM_CALL_VECTOR]

Interrupt(c):
    handler = Memory[INTERRUPT_VECTOR_BASE + c]

Exception(c):
    handler = Memory[EXCEPTION_VECTOR_BASE + c]
```

The selected handler address is then loaded into the PC as part of the corresponding execution-state transition.
This allows Kernel software to relocate individual handlers simply by changing their vector entries.

## 36.8 Vector Entry Representation

Every vector entry contains a complete 27-trit Word address.
No truncation, sign extension, or additional address calculation is applied to the value stored in a vector entry.

For example:

`Memory[SYSTEM_CALL_VECTOR] = H`

means:

`PC ← H`

Likewise:

`Memory[INTERRUPT_VECTOR_BASE + c] = H`

means:

`PC ← H`

The vector table therefore uses the same Word-sized address representation as all other TVM memory operations.

## 36.9 Reset-Time Vector Initialization

After reset, the processor enters Kernel mode with:

```text
PC  = 0
SP  = 0
KSP = 0
PL  = Kernel
STATUS = Equal
```

The vector table is not populated with mandatory handler addresses by the hardware reset operation.
Instead, the initial Kernel software is responsible for initializing the vector table before enabling normal User-mode execution or accepting external interrupts.

Kernel initialization therefore performs conceptually:

```text
Memory[SYSTEM_CALL_VECTOR] = system_call_handler
Memory[INTERRUPT_VECTOR_BASE + i] = interrupt_handler_i
Memory[EXCEPTION_VECTOR_BASE + i] = exception_handler_i
```

for every vector that the execution environment intends to support.
Unsupported or uninitialized vectors contain no architecturally valid handler address.

## 36.10 Vector Initialization Ordering

The Kernel shall initialize vector entries before enabling the corresponding execution mechanism.
The required initialization sequence is:

1. Establish Kernel stack.
2. Establish KSP.
3. Populate system-call vector.
4. Populate required exception vectors.
5. Populate required interrupt vectors.
6. Configure interrupt acceptance.
7. Establish User-mode execution state.
8. Transfer control to User mode.

This prevents an interrupt, exception, or system call from entering an uninitialized handler.

## 36.11 SYS Entry Resolution

When `SYS` is executed:

1. Save the current execution context.
2. Record cause = `SystemCall`.
3. Enter Kernel mode.
4. Switch to the Kernel stack.
5. Read `Memory[SYSTEM_CALL_VECTOR]`.
6. Load the resulting Word into PC.
7. Begin Kernel execution.

The saved PC remains:

`PC + 1`

where PC is the address of the `SYS` instruction.
The system-call handler therefore returns to the instruction immediately following `SYS` through `IRET`.

## 36.12 Interrupt Entry Resolution

When an interrupt with cause `c` is accepted:

1. Save the current execution context.
2. Record cause = `Interrupt(c)`.
3. Enter Kernel mode.
4. Switch to the Kernel stack.
5. Calculate `INTERRUPT_VECTOR_BASE + c`.
6. Read the vector entry.
7. Load the resulting Word into PC.
8. Begin Kernel execution.

The saved PC identifies the next instruction boundary at which execution would have continued.
`IRET` restores that saved execution state.

## 36.13 Exception Entry Resolution

When an exception with cause `c` is raised:

1. Save the current execution context.
2. Record cause = `Exception(c)`.
3. Enter Kernel mode.
4. Switch to the Kernel stack.
5. Calculate `EXCEPTION_VECTOR_BASE + c`.
6. Read the vector entry.
7. Load the resulting Word into PC.
8. Begin Kernel execution.

The exception handler may inspect the saved context and determine whether the interrupted execution can safely continue.

## 36.14 Invalid Vector Handling

A vector entry that does not contain a valid handler address represents an uninitialized or unsupported vector.
If execution attempts to enter such a vector, the processor raises a secondary architectural fault rather than beginning execution at an undefined address.
The precise secondary-fault cause is implementation-defined until the complete exception-cause namespace is finalized.
A conforming Kernel should therefore initialize every vector that it intends to expose before enabling the corresponding mechanism.

## 36.15 Vector Table Protection

The vector table is Kernel-owned architectural state.
User-mode software shall not modify:

* `VECTOR_BASE`;
* system-call vector entries;
* interrupt vector entries;
* exception vector entries.

Attempts to modify vector-table memory from User mode result in a privilege exception.
Kernel-mode software may modify vector entries as part of system initialization, handler registration, or controlled operating-system reconfiguration.

## 36.17 Vector Relocation

The base TVM vector table is fixed at:

`VECTOR_BASE = 1`

and therefore does not contain a programmable vector-base register.
This deliberately keeps the initial TVM privilege architecture simple.
Future architecture revisions may introduce a programmable vector base if requirements for virtualization, multiple operating systems, or isolated execution environments justify it.
Such an extension shall not change the semantics of the existing vector lookup mechanism.

## 36.18 Vector Lookup Summary

The complete entry-point resolution mechanism is therefore:

```text
System Call:

    SYS
     │
     ▼
Memory[1]
     │
     ▼
System-call handler


Interrupt(c):

    Interrupt
        │
        ▼
Memory[2 + c]
        │
        ▼
Interrupt handler


Exception(c):

    Exception
        │
        ▼
Memory[29 + c]
        │
        ▼
Exception handler
```

where all addresses and arithmetic are performed in the native 27-trit Word domain.

## 36.19 Architectural Invariants

The following invariants shall hold:

* All vector entries are exactly one 27-trit Word.
* The system-call vector occupies `VECTOR_BASE`.
* Interrupt vectors occupy the 27 Words immediately following the system-call vector.
* Exception vectors occupy the 27 Words following the interrupt vector region.
* `SYS` always resolves through the system-call vector.
* Interrupts always resolve through the interrupt vector region.
* Exceptions always resolve through the exception vector region.
* Vector entries contain complete virtual Word addresses.
* Handler selection does not encode physical hardware addresses.
* Kernel software initializes vector entries before enabling the associated execution mechanism.
* User-mode software cannot modify vector-table state.
* Every accepted transition saves sufficient context for `IRET` to restore the previous execution state.

# 37. TVM Native Executable Format (.trn)

## 37.1 Overview
The TVM Native Executable Format (`.trn`) is the native executable representation of the Ternary Virtual Machine.
A `.trn` file shall be a pure ternary representation. Its serialized contents shall contain only the three TVM trit symbols:

* `n`
* `0`
* `1`

where:
* `n` represents the ternary value −1,
* `0` represents the ternary value 0,
* `1` represents the ternary value +1.

The `.trn` format shall not depend upon the host machine's byte ordering, integer representation, pointer width, character encoding, or binary executable format.
The format is intended to provide the equivalent architectural role of a conventional native executable format while remaining natively represented in the TVM ternary domain.
A `.trn` file shall therefore be regarded as a serialized TVM memory image with executable metadata and section information rather than as a conventional byte-oriented executable.

## 37.2 Ternary File Representation
The physical representation of a `.trn` file shall consist exclusively of the characters:

* `n`
* `0`
* `1`

No other character shall constitute executable file data.
Whitespace, line separators, or other presentation characters shall not form part of the canonical `.trn` representation.
The canonical representation shall therefore be a contiguous sequence of ternary symbols.
For example:

```text
n0010n1...
```

represents a sequence of ternary digits and does not represent ASCII text, hexadecimal data, or binary bytes.
Implementations may provide human-readable formatting tools which introduce line breaks or grouping for display purposes, but such formatting shall not alter the underlying ternary data.

## 37.3 Word Alignment
The logical contents of a `.trn` file shall be organized into TVM Words.
Each TVM Word contains exactly:

`27 trits`

All executable-format structures shall therefore occupy an integral number of 27-trit Words.
The canonical serialized representation shall preserve this alignment.
Consequently:

`1 TVM Word = 27 ternary symbols`

and the `.trn` file shall contain a total number of ternary symbols divisible by 27.
This requirement allows a `.trn` file to be interpreted directly as a sequence of TVM Words without introducing a host-dependent byte-packing layer.

## 37.4 Executable File Organization
A `.trn` executable shall consist of the following logical regions:

```text
┌──────────────────────────────┐
│          File Header         │
├──────────────────────────────┤
│        Section Table         │
├──────────────────────────────┤
│         Symbol Table         │
├──────────────────────────────┤
│            .text             │
├──────────────────────────────┤
│            .data             │
├──────────────────────────────┤
│           .rodata            │
├──────────────────────────────┤
│             .bss             │
└──────────────────────────────┘
```

The presence and ordering of optional sections shall be specified by the executable-format version.
The initial TVM executable format shall recognize the following sections:

* `.text` — executable instruction words.
* `.data` — initialized writable data.
* `.rodata` — initialized read-only data.
* `.bss` — zero-initialized writable storage.

The executable shall contain sufficient metadata for a loader to determine the location, size, and intended memory properties of each section.

## 37.5 File Header
Every `.trn` executable shall begin with a fixed-format file header.
The header shall identify at minimum:

* TVM architecture identifier.
* ISA version.
* Executable-format version.
* File flags.
* Entry-point address.
* Start-symbol identifier.
* Section-table location.
* Section count.
* Symbol-table location.
* Symbol count.
* Memory-layout information.

All header fields shall themselves be represented using TVM ternary encoding.
The header shall not contain host-specific binary structures.
The header shall occupy an integral number of TVM Words.

## 37.6 Architecture Identification
The executable shall contain an architecture identifier allowing a loader to determine whether the file targets the TVM architecture.
The architecture identifier shall be a numeric ternary value rather than a textual string.
An implementation encountering an unsupported architecture identifier shall reject the executable before attempting to execute its contents.

## 37.7 ISA and Executable-Format Versions
The executable shall contain separate version identifiers for:

* Architecture / ISA version
* Executable-format version

These values shall be independently versioned.
The ISA version determines the interpretation of instructions and architectural state.
The executable-format version determines the interpretation of the `.trn` container itself.
A loader shall reject an executable whose format version it does not support.
An executable shall not silently reinterpret a newer format using an older format definition.

## 37.8 Entry Point
Every executable shall define an entry point.
The entry point shall be represented as a 27-trit TVM Word address.
The entry point shall identify the first instruction to be executed after the executable has been loaded and the initial execution state has been established.
Unless otherwise specified by a future executable-format revision, the conventional entry symbol shall be:

`_start`

The assembler shall permit `_start` to be explicitly declared and shall use it as the default entry symbol.
If `_start` is absent and no explicit entry point is supplied, the assembler shall reject the source program.

## 37.9 Start Symbol
The executable shall preserve the identity of its start symbol through a numeric symbol identifier.
The textual name:

`_start`

shall exist at the assembly-language and symbol-table level, but executable metadata shall reference symbols using numeric identifiers or resolved addresses rather than embedding host character strings into the ternary executable representation.
For example:

```text
_start:
    LDI R1, 42
    HLT
```

shall result in a symbol-table entry conceptually equivalent to:

```text
Symbol ID → _start
Address   → .text + 0
```

The executable header shall identify this symbol as the program entry symbol.

## 37.10 Section Table
The section table shall describe every loadable or executable section contained within the `.trn` file.
Each section descriptor shall identify at minimum:

* section type,
* section flags,
* file location,
* section size,
* virtual memory address,
* alignment requirements.

All addresses and sizes shall use TVM-native ternary values.
The initial section types shall include:

* `TEXT`
* `DATA`
* `RODATA`
* `BSS`

The section table itself shall be Word-aligned.

## 37.11 .text Section
The `.text` section contains executable TVM instructions.
Each instruction occupies exactly one TVM Word:

`1 instruction = 27 trits`

The `.text` section shall therefore contain an integral number of 27-trit Words.
Instructions shall use the instruction encoding defined by the TVM ISA.
The assembler shall generate `.text` contents using the canonical TVM instruction encoding and shall not implement an independent instruction encoding scheme.

## 37.12 .data Section
The `.data` section contains initialized writable data.
Data shall be represented using TVM-native ternary Words.
The section shall be loaded into writable virtual memory before execution begins.
The executable shall provide sufficient metadata for the loader to determine the destination address and size of the initialized data.

## 37.13 .rodata Section
The `.rodata` section contains initialized read-only data.
Its contents shall use the TVM ternary representation.
The loader shall place `.rodata` according to the memory-layout and protection rules defined by the TVM memory and privilege architecture.
Executable code shall not modify `.rodata` under a conforming implementation.

## 37.14 .bss Section
The `.bss` section represents zero-initialized writable storage.
`.bss` shall not require explicit storage for every zero-valued Word in the executable file.
Instead, its metadata shall specify the required memory extent.
During loading, the loader shall allocate the corresponding virtual-memory region and initialize every Word in the region to:

`000000000000000000000000000`

The physical representation of `.bss` therefore consists primarily of metadata describing its required size rather than an explicit sequence of zero Words.

## 37.15 Symbol Table
A `.trn` executable may contain a symbol table.
Symbols shall be identified internally using numeric symbol identifiers.
A symbol shall contain, at minimum:

* symbol identifier,
* symbol type,
* section identifier,
* address or section-relative offset,
* symbol visibility.

The initial executable format shall support at least:

* `LOCAL`
* `GLOBAL`
* `ENTRY`

symbols.
The assembler shall maintain symbolic names during assembly but the serialized executable representation shall use ternary-encoded symbol metadata.

## 37.16 Relocation and Symbol Resolution
The initial `.trn` executable format shall support fully resolved executables.
The assembler shall resolve local labels and symbols before producing a runnable `.trn` executable.
Future revisions may introduce explicit relocation records to support separately assembled object files and a linker.
Until relocation is formally specified, unresolved symbols shall constitute an assembly error and shall prevent generation of a runnable `.trn` executable.

## 37.17 Memory Layout
The executable header and section table shall provide sufficient information for the loader to construct the initial TVM virtual-memory image.
At minimum, the loader shall be able to determine:

* `.text`  → executable memory
* `.rodata` → read-only memory
* `.data`  → writable initialized memory
* `.bss`   → writable zero-initialized memory

The exact virtual addresses shall be represented using 27-trit TVM Words.
The executable format shall not require a particular host operating-system memory layout.

## 37.18 Loading Procedure
Loading a `.trn` executable shall conceptually proceed as follows:

1. Validate ternary file representation.
2. Validate Word alignment.
3. Validate file header.
4. Validate architecture identifier.
5. Validate ISA version.
6. Validate executable-format version.
7. Read section table.
8. Construct `.text`.
9. Construct `.rodata`.
10. Construct `.data`.
11. Allocate and zero `.bss`.
12. Resolve the entry point.
13. Initialize CPU execution state.
14. Set PC to the executable entry point.
15. Begin instruction execution.

The loader shall reject malformed or architecturally incompatible executables before beginning execution.

## 37.19 Initial CPU State
After loading an executable and before its first instruction is executed, the loader shall establish the initial CPU state according to the TVM execution-state specification.
At minimum:

* `PC`     ← executable entry point
* `STATUS` ← Equal

The initial privilege level, stack pointer, register state, interrupt state, and other execution-state fields shall follow the privilege and execution-state architecture.
The executable itself shall not directly encode host-specific CPU state.

## 37.20 Executable Validation
A conforming loader shall validate at minimum:

* ternary-symbol validity,
* Word alignment,
* architecture compatibility,
* ISA compatibility,
* executable-format compatibility,
* section boundaries,
* section sizes,
* entry-point validity,
* symbol-table bounds,
* memory-layout consistency.

Malformed executables shall not be passed to the instruction executor.

## 37.21 Canonical Representation
The canonical `.trn` representation shall satisfy all of the following:

* Only `n`, `0`, and `1` occur as data symbols.
* Total trit count is divisible by 27.
* All structures are Word-aligned.
* All numeric metadata uses TVM ternary encoding.
* Instruction words use the canonical TVM ISA encoding.
* No host byte order is involved.
* No host binary executable format is required.

A `.trn` executable shall therefore be directly interpretable as a sequence of TVM-native ternary Words.

## 37.22 Future Extensions
The initial executable format is intentionally designed to permit future extensions without changing the fundamental ternary representation.
Future revisions may introduce:

* relocation records,
* dynamic linking,
* shared libraries,
* additional section types,
* debugging information,
* exception metadata,
* privilege metadata,
* capability information,
* memory protection attributes,
* executable signatures,
* cryptographic integrity information,
* compressed sections,
* separate object-file and executable-file formats.

Such extensions shall preserve the fundamental requirement that the canonical `.trn` representation remains native to the TVM ternary domain.
