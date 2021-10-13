#include <cstdint>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#define OLC_PGE_APPLICATION
#include <olcPixelGameEngine.h>

#include "chip8.hpp"

#define CLOCK_SPEED 700.0f
#define DISPLAY_SCALE 6

class Display : public olc::PixelGameEngine {
  chip8::Chip8 *interp;
  float fAccumulatedTime = 0;

  uint64_t ticks = 0;
  bool clockPaused = false;

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

      DrawString((64 * DISPLAY_SCALE) + 8, 8, debugStr.str());
#endif

      if (!clockPaused || tickRequested) {
        ticks++;

        interp->Tick();
        fAccumulatedTime -= (1.0f / CLOCK_SPEED);

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
