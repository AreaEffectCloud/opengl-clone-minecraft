#include "include/chunk.h"
#include "include/chunkmesh.h"
#include "include/worldgen.h"
#include "include/world.h"
#include <stdlib.h>
#include <string.h>

extern "C" {

void chunk_init(struct Chunk* chunk, struct World* world, ivec3s offset) {
    if (!chunk) return;
    memset(chunk, 0, sizeof(struct Chunk));
    chunk->world = world;
    chunk->offset = offset;
    chunk->position.x = offset.x * CHUNK_SIZE_X;
    chunk->position.y = offset.y * CHUNK_SIZE_Y;
    chunk->position.z = offset.z * CHUNK_SIZE_Z;

    chunk->blocks = (BlockId*)malloc(sizeof(BlockId) * chunk->block_count);
    chunk->block_count = 0;
    chunk->mesh = chunkmesh_create();
    chunk->flags.empty = false;
    chunk->flags.generating = true;
}

void chunk_destroy(struct Chunk* chunk) {
    if (!chunk) return;
    if (chunk->blocks) {
        free(chunk->blocks);
        chunk->blocks = NULL;
    }
    if (chunk->mesh) {
        chunkmesh_destroy(chunk->mesh);
        chunk->mesh = NULL;
    }
}

BlockId chunk_get_block(const struct Chunk* chunk, ivec3s local) {
    if (!chunk || !chunk->blocks) return AIR;
    if (local.x < 0 || local.x >= CHUNK_SIZE_X ||
        local.y < 0 || local.y >= CHUNK_SIZE_Y ||
        local.z < 0 || local.z >= CHUNK_SIZE_Z) {
        return AIR;
    }
    size_t index = CHUNK_LOCAL_INDEX(local.x, local.y, local.z);
    return chunk->blocks[index];
}

void chunk_set_block(struct Chunk* chunk, ivec3s local, BlockId block_id) {
    if (!chunk || !chunk->blocks) return;
    if (local.x < 0 || local.x >= CHUNK_SIZE_X ||
        local.y < 0 || local.y >= CHUNK_SIZE_Y ||
        local.z < 0 || local.z >= CHUNK_SIZE_Z) {
        return;
    }
    size_t index = CHUNK_LOCAL_INDEX(local.x, local.y, local.z);
    BlockId previous = chunk->blocks[index];
    chunk->blocks[index] = block_id;
    if (previous == AIR && block_id != AIR) chunk->block_count++;
    if (previous != AIR && block_id == AIR && chunk->block_count > 0) chunk->block_count--;
}

BlockId chunk_get_block_world(const struct Chunk* chunk, ivec3s local) {
    if (!chunk || !chunk->world) return AIR;
    if (local.x >= 0 && local.x < CHUNK_SIZE_X &&
        local.y >= 0 && local.y < CHUNK_SIZE_Y &&
        local.z >= 0 && local.z < CHUNK_SIZE_Z) {
        return chunk_get_block(chunk, local);
    }

    ivec3s world_block_pos = { chunk->position.x + local.x, chunk->position.y + local.y, chunk->position.z + local.z };
    ivec3s chunk_off = world_blockpos_to_chunk_offset(world_block_pos);
    struct Chunk* neighbor_chunk = world_get_chunk(chunk->world, chunk_off);

    if (!neighbor_chunk) return AIR;
    ivec3s local_in = world_blockpos_to_chunk_local(world_block_pos);
    return chunk_get_block(neighbor_chunk, local_in);
}

void chunk_build_mesh(struct Chunk* chunk) {
    if (!chunk || !chunk->mesh) return;
    chunkmesh_build_from_chunk(chunk->mesh, chunk);
    chunkmesh_upload(chunk->mesh);
}

}