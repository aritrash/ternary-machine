# Software Requirements Specification
## Ternary Machine

**Document ID:** TM-SRS  
**Version:** 0.1  
**Status:** Draft  
**Date:** 13 August 2026  
**Repository:** `ternary-machine`

---

# 1. Introduction

## 1.1 Purpose

This document defines the software requirements for **Ternary Machine**, a hardware-independent computing platform based on a balanced-ternary virtual architecture.

The project aims to develop a complete software environment in which operating-system and application software execute against a defined virtual machine architecture rather than directly against the instruction set and hardware characteristics of a physical host processor.

The initial implementation will target conventional binary host architectures, with x86-64 and AArch64 identified as the first intended host architectures.

This document establishes the requirements and boundaries of the project. Detailed definitions of the virtual instruction set, register organization, instruction encoding, memory architecture, and other implementation-level architectural characteristics shall be defined in subsequent architecture specifications.

---

## 1.2 Scope

Ternary Machine shall comprise, progressively, the following major software components:

1. A balanced-ternary data representation system.
2. A ternary virtual machine architecture.
3. A reference virtual machine implementation.
4. A native ternary executable format.
5. An assembler and linker for the ternary architecture.
6. A virtual hardware environment.
7. An operating system targeting the virtual architecture.
8. Host-architecture execution backends.
9. Mechanisms for execution and translation of selected conventional binary software.
10. Development, debugging, testing, and inspection tools.

The system shall be designed so that software targeting the virtual architecture is independent of the physical host architecture.

---

## 1.3 Intended Audience

This specification is intended for:

- developers of the Ternary Machine platform;
- developers of the virtual machine;
- operating-system developers;
- compiler and toolchain developers;
- researchers investigating non-binary computing architectures;
- contributors developing host-architecture backends;
- researchers evaluating the performance and architectural properties of the system.

---

## 1.4 Terminology

| Term | Definition |
|---|---|
| **Ternary Machine** | The overall software platform defined by this project. |
| **Trit** | A balanced-ternary digit whose value is −1, 0, or +1. |
| **Tryte** | A fixed group of nine trits. |
| **Virtual Machine** | The software-defined computational environment executing the ternary architecture. |
| **Virtual Architecture** | The processor, memory, privilege, interrupt, and device abstractions presented to software. |
| **Ternary ISA** | The instruction-set architecture defined for the virtual machine. |
| **`.trn`** | The native executable representation for software targeting the ternary architecture. |
| **Host Architecture** | The physical processor architecture on which the virtual machine is implemented. |
| **Host Backend** | The architecture-specific implementation required to execute the virtual machine on a particular host architecture. |
| **B2T** | Binary-to-ternary translation functionality. |
| **T2B** | Ternary-to-host translation functionality. |
| **TVM** | Working abbreviation for the Ternary Virtual Machine. |

---

# 2. System Overview

## 2.1 Problem Statement

Conventional operating systems are generally designed around the instruction-set architecture and hardware characteristics of a particular processor family.

Consequently, an operating system kernel typically contains architecture-specific mechanisms for:

- processor initialization;
- memory management;
- interrupts;
- privilege management;
- timers;
- device interfaces;
- executable loading;
- and low-level execution.

This project investigates an alternative architecture in which these hardware-specific characteristics are placed below a defined virtual computing architecture.

Software shall therefore target the virtual architecture rather than a specific physical processor architecture.

---

## 2.2 Proposed System

The proposed system shall implement the following conceptual execution stack:

```text
┌──────────────────────────────────────┐
│            Applications              │
│               .trn                   │
├──────────────────────────────────────┤
│        Ternary Operating System      │
├──────────────────────────────────────┤
│          Ternary ABI / ISA           │
├──────────────────────────────────────┤
│      Ternary Virtual Machine         │
├──────────────────────────────────────┤
│       Host Execution Backend         │
├──────────────────┬───────────────────┤
│      x86-64      │      AArch64      │
└──────────────────┴───────────────────┘
```

The operating system and native applications shall interact exclusively with the virtual architecture.
The host backend shall provide the mechanisms necessary to execute the virtual architecture using the physical host.

## 2.3 Core Architectural Principle
The primary architectural principle of the project shall be:

