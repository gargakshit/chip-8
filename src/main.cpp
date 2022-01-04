#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_internal.h>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif

#include <iostream>
#include <stdlib.h>
#include <time.h>

#include "chip8.hpp"
#include "font.h"
#include "gui.hpp"

// Ummmm
#if defined(_MSC_VER) && (_MSC_VER >= 1900) &&                                 \
    !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

static void glfw_error_callback(int error, const char *description) {
  std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

int main(int args, char **argv) {
  if (args < 2) {
    std::cerr << "Usage:" << std::endl
              << argv[0] << " path/to/chip8/program" << std::endl;
    return 1;
  }

  srand((unsigned)time(NULL));

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

  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

  GLubyte displayPixels[64 * 32 * 3];

  for (int x = 0; x < 2048; x++) {
    displayPixels[x * 3] = 255;
    displayPixels[x * 3 + 1] = 255;
    displayPixels[x * 3 + 2] = 255;
  }

  GLuint displayTexture;
  glGenTextures(1, &displayTexture);
  glBindTexture(GL_TEXTURE_2D, displayTexture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 64, 32, 0, GL_RGB, GL_UNSIGNED_BYTE,
               displayPixels);

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();

  auto &style = ImGui::GetStyle();
  style.FrameRounding = 2;
  // style.FramePadding = ImVec2(2, 1);
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

  if (!interp.LoadProgram(argv[1])) {
    std::cerr << "Unable to load " << argv[1] << std::endl;
    return -1;
  }

  chip8::GUI gui(&interp, displayTexture, displayPixels);

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    gui.Render();

    // Rendering
    ImGui::Render();

    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w,
                 clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Update and Render additional Platform Windows
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
      GLFWwindow *backup_current_context = glfwGetCurrentContext();
      ImGui::UpdatePlatformWindows();
      ImGui::RenderPlatformWindowsDefault();
      glfwMakeContextCurrent(backup_current_context);
    }

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
