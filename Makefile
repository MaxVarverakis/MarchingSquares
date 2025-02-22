# Compiler and flags
CXX = /opt/homebrew/opt/llvm/bin/clang++
CXXFLAGS = -fcolor-diagnostics -fansi-escape-codes -g -pedantic-errors -Wall -Weffc++ -Wextra -Wconversion -Wsign-conversion -fopenmp -std=c++23 -O0 -arch arm64

# Include paths
INCLUDES = -Isrc -I../OpenGL_Framework/src -I/opt/homebrew/Cellar/SDL2/2.30.11/include -I/opt/homebrew/Cellar/glew/2.2.0_1/include -I/opt/homebrew/Cellar/glm/1.0.1/include

# Library paths and linker flags
LDFLAGS = -L../OpenGL_Framework/lib -L/opt/homebrew/Cellar/SDL2/2.30.11/lib -L/opt/homebrew/Cellar/glew/2.2.0_1/lib -L/opt/homebrew/Cellar/glm/1.0.1/lib
LINKER_FLAGS = -lopenglframework -lsdl2 -framework OpenGL -lglew -lglm

# Directories
SRC_DIR = src
BUILD_DIR = build
LIB_DIR = lib

# Source and object files
SRCS = $(wildcard $(SRC_DIR)/**/*.cpp) $(wildcard $(SRC_DIR)/main.cpp)
OBJS = $(SRCS:%.cpp=$(BUILD_DIR)/%.o)
TARGET = $(BUILD_DIR)/main

all: $(TARGET)

# Ensure the framework library is built before the project
$(TARGET): $(OBJS) ../OpenGL_Framework/$(LIB_DIR)/libopenglframework.a
	$(CXX) $(CXXFLAGS) $(LINKER_FLAGS) $(LDFLAGS) -o $@ $^

# Compile source files to object files
$(BUILD_DIR)/%.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
