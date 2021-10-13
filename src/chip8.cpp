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
  std::ifstream ifile;

  ifile.open(filename, std::ios::binary);

  if (!ifile) {
    return false;
  }

  int i = 0;
  char b;

  while (ifile.get(b)) {
    mem[i + 512] = b;
    i++;
  }

  ifile.close();

  return true;
}

void Chip8::stackPush(uint16_t data) { stack[sp++] = data; }

uint16_t Chip8::stackPop() { return stack[sp--]; }

void Chip8::Tick() {
  opcode = mem[pc] << 8 | mem[pc + 1]; // Fetch a 16bit opcode

  bool invalid = false;

  // Get the first nibble using a 0xF000 bitmask
  switch (opcode & 0xF000) {
  case 0x0000: {
    switch (opcode) {
    // Clear Screen
    case 0x00E0: {
      for (int i = 0; i < 2048; i++) {
        display[i] = false;
      }

      pc += 2;
      redraw = true;

      break;
    }

    // Return from subroutine
    case 0x00EE: {
      pc = stackPop();
      break;
    }

    // Call
    default: {
      invalid = true;
      break;
    }
    }

    break;
  }

  // 1NNN (Jump to NNN)
  case 0x1000: {
    pc = opcode & 0x0FFF;
    break;
  }

  // 2NNN (Call at NNN)
  case 0x2000: {
    stackPush(pc);
    pc = opcode & 0x0FFF;
    break;
  }

  // 3XNN (Skip next if NN == vX)
  case 0x3000: {
    if (reg[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF)) {
      pc += 2;
    }
    pc += 2;
    break;
  }

  // 4XNN (Skip next if NN != vX)
  case 0x4000: {
    if (reg[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF)) {
      pc += 2;
    }
    pc += 2;
    break;
  }

  // 5XY0 (Skip next if vX == xY)
  case 0x5000: {
    if (reg[(opcode & 0x0F00) >> 8] == reg[(opcode & 0x00F0) >> 4]) {
      pc += 2;
    }
    pc += 2;
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

  case 0x8000: {
    switch (opcode & 0x000F) {
    // 8XY0 (Assign vX = vY)
    case 0x0: {
      reg[(opcode & 0x0F00) >> 8] = reg[(opcode & 0x00F0) >> 4];
      pc += 2;
      break;
    }

    // 8XY1 (Assign vX = vX | vY)
    case 0x1: {
      reg[(opcode & 0x0F00) >> 8] =
          reg[(opcode & 0x0F00) >> 8] | reg[(opcode & 0x00F0) >> 4];
      pc += 2;
      break;
    }

    // 8XY2 (Assign vX = vX & vY)
    case 0x2: {
      reg[(opcode & 0x0F00) >> 8] =
          reg[(opcode & 0x0F00) >> 8] & reg[(opcode & 0x00F0) >> 4];
      pc += 2;
      break;
    }

    // 8XY3 (Assign vX = vX ^ vY)
    case 0x3: {
      reg[(opcode & 0x0F00) >> 8] =
          reg[(opcode & 0x0F00) >> 8] ^ reg[(opcode & 0x00F0) >> 4];
      pc += 2;
      break;
    }

    // 8XY4 (Assign vX += vY with carry)
    case 0x4: {
      if (reg[(opcode & 0x00F0) >> 4] > (0xFF - reg[(opcode & 0x0F00) >> 8])) {
        reg[0xF] = 1;
      } else {
        reg[0xF] = 0;
      }

      reg[(opcode & 0x0F00) >> 8] += reg[(opcode & 0x00F0) >> 4];
      pc += 2;
      break;
    }

    // 8XY5 (Assign vX -= vY with borrow)
    case 0x5: {
      if (reg[(opcode & 0x00F0) >> 4] > reg[(opcode & 0x0F00) >> 8]) {
        reg[0xF] = 1;
      } else {
        reg[0xF] = 0;
      }

      reg[(opcode & 0x0F00) >> 8] -= reg[(opcode & 0x00F0) >> 4];
      pc += 2;
      break;
    }

    // 8XY6 (Assign vX >>= 1 and store the LSB into vF)
    case 0x6: {
      reg[0xF] = reg[(opcode & 0x0F00) >> 8] & 0x1;
      reg[(opcode & 0x0F00) >> 8] >>= 1;
      pc += 2;
      break;
    }

    // 8XY7 (Assign vX = vY = vX with borrow)
    case 0x7: {
      if (reg[(opcode & 0x0F00) >> 8] > reg[(opcode & 0x00F0) >> 4]) {
        reg[0xF] = 0;
      } else {
        reg[0xF] = 1;
      }

      reg[(opcode & 0x0F00) >> 8] =
          reg[(opcode & 0x00F0) >> 4] - reg[(opcode & 0x0F00) >> 8];
      pc += 2;
      break;
    }

    // 8XYE (Assign vX <<= 1 and store the MSB into vF)
    case 0xE: {
      reg[0xF] = reg[(opcode & 0x0F00) >> 8] & 0x7;
      reg[(opcode & 0x0F00) >> 8] <<= 1;
      pc += 2;
      break;
    }

    default: {
      invalid = true;
      break;
    }
    }

    break;
  }

  // 9XY0 (Skip next if vX != xY)
  case 0x9000: {
    if (reg[(opcode & 0x0F00) >> 8] != reg[(opcode & 0x00F0) >> 4]) {
      pc += 2;
    }
    pc += 2;
    break;
  }

  // ANNN (Set I to NNN)
  case 0xA000: {
    index = opcode & 0x0FFF;
    pc += 2;
    break;
  }

  // BNNN (Jump to v0 + NNN)
  case 0xB000: {
    pc = (opcode & 0x0FFF) + reg[0];
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
        auto index = (x + xLine + ((y + yLine) * 64));

        if ((pixel & (0x80 >> xLine)) != 0) {
          if (display[index]) {
            reg[0xF] = 1;
          }

          display[index] ^= 1;
        }
      }
    }

    pc += 2;
    redraw = true;

    break;
  }

    // Wrap the program counter after 4kb
    pc %= 4096;

  default:
    invalid = true;
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
