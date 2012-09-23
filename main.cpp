#include "main.hpp"

using namespace std;

vector<vertex> vertices;
vector<triangle> triangles;
//vector<GLuint> indices;

/* todo:
 * [ ] Fix nipples on 0-coordinates (e.g. (0,1,0), (1,0,0))
 * [ ] Indices
 */

int main()
{
	srand(time(NULL));
	initGL();
	makePlanet(7);
	
	glm::mat4 model;
	glm::mat4 projection = glm::perspective(60.f, 800.f / 600.f, 1.f, 100.f);
	GLint mvpUniform = glGetUniformLocation(shaderProgram, "mvp");
	
	GLenum drawMode = GL_TRIANGLES;
	
	// tl;dr: Exit if escape or ctrl+c or ctrl+w are pressed
	while (glfwGetWindowParam(GLFW_OPENED) && !glfwGetKey(GLFW_KEY_ESC) && !((glfwGetKey(GLFW_KEY_LCTRL) || glfwGetKey(GLFW_KEY_RCTRL)) && (glfwGetKey('C') || glfwGetKey('W'))))
	{
		// update
		{
			int x, y;
			glfwGetMousePos(&x, &y);
			if (glfwGetMouseButton(GLFW_MOUSE_BUTTON_LEFT))  { rotx += y - msy; roty += x - msx; }
			if (glfwGetMouseButton(GLFW_MOUSE_BUTTON_RIGHT)) { rotz += x - msx; rotz += y - msy; }
			glfwGetKey('1') ? drawMode = GL_POINTS : (glfwGetKey('2') ? drawMode = GL_LINES : (glfwGetKey('3') ? drawMode = GL_TRIANGLES : 0));
			
			msx = x; msy = y;
			
			model = glm::rotate(glm::mat4(1), rotx, glm::vec3(1, 0, 0));
			model = glm::rotate(model,        roty, glm::vec3(0, 0, 1));
			model = glm::rotate(model,        rotz, glm::vec3(0, 1, 0));
			//model = glm::translate(model, glm::vec3(0));
			
			glm::mat4 view = glm::lookAt(glm::vec3(0, 0, 5-glfwGetMouseWheel()), glm::vec3(0), glm::vec3(0, 1, 0));
			
			glm::mat4 mvp = projection * view * model;
			glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, glm::value_ptr(mvp));
		}
		
		glClearColor(0.1f, 0.1f, 0.1f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(shaderProgram);
		glBindVertexArray(vao);
		glDrawArrays(drawMode, 0, vertices.size());
		
		glfwSwapBuffers();
	}
	
	cleanup();
	return 0;
}

double cerp(double a, double b, double t)
{
	double t2;
	t2 = (1 - cos(t*M_PI)) / 2;
	return(a*(1-t2) + b*t2);
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
			if (triangles.at(i).touched) { continue; }
			
			/* rough presentation of what's going on:
			 * 
			 *                 v1
			 *               O
			 *              / \
			 *             /   \
			 *       pos3 o     o pos1       <---  triangles.at(i)
			 *           /       \
			 *          /         \
			 *         O-----o-----O
			 *     v3       pos2      v2
			 * 
			 */
						
			glm::vec3 pos1 = (triangles.at(i).v1.position + triangles.at(i).v2.position) / 2.f; // midpoints between vertices v1 v2 and v3
			glm::vec3 pos2 = (triangles.at(i).v2.position + triangles.at(i).v3.position) / 2.f;
			glm::vec3 pos3 = (triangles.at(i).v3.position + triangles.at(i).v1.position) / 2.f;
			
			triangles.at(i).v1.position = glm::normalize(triangles.at(i).v1.position); // Pushing outwards (making them all the have same distance from center (0,0,0)) vertices that are already here
			triangles.at(i).v2.position = glm::normalize(triangles.at(i).v2.position);
			triangles.at(i).v3.position = glm::normalize(triangles.at(i).v3.position);
			
			pos1 = glm::normalize(pos1); // same thing for new vertices
			pos2 = glm::normalize(pos2);
			pos3 = glm::normalize(pos3);
			
			triangles.push_back(triangle(vertex(triangles.at(i).v1.position, n, w), vertex(pos1,                        n, w), vertex(pos3,                        n, w), true));
			triangles.push_back(triangle(vertex(pos1,                        n, w), vertex(triangles.at(i).v2.position, n, w), vertex(pos2,                        n, w), true));
			triangles.push_back(triangle(vertex(pos3,                        n, w), vertex(pos2,                        n, w), vertex(triangles.at(i).v3.position, n, w), true));
			triangles.at(i) =   triangle(vertex(pos1,                        n, w), vertex(pos2,                        n, w), vertex(pos3,                        n, w), true); // marking current big triangle as touched
			
			/* and in the end we have:
			 * 
			 *     v1
			 *        O---__
			 *        \      -
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
		{ triangles.at(i).touched = false; }
	}
	
	for (unsigned int i = 0; i < triangles.size(); i++)
	{
		vertices.push_back(triangles.at(i).v1);
		vertices.push_back(triangles.at(i).v2);
		vertices.push_back(triangles.at(i).v3);
	}
	
	for (unsigned int i = 0; i < vertices.size(); i++)
	{
		vertices.at(i).position *= cerp(0.9, 1.0, -glm::simplex(vertices.at(i).position*1.f));
		//cout << glm::length(vertices.at(i).position) << endl;
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
	if (glfwInit() == GL_FALSE) { cerr << "GLFW failed to initialize\n"; cleanup(); exit(1); }
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 2); glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 1); glfwOpenWindowHint(GLFW_WINDOW_NO_RESIZE, GL_TRUE);
	//glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 4);
	if (glfwOpenWindow(800, 600, 0, 0, 0, 0, 24, 8, GLFW_WINDOW) == GL_FALSE) { cerr << "Failed to open window\n"; cleanup(); exit(1); }
	glfwSetWindowTitle("Planet thingy");
	
	GLenum glewInitStatus = glewInit();
	if (glewInitStatus != GLEW_OK) { cerr << "GLEW failed to initialize: " << glewGetErrorString(glewInitStatus) << endl; cleanup(); exit(1); }
	
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
	ifstream fileStream (filename);
	if (!fileStream)
	{ cerr << "Error loading file \"" << filename << "\"\n"; cleanup(); exit(1); }
	stringstream ss;
	ss << fileStream.rdbuf();
	fileStream.close();

	string sourceS = ss.str();
	const char* source = sourceS.c_str();
		
	shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);
	
	GLint compileSuccess;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compileSuccess);
	if (compileSuccess == GL_FALSE) { glGetShaderInfoLog(shader, 512, NULL, compileLog); cerr << "Shader \"" << filename << "\" failed to compile. Error log:\n" << compileLog; glDeleteShader(shader); cleanup(); exit(1); }
}
