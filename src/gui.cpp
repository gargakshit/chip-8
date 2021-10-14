#include <GLFW/glfw3.h>
#include <imgui.h>

#include "beep.hpp"
#include "gui.hpp"

namespace chip8 {
GUI::GUI(Chip8 *c8, GLuint texture, GLubyte *pixels) {
  interp = c8;
  displayTexture = texture;
  displayPixels = pixels;
}

inline void GUI::RenderDisplay() {
  ImGui::Begin("Display", NULL,
               ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

  ImGui::SetWindowSize(
      ImVec2(32 + (64 * DISPLAY_SCALE), 48 + (32 * DISPLAY_SCALE)));

  if (interp->beep) {
    interp->beep = false;
    beep();
  }

  if (interp->redraw) {
    interp->redraw = false;
    // Draw scaled display
    for (int y = 0; y < 32; y++) {
      for (int x = 0; x < 64; x++) {
        int subpixel = interp->display[(y * 64) + x] ? 255 : 0;

        for (int xScale = 0; xScale < DISPLAY_SCALE; xScale++) {
          for (int yScale = 0; yScale < DISPLAY_SCALE; yScale++) {
            int i = ((((y * DISPLAY_SCALE * 64) * DISPLAY_SCALE) +
                      (x * DISPLAY_SCALE) + xScale) +
                     (yScale * DISPLAY_SCALE * 64)) *
                    3;

            displayPixels[i] = subpixel;
            displayPixels[i + 1] = subpixel;
            displayPixels[i + 2] = subpixel;
          }
        }
      }
    }
  }

  // Render it onto the opengl texture
  glBindTexture(GL_TEXTURE_2D, displayTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 64 * DISPLAY_SCALE, 32 * DISPLAY_SCALE,
               0, GL_RGB, GL_UNSIGNED_BYTE, displayPixels);

  ImGui::Image((void *)(intptr_t)displayTexture,
               ImVec2(64 * DISPLAY_SCALE, 32 * DISPLAY_SCALE));
  ImGui::End();
}

inline void GUI::RenderMetrics(float framerate) {
  ImGui::Begin("Metrics", NULL,
               ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
                   ImGuiWindowFlags_AlwaysAutoResize);
  ImGui::Text("FPS:   %f", framerate);
  ImGui::Text("Clock: %dHz", clock);
  ImGui::Text("Scale: %d", DISPLAY_SCALE);
  ImGui::End();
}

void GUI::Render() {
  auto framerate = ImGui::GetIO().Framerate;

  // Tick the clock CPU_CLOCK divided by framerate. Not the best way, but heh
  // it works while consuming sane amounts of CPU and GPU
  for (int i = 0; i < (clock / framerate); i++) {
    // Get the keyboard state every tick
    for (int i = 0; i < 16; i++) {
      interp->keypadState[i] = ImGui::IsKeyDown(keymap[i]);
    }

    interp->Tick();
  }

  RenderDisplay();
  RenderMetrics(framerate);
}
} // namespace chip8
