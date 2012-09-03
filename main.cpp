#include "main.h"

using namespace std;

int width = 16, height = 16, depth = 16;
vector<vertex> vertices;
vector<triangle> triangles;
//vector<GLuint> indices;

int main()
{
	srand(time(NULL));
	initGL();
	makePlanet(4);
	
	glm::mat4 model;
	glm::mat4 projection = glm::perspective(60.0f, 800.0f / 600.0f, 1.0f, 100.0f);
	glm::mat4 mvp;
	GLint mvpUniform = glGetUniformLocation(shaderProgram, "mvp");
	
	GLenum drawMode = GL_QUADS;
	int drawModei = 1;
	
	while (glfwGetWindowParam(GLFW_OPENED) && !glfwGetKey(GLFW_KEY_ESC))
	{
		// update
		{
			int x, y;
			glfwGetMousePos(&x, &y);
			if (glfwGetMouseButton(GLFW_MOUSE_BUTTON_LEFT))  { rotx += y - msy; roty += x - msx; }
			if (glfwGetMouseButton(GLFW_MOUSE_BUTTON_RIGHT)) { rotz += x - msx; rotz += y - msy; }
			glfwGetKey('1') ? drawModei = 1 : (glfwGetKey('2') ? drawModei = 2 : (glfwGetKey('3') ? drawModei = 3 : 0));
			switch (drawModei)\
			{
				case 1:
					drawMode = GL_POINTS;
					glfwSetWindowTitle("Planet thingy; GL_POINTS");
					break;
				case 2:
					drawMode = GL_LINES;
					glfwSetWindowTitle("Planet thingy; GL_LINES");
					break;
				case 3:
					drawMode = GL_TRIANGLES;
					glfwSetWindowTitle("Planet thingy; GL_TRIANGLES");
					break;
			}
			
			msx = x; msy = y;
			
			model = glm::rotate(glm::mat4(1.f), rotx, glm::vec3(1, 0, 0));
			model = glm::rotate(model, 			roty, glm::vec3(0, 0, 1));
			model = glm::rotate(model, 			rotz, glm::vec3(0, 1, 0));
			model = glm::translate(model, glm::vec3(0, 0, 0)/*glm::vec3(-width/2.f, -height/2.f, -depth/2.f)*/);
			
			glm::mat4 view = glm::lookAt(glm::vec3(0, 0, 5-glfwGetMouseWheel()), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
			
			mvp = projection * view * model;
			glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, glm::value_ptr(mvp));
		}
		
		glClearColor(0.1f, 0.1f, 0.1f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDrawArrays(drawMode, 0, vertices.size());
		//glDrawElements(drawMode, indices.size(), GL_UNSIGNED_INT, 0);
		glfwSwapBuffers();
	}
	
	cleanup();
	return 0;
}

void makePlanet(unsigned char iterations)
{	
	vertices.reserve(100);
	const glm::vec3 w = glm::vec3(1, 1, 1); // default color
	const glm::vec3 n = glm::vec3(0, 1, 0); // default normal
	triangles.reserve(pow(4, iterations+1));
	
	const float M_1_SQRT2 = 1.f/M_SQRT2;
	triangles.push_back(triangle(vertex(glm::vec3(0,  M_1_SQRT2, 0), w), vertex(glm::vec3(-0.5, 0,  0.5), w), vertex(glm::vec3( 0.5, 0,  0.5), w), n, false));
	triangles.push_back(triangle(vertex(glm::vec3(0,  M_1_SQRT2, 0), w), vertex(glm::vec3( 0.5, 0,  0.5), w), vertex(glm::vec3( 0.5, 0, -0.5), w), n, false));
	triangles.push_back(triangle(vertex(glm::vec3(0,  M_1_SQRT2, 0), w), vertex(glm::vec3( 0.5, 0, -0.5), w), vertex(glm::vec3(-0.5, 0, -0.5), w), n, false));
	triangles.push_back(triangle(vertex(glm::vec3(0,  M_1_SQRT2, 0), w), vertex(glm::vec3(-0.5, 0, -0.5), w), vertex(glm::vec3(-0.5, 0,  0.5), w), n, false));
	triangles.push_back(triangle(vertex(glm::vec3(0, -M_1_SQRT2, 0), w), vertex(glm::vec3(-0.5, 0,  0.5), w), vertex(glm::vec3( 0.5, 0,  0.5), w), n, false));
	triangles.push_back(triangle(vertex(glm::vec3(0, -M_1_SQRT2, 0), w), vertex(glm::vec3( 0.5, 0,  0.5), w), vertex(glm::vec3( 0.5, 0, -0.5), w), n, false));
	triangles.push_back(triangle(vertex(glm::vec3(0, -M_1_SQRT2, 0), w), vertex(glm::vec3( 0.5, 0, -0.5), w), vertex(glm::vec3(-0.5, 0, -0.5), w), n, false));
	triangles.push_back(triangle(vertex(glm::vec3(0, -M_1_SQRT2, 0), w), vertex(glm::vec3(-0.5, 0, -0.5), w), vertex(glm::vec3(-0.5, 0,  0.5), w), n, false));
	
	for (int it = 0; it < iterations; it++)
	{
		for (unsigned int i = 0; i < triangles.size(); i++)
		{
			if (triangles.at(i).touched) { continue; }
			glm::vec3 pos1 = (triangles.at(i).v1.position + triangles.at(i).v2.position) / 2.f;
			glm::vec3 pos2 = (triangles.at(i).v2.position + triangles.at(i).v3.position) / 2.f;
			glm::vec3 pos3 = (triangles.at(i).v3.position + triangles.at(i).v1.position) / 2.f;
			
			pos1 = glm::normalize(pos1);
			pos2 = glm::normalize(pos2);
			pos3 = glm::normalize(pos3);
			
			triangles.at(i).v1.position = glm::normalize(triangles.at(i).v1.position);
			triangles.at(i).v2.position = glm::normalize(triangles.at(i).v2.position);
			triangles.at(i).v3.position = glm::normalize(triangles.at(i).v3.position);
			
			triangles.push_back(triangle(vertex(triangles.at(i).v1.position, w), vertex(pos1, w), vertex(pos3, w), true));
			triangles.push_back(triangle(vertex(pos1, w), vertex(triangles.at(i).v2.position, w), vertex(pos2, w), true));
			triangles.push_back(triangle(vertex(pos3, w), vertex(pos2, w), vertex(triangles.at(i).v3.position, w), true));
			triangles.at(i) = triangle(vertex(pos1, w), vertex(pos2, w), vertex(pos3, w), true);
		}
		
		for (unsigned int i = 0; i < triangles.size(); i++)
		{
			triangles.at(i).touched = false;
		}
	}
	
	for (unsigned int i = 0; i < triangles.size(); i++)
	{
		vertices.push_back(triangles.at(i).v1);
		vertices.push_back(triangles.at(i).v2);
		vertices.push_back(triangles.at(i).v3);
	}	
	
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertex), vertices.data(), GL_STATIC_DRAW);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), 0);
	glVertexAttribPointer(norAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)(1*sizeof(glm::vec3)));
	glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)(2*sizeof(glm::vec3)));
	glEnableVertexAttribArray(posAttrib);
	glEnableVertexAttribArray(norAttrib);
	glEnableVertexAttribArray(colAttrib);
}

