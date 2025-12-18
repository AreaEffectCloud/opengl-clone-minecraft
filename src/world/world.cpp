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
        if (h >= CHUNK_SIZE_Y - 1) h = CHUNK_SIZE_Y - 2;
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