* Software executing within the Ternary Machine environment shall depend upon the defined virtual architecture rather than upon the instruction-set architecture or hardware characteristics of the physical host.
* Host-specific implementation shall be isolated below the virtual-machine boundary.

## 2.4 Balanced-Ternary Foundation
The computational model shall use balanced ternary as its fundamental numerical representation.
The balanced-ternary domain shall consist of:
`{−1, 0, +1}`

The project shall distinguish between the mathematical representation of ternary information and its eventual physical representation on binary host hardware.
The research foundation establishes balanced ternary as the numerical foundation of the computational model.
The Ternary Machine project shall not assume, solely from the use of ternary representation, that the resulting architecture is inherently superior to binary architectures. Such properties shall be subject to experimental evaluation.

# 3. Goals and Objectives

## 3.1 Primary Goals

**G-001 — Hardware Independence**
Provide an execution environment in which the same conforming operating-system and application software can execute on multiple physical processor architectures.

**G-002 — Ternary Native Execution**
Define a native balanced-ternary software representation and execution environment.

**G-003 — Virtual Architecture**
Define a complete virtual computer sufficiently capable of supporting an operating system.

**G-004 — Operating System**
Develop an operating system targeting the virtual architecture rather than a physical CPU architecture.

**G-005 — Host Portability**
Provide host implementations capable of executing the virtual architecture on multiple processor architectures.

**G-006 — Engineering Rigor**
Develop the platform using documented specifications, automated testing, reproducible builds, version control, and traceable requirements.

**G-007 — Experimental Evaluation**
Provide sufficient instrumentation and observability to experimentally evaluate the architectural properties and performance characteristics of the system.

# 4. Non-Goals
The following shall not be requirements of the initial project.

**NG-001 — Physical Ternary Processor**
The initial project shall not require fabrication of a physical ternary processor.

**NG-002 — Ternary Semiconductor Design**
Transistor-, memristor-, photonic-, or other device-level implementations are outside the initial software project.

**NG-003 — Universal Binary Compatibility**
The project shall not initially guarantee that arbitrary existing binary executables can be converted into native ternary applications.

**NG-004 — Immediate JIT Optimization**
A high-performance JIT or dynamic binary translator shall not be required for the first reference implementation.

**NG-005 — Immediate Hardware Implementation**
FPGA or ASIC implementation shall not be a prerequisite for the initial software platform.

**NG-006 — Assumption of Ternary Superiority**
The project shall not treat ternary computing as inherently superior to binary computing. Advantages and disadvantages shall be established through analysis and experimentation.

# 5. System Architecture

## 5.1 Architectural Layers
The system shall consist conceptually of the following layers:

```text
Application Layer
       │
       ▼
Ternary ABI
       │
       ▼
Ternary Operating System
       │
       ▼
Ternary Virtual Architecture
       │
       ▼
Virtual Machine
       │
       ▼
Host Backend
       │
       ▼
Physical Hardware
```

Each layer shall have a defined interface with the layer immediately below it.

## 5.2 Virtual Machine
The virtual machine shall provide the execution environment required by the virtual architecture.
It shall ultimately provide:
* virtual processor state;
* virtual memory;
* address translation;
* interrupts and exceptions;
* timers;
* virtual devices;
* program loading;
* and execution control.

The initial reference implementation shall prioritize correctness and observability over execution performance.

## 5.3 Virtual Processor
The virtual processor shall provide:
* architectural registers;
* an instruction pointer;
* defined execution state;
* instruction decoding;
* instruction execution;
* control-flow operations;
* arithmetic and logical operations;
* memory-access operations;
* system-level operations.

The exact register count, register width, instruction encoding, and instruction set shall be defined by the subsequent Ternary Machine Architecture Specification.

## 5.4 Virtual Memory
The virtual machine shall provide an addressable memory abstraction independent of the host's physical memory organization.
The architecture shall support the eventual implementation of:
* virtual addresses;
* physical virtual-machine addresses;
* memory protection;
* address translation;
* isolation between execution contexts;
* and memory-mapped virtual devices.

The precise address width and page-translation architecture shall be specified separately.