glm::vec3 tn(triangle tri)
{
	glm::vec3 a = tri.v1.position;
	glm::vec3 b = tri.v2.position;
	glm::vec3 c = tri.v3.position;
	return glm::normalize(glm::cross(c - a, b - a));
}

void initGL()
{
	if (glfwInit() == GL_FALSE) { cerr << "GLFW didn't initialize\n"; cleanup(); exit(1); }
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 2); glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 1);
	glfwOpenWindowHint(GLFW_WINDOW_NO_RESIZE, GL_TRUE);
	//glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 4);
	glfwOpenWindow(800, 600, 0, 0, 0, 0, 24, 0, GLFW_WINDOW);
	glfwSetWindowTitle("Planet thingy");
	
	if (glewInit() != GLEW_OK) { cerr << "GLEW didn't initialize\n"; cleanup(); exit(1); }
	
	glViewport(0, 0, 800, 600);
	glEnable(GL_DEPTH_TEST);
	//glFrontFace(GL_CW);
	//glEnable(GL_CULL_FACE);
	glPointSize(3.f);
	
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	//glGenBuffers(1, &ibo);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	
	loadShader(GL_VERTEX_SHADER, vertexShader, "shaders/vert.txt");
	loadShader(GL_FRAGMENT_SHADER, fragmentShader, "shaders/frag.txt");
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	glUseProgram(shaderProgram);
	
	posAttrib = glGetAttribLocation(shaderProgram, "position");
	colAttrib = glGetAttribLocation(shaderProgram, "color");
	norAttrib = glGetAttribLocation(shaderProgram, "normal");
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
