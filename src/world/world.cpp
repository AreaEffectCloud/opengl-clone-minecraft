#include "world.hpp"
#include <cstdio>
#include <cmath>
#include <cinttypes>
#include <iostream>

namespace ocm {

    World::~World() {
        destroy();
    }

    void World::init(uint32_t seed) {
        m_seed = seed;
        m_spawn_chunk.reset();
        // generate spawn chunk at (0,0) by default
        generate_chunk(0, 0);
        std::printf("[World] initialized with seed=%u\n", m_seed);
    }

    void World::destroy() {
        m_spawn_chunk.reset();
    }

    double World::pseudo_noise(int x, int z, uint32_t seed) const {
        // 32/64-bit hashing based pseudo-noise in [0, 1)
        uint64_t h = static_cast<uint64_t>(x) * 73428767ULL;
        h ^= static_cast<uint64_t>(z) * 91278341ULL;
        h ^= static_cast<uint64_t>(seed) + 0x9e3779b97f4a7c15ULL;
        h = (h << 13) ^ h;
        uint64_t res = (h * (h * h * 15731ULL + 789221ULL) + 1376312589ULL) & 0x7fffffffffffffffULL;
        return static_cast<double>(res) / static_cast<double>(0x7fffffffffffffffULL);
    }

    int World::sample_height(int world_x, int world_z) const {
        // Fractal-like combination of pseudo-noise octaves
        double height = 0.0;
        double freq = 1.0 / 64.0;
        double amp = 24.0;
        
        for (int octave = 0; octave < 4; octave++) {
            double v = pseudo_noise(static_cast<int>(std::floor(world_x * freq)),
                                    static_cast<int>(std::floor(world_z * freq)), 
                                    m_seed + static_cast<uint32_t>(octave * 101));
            height += v * amp;
            freq *= 2.0;
            amp *= 0.5;
        }
        int base = 32;
        int h = base + static_cast<int>(std::floor(height));
        if (h < 1) h = 1;
        if (h >= CHUNK_SIZE_Y - 1) h = CHUNK_SIZE_Y -2;
        return h;
    }

    void World::generate_chunk(int cx, int cz) {
        // Replace existing spawn chunk (simple)
        m_spawn_chunk = std::make_unique<Chunk>(cx, cz);
        m_spawn_cx = cx;
        m_spawn_cz = cz;

        for (int z = 0; z < CHUNK_SIZE_Z; z++) {
            for (int x = 0; x < CHUNK_SIZE_X; x++) {
                int world_x = cx * CHUNK_SIZE_X + x;
                int world_z = cz * CHUNK_SIZE_Z + z;
                int h = sample_height(world_x, world_z);

                for (int y = 0; y < CHUNK_SIZE_Y; y++) {
                    uint8_t id = static_cast<uint8_t>(BlockID::AIR);
                    if (y <= h - 5) {
                        id = static_cast<uint8_t>(BlockID::STONE);
                    } else if (y <= h - 1) {
                        id = static_cast<uint8_t>(BlockID::DIRT);
                    } else if (y == h) {
                        id = static_cast<uint8_t>(BlockID::GRASS);
                    }
                    m_spawn_chunk->set_block(x, y, z, id);
                }   
            }
        }
        std::printf("[World] generated chunk at (%d, %d)\n", cx, cz);
    }

    BlockID World::get_block(int world_x, int y, int world_z) const {
        if (!m_spawn_chunk) return BlockID::AIR;
        // only supports spawn chunk (cx,cz)
        int local_x = world_x - m_spawn_cx * CHUNK_SIZE_X;
        int local_z = world_z - m_spawn_cz * CHUNK_SIZE_Z;

        if (local_x < 0 || local_x >= CHUNK_SIZE_X || local_z < 0 || local_z >= CHUNK_SIZE_Z || y < 0 || y >= CHUNK_SIZE_Y) {
            return BlockID::AIR;
        }

        uint8_t id = m_spawn_chunk->get_block(local_x, y, local_z);
        return static_cast<BlockID>(id);
    }

