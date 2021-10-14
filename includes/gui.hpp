#include <GLFW/glfw3.h>

#include "chip8.hpp"

#define DISPLAY_SCALE 12

namespace chip8 {
#pragma once
class GUI {
  GLuint keymap[16] = {
      GLFW_KEY_X,                         // |       | X (0) |       |       |
      GLFW_KEY_1, GLFW_KEY_2, GLFW_KEY_3, // | 1 (1) | 2 (2) | 3 (3) |       |
      GLFW_KEY_Q, GLFW_KEY_W, GLFW_KEY_E, // | Q (4) | W (5) | E (6) |       |
      GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D, // | A (7) | S (8) | D (9) |       |
      GLFW_KEY_Z, GLFW_KEY_C,             // | Z (A) |       | C (B) |       |
      GLFW_KEY_4,                         // |       |       |       | 4 (C) |
      GLFW_KEY_R,                         // |       |       |       | R (D) |
      GLFW_KEY_F,                         // |       |       |       | F (E) |
      GLFW_KEY_V,                         // |       |       |       | V (F) |
  };

  int clock = 960;
  Chip8 *interp;

  GLuint displayTexture;
  GLubyte *displayPixels;

  inline void RenderDisplay();
  inline void RenderMetrics(float framerate);

public:
  GUI(Chip8 *, GLuint, GLubyte *);
  void Render();
};
} // namespace chip8
