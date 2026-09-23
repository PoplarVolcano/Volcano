#include "volpch.h"

#define IMGUI_IMPL_OPENGL_LOADER_GLAD //opengl的头文件需要的定义，说明使用的是gald
#include "backends/imgui_impl_opengl3.cpp"  // 注意这里是cpp文件！！！
#include "backends/imgui_impl_glfw.cpp"