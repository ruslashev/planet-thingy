#include "window.hpp"

Window::Window(int newWidth, int newHeight, const char *title)
{
	if (!glfwInit())
		fatal(1, "Failed to initialize GLFW\n");

	glfwSetErrorCallback(CallbackError);
	glfwSetFramebufferSizeCallback(win, CallbackFBsizeChange);

	win = glfwCreateWindow(newWidth, newHeight, title, NULL, NULL);
	if (!win)
		fatal(1, "Failed to open window\n");

	glfwMakeContextCurrent(win);
	CallbackFBsizeChange(win, newWidth, newHeight);
}

void CallbackError(int errorCode, const char *description)
{
	fatal(1, "GLFW error %X: \"%s\"\n", errorCode, description);
}
void CallbackFBsizeChange(GLFWwindow *window, int width, int height)
{
	printf("Changed viewport to %dx%d\n", width, height);
	glViewport(0, 0, width, height);
}

Window::~Window()
{
	glfwDestroyWindow(win);
	glfwTerminate();
}