## 5.5 Virtual Devices
The virtual machine shall provide a defined device abstraction sufficient to support the operating system.
The initial device model shall consider:
* console;
* timer;
* interrupt controller;
* persistent storage;
* random-number source;
* and network interface.

Individual devices shall be introduced incrementally rather than all being implemented in the first version.

# 6. Functional Requirements

## 6.1 Ternary Data Representation
**FR-DATA-001**
The system shall provide a representation of balanced-ternary values.

**FR-DATA-002**
The system shall represent the three fundamental ternary states −1, 0, and +1.

**FR-DATA-003**
The system shall provide a defined representation for groups of ternary digits, including the project's nine-trit tryte.

**FR-DATA-004**
The system shall provide arithmetic and logical operations required by the virtual architecture.

## 6.2 Virtual Processor
**FR-CPU-001**
The virtual machine shall provide a virtual processor.

**FR-CPU-002**
The virtual processor shall execute instructions defined by the project's Ternary ISA.

**FR-CPU-003**
The virtual processor shall maintain architecturally defined execution state.

**FR-CPU-004**
The virtual processor shall support deterministic instruction execution.

## 6.3 Memory
**FR-MEM-001**
The virtual machine shall provide virtual memory to software executing within the virtual architecture.

**FR-MEM-002**
The virtual memory system shall provide defined mechanisms for reading and writing memory.

**FR-MEM-003**
The architecture shall support memory protection mechanisms sufficient to isolate operating-system and user-level execution.

**FR-MEM-004**
The virtual memory architecture shall be independent of the physical host's memory-management architecture.

## 6.4 Interrupts and Exceptions
**FR-INT-001**
The virtual machine shall provide a defined interrupt mechanism.

**FR-INT-002**
The virtual processor shall provide a defined mechanism for handling exceptional execution conditions.

**FR-INT-003**
Virtual interrupts shall be independent of the interrupt architecture of the host processor.

## 6.5 Executable Format
**FR-EXE-001**
The system shall define a native executable format for ternary applications.

**FR-EXE-002**
Native ternary executables shall use the `.trn` file extension.

**FR-EXE-003**
The executable format shall identify the target virtual architecture and architecture version.

**FR-EXE-004**
The virtual machine shall validate an executable before execution.

**FR-EXE-005**
The executable format shall support an explicitly defined entry point.

## 6.6 Toolchain
**FR-TOOL-001**
The project shall provide an assembler capable of producing native ternary object or executable representations.

**FR-TOOL-002**
The project shall provide a linker capable of producing executable `.trn` images.

**FR-TOOL-003**
The project shall provide tools for inspecting ternary executable files.

**FR-TOOL-004**
Toolchain output shall conform to the relevant version of the Ternary Machine specification.

# 7. Operating System Requirements

## 7.1 Kernel
**FR-OS-001**
The project shall provide an operating-system kernel targeting the Ternary Machine virtual architecture.

**FR-OS-002**
The kernel shall not require knowledge of the physical host processor architecture.

**FR-OS-003**
The kernel shall execute using the virtual processor and virtual hardware interfaces defined by the platform.

## 7.2 Process Execution
The operating system shall eventually provide:
* execution contexts;
* process or task management;
* scheduling;
* memory isolation;
* system calls;
* inter-process communication;
* executable loading;
* and user-level execution.

These capabilities shall be implemented incrementally.

## 7.3 User Environment
The operating system shall eventually support:

```text
kernel
  │
  ▼
init
  │
  ├── system services
  ├── shell
  └── user applications
```

Native applications shall use the Ternary ABI rather than directly interacting with the physical host architecture.

# 8. Host Architecture Independence

## 8.1 Host Abstraction
**FR-HOST-001**
The virtual architecture shall provide an abstraction boundary between guest software and physical host hardware.

**FR-HOST-002**
Host-specific implementation shall not alter the architectural semantics of conforming virtual-machine software.

**FR-HOST-003**
The same conforming `.trn` executable shall be capable of execution on every host implementation supporting the corresponding virtual architecture version.

## 8.2 Initial Host Architectures
The project shall initially target:
* x86-64
* AArch64

Additional host architectures may be introduced later without requiring changes to guest software.

## 8.3 Determinism
**NFR-HOST-001**
For deterministic programs, the reference virtual machine shall produce equivalent architectural results regardless of the supported host architecture.

