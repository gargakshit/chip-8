#include <GLFW/glfw3.h>
#include <gl/gl.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_internal.h>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif

#include <iostream>

#include "chip8.hpp"
#include "font.h"

#define DISPLAY_SCALE 12

// Ummmm
#if defined(_MSC_VER) && (_MSC_VER >= 1900) &&                                 \
    !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

static void glfw_error_callback(int error, const char *description) {
  std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

// This assumes the pixels were already allocated to match the scale
void RenderDisplay(GLubyte *pixels, GLuint texture, chip8::Chip8 *interp) {
  ImGui::Begin("Display", NULL,
               ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
                   ImGuiWindowFlags_NoNavFocus);

  ImGui::SetWindowSize(
      ImVec2(32 + (64 * DISPLAY_SCALE), 48 + (32 * DISPLAY_SCALE)));

  if (interp->redraw) {
    for (int y = 0; y < 32; y++) {
      for (int x = 0; x < 64; x++) {
        int subpixel = interp->display[(y * 64) + x] ? 255 : 0;

        for (int xScale = 0; xScale < DISPLAY_SCALE; xScale++) {
          for (int yScale = 0; yScale < DISPLAY_SCALE; yScale++) {
            int i = ((((y * DISPLAY_SCALE * 64) * DISPLAY_SCALE) +
                      (x * DISPLAY_SCALE) + xScale) +
                     (yScale * DISPLAY_SCALE * 64)) *
                    3;

            pixels[i] = subpixel;
            pixels[i + 1] = subpixel;
            pixels[i + 2] = subpixel;
          }
        }
      }
    }
  }

  // Render it onto the opengl texture
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 64 * DISPLAY_SCALE, 32 * DISPLAY_SCALE,
               0, GL_RGB, GL_UNSIGNED_BYTE, pixels);

  ImGui::Image((void *)(intptr_t)texture,
               ImVec2(64 * DISPLAY_SCALE, 32 * DISPLAY_SCALE));
  ImGui::End();
}

int main(int, char **) {
  // Setup window
  glfwSetErrorCallback(glfw_error_callback);
  if (!glfwInit()) {
    return 1;
  }

  // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
  const char *glsl_version = "#version 100";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
  const char *glsl_version = "#version 150";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);           // Required on Mac
#else
  const char *glsl_version = "#version 130";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif

  // Create window with graphics context
  GLFWwindow *window = glfwCreateWindow(1280, 720, "Chip8", NULL, NULL);
  if (window == NULL) {
    return 1;
  }

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1); // Enable vsync

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;

  // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  GLubyte displayPixels[64 * 32 * DISPLAY_SCALE * DISPLAY_SCALE * 3];
  int pixelIndex = 0;

  for (int x = 0; x < 2048 * DISPLAY_SCALE * DISPLAY_SCALE; x++) {
    displayPixels[pixelIndex++] = 255;
    displayPixels[pixelIndex++] = 0;
    displayPixels[pixelIndex++] = 0;
  }

  GLuint displayTexture;
  glGenTextures(1, &displayTexture);
  glBindTexture(GL_TEXTURE_2D, displayTexture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 64 * DISPLAY_SCALE, 32 * DISPLAY_SCALE,
               0, GL_RGB, GL_UNSIGNED_BYTE, displayPixels);

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();

  auto &style = ImGui::GetStyle();
  style.FrameRounding = 2;
  style.FramePadding = ImVec2(12, 1);
  style.WindowRounding = 4;
  style.WindowPadding = ImVec2(16, 12);
  style.Colors[ImGuiCol_WindowBg] =
      ImVec4(0.094f, 0.094f, 0.101f, 1); // ~#18181A

  ImFont *font = io.Fonts->AddFontFromMemoryCompressedTTF(
      jetbrains_mono_compressed_data, jetbrains_mono_compressed_size, 18);
  io.Fonts->Build();
  ImGui::SetCurrentFont(font);

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);

  // Our state
  auto clear_color = ImVec4(0.024f, 0.024f, 0.03f, 1.00f);

  chip8::Chip8 interp;
  interp.Reset();

  if (!interp.LoadProgram("./programs/IBM_Logo.ch8")) {
    std::cerr << "Unable to load" << std::endl;
    return -1;
  }

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    interp.Tick();
    RenderDisplay(displayPixels, displayTexture, &interp);

    // Rendering
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w,
                 clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
  }

  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
