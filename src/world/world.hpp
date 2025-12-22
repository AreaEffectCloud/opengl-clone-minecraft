#pragma once

#include <cstdint>
#include <memory>
#include <vector>
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
    
            BlockID get_block(int world_x, int y, int world_z) const;
    
            int sample_height(int world_x, int world_z) const;
            void dump_stats() const;
    
            uint32_t seed() const noexcept { return m_seed; }
        
        private:
            uint32_t m_seed = 0;
            ChunkPtr m_spawn_chunk;
            int m_spawn_cx = 0;
            int m_spawn_cz = 0;
    
            // double pseudo_noise(int x, int z, uint32_t seed) const;
            float perlin_noise(float x, float y, float z) const;
            float fade(float t) const;
            float lerp(float a, float b, float t) const;
            float grad(int hash, float x, float y, float z) const;
            
            // Permutation table for Perlin noise
            std::vector<int> p;
    };
} // namespace ocm