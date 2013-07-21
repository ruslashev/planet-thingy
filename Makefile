CXX = g++
OBJS = $(patsubst src/%.cpp, objs/%.o, $(shell find src -type f -name "*.cpp" ))
EXECNAME = planet
LIBS = -lglfw

all: $(EXECNAME)
	./$(EXECNAME)

objs/%.o: src/%.cpp
	$(CXX) -c -o $@ $< -Wall -g3 -Og -std=c++0x

$(EXECNAME): $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS) $(LIBS)

clean:
	-rm -f objs/*.o $(EXECNAME)

