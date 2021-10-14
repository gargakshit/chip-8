#include <GLFW/glfw3.h>
#include <imgui.h>

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

  ImVec4 fgColor = ImVec4(0.0f, 1.0f, 0.611f, 1.0f);
  ImVec4 bgColor = ImVec4(0.047f, 0.047f, 0.047f, 1.0f);
  ImVec4 labelColor = ImVec4(1.0f, 0.3f, 0.3f, 1.0f);
  ImVec4 successColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);

  int ticks = 0;
  bool tickRequested = false;

  int clock = 960;
  int prevClock = clock;
  Chip8 *interp;

  GLuint displayTexture;
  GLubyte *displayPixels;

  inline void Tick();

  inline void RenderDisplay(float);
  inline void RenderGeneral(float);
  inline void RenderCPUState();
  inline void RenderDebug();
  inline void RenderMemory();
  inline void RenderKeypadState();
  inline void RenderStack();

public:
  GUI(Chip8 *, GLuint, GLubyte *);
  void Render();
};
} // namespace chip8