    void World::dump_stats() const {
        if (!m_spawn_chunk) {
            std::puts("[World] no chunks generated.\n");
            return;
        }
        // compute min/max height (based on first column of chunk)
        int minh = CHUNK_SIZE_Y, maxh = 0;
        for (int z = 0; z < CHUNK_SIZE_Z; z++) {
            for (int x = 0; x < CHUNK_SIZE_X; x++) {
                int world_x = m_spawn_cx * CHUNK_SIZE_X + x;
                int world_z = m_spawn_cz * CHUNK_SIZE_Z + z;
                int h = sample_height(world_x, world_z);
                if (h < minh) minh = h;
                if (h > maxh) maxh = h;
            }
        }
        std::printf("[World] chunk (%d, %d) height Min=%d, Max=%d\n", m_spawn_cx, m_spawn_cz, minh, maxh);

        // print center cross sample heights for quick visual check
        int cx = CHUNK_SIZE_X / 2;
        int cz = CHUNK_SIZE_Z / 2;

        std::printf("[World] center row heights (z=%d): ", cz);
        for (int x = 0; x < CHUNK_SIZE_X; x++) {
            int world_x = m_spawn_cx * CHUNK_SIZE_X + x;
            int world_z = m_spawn_cz * CHUNK_SIZE_Z + cz;
            int h = sample_height(world_x, world_z);
            std::printf("%d ", h);
        }
        std::printf("\n");
    }
} // namespace ocm


// extern "C" {

// // convert chunk offset (x,z) to index in 2D array stored row-major
// size_t world_chunk_index(const struct World* world, ivec3s offset) {
//     if (!world) return (size_t)-1;
//     s64 px = offset.x - world->chunks_origin.x;
//     s64 pz = offset.z - world->chunks_origin.z;
//     if (px < 0 || pz < 0) return (size_t)-1;
//     return (size_t)(pz * (s64)world->chunks_size + px);
// }

// ivec3s world_blockpos_to_chunk_offset(ivec3s pos) {
//     ivec3s offset;
//     offset.x = (s32)floorf((f32)pos.x / (f32)CHUNK_SIZE_X);
//     offset.y = (s32)floorf((f32)pos.y / (f32)CHUNK_SIZE_Y);
//     offset.z = (s32)floorf((f32)pos.z / (f32)CHUNK_SIZE_Z);
//     return offset;
// }

// ivec3s world_blockpos_to_chunk_local(ivec3s pos) {
//     ivec3s local;
//     local.x = pos.x % CHUNK_SIZE_X; if (local.x < 0) local.x += CHUNK_SIZE_X;
//     local.y = pos.y % CHUNK_SIZE_Y; if (local.y < 0) local.y += CHUNK_SIZE_Y;
//     local.z = pos.z % CHUNK_SIZE_Z; if (local.z < 0) local.z += CHUNK_SIZE_Z;
//     return local;
// }

// void world_init(struct World* world, u64 seed, size_t chunks_size) {
//     if (!world) return;
//     memset(world, 0, sizeof(struct World));

//     world->seed = seed;
//     world->chunks_size = (chunks_size == 0) ? 8 : chunks_size;
//     world->chunks = (struct Chunk**)calloc(world->chunks_size * world->chunks_size, sizeof(struct Chunk*));
//     world->chunks_origin = {0, 0, 0};
//     world->center_offset = {0, 0, 0};
//     world->heightmaps = NULL;
// }

// void world_destroy(struct World* world) {
//     if (!world) return;
//     if (world->chunks) {
//         size_t n = world->chunks_size * world->chunks_size;
//         for (size_t i = 0; i< n; ++i) {
//             if (world->chunks[i]) {
//                 chunk_destroy(world->chunks[i]);
//                 free(world->chunks[i]);
//                 world->chunks[i] = NULL;
//             }
//         }
//         free(world->chunks);
//         world->chunks = NULL;
//     }
//     if (world->heightmaps) {
//         free(world->heightmaps);
//         world->heightmaps = NULL;
//     }
// }

// struct Chunk* world_create_chunk(struct World* world, ivec3s offset) {
//     if (!world) return NULL;
//     size_t idx = world_chunk_index(world, offset);
//     if (idx == (size_t)-1 || idx >= (world->chunks_size * world->chunks_size)) {
//         return NULL;
//     }
//     if (world->chunks[idx] != NULL) return world->chunks[idx];

//     struct Chunk* chunk = (struct Chunk*)malloc(sizeof(struct Chunk));
//     chunk_init(chunk, world, offset);

//     worldgen_generate_chunk(chunk);
//     chunk_build_mesh(chunk);
//     world->chunks[idx] = chunk;
//     return chunk;
// }

// void world_destroy_chunk(struct World* world, ivec3s offset) {
//     if (!world) return;
//     size_t idx = world_chunk_index(world, offset);
//     if (idx == (size_t)-1) return;
//     if (world->chunks[idx]) {
//         chunk_destroy(world->chunks[idx]);
//         free(world->chunks[idx]);
//         world->chunks[idx] = NULL;
//     }
// }

// struct Chunk* world_get_chunk(struct World* world, ivec3s offset) {
//     if (!world) return NULL;
//     size_t idx = world_chunk_index(world, offset);
//     if (idx == (size_t)-1 || idx >= (world->chunks_size * world->chunks_size)) return NULL;
//     return world->chunks[idx];
// }

// }