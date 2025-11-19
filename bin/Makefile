# default
# all:
# 	g++ -g --std=c++17 -I../include -L../lib ../src/*.cpp ../src/include/glad.c -lglfw3dll -o main

CXX = g++
CXXFLAGS = -g --std=c++17 -I../include -L../lib ../src/util/glad.c -lglfw3dll
SRC = ../src/gfx/*.cpp ../src/util/*.cpp ../src/world/*.cpp ../src/main_worldgen.cpp

# worldgen: ../src/world/chunk.cpp ../src/world/world.cpp ../src/main_worldgen.cpp
# 	$(CXX) $(CXXFLAGS) -o worldgen $^

game: $(SRC)
	$(CXX) $(CXXFLAGS) -o game $(SRC)