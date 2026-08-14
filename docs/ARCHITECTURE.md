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

~~~text
nn0

represents the three-trit opcode:

−1, −1, 0
~~~

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

~~~text
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
~~~

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

~~~text
MST                                      LST
 │                                        │
 ▼                                        ▼
t[N−1]  t[N−2]  ...  t[2]  t[1]  t[0]
~~~

The architectural significance of each trit is therefore determined by its position within the value.

## 11. Register Architecture

TVM provides nine general-purpose registers:

`R0 R1 R2 R3 R4 R5 R6 R7 R8`

Each register stores exactly one TVM Word.
A TVM Word consists of three Trytes:

~~~text
1 Word = 3 Trytes
       = 27 trits
~~~

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

~~~text
Opcode | Rd | Rs1 | Rs2 | Cargo
   3      2     2     2     18
~~~

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

~~~text
Opcode | Rd | Rs1 | Rs2 | Immediate
   3      2     2     2       18
~~~

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
~~~text
Rd ← Rs1 + Rs2
~~~
The operation is performed using balanced-ternary arithmetic.
The precise result-width and overflow semantics remain an architectural item to be finalized.

### 14.3 SUB — Ternary Subtraction
**Opcode:** `nn1`
**Syntax:** `SUB Rd, Rs1, Rs2`

**Operation:**
~~~text
Rd ← Rs1 − Rs2
~~~
The legacy architecture describes subtraction as inversion of Operand B followed by ternary addition.
The precise result-width and overflow semantics remain an architectural item to be finalized.

### 14.4 MUL — Ternary Multiplication
**Opcode:** `n0n`
**Syntax:** `MUL Rd, Rs1, Rs2`

**Operation:**
~~~text
Rd ← Rs1 × Rs2
~~~
The multiplication result shall be reduced according to the finalized TVM Word arithmetic rules.

### 14.5 CMP — Compare
**Opcode:** `n00`
**Syntax:** `CMP Rs1, Rs2`

**Operation:**
~~~text
compare Rs1 and Rs2
~~~
CMP shall not modify the destination register.
The comparison result shall be represented by the STATUS architectural state.
The legacy architecture defines the comparison result as:

~~~text
−1  → less than
 0  → equal
+1  → greater than
~~~
This result is subsequently consumed by conditional branch instructions.

### 14.6 TAND — Ternary AND
**Opcode:** `n01`
**Syntax:** `TAND Rd, Rs1, Rs2`

The operation is defined trit-wise using the mathematical minimum:
~~~text
Rd[i] ← min(Rs1[i], Rs2[i])
~~~
for every trit position `i`.

### 14.7 TOR — Ternary OR
**Opcode:** `n1n`
**Syntax:** `TOR Rd, Rs1, Rs2`

The operation is defined trit-wise using the mathematical maximum:
~~~text
Rd[i] ← max(Rs1[i], Rs2[i])
~~~
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
~~~text
−1 → +1
 0 →  0
+1 → −1
~~~

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
~~~text
Rd ← sign_extend_or_define(immediate)
~~~
The exact extension rule from 18 trits to the 27-trit Word shall be specified by the numerical representation section.

### 15.2 MOV — Move
**Opcode:** `0n1`
**Syntax:** `MOV Rd, Rs1`

**Operation:**
~~~text
Rd ← Rs1
~~~
No memory access is performed.

### 15.3 LD — Load
**Opcode:** `00n`
**Syntax:** `LD Rd, Rs1, offset`

LD reads a value from virtual memory and places it into Rd.
The effective address calculation shall be:
~~~text
effective_address = base + offset
~~~
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
~~~text
Rd ← address(Rs1 + offset)
~~~

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
~~~text
STATUS = 0
~~~
Otherwise execution continues sequentially.

### 16.3 BGT — Branch if Greater Than
**Opcode:** `011`
**Syntax:** `BGT target`

The branch is taken when:
~~~text
STATUS = +1
~~~
Otherwise execution continues sequentially.

### 16.4 BLT — Branch if Less Than
**Opcode:** `1nn`
**Syntax:** `BLT target`

The branch is taken when:
~~~text
STATUS = −1
~~~
Otherwise execution continues sequentially.

### 16.5 CALL — Subroutine Call
**Opcode:** `1n0`
**Syntax:** `CALL target`

CALL transfers execution to a subroutine and preserves the return location on the stack.
Conceptually:
~~~text
push return_address
PC ← target
~~~
The exact value pushed and the precise stack update semantics shall be defined by the SP/PC architecture.

### 16.6 RET — Return
**Opcode:** `1n1`
**Syntax:** `RET`

RET restores a previously saved return address from the stack.
Conceptually:
~~~text
PC ← pop()
~~~

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

~~~text
−1
 0
+1
~~~

and is established by `CMP`.
Conditional branch instructions consume this state:

~~~text
STATUS =  0 → BEQ
STATUS = +1 → BGT
STATUS = −1 → BLT
~~~

Additional status flags, such as arithmetic overflow or interrupt state, may be introduced only if required by later architectural decisions.

## 21. Instruction Execution Model
A conforming TVM implementation shall present the following architectural execution sequence:

~~~text
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
~~~

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

~~~text
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
~~~

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

~~~text
TVM
 │
 ▼
Host Abstraction
 │
 ├── x86-64
 │
 └── AArch64
~~~

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
