#include <GL/glew.h>
#include <cstdarg>
#include <cstdio>
#include <fstream>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/noise.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

struct vertex
{
	glm::vec3 position;
	glm::vec3 color;
	vertex(glm::vec3 n_position, glm::vec3 n_color)
		: position(n_position)
		, color(n_color)
	{
	}
	vertex(glm::vec3 n_position)
		: position(n_position)
		, color(glm::vec3(1, 1, 1))
	{
	}
	vertex(float x, float y, float z)
		: position(glm::vec3(x, y, z))
		, color(glm::vec3(1, 1, 1))
	{
	}
};

struct triangle
{
	vertex v1;
	vertex v2;
	vertex v3;
	bool touched;
	triangle(vertex v1, vertex v2, vertex v3, bool touched)
		: v1(v1)
		, v2(v2)
		, v3(v3)
		, touched(touched)
	{
	};
	triangle(float x1, float y1, float z1, float x2, float y2, float z2, float x3
			, float y3, float z3, bool n_touched = false)
		: v1(x1, y1, z1)
		, v2(x2, y2, z2)
		, v3(x3, y3, z3)
		, touched(n_touched)
	{
	};
};

static GLuint vbo, vao, vertexShader, fragmentShader, shaderProgram;
static GLfloat rotx = 0.0f, roty = 0.0f, rotz = 0.0f;
static GLFWwindow *win;
static std::vector<vertex> vertices;
static std::vector<triangle> triangles;
// std::vector<GLuint> indices;

/* todo:
 * [ ] Fix nipples on 0-coordinates (e.g. (0,1,0), (1,0,0))
 * [ ] Indices
 */

static void cleanup()
{
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
	glDetachShader(shaderProgram, vertexShader);
	glDetachShader(shaderProgram, fragmentShader);
	glDeleteProgram(shaderProgram);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	glfwTerminate();
}

void assertf(bool condition, const char *format, ...)
{
	if (condition)
		return;
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
	printf("\n");
	cleanup();
	exit(1);
}

static void loadShader(GLenum type, GLuint& shader, const char* filename)
{
	char compileLog[513];
	std::ifstream fileStream (filename);
	assertf(fileStream.good(), "Error loading file \"%s\"", filename);
	std::stringstream ss;
	ss << fileStream.rdbuf();
	fileStream.close();

	std::string sourceS = ss.str();
	const char* source = sourceS.c_str();

	shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);

	GLint compileSuccess;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compileSuccess);
	if (compileSuccess == GL_FALSE)
	{
		glGetShaderInfoLog(shader, 512, NULL, compileLog);
		glDeleteShader(shader);
		assertf(0, "Shader \"%s\" failed to compile. Error log:\n%s", filename
				, compileLog);
	}
}

static void initGL()
{
	assertf(glfwInit() != GL_FALSE, "GLFW failed to initialize");
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	//glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 16);
	win = glfwCreateWindow(800, 600, "Planet thingy", nullptr, nullptr);
	assertf(win, "Failed to open window");
	glfwMakeContextCurrent(win);

	GLenum glewInitStatus = glewInit();
	assertf(glewInitStatus == GLEW_OK, "GLEW failed to initialize: %s"
			, glewGetErrorString(glewInitStatus));

	glViewport(0, 0, 800, 600);
	glEnable(GL_DEPTH_TEST);
	//glFrontFace(GL_CW);
	//glEnable(GL_CULL_FACE);
	glPointSize(4.f);
	glClearColor(0.1f, 0.1f, 0.1f, 1.f);

	loadShader(GL_VERTEX_SHADER, vertexShader, "shaders/vert.glsl");
	loadShader(GL_FRAGMENT_SHADER, fragmentShader, "shaders/frag.glsl");
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	GLint posAttrib = glGetAttribLocation(shaderProgram, "vposition")
		, colAttrib = glGetAttribLocation(shaderProgram, "vcolor");

	glEnableVertexAttribArray(posAttrib);
	glEnableVertexAttribArray(colAttrib);
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(vertex)
			, (void*)offsetof(vertex, position));
	glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(vertex)
			, (void*)offsetof(vertex, color));
}

double expOut(double a, double b, double tim)
{
	float t = 1 - tim;
	return a + (b - a) * t * t;
}

