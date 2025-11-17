#ifndef CHUNK_H
#define CHUNK_H

#include "types.h"
#include "block.h"
#include "chunkmesh.h"
#include <stddef.h>
#include <stdbool.h>

struct World;

struct Chunk {
    struct World* world;
    ivec3s offset;
    ivec3s position;

    BlockId* blocks;
    size_t block_count;

    struct {
        bool empty:1;
        bool generating:1;
    } flags;

    struct ChunkMesh* mesh;
};

void chunk_init(struct Chunk* chunk, struct World* world, ivec3s offset);
void chunk_destroy(struct Chunk* chunk);

BlockId chunk_get_block(const struct Chunk* chunk, ivec3s local);
void chunk_set_block(struct Chunk* chunk, ivec3s local, BlockId block_id);

BlockId chunk_get_block_world(const struct Chunk* chunk, ivec3s local);

void chunk_build_mesh(struct Chunk* chunk);

#endif