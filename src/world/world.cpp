#include "world.hpp"
#include <cstdio>
#include <cmath>
#include <cinttypes>
#include <iostream>
#include <numeric>

namespace ocm {
    World::World() {
        p.resize(512);
        std::vector<int> perm(256);
        std::iota(perm.begin(), perm.end(), 0); // Fill with values 0..255

        // TODO: Shuffle perm based on seed for better randomness

        for (int i = 0; i < 256; i++) {
            p[i] = p[i + 256] = perm[i];
        }
    }

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

    // double World::pseudo_noise(int x, int z, uint32_t seed) const {
    //     // 32/64-bit hashing based pseudo-noise in [0, 1)
    //     uint64_t h = static_cast<uint64_t>(x) * 73428767ULL;
    //     h ^= static_cast<uint64_t>(z) * 91278341ULL;
    //     h ^= static_cast<uint64_t>(seed) + 0x9e3779b97f4a7c15ULL;
    //     h = (h << 13) ^ h;
    //     uint64_t res = (h * (h * h * 15731ULL + 789221ULL) + 1376312589ULL) & 0x7fffffffffffffffULL;
    //     return static_cast<double>(res) / static_cast<double>(0x7fffffffffffffffULL);
    // }

    // Noise functions for terrain generation
    float World::fade(float t) const {
        return t * t * t * (t * (t * 6 - 15) + 10);
    }

    float World::lerp(float a, float b, float t) const {
        return a + t * (b - a);
    }

    float World::grad(int hash, float x, float y, float z) const {
        int h = hash & 15;
        float u = h < 8 ? x : y;
        float v = h < 4 ? y : (h == 12 || h == 14 ? x : z);
        return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
    }

    float World::perlin_noise(float x, float y, float z) const {
        int X = static_cast<int>(std::floor(x)) & 255;
        int Y = static_cast<int>(std::floor(y)) & 255;
        int Z = static_cast<int>(std::floor(z)) & 255;

        x -= std::floor(x);
        y -= std::floor(y);
        z -= std::floor(z);

        float u = fade(x);
        float v = fade(y);
        float w = fade(z);

        int A = p[X] + Y;
        int AA = p[A] + Z;
        int AB = p[A + 1] + Z;
        int B = p[X + 1] + Y;
        int BA = p[B] + Z;
        int BB = p[B + 1] + Z;

        float res = lerp(
            lerp(lerp(grad(p[AA], x, y, z), grad(p[BA], x - 1, y, z), u),
                lerp(grad(p[AB], x, y - 1, z), grad(p[BB], x - 1, y - 1, z), u), v),
            lerp(lerp(grad(p[AA + 1], x, y, z - 1), grad(p[BA + 1], x - 1, y, z - 1), u),
                lerp(grad(p[AB + 1], x, y - 1, z - 1), grad(p[BB + 1], x - 1, y - 1, z - 1), u), v), w);

        return (res + 1.0f) / 2.0f; // Normalize to [0,1]
    }

    // Not used currently
    int World::sample_height(int world_x, int world_z) const {
        float noise_scale_x = 0.05f;
        float noise_scale_z = 0.05f;
        float height_map = 8.0f;
        float base_height = 4.0f;

        float noise_val = perlin_noise(
            static_cast<float>(world_x) * noise_scale_x,
            0.1f,
            static_cast<float>(world_z) * noise_scale_z
        );

        int height = static_cast<int>(base_height + noise_val * height_map);
        if (height < 0) height = 0;
        if (height >= CHUNK_SIZE_Y) height = CHUNK_SIZE_Y - 1;

        return height;
    }

    void World::generate_chunk(int cx, int cz) {
        // Replace existing spawn chunk (simple)
        m_spawn_chunk = std::make_unique<Chunk>(cx, cz);

        // ノイズのスケールやオフセットを調整して地形の見た目を制御
        float noise_scale_x = 0.05f;
        float noise_scale_z = 0.05f;
        float height_map = 8.0f;
        float base_height = 4.0f;

        for (int x = 0; x < CHUNK_SIZE_X; x++) {
            for (int z = 0; z < CHUNK_SIZE_Z; z++) {
                
                // translate chunk-local coordinates to world coordinates
                int world_x = static_cast<float>(cx * CHUNK_SIZE_X + x);
                int world_z = static_cast<int>(cz * CHUNK_SIZE_Z + z);

                float noise_val = perlin_noise(
                    world_x * noise_scale_x,
                    0.1f,
                    world_z * noise_scale_z
                );

                int height = static_cast<int>(base_height + noise_val * height_map);

                for (int y = 0; y < height; y++) {
                    uint8_t id = static_cast<uint8_t>(BlockID::AIR);
                    if (y == height - 1) {
                        id = static_cast<uint8_t>(BlockID::GRASS);
                    } else if (y >= height - 4) {
                        id = static_cast<uint8_t>(BlockID::DIRT);
                    } else {
                        id = static_cast<uint8_t>(BlockID::STONE);
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
