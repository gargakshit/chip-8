#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#define OLC_PGE_APPLICATION
#include <olcPixelGameEngine.h>

#include "beep.hpp"
#include "chip8.hpp"

#define CLOCK_SPEED 700.0f
// #define CLOCK_SPEED 150.0f
#define DISPLAY_SCALE 6

class Display : public olc::PixelGameEngine {
  chip8::Chip8 *interp;
  float fAccumulatedTime = 0;

  uint64_t ticks = 0;

  // Start with a paused clock on debug mode to enable stepping as soon as the
  // interpreter is ready
#ifdef _DEBUG
  bool clockPaused = true;
#else
  bool clockPaused = false;
#endif

  // Original chip-8 keypad layout:
  // -----------------
  // | 1 | 2 | 3 | C |
  // | 4 | 5 | 6 | D |
  // | 7 | 8 | 9 | E |
  // | A | 0 | B | F |
  // -----------------
  //
  // Emulated keypad layout on QWERTY:
  // -----------------
  // | 1 | 2 | 3 | 4 |
  // | Q | W | E | R |
  // | A | S | D | F |
  // | Z | X | C | V |
  // -----------------
  // Emulated (Actual)
  olc::Key keypadLayout[16] = {
      olc::X,                    // |       | X (0) |       |       |
      olc::K1, olc::K2, olc::K3, // | 1 (1) | 2 (2) | 3 (3) |       |
      olc::Q,  olc::W,  olc::E,  // | Q (4) | W (5) | E (6) |       |
      olc::A,  olc::S,  olc::D,  // | A (7) | S (8) | D (9) |       |
      olc::Z,  olc::C,           // | Z (A) |       | C (B) |       |
      olc::K4,                   // |       |       |       | 4 (C) |
      olc::R,                    // |       |       |       | R (D) |
      olc::F,                    // |       |       |       | F (E) |
      olc::V,                    // |       |       |       | V (F) |
  };

public:
  Display(chip8::Chip8 *i) {
#ifdef _DEBUG
    sAppName = "Chip-8 DEBUG";
#else
    sAppName = "Chip-8";
#endif

    interp = i;
  }

  bool OnUserCreate() override {
    Clear(olc::DARK_BLUE);
    FillRect(0, 0, 64 * DISPLAY_SCALE, 32 * DISPLAY_SCALE, olc::BLACK);

    DrawString(4, (32 * DISPLAY_SCALE) + 4,
               "Space: Pause    Enter: Tick clock");

    return true;
  }