# 9. Binary Compatibility
Binary compatibility shall be considered an extension of the platform, rather than a prerequisite for the initial VM.
The intended conceptual pipeline is:

```text
Existing binary
      │
      ▼
Architecture decoder
      │
      ▼
Intermediate representation
      │
      ▼
Ternary representation
      │
      ▼
.trn
```

**FR-BIN-001**
The system shall provide an extensible mechanism for translating supported binary programs into the ternary execution environment.

**FR-BIN-002**
Binary translation shall explicitly identify the source architecture.

**FR-BIN-003**
Binary translation shall not claim compatibility with unsupported instructions, operating-system interfaces, or architectural assumptions.

**FR-BIN-004**
The first implementation shall restrict binary translation to a clearly defined compatibility subset.
This prevents B2T from becoming an uncontrolled requirement before the native ternary platform itself exists.

# 10. Non-Functional Requirements

## 10.1 Correctness
**NFR-COR-001**
The reference virtual machine shall conform to the formally defined semantics of the Ternary Machine architecture.

**NFR-COR-002**
Architectural behaviour shall be covered by automated tests.

## 10.2 Determinism
**NFR-DET-001**
Identical initial machine states and identical inputs shall produce identical architectural results for deterministic workloads.

## 10.3 Portability
**NFR-PORT-001**
Architecture-independent components shall not contain dependencies on a specific host instruction set.

**NFR-PORT-002**
Host-specific functionality shall be isolated within explicitly defined host backends.

## 10.4 Maintainability
**NFR-MNT-001**
The project shall maintain a separation between specification, architecture-independent implementation, and host-specific implementation.

**NFR-MNT-002**
Public architectural interfaces shall be documented.

**NFR-MNT-003**
Changes to architectural behaviour shall be reflected in the relevant versioned specification.

## 10.5 Testability
**NFR-TEST-001**
Each major subsystem shall have independently executable automated tests.

**NFR-TEST-002**
The project shall maintain architecture conformance tests independent of the reference VM implementation where practical.

**NFR-TEST-003**
Cross-host tests shall verify that supported host implementations produce equivalent architectural results.

## 10.6 Observability
The reference implementation shall provide sufficient diagnostic facilities to inspect:
* virtual CPU state;
* memory state;
* instruction execution;
* interrupts;
* virtual device activity;
* executable loading;
* and execution faults.

This capability shall be available during development even if it is not exposed to production applications.

# 11. Software Engineering Requirements

## 11.1 Development Languages
The project shall use languages according to subsystem requirements rather than for novelty.
The initial allocation shall be:

| Component | Language | Rationale |
| :--- | :--- | :--- |
| Virtual machine | C++ | Precise low-level control and efficient host-side execution |
| Toolchain | C++ | Binary-format and architecture tooling |
| Operating system | Rust | Memory safety with low-level hardware control |
| Host-specific assembly | Assembly | Only where required by the host architecture |
| Documentation | Markdown | Version-controlled engineering documentation |

No additional programming language shall be introduced without a specific technical justification.

## 11.2 Build System
The C++ components shall use a reproducible, platform-independent build system.
The initial build system shall be based on:
* CMake;
* Ninja;
* a standards-conforming modern C++ compiler.

The Rust operating-system components shall use Cargo.

## 11.3 Version Control
Git shall be used as the project's version-control system.
The repository shall contain:
* source code;
* specifications;
* tests;
* build configuration;
* documentation;
* and project metadata required to reproduce development builds.

## 11.4 Continuous Integration
The project shall eventually use automated CI to perform:
* compilation;
* unit tests;
* architecture tests;
* static analysis where applicable;
* formatting checks;
* and integration tests.

CI shall be introduced once the first executable implementation exists.

# 12. Verification and Validation
The project shall follow a layered verification strategy.

```text
Ternary arithmetic
        ↓
ISA semantics
        ↓
CPU execution
        ↓
Memory subsystem
        ↓
Virtual devices
        ↓
Executable loader
        ↓
Operating system
        ↓
Cross-host execution
```

Each layer shall have tests before dependent layers are considered stable.

## 12.1 Verification Milestones

