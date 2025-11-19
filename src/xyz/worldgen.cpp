#include "include/worldgen.h"
#include "include/block.h"
#include "include/world.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>

extern "C" {

// noise
static inline u32 hash_u32(u64 seed, s64 x, s64 z) {
    u64 h = seed;
    h ^= (u64)x + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
    h ^= (u64)z + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
    h = (h ^ ( h >> 30)) * 0xbf58476d1ce4e5b9ULL;
    h = (h ^ ( h >> 27)) * 0x94d049bb133111ebULL;
    h = h ^ ( h >> 31);
    return (u32)(h & 0xFFFFFFFFu);
}

f32 worldgen_noise_sample(u64 seed, f32 x, f32 z) {
    s64 ix = (s64)floorf(x);
    s64 iz = (s64)floorf(z);
    f32 fx = x - (f32)ix;
    f32 fz = z - (f32)iz;

    u32 h00 = hash_u32(seed, ix + 0, iz + 0);
    u32 h10 = hash_u32(seed, ix + 1, iz + 0);
    u32 h01 = hash_u32(seed, ix + 0, iz + 1);
    u32 h11 = hash_u32(seed, ix + 1, iz + 1);

    f32 v00 = (f32)(h00) / (f32)UINT32_MAX;
    f32 v10 = (f32)(h10) / (f32)UINT32_MAX;
    f32 v01 = (f32)(h01) / (f32)UINT32_MAX;
    f32 v11 = (f32)(h11) / (f32)UINT32_MAX;

    auto smooth = [](f32 t) {
        return t * t * (3.0f - 2.0f * t);
    };

    f32 sx = smooth(fx);
    f32 a = v00 * (1.0f - sx) + v10 * sx;
    f32 b = v01 * (1.0f - sx) + v11 * sx;
    f32 sz = smooth(fz);
    return a * (1.0f - sz) + b * sz;
}

void worldgen_fill_column(struct Chunk* chunk, ivec3s local_pos, s64 height) {
    if (!chunk || !chunk->blocks) return;

    s64 base_y = chunk->offset.y;
    for (s64 y = 0; y < CHUNK_SIZE_Y; ++y) {
        size_t index = CHUNK_LOCAL_INDEX(local_pos.x, (s32)y, local_pos.z);
        if ((base_y + y) <= height) {
            if ((base_y + y) <= (height - 3)) {
                chunk->blocks[index] = STONE;
            } else if ((base_y + y) < height) {
                chunk->blocks[index] = DIRT;
            } else {
                chunk->blocks[index] = GRASS;
            }
        } else {
            chunk->blocks[index] = AIR;
        }
    }
}

void worldgen_generate_chunk(struct Chunk* chunk) {
    if (!chunk || !chunk->world) return;

    u64 seed = chunk->world->seed;
    for (s32 lx = 0; lx < CHUNK_SIZE_X; ++lx) {
        for (s32 lz = 0; lz < CHUNK_SIZE_Z; ++lz) {
            s64 wx = (s64)chunk->position.x + (s64)lx;
            s64 wz = (s64)chunk->position.z + (s64)lz;

            f32 n1 = worldgen_noise_sample(seed, (f32)wx * 0.03f, (f32)wz * 0.03f);
            f32 n2 = worldgen_noise_sample(seed + 1, (f32)wx * 0.08f, (f32)wz * 0.08f);
            f32 n3 = worldgen_noise_sample(seed + 2, (f32)wx * 0.2f, (f32)wz * 0.2f);

            f32 heightf = (n1 * 0.6f + n2 * 0.3f + n3 * 0.1f);
            s64 world_height = (s64)floorf(heightf * (CHUNK_SIZE_Y * 0.5f)) + (CHUNK_SIZE_Y / 4);

            ivec3s local = { lx, 0, lz };
            worldgen_fill_column(chunk, local, world_height);
        }
    }
    chunk->flags.generating = false;
}

} // extern "C"