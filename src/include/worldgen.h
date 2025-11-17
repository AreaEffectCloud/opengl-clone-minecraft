#ifndef WORLDGEN_H
#define WORLDGEN_H

#include "types.h"
#include "block.h"
#include "chunk.h"
#include <stdint.h>
#include <stdbool.h>

struct WorldGenData {
    s64 h; // height
    f32 t; // biome noise
    f32 r; // roughness
};

struct Heightmap {
    ivec2s offset;
    s64* data;
    struct WorldGenData* worldgen_data;
    bool generated;
};

void worldgen_generate_chunk(struct Chunk* chunk);
f32 worldgen_noise_sample(u64 seed, f32 x, f32 z);
void worldgen_fill_column(struct Chunk* chunk, ivec3s local_pos, s64 height);

#endif