  bool OnUserUpdate(float fElapsedTime) override {
    fAccumulatedTime += fElapsedTime;
    // Tick the CPU clock roughly at ~700Hz. This is not accurate by any means,
    // but works fine when the yielded framerate is more than 700. Due to how
    // this works, this requires VSync to be disabled.
    if (fAccumulatedTime >= 1.0f / CLOCK_SPEED) {
      if (GetKey(olc::SPACE).bPressed) {
        clockPaused ^= 1;
      }

      bool tickRequested = GetKey(olc::ENTER).bPressed;

#ifdef _DEBUG
      FillRect(64 * DISPLAY_SCALE, 0, ScreenWidth() - (DISPLAY_SCALE * 64),
               ScreenHeight(), olc::DARK_BLUE);

      std::ostringstream debugStr;
      debugStr << std::hex << std::setfill('0');

      if (clockPaused) {
        debugStr << "PAUSED ";
      }

      // Add CPU ticks
      debugStr << std::setw(8) << ticks << " ticks\n\n";
      // Add PC
      debugStr << "PC: " << std::setw(4) << interp->pc << "    ";
      // Add current opcode
      debugStr << "OP: " << std::setw(4) << interp->opcode << "\n\n";
      // Add register data
      debugStr << "v0 v1 v2 v3 v4 v5 v6 v7\n";
      debugStr << std::setw(2) << +interp->reg[0];
      debugStr << " " << std::setw(2) << +interp->reg[1];
      debugStr << " " << std::setw(2) << +interp->reg[2];
      debugStr << " " << std::setw(2) << +interp->reg[3];
      debugStr << " " << std::setw(2) << +interp->reg[4];
      debugStr << " " << std::setw(2) << +interp->reg[5];
      debugStr << " " << std::setw(2) << +interp->reg[6];
      debugStr << " " << std::setw(2) << +interp->reg[7];
      debugStr << "\n\nv8 v9 vA vB vC vD vE vF\n";
      debugStr << std::setw(2) << +interp->reg[8];
      debugStr << " " << std::setw(2) << +interp->reg[9];
      debugStr << " " << std::setw(2) << +interp->reg[10];
      debugStr << " " << std::setw(2) << +interp->reg[11];
      debugStr << " " << std::setw(2) << +interp->reg[12];
      debugStr << " " << std::setw(2) << +interp->reg[13];
      debugStr << " " << std::setw(2) << +interp->reg[14];
      debugStr << " " << std::setw(2) << +interp->reg[15] << "\n\n";
      // Add index register
      debugStr << "Index Register: " << std::setw(4) << interp->index << "\n\n";
      // Add keypad state
      debugStr << "1: " << interp->keypadState[0]
               << " 2: " << interp->keypadState[1]
               << " 3: " << interp->keypadState[2]
               << " C: " << interp->keypadState[3]
               << "\n4: " << interp->keypadState[4]
               << " 5: " << interp->keypadState[5]
               << " 6: " << interp->keypadState[6]
               << " D: " << interp->keypadState[7]
               << "\n7: " << interp->keypadState[8]
               << " 8: " << interp->keypadState[9]
               << " 9: " << interp->keypadState[10]
               << " E: " << interp->keypadState[11]
               << "\nA: " << interp->keypadState[12]
               << " 0: " << interp->keypadState[13]
               << " B: " << interp->keypadState[14]
               << " F: " << interp->keypadState[15] << "\n\n";

      debugStr << "Timers\n"
               << "Delay: " << std::setw(2) << +interp->delayTimer
               << "  Sound: " << std::setw(2) << +interp->soundTimer << "\n\n";

      DrawString((64 * DISPLAY_SCALE) + 8, 8, debugStr.str());
#endif

      fAccumulatedTime -= (1.0f / CLOCK_SPEED);

      if (!clockPaused || tickRequested) {
        ticks++;

        for (int i = 0; i < 16; i++) {
          interp->keypadState[i] = GetKey(keypadLayout[i]).bHeld;
        }

        interp->Tick();

        if (interp->redraw) {
          interp->redraw = false;

          for (int y = 0; y < 32; y++) {
            for (int x = 0; x < 64; x++) {
              FillRect(x * DISPLAY_SCALE, y * DISPLAY_SCALE, DISPLAY_SCALE,
                       DISPLAY_SCALE,
                       interp->display[(y * 64) + x] ? olc::WHITE : olc::BLACK);
            }
          }
        }

        if (interp->beep) {
          interp->beep = false;
          chip8::beep();
        }
      }
    }

    return true;
  }
};

int main(int args, char **argv) {
  if (args != 2) {
    std::cerr << "No chip-8 program specified" << std::endl;
    return 1;
  }

  // Seed the RNG
  srand((unsigned)time(NULL));

  std::cerr << "Loading " << argv[1] << std::endl;

  chip8::Chip8 chip8;
  Display application(&chip8);

  chip8.Reset();

  if (!chip8.LoadProgram(argv[1])) {
    std::cerr << "Unable to load " << argv[1] << std::endl
              << "Aborting" << std::endl;

    return 1;
  }

#ifdef _DEBUG
  if (application.Construct(640, 208, 2, 2)) {
#else
  if (application.Construct(384, 208, 2, 2)) {
#endif
    application.Start();
  }

  return 0;
}
