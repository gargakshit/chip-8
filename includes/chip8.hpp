#include <array>
#include <cstdint>
#include <string>

namespace chip8 {
#pragma once
class Chip8 {
  uint8_t font[80] = {
      0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
      0x20, 0x60, 0x20, 0x20, 0x70, // 1
      0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
      0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
      0x90, 0x90, 0xF0, 0x10, 0x10, // 4
      0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
      0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
      0xF0, 0x10, 0x20, 0x40, 0x40, // 7
      0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
      0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
      0xF0, 0x90, 0xF0, 0x90, 0x90, // A
      0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
      0xF0, 0x80, 0x80, 0x80, 0xF0, // C
      0xE0, 0x90, 0x90, 0x90, 0xE0, // D
      0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
      0xF0, 0x80, 0xF0, 0x80, 0x80, // F
  };

  void stackPush(uint16_t data);
  uint16_t stackPop();

public:
  std::array<bool, 64 * 32> display; // State of the 64x32 monochrome display
  bool redraw = false; // Only redraw when requested. The display module must
                       // set it to false after drawing.

  bool beep = true; // Signal the display that the system needs to "beep". The
                    // display module must set it to false after beeping

  std::array<uint8_t, 4096> mem; // 4kb memory

  std::array<bool, 16> keypadState; // State of the keypad (0 to F)

  std::array<uint8_t, 16> reg; // 16 registers (v0 to vF)
  uint16_t pc;                 // Program Counter
  uint16_t index;              // Index register
  uint16_t opcode;             // Current opcode

  std::array<uint16_t, 16> stack; // Stack with size of 16
  uint8_t sp;                     // Stack pointer

  uint8_t delayTimer; // 8 bit delay timer
  uint8_t soundTimer; // 8 bit sound timer

  void Reset();
  bool LoadProgram(const std::string &filename);
  void Tick();
  void TickTimer();
};
} // namespace chip8
