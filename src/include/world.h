#ifndef WORLD_H
#define WORLD_H

#include "types.h"
#include "chunk.h"
#include "worldgen.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

struct World {
    size_t chunks_size;
    struct Chunk** chunks;
    ivec3s chunks_origin;
    ivec3s center_offset;
    u64 seed; 

    struct Heightmap** heightmaps;
};

void world_init(struct World* world, u64 seed, size_t chunks_size);
void world_destroy(struct World* world);

struct Chunk* world_create_chunk(struct World* world, ivec3s offset);
void world_destroy_chunk(struct World* world, ivec3s offset);
struct Chunk* world_get_chunk(struct World* world, ivec3s offset);
size_t world_chunk_index(const struct World* world, ivec3s offset);

ivec3s world_blockpos_to_chunk_offset(ivec3s pos);
ivec3s world_blockpos_to_chunk_local(ivec3s pos);

#endif // WORLD_H