#define OLC_PGE_APPLICATION
#include <olcPixelGameEngine.h>

class Example : public olc::PixelGameEngine {
public:
  Example() { sAppName = "Example"; }

  bool OnUserCreate() override { return true; }

  bool OnUserUpdate(float fElapsedTime) override {
    Clear(olc::BLACK);
    Draw(GetMouseX(), GetMouseY(), olc::RED);

    return true;
  }
};

int main() {
  Example example;

  if (example.Construct(128, 128, 4, 4, false, true)) {
    example.Start();
  }

  return 0;
}
