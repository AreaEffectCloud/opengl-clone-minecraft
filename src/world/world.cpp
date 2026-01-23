#include "world.hpp"
#include <algorithm>
#include <cstdio>
#include <cmath>
#include <cinttypes>
#include <iostream>
#include <numeric>
#include <map>

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
        m_chunks.clear();
        // generate spawn chunk at (0,0) by default
        generate_chunk(0, 0);
        std::printf("[World] initialized with seed=%u\n", m_seed);
    }

    void World::destroy() {
        m_chunks.clear();
    }

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

    float World::fractal_noise(float x, float z, int octaves, float persistence, float lacunarity) const {
        float total = 0.0f;
        float frequency = 1.0f;
        float amplitude = 1.0f;
        float maxValue = 0.0f; // Used for normalizing result to [0,1]

        for (int i = 0; i < octaves; i++) {
            total += perlin_noise(x * frequency, 0.1f, z * frequency) * amplitude;

            maxValue += amplitude;
            amplitude *= persistence;
            frequency *= lacunarity;
        }

        return total / maxValue;
    }

    void World::generate_chunk(int cx, int cz) {
        // 複数のチャンクを管理する場合、 
        // std::map<std::pair<int, int>, ChunkPtr> m_chunks 等での管理が理想です
        auto chunk = std::make_unique<Chunk>(cx, cz);
        // 海面の高さ
        const int SEA_LEVEL = 48;

        for (int x = 0; x < CHUNK_SIZE_X; x++) {
            for (int z = 0; z < CHUNK_SIZE_Z; z++) {
                // translate chunk-local coordinates to world coordinates
                float world_x = static_cast<float>(cx * CHUNK_SIZE_X + x);
                float world_z = static_cast<float>(cz * CHUNK_SIZE_Z + z);

                // 1. 地形の高さ
                // セレクターノイズ: 地形の種類を決定
                float selector = fractal_noise(world_x * 0.005f, world_z * 0.005f, 3, 0.5f, 2.0f);
                // 境界をはっきりさせる
                selector = std::clamp((selector - 0.4f) * 5.0f, 0.0f, 1.0f);

                // Plain noise
                float plain_height = fractal_noise(world_x * 0.01f, world_z * 0.01f, 4, 0.5f, 2.0f) * 20.0f + 35.0f;

                // Mountain noise
                float mountain_height = fractal_noise(world_x * 0.02f, world_z * 0.02f, 6, 0.6f, 2.0f);
                mountain_height = std::pow(mountain_height, 1.5f) * 80.0f + 40.0f;

                // blend two heights
                int terrain_height = static_cast<int>(lerp(plain_height, mountain_height, selector));

                // 2. Humidity noise
                float humidity = fractal_noise(world_x * 0.008f, world_z * 0.008f, 2, 0.5f, 2.0f);
                bool is_desert = (humidity < 0.35f);

                // Set blocks up to terrain_height
                for (int y = 0; y < CHUNK_SIZE_Y; y++) {
                    uint8_t id = static_cast<uint8_t>(BlockID::AIR);

                    if (y < terrain_height) {
                        // 地面の下
                        if (y == terrain_height - 1) {
                            // 表面のブロック
                            if (is_desert) {
                                id = static_cast<uint8_t>(BlockID::SAND);
                            } else {
                                // 海面よりも下で表面なら砂
                                id = (y < SEA_LEVEL + 2) ? static_cast<uint8_t>(BlockID::SAND) : static_cast<uint8_t>(BlockID::GRASS);
                            }

                            // 山岳地帯の山頂付近は石
                            // if (selector > 0.7f && y > 80) id = static_cast<uint8_t>(BlockID::STONE);
                            // else id = static_cast<uint8_t>(BlockID::GRASS);

                        } else if (y >= terrain_height - 4) {
                            // 地表近くの層
                            id = is_desert ? static_cast<uint8_t>(BlockID::SAND) : static_cast<uint8_t>(BlockID::DIRT);
                        } else {
                            // 深い層
                            id = static_cast<uint8_t>(BlockID::STONE);
                        }
                    } else {
                        // 地面よりも上で海面以下なら水で満たす
                        if (y <= SEA_LEVEL) {
                            id = static_cast<uint8_t>(BlockID::WATER);
                        }
                    }
                    chunk->set_block(x, y, z, id);
                }   
            }
        }
        m_chunks[{cx, cz}] = std::move(chunk);
    }

    void World::generate_world(int width, int depth) {
        m_chunks.clear();
        for (int cz = 0; cz < depth; cz++) {
            for (int cx = 0; cx < width; cx++) {
                generate_chunk(cx, cz);
            }
        }
    }

    BlockID World::get_block(int wx, int wy, int wz) const {
        if (wy < 0 || wy >= CHUNK_SIZE_Y) return BlockID::AIR;

        // ワールド座標からチャンク座標 (cx, cz) を計算
        // 正の座標系を想定しているが、負の座標も考慮する場合はfloorを使う
        int cx = wx >= 0 ? wx / CHUNK_SIZE_X : (wx + 1) / CHUNK_SIZE_X - 1;
        int cz = wz >= 0 ? wz / CHUNK_SIZE_Z : (wz + 1) / CHUNK_SIZE_Z - 1;

        // チャンク内ローカル座標を計算
        int lx = wx - (cx * CHUNK_SIZE_X);
        int lz = wz - (cz * CHUNK_SIZE_Z);

        auto it = m_chunks.find({cx, cz});
        if (it != m_chunks.end()) {
            return static_cast<BlockID>(it->second->get_block(lx, wy, lz));
        }
        return BlockID::AIR;
    }

    int World::sample_height(int world_x, int world_z) const {
        float noise_val = fractal_noise(
            world_x * 0.03f,
            world_z * 0.03f,
            4,      // octaves
            0.5f,   // persistence
            2.0f    // lacunarity
        );

        int height = static_cast<int>(noise_val * (CHUNK_SIZE_Y - 1)) + 1;
        return std::min(std::max(height, 0), CHUNK_SIZE_Y - 1);
    }
    
    void World::dump_stats() const {
        if (m_chunks.empty()) {
            std::puts("[World] no chunks generated.\n");
            return;
        }

        int total_min_h = CHUNK_SIZE_Y;
        int total_max_h = 0;

        // 全チャンクをループして統計を取る
        for (auto const& [coords, chunkPtr] : m_chunks) {
            int cx = coords.first;
            int cz = coords.second;

            int chunk_min_h = CHUNK_SIZE_Y;
            int chunk_max_h = 0;

            // チャンク内の全カラムの高さをサンプリング
            for (int z = 0; z < CHUNK_SIZE_Z; z++) {
                for (int x = 0; x < CHUNK_SIZE_X; x++) {
                    int world_x = cx * CHUNK_SIZE_X + x;
                    int world_z = cz * CHUNK_SIZE_Z + z;
                    
                    int h = sample_height(world_x, world_z);
                    
                    if (h < chunk_min_h) chunk_min_h = h;
                    if (h > chunk_max_h) chunk_max_h = h;
                }
            }
            // ワールド全体の最小・最大を更新
            if (chunk_min_h < total_min_h) total_min_h = chunk_min_h;
            if (chunk_max_h > total_max_h) total_max_h = chunk_max_h;
        }
        std::printf("---------------------------\n");
        std::printf("Global Height Range: [%d - %d]\n", total_min_h, total_max_h);
        std::printf("---------------------------\n");
    }

    bool World::has_chunk(int cx, int cz) const {
        return m_chunks.find({cx, cz}) != m_chunks.end();
    }

    void World::update(float playerX, float playerZ, int viewDistance) {
        // プレイヤーが今どのチャンクにいるか
        int pCX = static_cast<int>(std::floor(playerX / static_cast<float>(CHUNK_SIZE_X)));
        int pCZ = static_cast<int>(std::floor(playerZ / static_cast<float>(CHUNK_SIZE_Z)));

        // bool chunkGenerated = false;

        // プレイヤーの周囲 (viewDistance) のチャンクをチェック
        for (int cz = pCZ - viewDistance; cz <= pCZ + viewDistance; cz++) {
            for (int cx = pCX - viewDistance; cx <= pCX + viewDistance; cx++) {
                
                // チャンクが存在しない場合のみ生成処理を行う
                if (!has_chunk(cx, cz)) {
                    generate_chunk(cx, cz);
                    // std::printf("[World] Generated chunk (%d, %d)\n", cx, cz);

                    // 新しく生成されたチャンク自身をメッシュの更新対象へ
                    m_chunks[{cx, cz}]->set_dirty(true);

                    // 隣接する4チャンクがある場合、それらも更新対象へ
                    auto set_neighbor_dirty = [&](int ncx, int ncz) {
                        auto it = m_chunks.find({ncx, ncz});
                        if (it != m_chunks.end()) {
                            it->second->set_dirty(true);
                        }
                    };

                    set_neighbor_dirty(cx + 1, cz); // X+
                    set_neighbor_dirty(cx - 1, cz); // X-
                    set_neighbor_dirty(cx, cz + 1); // Z+
                    set_neighbor_dirty(cx, cz - 1); // Z-

                    // ワールド全体でメッシュ更新の必要があることを示す
                    m_needsMeshUpdate = true;
                }
            }
        }
    }

    std::vector<Chunk*> World::get_visible_chunks(const glm::vec3& camPos, int viewDistance) {
        std::vector<Chunk*> visibleChunks;

        int pCX = static_cast<int>(std::floor(camPos.x / static_cast<float>(CHUNK_SIZE_X)));
        int pCZ = static_cast<int>(std::floor(camPos.z / static_cast<float>(CHUNK_SIZE_Z)));

        for (int cz = pCZ - viewDistance; cz <= pCZ + viewDistance; cz++) {
            for (int cx = pCX - viewDistance; cx <= pCX + viewDistance; cx++) {
                auto it = m_chunks.find({cx, cz});
                if (it != m_chunks.end()) {
                    visibleChunks.push_back(it->second.get());
                }
            }
        }
        return visibleChunks;
    }

    Chunk* World::get_chunk_ptr(int cx, int cz) const {
        auto it = m_chunks.find({cx, cz});
        if (it != m_chunks.end()) {
            return it->second.get();
        }
        return nullptr;
    }

    std::vector<Chunk*> World::get_all_chunks_raw_ptr() const {
        std::vector<Chunk*> ptrs;
        ptrs.reserve(m_chunks.size());
        for (auto const& [coords, chunk] : m_chunks) {
            ptrs.push_back(chunk.get());
        }
        return ptrs;
    }

    bool World::is_opaque(int wx, int wy, int wz) const {
        BlockID id = get_block(wx, wy, wz);
        return (id == BlockID::AIR || id == BlockID::WATER) ? false : true;
    }
} // namespace ocm