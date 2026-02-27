# WolfmanAlpha Mechanical Computer Blueprint Schematic

## Purpose
This document is a working blueprint for a gear-and-clock mechanical computer architecture used by WolfmanAlpha.

## System Overview
The machine is organized as six physical subsystems connected by a mechanical timing spine:

1. Clock and escapement train
2. Instruction/control deck
3. Gear CPU core and register bank
4. Mechanical RAM drum
5. Mechanical storage drum
6. Transfer bus and selector matrix

Each instruction executes over clock phases and moves information as bit states on gear teeth.

## Component Blueprint
### A. Clock and Escapement
- Master spring/flywheel drives the system.
- Escapement outputs discrete ticks to the phase cam.
- Phase cam provides four micro-phases per instruction cycle:
  - `P0`: fetch
  - `P1`: decode/select
  - `P2`: execute
  - `P3`: writeback
- Clock maps to software component: `MechanicalClock`.

### B. Gear Bus and Selector Matrix
- 64-bit bus represented by 64 parallel gear rods.
- Tri-state equivalent is implemented by selector clutches.
- Only one source clutch may engage per phase.
- Software mapping:
  - `GearBus::writeBits(...)`
  - `GearBus::readBits()`

### C. Register Bank
- 8 general-purpose registers, 64 bits each.
- Each bit cell is a two-position detent gear.
- Register read: engage source register clutch to bus.
- Register write: engage sink register clutch from bus at `P3`.
- Software mapping: `GearRegisterBank`.

### D. Mechanical RAM Drum
- Volatile drum memory: 512 words x 64 bits (default in app startup).
- Addressed by rotational index and head position.
- One read/write port through bus matrix.
- Read latency modeled as one instruction phase.
- Software mapping: `GearRAM`.

### E. Mechanical Storage Drum
- Persistent drum memory: 8192 words x 64 bits.
- Includes seek head and track selector.
- Used for long-term program/data storage.
- Software mapping: `GearStorage`.

### F. Gear CPU Core
- Executes instruction set:
  - `MOVI, MOV, LOAD, STORE, ADD, AND, OR, XOR, JMP, JZ, HALT`
- ALU built from bitwise combiner gears plus carry train.
- CPU increments instruction pointer unless branch changes flow.
- One mechanical tick consumed per instruction step.
- Software mapping: `GearCPUCore`.

## Instruction Cycle Schematic
1. `P0 Fetch`:
   - Instruction at `IP` selected.
   - Control deck cams set source/sink clutches.
2. `P1 Decode`:
   - Operand source registers or memory selected.
3. `P2 Execute`:
   - ALU or transfer path runs.
   - Branch target resolves if branch op.
4. `P3 Writeback`:
   - Destination register or memory writes.
   - Tick counter advances.

## Mechanical Constraints
- Bus width: fixed at 64 bits in current integrated system.
- Register file count: 8.
- RAM words: 512.
- Storage words: 8192.
- Clock nominal: 24 Hz in startup configuration.

## Signal/Power Flow
- Power flow: spring/flywheel -> gear train -> phase cam -> subsystem clutches.
- Data flow: source component -> bus -> destination component.
- Timing authority: clock phase cam gates all movements.

## Assembly Notes
- Align gear lash with minimal backlash before locking clutch forks.
- Calibrate phase cam to ensure writeback occurs after execute phase.
- Ensure no dual-source bus clutch engagement in same phase.

## Verification Checklist
- Tick progression increments once per `cpu.step()`.
- `LOAD/STORE` preserve bit-exact 64-bit data patterns.
- `ADD` carry chain works across all 64 bits.
- `JMP/JZ` update `IP` as expected.
- Register and memory boundaries throw range errors.

## Referenced Implementation
- Mechanical components API:
  - `include/wolfman_alpha/wa_components.hpp`
  - `src/wa_components.cpp`
- Console and machine integration:
  - `apps/wa_console.cpp`
  - `src/wa_io.cpp`
