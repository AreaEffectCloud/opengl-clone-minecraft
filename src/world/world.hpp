#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include <map>
#include <optional>
#include "../block/block.hpp"
#include "chunk.hpp"

namespace ocm {
    class World {
        public:
            World();
            ~World();
    
            void init(uint32_t seed);
            void destroy();
    
            void generate_chunk(int cx, int cz);
            void generate_world(int width, int depth);
    
            BlockID get_block(int world_x, int world_y, int world_z) const;
    
            int sample_height(int world_x, int world_z) const;
            void dump_stats() const;
    
            uint32_t seed() const noexcept { return m_seed; }
        
        private:
            uint32_t m_seed = 0;
            std::map<std::pair<int, int>, ChunkPtr> m_chunks;
    
            float perlin_noise(float x, float y, float z) const;
            float fade(float t) const;
            float lerp(float a, float b, float t) const;
            float grad(int hash, float x, float y, float z) const;
            
            // Permutation table for Perlin noise
            std::vector<int> p;
            float fractal_noise(float x, float z, int octaves, float persistence, float lacunarity) const;
    };
} // namespace ocm