**V-001 — Ternary correctness**
Verify arithmetic and representation operations.

**V-002 — ISA conformance**
Verify individual instructions against the architecture specification.

**V-003 — VM conformance**
Verify complete programs against expected machine states.

**V-004 — Executable compatibility**
Verify `.trn` generation and loading.

**V-005 — OS integration**
Verify kernel boot and system functionality.

**V-006 — Cross-host equivalence**
Run identical workloads on multiple host architectures and compare architectural results.

# 13. Constraints and Assumptions

## 13.1 Constraints
* The initial project is a software project.
* Physical ternary hardware is not required.
* The first host implementations are expected to execute on conventional binary processors.
* The project shall not depend on the physical host having native ternary arithmetic.

## 13.2 Assumptions
* The virtual machine shall be capable of representing the required ternary state using conventional binary host memory.
* The host implementation may internally represent ternary values using binary data structures.
* The physical representation of ternary information is an implementation detail of the virtual machine and shall not alter the virtual architecture's observable semantics.

# 14. Risks

| Risk | Impact | Mitigation |
| :--- | :--- | :--- |
| ISA becomes unnecessarily complex | High | Begin with minimal instruction set |
| Ternary encoding creates excessive overhead | High | Measure encoding density experimentally |
| VM performance is poor | Medium | Establish correctness-first reference VM, optimize later |
| Binary translation proves impractical | High | Keep B2T outside initial critical path |
| Virtual memory model becomes unnecessarily complicated | High | Specify and test memory model independently |
| Architecture leaks host assumptions | High | Enforce strict host/guest boundary |
| Specification and implementation diverge | High | Conformance tests and versioned specifications |
| Project scope becomes excessive | High | Milestone-based development and explicit non-goals |

# 15. Initial Development Roadmap
The project shall be developed incrementally.

**Phase 0 — Foundation**
* Repository
* SRS
* Architecture documentation
* Build infrastructure
* Testing infrastructure

**Phase 1 — Ternary Core**
* Trit representation
* Tryte representation
* Ternary arithmetic
* Ternary logical operations
* Serialization

**Phase 2 — Virtual Architecture**
* Register model
* Instruction model
* Instruction encoding
* Memory model
* Privilege model
* Interrupt model

**Phase 3 — Reference VM**
* CPU implementation
* Memory implementation
* Instruction decoder
* Instruction executor
* Initial virtual devices

**Phase 4 — Native Toolchain**
* Assembler
* Object representation
* Linker
* `.trn` executable format
* Loader

**Phase 5 — Operating System**
* Kernel initialization
* Memory management
* Interrupt handling
* Scheduler
* Processes
* System calls
* Filesystem
* Userland

**Phase 6 — Host Portability**
* x86-64 backend
* AArch64 backend
* Cross-host conformance testing

**Phase 7 — Binary Translation**
* Source binary decoding
* Intermediate representation
* Binary-to-ternary translation
* Compatibility subset

**Phase 8 — Optimization**
* Interpreter optimization
* Translation caching
* JIT/DBT investigation
* Performance evaluation

# 16. Initial Success Criteria
The first major release shall be considered successful when the project can demonstrate:

```text
                  source
                    │
                    ▼
              ternary assembly
                    │
                    ▼
                 assembler
                    │
                    ▼
                 linker
                    │
                    ▼
                 hello.trn
                    │
                    ▼
              Ternary VM
                    │
                    ▼
             Ternary Kernel
                    │
                    ▼
             user application
```

and the same `.trn` application and operating-system image can execute with equivalent architectural behaviour on at least:

```text
x86-64 host
     │
     ▼
Ternary Machine

AArch64 host
     │
     ▼
Ternary Machine
```

# 17. Requirements Traceability
Every functional and non-functional requirement shall have a unique identifier.
Requirements shall eventually be traceable through:

```text
Requirement
     ↓
Architecture specification
     ↓
Implementation
     ↓
Test
     ↓
Release
```

A separate requirements traceability matrix may be maintained outside this SRS.

# 18. Document Status
This document represents SRS version 0.1.
It intentionally does not define the complete Ternary Machine ISA or microarchitecture.
Those details shall be developed in subsequent specifications following validation of the requirements established here.
