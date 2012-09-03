compiler_options = -Wall

libraries = -lglfw -lGL -lGLEW
source_files = main.cpp

exec_name = "planet thingies"

default: all

all:
	g++ $(compiler_options) -o $(exec_name) $(source_files) $(libraries)

clean:
	-rm -f $(exec_name)