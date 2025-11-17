#include "include/world.h"
#include "include/chunk.h"
#include "include/worldgen.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>

extern "C" {

// convert chunk offset (x,z) to index in 2D array stored row-major
size_t world_chunk_index(const struct World* world, ivec3s offset) {
    if (!world) return (size_t)-1;
    s64 px = offset.x - world->chunks_origin.x;
    s64 pz = offset.z - world->chunks_origin.z;
    if (px < 0 || pz < 0) return (size_t)-1;
    return (size_t)(pz * (s64)world->chunks_size + px);
}

ivec3s world_blockpos_to_chunk_offset(ivec3s pos) {
    ivec3s offset;
    offset.x = (s32)floorf((f32)pos.x / (f32)CHUNK_SIZE_X);
    offset.y = (s32)floorf((f32)pos.y / (f32)CHUNK_SIZE_Y);
    offset.z = (s32)floorf((f32)pos.z / (f32)CHUNK_SIZE_Z);
    return offset;
}

ivec3s world_blockpos_to_chunk_local(ivec3s pos) {
    ivec3s local;
    local.x = pos.x % CHUNK_SIZE_X; if (local.x < 0) local.x += CHUNK_SIZE_X;
    local.y = pos.y % CHUNK_SIZE_Y; if (local.y < 0) local.y += CHUNK_SIZE_Y;
    local.z = pos.z % CHUNK_SIZE_Z; if (local.z < 0) local.z += CHUNK_SIZE_Z;
    return local;
}

void world_init(struct World* world, u64 seed, size_t chunks_size) {
    if (!world) return;
    memset(world, 0, sizeof(struct World));

    world->seed = seed;
    world->chunks_size = (chunks_size == 0) ? 8 : chunks_size;
    world->chunks = (struct Chunk**)calloc(world->chunks_size * world->chunks_size, sizeof(struct Chunk*));
    world->chunks_origin = {0, 0, 0};
    world->center_offset = {0, 0, 0};
    world->heightmaps = NULL;
}

void world_destroy(struct World* world) {
    if (!world) return;
    if (world->chunks) {
        size_t n = world->chunks_size * world->chunks_size;
        for (size_t i = 0; i< n; ++i) {
            if (world->chunks[i]) {
                chunk_destroy(world->chunks[i]);
                free(world->chunks[i]);
                world->chunks[i] = NULL;
            }
        }
        free(world->chunks);
        world->chunks = NULL;
    }
    if (world->heightmaps) {
        free(world->heightmaps);
        world->heightmaps = NULL;
    }
}

struct Chunk* world_create_chunk(struct World* world, ivec3s offset) {
    if (!world) return NULL;
    size_t idx = world_chunk_index(world, offset);
    if (idx == (size_t)-1 || idx >= (world->chunks_size * world->chunks_size)) {
        return NULL;
    }
    if (world->chunks[idx] != NULL) return world->chunks[idx];

    struct Chunk* chunk = (struct Chunk*)malloc(sizeof(struct Chunk));
    chunk_init(chunk, world, offset);

    worldgen_generate_chunk(chunk);
    chunk_build_mesh(chunk);
    world->chunks[idx] = chunk;
    return chunk;
}

void world_destroy_chunk(struct World* world, ivec3s offset) {
    if (!world) return;
    size_t idx = world_chunk_index(world, offset);
    if (idx == (size_t)-1) return;
    if (world->chunks[idx]) {
        chunk_destroy(world->chunks[idx]);
        free(world->chunks[idx]);
        world->chunks[idx] = NULL;
    }
}

struct Chunk* world_get_chunk(struct World* world, ivec3s offset) {
    if (!world) return NULL;
    size_t idx = world_chunk_index(world, offset);
    if (idx == (size_t)-1 || idx >= (world->chunks_size * world->chunks_size)) return NULL;
    return world->chunks[idx];
}

}