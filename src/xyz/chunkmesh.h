#ifndef CHUNK_MESH_H
#define CHUNK_MESH_H

#include "types.h"
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

typedef unsigned int GLHandle;

enum ChunkMeshPart {
    CHUNK_MESH_OPAQUE = 0,
    CHUNK_MESH_TRANSPARENT = 1,
};

struct Vertex {
    f32 px, py, pz;
    f32 u, v;
};

struct ChunkMesh {
    struct Vertex* vertices;
    u32* indices;
    size_t vertex_count;
    size_t index_count;

    GLHandle vao;
    GLHandle vbo;
    GLHandle ebo;
    bool uploaded;
};

struct ChunkMesh* chunkmesh_create(void);
void chunkmesh_destroy(struct ChunkMesh* mesh);
void chunkmesh_build_from_chunk(struct ChunkMesh* mesh, struct Chunk* chunk);
void chunkmesh_upload(struct ChunkMesh* mesh);
void chunkmesh_render(struct ChunkMesh* mesh, enum ChunkMeshPart part);

#endif