# default
# all:
# 	g++ -g --std=c++17 -I../include -L../lib ../src/*.cpp ../src/include/glad.c -lglfw3dll -o main

CXX = g++
CXXFLAGS = -g --std=c++17 -I../include -L../lib ../src/include/glad.c -lglfw3dll
# CXXLIBS = ../src/include/glad.c -lglfw3dll

worldgen: ../src/world/chunk.cpp ../src/world/world.cpp ../src/main_worldgen.cpp
	$(CXX) $(CXXFLAGS) -o worldgen $^