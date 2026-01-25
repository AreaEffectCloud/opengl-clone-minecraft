#include "world.hpp"
#include "structures.hpp"
#include <algorithm>
#include <cstdio>
#include <cmath>
#include <cinttypes>
#include <iostream>
#include <numeric>
#include <map>
#include <random>

namespace ocm {
    World::World() {
        // Initialize permutation table
        // p.resize(256);
        // std::vector<int> perm(256);
        // std::iota(perm.begin(), perm.end(), 0); // Fill with values 0..255
        // TODO: Shuffle perm based on seed for better randomness

        // for (int i = 0; i < 256; i++) {
        //     p[i] = p[i + 256] = perm[i];
        // }
    }

    World::~World() {
        destroy();
    }

    void World::init(uint32_t seed) {
        m_seed = seed;
        m_chunks.clear();

        // 置換テーブル p をシード値に基づいてシャッフル
        p.resize(256);
        std::iota(p.begin(), p.end(), 0);
        std::default_random_engine engine(seed);
        std::shuffle(p.begin(), p.end(), engine);

        // 参照を楽にするために 256〜511 にコピー
        p.insert(p.end(), p.begin(), p.end());

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

    float World::get_noise_random(int x, int z) {
        unsigned int n = (unsigned int)(x * 374761393 + z * 668265263 + m_seed);
        n = (n ^ (n >> 13)) * 1274126177;
        return (float)(n & 0x7fffffff) / 0x7fffffff;
    }

    void World::generate_chunk(int cx, int cz) {
        // 複数のチャンクを管理する場合、 
        // std::map<std::pair<int, int>, ChunkPtr> m_chunks 等での管理が理想です
        auto chunk = std::make_unique<Chunk>(cx, cz);
        int terrain_height_map[CHUNK_SIZE_X][CHUNK_SIZE_Z];

        // 海面の高さ
        const int SEA_LEVEL = 63;

        // シード値によるオフセット
        float offsetX = static_cast<float>(m_seed % 10000);
        float offsetZ = static_cast<float>((m_seed / 10000) % 10000);

        // 地形配置
        for (int x = 0; x < CHUNK_SIZE_X; x++) {
            for (int z = 0; z < CHUNK_SIZE_Z; z++) {
                // translate chunk-local coordinates to world coordinates
                float world_x = static_cast<float>(cx * CHUNK_SIZE_X + x);
                float world_z = static_cast<float>(cz * CHUNK_SIZE_Z + z);

                // 1. バイオーム判定用のノイズ
                float selector_raw = fractal_noise(world_x * 0.002f + offsetX, world_z * 0.002f + offsetZ, 3, 0.5f, 2.0f);
                float mountain_weight = std::clamp((selector_raw - 0.5f) * 3.3f, 0.0f, 1.0f);
                float selector = std::clamp((selector_raw - 0.3f) * 2.0f, 0.0f, 1.0f);

                float humidity = fractal_noise(world_x * 0.01f + offsetX, world_z * 0.01f + offsetZ, 2, 0.5f, 2.0f);

                // 2. 地形の高さ計算
                // 平原・砂漠
                float lowland_height = fractal_noise(world_x * 0.008f + offsetX, world_z * 0.008f + offsetZ, 3, 0.3f, 2.0f) * 10.0f + 64.0f;
                // 山岳 (累乗で急傾斜)
                float mountain_height_raw = fractal_noise(world_x * 0.012f + offsetX, world_z * 0.012f + offsetZ, 5, 0.55f, 2.0f);
                mountain_height_raw = std::max(0.0f, mountain_height_raw);
                float mountain_height = std::pow(mountain_height_raw, 2.0f) * 120.0f + 63.0f;

                // 基本地形の合成
                float original_height = lerp(lowland_height, mountain_height, mountain_weight);

                // 3. 川の計算
                float river_n = fractal_noise(world_x * 0.005f + offsetX, world_z * 0.005f + offsetZ, 2, 0.5f, 2.0f);
                float river_v = std::abs(river_n - 0.5f) * 2.0f;
                float river_mask = std::clamp(river_v / 0.15f, 0.0f, 1.0f);

                // 川の深さ
                float river_depth = 10.0f * (1.0f - river_mask);
                original_height -= river_depth;

                // blend two heights
                int terrain_height = static_cast<int>(original_height);
                terrain_height_map[x][z] = terrain_height;

                // 2. Humidity noise
                bool is_mountain = (mountain_weight > 0.4f);
                bool is_desert = (!is_mountain && humidity < 0.35f);
                bool is_beach = (!is_mountain && !is_desert && terrain_height >= SEA_LEVEL && terrain_height <= 65);

                // Set blocks up to terrain_height
                for (int y = 0; y < CHUNK_SIZE_Y; y++) {
                    uint8_t id = static_cast<uint8_t>(BlockID::AIR);

                    if (y < terrain_height) {
                        // 地面の下
                        if (y == terrain_height - 1) {

                            if ((is_desert && y < 70) || is_beach || (river_depth > 0.5f && river_depth < 4.0f)) {
                                id = static_cast<uint8_t>(BlockID::SAND);

                            } else if (is_mountain) {
                                if (y > 95) {
                                    id = static_cast<uint8_t>(BlockID::STONE); // 山頂
                                } else {
                                    id = static_cast<uint8_t>(BlockID::GRASS);
                                }

                            } else if (y < SEA_LEVEL) {
                                id = (river_depth >= 4.0f) ? static_cast<uint8_t>(BlockID::DIRT) : static_cast<uint8_t>(BlockID::SAND);
                            } else {
                                id = static_cast<uint8_t>(BlockID::GRASS);
                            }

                        } else {
                            bool force_dirt = (y < SEA_LEVEL + 2);
                            if (force_dirt) {
                                if (y >= terrain_height - 5) {
                                    id = (is_desert && y < 70) ? static_cast<uint8_t>(BlockID::SAND) : static_cast<uint8_t>(BlockID::DIRT);
                                } else {
                                    id = static_cast<uint8_t>(BlockID::STONE);
                                }
                            } else if (is_mountain && y > 90) {
                                if (y >= terrain_height - 2) id = static_cast<uint8_t>(BlockID::DIRT);
                                else id = static_cast<uint8_t>(BlockID::STONE);
                            } else {
                                if (y >= terrain_height - 4) {
                                    id = (is_desert && y < 70) ? static_cast<uint8_t>(BlockID::SAND) : static_cast<uint8_t>(BlockID::DIRT);
                                } else {
                                    id = static_cast<uint8_t>(BlockID::STONE);
                                }
                            }
                        }
                    } else if (y <= SEA_LEVEL) {
                        id = static_cast<uint8_t>(BlockID::WATER);
                    }
                    chunk->set_block(x, y, z, id);
                }   
            }
        }

        // デコレーション
        for (int x = 2; x < CHUNK_SIZE_X - 2; x++) { // 境界ギリギリを避ける
            for (int z = 2; z < CHUNK_SIZE_Z - 2; z++) {
                int wx = cx * CHUNK_SIZE_X + x;
                int wz = cz * CHUNK_SIZE_Z + z;

                // その地点のバイオームと高さを再取得
                int ground_y = terrain_height_map[x][z]; 
                uint8_t surface_id = chunk->get_block(x, ground_y - 1, z);

                float r = get_noise_random(wx, wz);

                // 平原の木 (0.5% の確率)
                if (surface_id == static_cast<uint8_t>(BlockID::GRASS) && r < 0.01f) {
                    for (const auto& b : structures::OAK_TREE) {
                        chunk->set_block(x + b.dx, ground_y + b.dy, z + b.dz, b.id);
                    }
                }
                // 砂漠のサボテン (0.3% の確率)
                else if (surface_id == static_cast<uint8_t>(BlockID::SAND) && r < 0.003f && ground_y > SEA_LEVEL) {
                    for (const auto& b : structures::CACTUS) {
                        chunk->set_block(x + b.dx, ground_y + b.dy, z + b.dz, b.id);
                    }
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
        if (id == BlockID::AIR || 
            id == BlockID::WATER || 
            id == BlockID::CACTUS ||
            id == BlockID::LEAVES) {
            return false;
        }
        return true;
    }
} // namespace ocm