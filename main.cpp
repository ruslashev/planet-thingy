#include <GL/glew.h>
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

	glfwTerminate();
}

static GLFWwindow *win;
static std::vector<vertex> vertices;
static std::vector<triangle> triangles;
//vector<GLuint> indices;

/* todo:
 * [ ] Fix nipples on 0-coordinates (e.g. (0,1,0), (1,0,0))
 * [ ] Indices
 */

int main()
{
	srand(time(NULL));
	initGL();
	makePlanet(5);

	glm::mat4 model;
	glm::mat4 projection = glm::perspective(60.f, 800.f / 600.f, 1.f, 100.f);
	GLint mvpUniform = glGetUniformLocation(shaderProgram, "mvp");

	GLenum drawMode = GL_TRIANGLES;

	while (!glfwWindowShouldClose(win))
	{
		// update
		{
			double x, y;
			glfwGetCursorPos(win, &x, &y);
			if (glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_LEFT))  { rotx += y - msy; roty += x - msx; }
			if (glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_RIGHT)) { rotz += x - msx; rotz += y - msy; }
			glfwGetKey(win, '1') ? drawMode = GL_POINTS : (glfwGetKey(win, '2') ? drawMode = GL_LINES : (glfwGetKey(win, '3') ? drawMode = GL_TRIANGLES : 0));

			msx = x; msy = y;

			model = glm::rotate(glm::mat4(1), rotx, glm::vec3(1, 0, 0));
			model = glm::rotate(model,        roty, glm::vec3(0, 0, 1));
			model = glm::rotate(model,        rotz, glm::vec3(0, 1, 0));
			//model = glm::translate(model, glm::vec3(0));

			glm::mat4 view = glm::lookAt(glm::vec3(0, 0, 1), glm::vec3(0), glm::vec3(0, 1, 0));

			glm::mat4 mvp = projection * view * model;
			glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, glm::value_ptr(mvp));
		}

		glClearColor(0.1f, 0.1f, 0.1f, 1.f);
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

double expOut(double a, double b, double tim)
{
	float t = 1 - tim;
	return a + (b - a) * t * t;
}

