#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include <map>
#include <optional>
#include "../block/block.hpp"
#include "chunk.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace ocm {
    class World {
        public:
            World();
            ~World();
    
            uint32_t seed() const noexcept { return m_seed; }
            void init(uint32_t seed);
            void destroy();

            float fade(float t) const;
            float lerp(float a, float b, float t) const;
            float grad(int hash, float x, float y, float z) const;
            float perlin_noise(float x, float y, float z) const;
            float fractal_noise(float x, float z, int octaves, float persistence, float lacunarity) const;
    
            void generate_chunk(int cx, int cz);
            void generate_world(int width, int depth);
            BlockID get_block(int wx, int wy, int wz) const;
            
            int sample_height(int world_x, int world_z) const;
            void dump_stats() const;

            bool has_chunk(int cx, int cz) const;
            bool m_needsMeshUpdate = false; // メッシュ更新が必要かどうか
            void update(float playerX, float playerZ, int viewDistance);

            std::vector<Chunk*> get_visible_chunks(const glm::vec3& camPos, int viewDistance);
    
        private:
            uint32_t m_seed = 0;
            std::map<std::pair<int, int>, ChunkPtr> m_chunks;
            // Permutation table for Perlin noise
            std::vector<int> p;
    };
} // namespace ocm