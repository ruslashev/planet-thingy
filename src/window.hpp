#ifndef WINDOW_HPP
#define WINDOW_HPP
#include <GLFW/glfw3.h>
#include "utils.hpp"

void ErrorCallback(int errorCode, const char *description);
void FBsizeChangeCallback(GLFWwindow *window, int width, int height);

class Window
{
public:
	GLFWwindow *glfwwin;
	int width, height;

	Window(int newWidth, int newHeight, const char *title);
	~Window();
};

#endif
