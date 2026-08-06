# CHIP-8 Emulator

A CHIP-8 interpreter/emulator written in C, using SDL for graphics and input. CHIP-8 is a simple, interpreted programming language from the 1970s originally used on 8-bit microcomputers; emulating it is a classic starting project for learning how CPUs, memory, and instruction decoding work.

## Features

- Full CHIP-8 CPU core: fetch-decode-execute loop operating on 4KB of addressable memory
- 16 general-purpose 8-bit registers (`V0`–`VF`), 16-bit index register (`I`), program counter, and a call stack for subroutines
- Delay and sound timer registers
- Built-in hexadecimal font set for rendering digits `0`–`F`
- Sprite drawing with collision detection (`VF` flag)
- SDL-based rendering and keyboard input handling

## Implemented Opcodes

| Opcode | Description |
|--------|-------------|
| `00E0` | Clear the screen |
| `00EE` | Return from subroutine |
| `1NNN` | Jump to address `NNN` |
| `2NNN` | Call subroutine at `NNN` |
| `3XNN` | Skip next instruction if `VX == NN` |
| `4XNN` | Skip next instruction if `VX != NN` |
| `5XY0` | Skip next instruction if `VX == VY` |
| `6XNN` | Set `VX = NN` |
| `7XNN` | Add `NN` to `VX` |
| `8XY0`–`8XYE` | Register-to-register arithmetic and bitwise ops (assign, OR, AND, XOR, add with carry, subtract with borrow, shifts) |
| `9XY0` | Skip next instruction if `VX != VY` |
| `ANNN` | Set index register `I = NNN` |
| `BNNN` | Jump to `NNN + V0` |
| `CXNN` | Set `VX` to a random number masked by `NN` |
| `DXYN` | Draw an `N`-byte sprite at `(VX, VY)` |
| `EX9E` / `EXA1` | Skip next instruction based on key press state |
| `FX07`, `FX15`, `FX18` | Read/set delay and sound timers |
| `FX0A` | Wait for a key press, store in `VX` |
| `FX1E` | Add `VX` to `I` |
| `FX29` | Set `I` to the sprite location for the character in `VX` |
| `FX33` | Store the binary-coded decimal representation of `VX` at `I`, `I+1`, `I+2` |
| `FX55` / `FX65` | Store/load registers `V0`–`VX` to/from memory starting at `I` |

## Project Structure

```
.
├── mychip8.c        # Core CHIP-8 CPU: opcode fetch/decode/execute, registers, memory access
├── sdl-basecode.c    # SDL setup: window, rendering, and input handling
└── README.md         # This file
```

## Requirements

- A C compiler (GCC/Clang)
- [SDL 1.2](https://www.libsdl.org/) development libraries (`SDL/SDL.h`)

On Debian/Ubuntu:

```bash
sudo apt-get install libsdl1.2-dev
```

## Building

```bash
gcc mychip8.c sdl-basecode.c -o chip8 -lSDL
```

*(Exact build command may vary depending on how the SDL frontend and CPU core are wired together — adjust flags/paths as needed for your setup.)*

## Usage

Run the emulator with a CHIP-8 ROM:

```bash
./chip8 path/to/rom.ch8
```

ROMs are loaded into memory starting at address `0x200`, matching the original CHIP-8 convention.

## Status

This is a work-in-progress project. Most of the standard CHIP-8 instruction set is implemented (see table above), including sprite drawing, timers, and keypad handling. Known rough edges:
- `FX0A` (wait for key press) has an early `break` missing in one branch, which may cause fallthrough into `FX15`.
- Sound output (buzzer on `ST > 0`) is not yet wired up beyond the timer register itself.

## References

- [Cowgod's CHIP-8 Technical Reference / EMUL8 HOWTO](http://www.emulation.org/EMUL8/HOWTO.html)
- [Tobias V. Langhoff — "How to write an emulator (CHIP-8 interpreter)"](https://tobiasvl.github.io/blog/write-a-chip-8-emulator/)

## Author

Tim Huynh
