# default
# all:
# 	g++ -g --std=c++17 -I../include -L../lib ../src/*.cpp ../src/include/glad.c -lglfw3dll -o main

CXX = g++
CXXFLAGS = -g --std=c++17 -I../include -L../lib
LIBS = ../src/util/glad.c -lglfw3dll
SRC = ../src/gfx/*.cpp ../src/util/*.cpp ../src/world/*.cpp ../src/main.cpp

game:
	$(CXX) $(CXXFLAGS) $(SRC) $(LIBS) -o game