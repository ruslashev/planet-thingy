#include <GL/glew.h>
#include <GL/glfw.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/noise.hpp>
#include <cstdio>
#include <vector>
#include <ctime>
#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <math.h>

struct vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 color;
	vertex(glm::vec3 position, glm::vec3 normal, glm::vec3 color) : position(position), normal(normal), color(color) {};
};

struct triangle
{
	vertex v1;
	vertex v2;
	vertex v3;
	bool touched;
	triangle(vertex v1, vertex v2, vertex v3, bool touched) : v1(v1), v2(v2), v3(v3), touched(touched) {};
};

GLuint vbo, vao, ibo, vertexShader, fragmentShader, shaderProgram;
GLuint posAttrib, colAttrib, norAttrib;
GLfloat rotx = 0.0f, roty = 0.0f, rotz = 0.0f;
int msx = 0, msy = 0;

void initGL();
void makePlanet(unsigned char iterations);

void loadShader(GLenum type, GLuint& shader, const char* filename);

void cleanup()
{
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
	glDetachShader(shaderProgram, vertexShader);
	glDetachShader(shaderProgram, fragmentShader);
	glDeleteProgram(shaderProgram);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	
	glfwCloseWindow();
	glfwTerminate();
}
