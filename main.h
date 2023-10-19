// Dear ImGui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

// This example can also compile and run with Emscripten! See 'Makefile.emscripten' for details.
#ifdef __EMSCRIPTEN__
#include "../libs/emscripten/emscripten_mainloop_stub.h"
#endif

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

#include <random>
#include <vector>
#include <iostream>
#include <ctime>

#define INTSTR(int) std::to_string(int)
std::vector<int> method(int N, std::vector<std::vector<int>>& distance, int step, int start_from, bool enable_dumb_insert = false);

// Функции для вычисления точного оптимального значения
void print_optimal(std::vector<std::vector<int>>& distance);
void optimal_internal(std::vector<bool> used, std::vector<int> trace, int overall_distance, std::vector<std::vector<int>>& distance);
bool is_optimal(int input, bool RESET = false);
std::vector<int> get_current_optimal_trace(std::vector<int> trace_input = {});
// ...

// Разное
std::string to_string(std::vector<int> input);
int randomnumber(int included_min, int included_max);
void print_step(int step, std::vector<int> trace, int selected_town_in, int selected_town_out, int overall_distance, int distance);
// ...
