#ifndef CHUNK_H
#define CHUNK_H

#include "util.h"
#include "world.h"

#define CHUNK_SIZE ((ivec3s) {{ 16, 16, 16 }})
#define CHUNK_SIZE_F ((ivec3) {{ 16, 16, 16 }})

// #define CHUNK_VOLUME(CHUNK_SIZE.x * CHUNK_SIZE.y* CHUNK_SIZE.z)

struct Face {
    size_t indices_base;
    vec3s position;
    // f32 distance;
};

struct MeshBuffer {
    void* data;
    size_t index, count, capacity;
};

struct Mesh {
    struct Chunk* chunk;

    struct MeshBuffer data, faces, indices;
    size_t vertex_count;

    // struct VAO vao;
    // struct VBO vbo, ibo;
};

struct Chunk {
    struct World* world;
    ivec3s offset, position;
};

#endif