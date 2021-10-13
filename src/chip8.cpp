#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>

#include "chip8.hpp"

namespace chip8 {
void Chip8::Reset() {
  // Program Counter starts at 0x200
  pc = 0x200;

  // Reset other state
  index = 0;
  sp = 0;
  opcode = 0;

  // Reset timers
  delayTimer = 0;
  soundTimer = 0;

  // Clear display
  for (int i = 0; i < display.size(); i++) {
    display[i] = false;
  }

  // Clear registers
  for (int i = 0; i < reg.size(); i++) {
    reg[i] = 0;
  }

  // Clear stack
  for (int i = 0; i < stack.size(); i++) {
    stack[i] = 0;
  }

  // Load the font into memory
  for (int i = 0; i < 80; i++) {
    mem[i] = font[i];
  }
}

bool Chip8::LoadProgram(std::string filename) {
  std::streampos fileSize;
  std::ifstream ifile;

  ifile.open(filename, std::ios::binary);

  if (!ifile) {
    return false;
  }

  // Get its size
  ifile.seekg(0, std::ios::end);
  fileSize = ifile.tellg();
  ifile.seekg(0, std::ios::beg);

  int i = 0;
  char b;

  while (ifile.get(b)) {
    mem[i + 512] = b;
    i++;
  }

  ifile.close();

  return true;
}

void Chip8::Tick() {
  opcode = mem[pc] << 8 | mem[pc + 1]; // Fetch a 16bit opcode

  bool invalid = false;

  // Get the first nibble using a 0xF000 bitmask
  switch (opcode & 0xF000) {
  case 0x0000: {
    switch (opcode) {
    // Clear Screen
    case 0x00E0:
      for (int i = 0; i < 2048; i++) {
        display[i] = false;
      }
      draw = true;

      pc += 2;

      break;

      // Return from subroutine
    case 0x00EE:
      // TODO: implement
      pc += 2;
      break;

      // Call
    default:
      invalid = true;
      pc += 2;

      break;
    }

    break;
  }

  // 1NNN (Jump to NNN)
  case 0x1000: {
    pc = opcode & 0x0FFF;
    break;
  }

  // 6XNN (Set reg X to NN)
  case 0x6000: {
    reg[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
    pc += 2;
    break;
  }

  // 7XNN (Add NN to reg X)
  case 0x7000: {
    reg[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
    pc += 2;
    break;
  }

  // ANNN (Set I to NNN)
  case 0xA000: {
    index = opcode & 0x0FFF;
    pc += 2;
    break;
  }

  // DXYN (Display X, Y, N)
  case 0xD000: {
    auto x = reg[(opcode & 0x0F00) >> 8];
    auto y = reg[(opcode & 0x00F0) >> 4];
    uint8_t height = opcode & 0x000F;

    // Clear the status (F) reg
    reg[0xF] = 0;
    for (int yLine = 0; yLine < height; yLine++) {
      auto pixel = mem[index + yLine];

      for (int xLine = 0; xLine < 8; xLine++) {
        auto idx = x + xLine + ((y + yLine) * 64);

        if ((pixel & (0x80 >> xLine)) != 0) {
          if (display[idx]) {
            reg[0xF] = 1;
          }

          display[idx] = !display[idx];
        }
      }
    }

    pc += 2;
    draw = true;

    break;
  }

    // Wrap the program counter after 4kb
    pc %= 4096;

  default:
    invalid = true;
    pc += 2;

    break;
  }

  // Update timers
  if (delayTimer > 0) {
    delayTimer--;
  }

  // TODO: add actual sound
  if (soundTimer > 0) {
    soundTimer--;
  }

  if (invalid) {
    std::cerr << "Invalid opcode: " << opcode << std::endl;
  }
}
} // namespace chip8
