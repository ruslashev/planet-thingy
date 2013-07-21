#include "window.hpp"

Window::Window(int newWidth, int newHeight, const char *title)
{
	if (!glfwInit())
		fatal(1, "Failed to initialize GLFW\n");

	glfwSetErrorCallback(ErrorCallback);

	glfwwin = glfwCreateWindow(newWidth, newHeight, title, NULL, NULL);
	if (!glfwwin)
		fatal(1, "Failed to open window\n");

	glfwMakeContextCurrent(glfwwin);
	glfwSetFramebufferSizeCallback(glfwwin, FBsizeChangeCallback);
}

void ErrorCallback(int errorCode, const char *description)
{
	fatal(1, "GLFW error %X: \"%s\"", errorCode, description);
}
void FBsizeChangeCallback(GLFWwindow *window, int width, int height)
{
	glViewport(0, 0, width, height);
}

Window::~Window()
{
	glfwDestroyWindow(glfwwin);
	glfwTerminate();
}
