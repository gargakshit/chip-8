#include <iostream>

#define OLC_PGE_APPLICATION
#include <olcPixelGameEngine.h>

#include "chip8.hpp"

#define CLOCK_SPEED 700.0f

class Display : public olc::PixelGameEngine {
  chip8::Chip8 *interp;
  float fAccumulatedTime = 0;

public:
  Display(chip8::Chip8 *i) {
    sAppName = "Chip-8";
    interp = i;
  }

  bool OnUserCreate() override { return true; }

  bool OnUserUpdate(float fElapsedTime) override {
    fAccumulatedTime += fElapsedTime;
    // Tick the CPU clock roughly at ~700Hz. This is not accurate by any means,
    // but works fine when the yielded framerate is more than 700. Due to how
    // this works, this requires VSync to be disabled.
    if (fAccumulatedTime >= 1.0f / CLOCK_SPEED) {
      interp->Tick();
      fAccumulatedTime -= (1.0f / CLOCK_SPEED);
    }

    for (int y = 0; y < ScreenHeight(); y++) {
      for (int x = 0; x < ScreenWidth(); x++) {
        Draw(x, y, interp->display[(y * 64) + x] ? olc::WHITE : olc::BLACK);
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

  if (application.Construct(64, 32, 16, 16)) {
    application.Start();
  }

  return 0;
}
