# WolfmanAlpha — Gear CPU + Zodiac Dial + Sound Pack (C++17)

Simulates a mechanical "gear computer":
- 10 rings × 360 gears (default)
- Movable ring offsets (Stargate-style)
- Three CPU profiles: 64 / 360 / 720-bit words
- Mechanical component stack: clock, bus, CPU core, register bank, RAM, storage
- "Black Zodiac" 13-sign dial overlay (Z0..Z12)
- Generates original "haunted clockwork" WAV sound pack (not film audio)

## Build
```bash
cmake -S . -B build
cmake --build build -j
./build/wa_console
```

## Console commands
- `help`
- `mode 64|360|720`
- `cap [1|2]`
- `set r i v`
- `flip r i`
- `shift r LEFT|RIGHT k`
- `tick k`
- `print r [count]`
- `regs [countBits]`
- `run n`
- `step`
- `glyph r`
- `dial r Z0..Z12`
- `calc eval <expr>`
- `calc evalx <x> <expr>`
- `calc deriv <x> <expr>`
- `calc integ <a> <b> <n> <expr>`
- `calc quad <a> <b> <c>`
- `calc solve <lhs=rhs>`
- `equation <lhs=rhs>`
- `quit`

Console I/O now uses a clock system:
- Prompt shows live clock, command count, and gear ticks.
- Input/Output/Event panes store timestamped lines.
- `clock status` reports command and tick counters.

## Sound files
Generated into `assets/` at startup:
- `clockwork_loop.wav`
- `ratchet_tick.wav`
- `gear_whirr.wav`
- `haunted_drone.wav`
- `zodiac_13_pulse.wav`

## Gear math library (`wa_math.hpp`)
Core compute helpers for WolfmanAlpha gear systems:
- Geometry: `deg_to_rad`, `rad_to_deg`, `polar`, `ring_radius_from_pitch_radius`
- Mod arithmetic: `normalize_mod`, `add_mod`, `sub_mod`, `mul_mod`, `pow_mod`, `gcd`, `lcm`
- Ring transforms: `logical_to_physical_index`, `physical_to_logical_index`, `apply_shift_offset`, `shortest_signed_steps`
- Angle/index mapping: `ring_index_to_angle_deg`, `ring_index_to_angle_rad`, `angle_rad_to_ring_index`
- Bit compute helpers: `xor_bits`, `and_bits`, `or_bits`, `not_bits`, `rotate_bits_left`, `rotate_bits_right`, `bits_to_u64`, `u64_to_bits`, `bits_to_string`

## Mechanical computer components (`wa_components.hpp`)
- `MechanicalClock`: tick source for mechanical cycles
- `GearBus`: bit-wide transfer bus between components
- `GearRegisterBank`: gear-register set
- `GearRAM`: volatile memory words
- `GearStorage`: persistent memory words with mechanical head seek
- `GearCPUCore`: instruction executor for `MOVI/MOV/LOAD/STORE/ADD/AND/OR/XOR/JMP/JZ/HALT`
- `MechanicalComputer`: integrated machine wrapper for all components

## Blueprint schematic
- Working blueprint document: `docs/BLUEPRINT_SCHEMATIC.md`
- Blueprint drawing image: `assets/blueprint_schematic.svg`
