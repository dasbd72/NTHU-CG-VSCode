ARCH=$(shell uname -p)
CC = gcc
CXX = g++

INCLUDES = -I$(glfw_inc) -I$(glad_inc) -I$(stb_inc) -Iinclude
LIBRARIES = -L$(glfw_lib) 

# Replace with your own glfw path
glfw = modules/glfw-3.4.bin.MACOS
glfw_inc = $(glfw)/include
ifeq ($(ARCH), arm)
	glfw_lib = $(glfw)/lib-arm64
else ifeq ($(ARCH), x86_64)
	glfw_lib = $(glfw)/lib-x86_64
else
	glfw_lib = $(glfw)/lib-universal
endif

# Replace with your own glad path
glad = modules/glad
glad_inc = $(glad)/include

# Replace with your own stb path
stb = modules/stb
stb_inc = $(stb)/include

CFLAGS = -Wall -O3 $(INCLUDES)
CXXFLAGS = -Wall -O3 -std=c++17 $(INCLUDES)
LDFLAGS = $(LIBRARIES) -framework OpenGL -rpath @executable_path/$(glfw_lib)

TARGET = main
cpp_files = $(wildcard src/*.cpp)
# create objects in obj folder
objects = $(addprefix obj/,$(notdir $(cpp_files:.cpp=.o))) obj/glad.o
headers =

all: $(TARGET)

$(TARGET): $(objects)
	$(CXX) -o $@ $^ $(glfw_lib)/libglfw.3.dylib $(LDFLAGS)

obj/%.o: src/%.cpp $(headers)
	mkdir -p obj
	$(CXX) -c -o $@ $< $(CXXFLAGS)

obj/glad.o: $(glad)/src/glad.c
	mkdir -p obj
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: clean
.NOTPARALLEL: clean
clean:
	rm -f $(TARGET) $(objects)

.PHONY: run
run: $(TARGET)
	cd src && ../$(TARGET)

.PHONY: cleanrun
cleanrun: clean run
