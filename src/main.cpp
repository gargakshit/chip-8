#include <iostream>

#define OLC_PGE_APPLICATION
#include <olcPixelGameEngine.h>

#include "chip8.hpp"

class Display : public olc::PixelGameEngine {
  chip8::Chip8 *interp;

public:
  Display(chip8::Chip8 *i) {
    sAppName = "Chip-8";
    interp = i;
  }

  bool OnUserCreate() override { return true; }

  bool OnUserUpdate(float fElapsedTime) override {
    interp->Tick();

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

  if (application.Construct(64, 32, 16, 16, false, true)) {
    application.Start();
  }

  return 0;
}