void makePlanet(unsigned char iterations)
{
	vertices.reserve(pow(4, iterations+1)*3);
	const glm::vec3 w = glm::vec3(1, 1, 1); // default color
	const glm::vec3 n = glm::vec3(0, 1, 0); // default normal
	triangles.reserve(pow(4, iterations+1));

	const float M_1_SQRT2 = 1.f/M_SQRT2;
	triangles.push_back(triangle(vertex(glm::vec3(0,  M_1_SQRT2, 0), n, w), vertex(glm::vec3(-0.5, 0,  0.5), n, w), vertex(glm::vec3( 0.5, 0,  0.5), n, w), false));
	triangles.push_back(triangle(vertex(glm::vec3(0,  M_1_SQRT2, 0), n, w), vertex(glm::vec3( 0.5, 0,  0.5), n, w), vertex(glm::vec3( 0.5, 0, -0.5), n, w), false));
	triangles.push_back(triangle(vertex(glm::vec3(0,  M_1_SQRT2, 0), n, w), vertex(glm::vec3( 0.5, 0, -0.5), n, w), vertex(glm::vec3(-0.5, 0, -0.5), n, w), false));
	triangles.push_back(triangle(vertex(glm::vec3(0,  M_1_SQRT2, 0), n, w), vertex(glm::vec3(-0.5, 0, -0.5), n, w), vertex(glm::vec3(-0.5, 0,  0.5), n, w), false));
	triangles.push_back(triangle(vertex(glm::vec3(0, -M_1_SQRT2, 0), n, w), vertex(glm::vec3(-0.5, 0,  0.5), n, w), vertex(glm::vec3( 0.5, 0,  0.5), n, w), false));
	triangles.push_back(triangle(vertex(glm::vec3(0, -M_1_SQRT2, 0), n, w), vertex(glm::vec3( 0.5, 0,  0.5), n, w), vertex(glm::vec3( 0.5, 0, -0.5), n, w), false));
	triangles.push_back(triangle(vertex(glm::vec3(0, -M_1_SQRT2, 0), n, w), vertex(glm::vec3( 0.5, 0, -0.5), n, w), vertex(glm::vec3(-0.5, 0, -0.5), n, w), false));
	triangles.push_back(triangle(vertex(glm::vec3(0, -M_1_SQRT2, 0), n, w), vertex(glm::vec3(-0.5, 0, -0.5), n, w), vertex(glm::vec3(-0.5, 0,  0.5), n, w), false));

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
			glm::vec3 pos1 = (triangles[i].v1.position + triangles[i].v2.position) / 2.f;
			glm::vec3 pos2 = (triangles[i].v2.position + triangles[i].v3.position) / 2.f;
			glm::vec3 pos3 = (triangles[i].v3.position + triangles[i].v1.position) / 2.f;

			// Pushing outwards (making them all the have same distance from center (0,0,0)) vertices that are already here
			triangles[i].v1.position = glm::normalize(triangles[i].v1.position);
			triangles[i].v2.position = glm::normalize(triangles[i].v2.position);
			triangles[i].v3.position = glm::normalize(triangles[i].v3.position);

			// same thing for new vertices
			pos1 = glm::normalize(pos1);
			pos2 = glm::normalize(pos2);
			pos3 = glm::normalize(pos3);

			triangles.push_back(triangle(vertex(triangles[i].v1.position, n, w), vertex(pos1,                     n, w), vertex(pos3,                     n, w), true));
			triangles.push_back(triangle(vertex(pos1,                     n, w), vertex(triangles[i].v2.position, n, w), vertex(pos2,                     n, w), true));
			triangles.push_back(triangle(vertex(pos3,                     n, w), vertex(pos2,                     n, w), vertex(triangles[i].v3.position, n, w), true));
			// marking current big triangle as touched
			triangles[i] = triangle(vertex(pos1, n, w), vertex(pos2, n, w), vertex(pos3, n, w), true);

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
	{
		vertices[i].position *= expOut(0.95, 1., -glm::simplex(vertices[i].position*5.f));
	}

	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertex), vertices.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), 0);
	glEnableVertexAttribArray(norAttrib);
	glVertexAttribPointer(norAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)(1*sizeof(glm::vec3)));
	glEnableVertexAttribArray(colAttrib);
	glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)(2*sizeof(glm::vec3)));
}

void initGL()
{
	if (glfwInit() == GL_FALSE) { std::cerr << "GLFW failed to initialize\n"; cleanup(); exit(1); }
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2); glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	//glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 16);
	win = glfwCreateWindow(800, 600, "Planet thingy", nullptr, nullptr);
	if (!win)
	{
		std::cerr << "Failed to open window\n"; cleanup(); exit(1);
	}
	glfwMakeContextCurrent(win);

	GLenum glewInitStatus = glewInit();
	if (glewInitStatus != GLEW_OK) { std::cerr << "GLEW failed to initialize: " << glewGetErrorString(glewInitStatus) << std::endl; cleanup(); exit(1); }

	glViewport(0, 0, 800, 600);
	glEnable(GL_DEPTH_TEST);
	//glFrontFace(GL_CW);
	//glEnable(GL_CULL_FACE);
	glPointSize(4.f);

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

	posAttrib = glGetAttribLocation(shaderProgram, "vposition");
	colAttrib = glGetAttribLocation(shaderProgram, "vcolor");
	norAttrib = glGetAttribLocation(shaderProgram, "vnormal");
}

void loadShader(GLenum type, GLuint& shader, const char* filename)
{
	char compileLog[513];
	std::ifstream fileStream (filename);
	if (!fileStream)
	{ std::cerr << "Error loading file \"" << filename << "\"\n"; cleanup(); exit(1); }
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
	if (compileSuccess == GL_FALSE) { glGetShaderInfoLog(shader, 512, NULL, compileLog); std::cerr << "Shader \"" << filename << "\" failed to compile. Error log:\n" << compileLog; glDeleteShader(shader); cleanup(); exit(1); }
}

