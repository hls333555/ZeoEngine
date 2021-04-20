#include "ZEpch.h"

#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#include <backends/imgui_impl_glfw.cpp>
#include <backends/imgui_impl_opengl3.cpp>
// Add extra needed imgui cpp files here
#include <misc/cpp/imgui_stdlib.cpp>

// To use FreeType as the default font rasterizer
// 1. Get latest FreeType binaries or build yourself (under Windows you may use vcpkg with "vcpkg install freetype:x64-windows", "vcpkg integrate install")
// 2. Add #define IMGUI_ENABLE_FREETYPE in MyImGuiConfig.h file
// 3. Uncomment the following include
//#include <misc/freetype/imgui_freetype.cpp>
