#ifndef WORLD_H
#define WORLD_H

#include "block.h"
#include <iostream>

const int CHUNK_SIZE_X = 16;
const int CHUNK_SIZE_Y = 2;
const int CHUNK_SIZE_Z = 16;

class World {
    public:
        BlockType data[CHUNK_SIZE_X][CHUNK_SIZE_Y][CHUNK_SIZE_Z];

        World() {
            generateSimpleTerrain();
        }

        BlockType getBlock(int x, int y, int z) const {
            if (x < 0 || x >= CHUNK_SIZE_X || 
                y < 0 || y >= CHUNK_SIZE_Y || 
                z < 0 || z >= CHUNK_SIZE_Z) {
                return BlockType::AIR; // Out of bounds
            }
            return data[x][y][z];
        }

    private:
        void generateSimpleTerrain() {
            for (int x = 0; x < CHUNK_SIZE_X; ++x) {
                for (int z = 0; z < CHUNK_SIZE_Z; ++z) {
                    data[x][0][z] = BlockType::STONE;
                    data[x][1][z] = BlockType::DIRT;
                }
            }
        }
};

#endif // WORLD_H