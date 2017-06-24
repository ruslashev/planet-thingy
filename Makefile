# for compiling on Windows make them something like -lglfw -lopengl32 -lglew32s
libraries = -lglfw -lGL -lGLEW

default: all

all:
	g++ -Wall -o planet-thingy main.cpp $(libraries)
	./planet-thingy

