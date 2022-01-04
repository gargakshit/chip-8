#include <chrono>
#include <cstdint>

#include <GLFW/glfw3.h>
#include <imgui.h>

#include "beep.hpp"
#include "gui.hpp"

using std::chrono::high_resolution_clock;

namespace chip8 {
GUI::GUI(Chip8 *c8, GLuint texture, GLubyte *pixels) {
  interp = c8;
  displayTexture = texture;
  displayPixels = pixels;
  lastTimer = high_resolution_clock::now();
  memoryEditor.Cols = 8;
}

inline void GUI::Tick() {
  // Get the keyboard state every tick only if the current window has focus
  if (ImGui::IsWindowFocused()) {
    for (int i = 0; i < 16; i++) {
      interp->keypadState[i] = ImGui::IsKeyDown(keymap[i]);
    }
  }

  ticks++;
  interp->Tick();
}

inline void GUI::RenderDisplay(float framerate) {
  ImGui::Begin("Display", NULL,
               ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
                   ImGuiWindowFlags_NoTitleBar);

  ImGui::SetWindowSize(
      ImVec2(32 + (64 * DISPLAY_SCALE), 48 + (32 * DISPLAY_SCALE)));

  // Explicitly requested
  if (tickRequested) {
    tickRequested = false;
    Tick();
  }

  // Tick the clock frequency divided by framerate. Not the best way, but heh
  // it works while consuming sane amounts of CPU and GPU
  for (int i = 0; i < (clockSpeed / framerate); i++) {
    Tick();
  }

  auto currentTime = high_resolution_clock::now();
  if ((currentTime - lastTimer).count() >= 16666666) {
    interp->TickTimer();
    lastTimer = currentTime;
  }

  if (interp->beep) {
    interp->beep = false;
    beep();
  }

  if (interp->redraw) {
    interp->redraw = false;

    GLubyte fg[3] = {
        static_cast<GLubyte>(fgColor.x * 255),
        static_cast<GLubyte>(fgColor.y * 255),
        static_cast<GLubyte>(fgColor.z * 255),
    };
    GLubyte bg[3] = {
        static_cast<GLubyte>(bgColor.x * 255),
        static_cast<GLubyte>(bgColor.y * 255),
        static_cast<GLubyte>(bgColor.z * 255),
    };

    // Draw the display. Scaling is handled by opengl's nearest neighbour
    for (int i = 0; i < 64 * 32; i++) {
      auto subpixel = interp->display[i] ? fg : bg;

      for (int j = 0; j < 3; j++) {
        displayPixels[i * 3 + j] = subpixel[j];
      }
    }

    // Render it onto the opengl texture
    glBindTexture(GL_TEXTURE_2D, displayTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 64, 32, 0, GL_RGB, GL_UNSIGNED_BYTE,
                 displayPixels);
    glBindTexture(GL_TEXTURE_2D, 0);
  }

  ImGui::Image((void *)(intptr_t)displayTexture,
               ImVec2(64 * DISPLAY_SCALE, 32 * DISPLAY_SCALE));
  ImGui::End();
}

inline void GUI::RenderGeneral(float framerate) {
  ImGui::Begin("General", NULL, ImGuiWindowFlags_AlwaysAutoResize);

  ImGui::TextColored(labelColor, "FPS:");
  ImGui::SameLine();
  ImGui::Text("%f", framerate);

  ImGui::TextColored(labelColor, "Ticks:");
  ImGui::SameLine();
  ImGui::Text("%d", ticks);

  ImGui::TextColored(labelColor, "Display Scale:");
  ImGui::SameLine();
  ImGui::Text("%d", DISPLAY_SCALE);

  ImGui::TextColored(labelColor, "Clock:");
  ImGui::SameLine();
  ImGui::InputInt("Hz", &clockSpeed);

  ImGui::ColorEdit3("FG Color", (float *)&fgColor);
  ImGui::ColorEdit3("BG Color", (float *)&bgColor);

  ImGui::End();
}

inline void GUI::RenderCPUState() {
  ImGui::Begin("CPU State", NULL, ImGuiWindowFlags_AlwaysAutoResize);

  ImGui::TextColored(labelColor, "PC:");
  ImGui::SameLine();
  ImGui::Text("%04X", interp->pc);

  ImGui::TextColored(labelColor, "IR:");
  ImGui::SameLine();
  ImGui::Text("%04X", interp->index);

  ImGui::TextColored(labelColor, "OP:");
  ImGui::SameLine();
  ImGui::Text("%04X", interp->opcode);

  // Render registers
  ImGui::NewLine();
  ImGui::TextColored(labelColor, "Reg (V)");

  for (int i = 0; i < 16; i++) {
    ImGui::TextColored(labelColor, "%01X:", i);
    ImGui::SameLine();
    ImGui::Text("%02X  ", interp->reg[i]);
    if (i % 2 == 0) {
      ImGui::SameLine();
    }
  }

  ImGui::NewLine();

  ImGui::TextColored(labelColor, "DT:");
  ImGui::SameLine();
  ImGui::Text("%02X", interp->delayTimer);

  ImGui::TextColored(labelColor, "ST:");
  ImGui::SameLine();
  ImGui::Text("%02X", interp->soundTimer);

  ImGui::End();
}

inline void GUI::RenderDebug() {
  ImGui::Begin("Debug", NULL, ImGuiWindowFlags_AlwaysAutoResize);

  ImGui::TextColored(labelColor, "Status");
  ImGui::Text(clockSpeed == 0 ? "Paused" : "Running");

  ImGui::TextColored(labelColor, "Clock");
  if (ImGui::Button(clockSpeed == 0 ? "Resume" : "Pause")) {
    if (clockSpeed == 0) {
      clockSpeed = prevClockSpeed;
    } else {
      prevClockSpeed = clockSpeed;
      clockSpeed = 0;
    }
  }

  if (ImGui::Button("Tick")) {
    tickRequested = true;
  }

  ImGui::End();
}

inline void GUI::RenderMemory() {
  memoryEditor.DrawWindow("Memory Editor", std::data(interp->mem), 4096);
}

inline void GUI::RenderKeypadState() {
  ImGui::Begin("Keypad", NULL, ImGuiWindowFlags_AlwaysAutoResize);

  ImGui::TextColored(interp->keypadState[0x1] ? successColor : labelColor, "1");
  ImGui::SameLine();
  ImGui::TextColored(interp->keypadState[0x2] ? successColor : labelColor, "2");
  ImGui::SameLine();
  ImGui::TextColored(interp->keypadState[0x3] ? successColor : labelColor, "3");
  ImGui::SameLine();
  ImGui::TextColored(interp->keypadState[0xC] ? successColor : labelColor, "C");
  ImGui::Separator();

  ImGui::TextColored(interp->keypadState[0x4] ? successColor : labelColor, "4");
  ImGui::SameLine();
  ImGui::TextColored(interp->keypadState[0x5] ? successColor : labelColor, "5");
  ImGui::SameLine();
  ImGui::TextColored(interp->keypadState[0x6] ? successColor : labelColor, "6");
  ImGui::SameLine();
  ImGui::TextColored(interp->keypadState[0xD] ? successColor : labelColor, "D");
  ImGui::Separator();

  ImGui::TextColored(interp->keypadState[0x7] ? successColor : labelColor, "7");
  ImGui::SameLine();
  ImGui::TextColored(interp->keypadState[0x8] ? successColor : labelColor, "8");
  ImGui::SameLine();
  ImGui::TextColored(interp->keypadState[0x9] ? successColor : labelColor, "9");
  ImGui::SameLine();
  ImGui::TextColored(interp->keypadState[0xE] ? successColor : labelColor, "E");
  ImGui::Separator();

  ImGui::TextColored(interp->keypadState[0xA] ? successColor : labelColor, "A");
  ImGui::SameLine();
  ImGui::TextColored(interp->keypadState[0x0] ? successColor : labelColor, "0");
  ImGui::SameLine();
  ImGui::TextColored(interp->keypadState[0xB] ? successColor : labelColor, "B");
  ImGui::SameLine();
  ImGui::TextColored(interp->keypadState[0xF] ? successColor : labelColor, "F");
  ImGui::Separator();

  ImGui::End();
}

void GUI::RenderStack() {
  ImGui::Begin("Stack", NULL, ImGuiWindowFlags_AlwaysAutoResize);

  for (int i = 0; i < 16; i++) {
    ImGui::TextColored(interp->sp == i ? successColor : labelColor, "%X", i);
    ImGui::SameLine();
    ImGui::Text("%04X", interp->stack[i]);
  }

  ImGui::End();
}

void GUI::Render() {
  auto framerate = ImGui::GetIO().Framerate;

  RenderDisplay(framerate);
  RenderGeneral(framerate);
  RenderCPUState();
  RenderDebug();
  RenderMemory();
  RenderKeypadState();
  RenderStack();
}
} // namespace chip8
