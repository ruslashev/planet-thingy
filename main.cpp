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

typedef unsigned int element;

struct triangle
{
	element v1, v2, v3;
	triangle(element n_v1, element n_v2, element n_v3)
		: v1(n_v1)
		, v2(n_v2)
		, v3(n_v3)
	{
	};
};

struct model
{
	std::vector<glm::vec3> vertices;
	std::vector<triangle> triangles;
};

static GLuint vbo, ebo, vao, vertexShader, fragmentShader, shaderProgram;
static GLFWwindow *win;

static void cleanup()
{
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);
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

	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

	GLint posAttrib = glGetAttribLocation(shaderProgram, "vposition");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);
}

double expOut(double a, double b, double tim)
{
	float t = 1 - tim;
	return a + (b - a) * t * t;
}

void makePlanet(std::vector<glm::vec3> *vertices
		, std::vector<element> *elements, int iterations)
{
	// vertices->reserve(pow(4, iterations + 1) * 3);
	// triangles.reserve(pow(4, iterations + 1));

	model m;

	const float M1S2 = 1.f/M_SQRT2;
	m.vertices.emplace_back(   0,  M1S2,    0);
	m.vertices.emplace_back(   0, -M1S2,    0);
	m.vertices.emplace_back( 0.5,     0,  0.5);
	m.vertices.emplace_back( 0.5,     0, -0.5);
	m.vertices.emplace_back(-0.5,     0,  0.5);
	m.vertices.emplace_back(-0.5,     0, -0.5);

	m.triangles.emplace_back(0, 4, 2);
	m.triangles.emplace_back(0, 2, 3);
	m.triangles.emplace_back(0, 3, 5);
	m.triangles.emplace_back(0, 5, 4);
	m.triangles.emplace_back(1, 4, 2);
	m.triangles.emplace_back(1, 2, 3);
	m.triangles.emplace_back(1, 3, 5);
	m.triangles.emplace_back(1, 5, 4);

	for (int it = 0; it < iterations; ++it)
	{
		model new_model;
		new_model.vertices = std::move(m.vertices);
		for (size_t i = 0; i < m.triangles.size(); ++i)
		{
			/* rough presentation of what's going on:
			 *       v1
			 *       O
			 *      / \
			 *  n3 o   o n1 <-- triangles[i]
			 *    /     \
			 *   O---o---O
			 * v3   n2    v2
			 */

			// midpoints n1,... between existing vertices v1,...
			const glm::vec3 v1 = new_model.vertices[m.triangles[i].v1]
				, v2 = new_model.vertices[m.triangles[i].v2]
				, v3 = new_model.vertices[m.triangles[i].v3]
				, n1 = (v1 + v2) / 2.f, n2 = (v2 + v3) / 2.f, n3 = (v3 + v1) / 2.f;

			// new midpoints are pushed as new vertices
			new_model.vertices.push_back(n1);
			new_model.vertices.push_back(n2);
			new_model.vertices.push_back(n3);

			// and their indices are remembered aswell as old ones
			const element en1 = new_model.vertices.size() - 3
				, en2 = new_model.vertices.size() - 2
				, en3 = new_model.vertices.size() - 1
				, ev1 = m.triangles[i].v1
				, ev2 = m.triangles[i].v2
				, ev3 = m.triangles[i].v3;

			// to construct new triangles
			new_model.triangles.emplace_back(ev1, en1, en3);
			new_model.triangles.emplace_back(en1, ev2, en2);
			new_model.triangles.emplace_back(en3, en2, ev3);
			new_model.triangles.emplace_back(en1, en2, en3);
		}

		m = std::move(new_model);
	}

	for (const glm::vec3 &v : m.vertices)
		vertices->push_back(glm::normalize(v));

	for (const triangle &t : m.triangles)
	{
		elements->push_back(t.v1);
		elements->push_back(t.v2);
		elements->push_back(t.v3);
	}

	for (size_t i = 0; i < vertices->size(); ++i)
		(*vertices)[i] *= expOut(0.95, 1., -glm::simplex((*vertices)[i] * 5.f));
}

int main()
{
	srand(time(NULL));
	initGL();
	std::vector<glm::vec3> vertices;
	std::vector<element> elements;
	makePlanet(&vertices, &elements, 5);

	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3)
			, vertices.data(), GL_STATIC_DRAW);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, elements.size() * sizeof(element)
			, elements.data(), GL_STATIC_DRAW);

	const glm::mat4 projection = glm::perspective(glm::radians(60.f)
			, 800.f / 600.f, 0.1f, 100.f);
	const GLint mvpUniform = glGetUniformLocation(shaderProgram, "mvp");

	GLenum drawMode = GL_TRIANGLES;

	const float rotationSensitivity = 0.9, zoomSensitivity = 0.1;
	float oldMouseX = 0, oldMouseY = 0, rotx = 0, roty = 0, rotz = 0, zoom = 0;

	while (!glfwWindowShouldClose(win))
	{
		// update
		{
			double x, y;
			glfwGetCursorPos(win, &x, &y);
			if (glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_LEFT))
			{
				rotx += (y - oldMouseY) * rotationSensitivity;
				roty += (x - oldMouseX) * rotationSensitivity;
			}
			if (glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_RIGHT))
				rotz += (x - oldMouseX) * rotationSensitivity;
			if (glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_MIDDLE))
				zoom += (y - oldMouseY) * zoomSensitivity;
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

			glm::mat4 view = glm::lookAt(glm::vec3(0, 0, 5.5f + zoom), glm::vec3(0)
					, glm::vec3(0, 1, 0));

			glUniformMatrix4fv(mvpUniform, 1, GL_FALSE
					, glm::value_ptr(projection * view * model));
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(shaderProgram);
		glDrawElements(drawMode, elements.size(), GL_UNSIGNED_INT, 0);

		glfwSwapBuffers(win);
		glfwPollEvents();
	}

	cleanup();
	return 0;
}