void makePlanet(unsigned char iterations)
{
	vertices.reserve(pow(4, iterations+1)*3);
	triangles.reserve(pow(4, iterations+1));

	const float M1S2 = 1.f/M_SQRT2;
	triangles.emplace_back(0,  M1S2, 0,  -0.5, 0,  0.5,   0.5, 0,  0.5);
	triangles.emplace_back(0,  M1S2, 0,   0.5, 0,  0.5,   0.5, 0, -0.5);
	triangles.emplace_back(0,  M1S2, 0,   0.5, 0, -0.5,  -0.5, 0, -0.5);
	triangles.emplace_back(0,  M1S2, 0,  -0.5, 0, -0.5,  -0.5, 0,  0.5);
	triangles.emplace_back(0, -M1S2, 0,  -0.5, 0,  0.5,   0.5, 0,  0.5);
	triangles.emplace_back(0, -M1S2, 0,   0.5, 0,  0.5,   0.5, 0, -0.5);
	triangles.emplace_back(0, -M1S2, 0,   0.5, 0, -0.5,  -0.5, 0, -0.5);
	triangles.emplace_back(0, -M1S2, 0,  -0.5, 0, -0.5,  -0.5, 0,  0.5);

	for (int it = 0; it < iterations; it++)
	{
		for (unsigned int i = 0; i < triangles.size(); i++)
		{
			if (triangles[i].touched)
				continue;

			/* rough presentation of what's going on:
			 *
			 *                 v1
			 *               O
			 *              / \
			 *             /   \
			 *       pos3 o     o pos1       <---  triangles[i]
			 *           /       \
			 *          /         \
			 *         O-----o-----O
			 *     v3       pos2      v2
			 *
			 */

			// midpoints between vertices v1 v2 and v3
			glm::vec3 pos1 = (triangles[i].v1.position + triangles[i].v2.position)
				/ 2.f;
			glm::vec3 pos2 = (triangles[i].v2.position + triangles[i].v3.position)
				/ 2.f;
			glm::vec3 pos3 = (triangles[i].v3.position + triangles[i].v1.position)
				/ 2.f;

			// Pushing outwards (making them all the have same distance from
			// center (0,0,0)) vertices that are already here
			triangles[i].v1.position = glm::normalize(triangles[i].v1.position);
			triangles[i].v2.position = glm::normalize(triangles[i].v2.position);
			triangles[i].v3.position = glm::normalize(triangles[i].v3.position);

			// same thing for new vertices
			pos1 = glm::normalize(pos1);
			pos2 = glm::normalize(pos2);
			pos3 = glm::normalize(pos3);

			triangles.emplace_back(triangles[i].v1.position, pos1, pos3, true);
			triangles.emplace_back(pos1, triangles[i].v2.position, pos2, true);
			triangles.emplace_back(pos3, pos2, triangles[i].v3.position, true);
			// marking current big triangle as touched
			triangles[i] = triangle(pos1, pos2, pos3, true);

			/* and in the end we have:
			 *
			 *     v1
			 *        O---___
			 *        \       -
			 *         \     _--o pos1
			 *          | _--   \-
			 *     pos3  o_     | -
			 *          |  \__  /  -
			 *          |     \/   -
			 *         /  __--o----O  v2
			 *      v3 O--
			 *                 pos2
			 */
		}

		for (unsigned int i = 0; i < triangles.size(); i++)
			triangles[i].touched = false;
	}

	for (unsigned int i = 0; i < triangles.size(); i++)
	{
		vertices.push_back(triangles[i].v1);
		vertices.push_back(triangles[i].v2);
		vertices.push_back(triangles[i].v3);
	}

	for (unsigned int i = 0; i < vertices.size(); i++)
		vertices[i].position *= expOut(0.95, 1.
				, -glm::simplex(vertices[i].position*5.f));

	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertex)
			, vertices.data(), GL_STATIC_DRAW);
}

int main()
{
	srand(time(NULL));
	initGL();
	makePlanet(5);

	glm::mat4 projection = glm::perspective(glm::radians(60.f), 800.f / 600.f
			, 1.f, 100.f);
	GLint mvpUniform = glGetUniformLocation(shaderProgram, "mvp");

	GLenum drawMode = GL_TRIANGLES;

	double oldMouseX = 0, oldMouseY = 0;

	while (!glfwWindowShouldClose(win))
	{
		// update
		{
			double x, y;
			glfwGetCursorPos(win, &x, &y);
			if (glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_LEFT))
			{
				rotx += y - oldMouseY;
				roty += x - oldMouseX;
			}
			if (glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_RIGHT))
			{
				rotz += x - oldMouseX;
				rotz += y - oldMouseY;
			}
			if (glfwGetKey(win, '1'))
				drawMode = GL_POINTS;
			if (glfwGetKey(win, '2'))
				drawMode = GL_LINES;
			if (glfwGetKey(win, '3'))
				drawMode = GL_TRIANGLES;

			oldMouseX = x;
			oldMouseY = y;

			glm::mat4 model = glm::rotate(glm::mat4(1), glm::radians(rotx)
					, glm::vec3(1, 0, 0));
			model = glm::rotate(model, glm::radians(roty), glm::vec3(0, 0, 1));
			model = glm::rotate(model, glm::radians(rotz), glm::vec3(0, 1, 0));

			glm::mat4 view = glm::lookAt(glm::vec3(0, 0, 5.5), glm::vec3(0)
					, glm::vec3(0, 1, 0));

			glUniformMatrix4fv(mvpUniform, 1, GL_FALSE
					, glm::value_ptr(projection * view * model));
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(shaderProgram);
		glBindVertexArray(vao);
		glDrawArrays(drawMode, 0, vertices.size());

		glfwSwapBuffers(win);
		glfwPollEvents();
	}

	cleanup();
	return 0;
}

