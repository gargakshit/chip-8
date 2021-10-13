#include <atomic>
#include <cstdint>
#include <iostream>
#include <stdint.h>
#include <thread>

#define OLC_PGE_APPLICATION
#include <olcPixelGameEngine.h>

#include "chip8.hpp"
#include "high_res_timer.hpp"

#define CLOCK_SPEED 700
#define NANO_SECONDS_PER_TICK 1428000

class Display : public olc::PixelGameEngine {
  chip8::Chip8 *interp;
  float fAccumulatedTime = 0;

  std::atomic_bool ticking = false;
  std::thread ticking_thread;

public:
  Display(chip8::Chip8 *i) {
    sAppName = "Chip-8";
    interp = i;
  }

  void ticker() {
    uint64_t start = timer::ns();
    uint64_t delta = 0;

    while (ticking) {
      auto current = timer::ns();
      delta += current - start;
      start = current;

      if (delta >= NANO_SECONDS_PER_TICK) {
        interp->Tick();
        delta = 0;
      }
    }
  }

  bool OnUserCreate() override {
    if (ticking) {
      return false;
    }

    Clear(olc::BLACK);

    ticking = true;
    ticking_thread = std::thread(&Display::ticker, this);

    return true;
  }

  bool OnUserDestroy() override {
    ticking = false;

    return true;
  }

  bool OnUserUpdate(float fElapsedTime) override {
    fAccumulatedTime += fElapsedTime;
    if (interp->draw) {
      interp->draw = false;

      for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 64; x++) {
          Draw(x, y, interp->display[(y * 64) + x] ? olc::WHITE : olc::BLACK);
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

  if (application.Construct(64, 32, 16, 16, false, true)) {
    application.Start();
  }

  return 0;
}
