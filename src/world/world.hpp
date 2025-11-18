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
            World() = default;
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
    
            double pseudo_noise(int x, int z, uint32_t seed) const;
    };
} // namespace ocm


// #include "types.h"
// #include "chunk.h"
// #include "worldgen.h"
// #include <stddef.h>
// #include <stdint.h>
// #include <stdbool.h>

// struct World {
//     size_t chunks_size;
//     struct Chunk** chunks;
//     ivec3s chunks_origin;
//     ivec3s center_offset;
//     u64 seed; 

//     struct Heightmap** heightmaps;
// };

// void world_init(struct World* world, u64 seed, size_t chunks_size);
// void world_destroy(struct World* world);

// struct Chunk* world_create_chunk(struct World* world, ivec3s offset);
// void world_destroy_chunk(struct World* world, ivec3s offset);
// struct Chunk* world_get_chunk(struct World* world, ivec3s offset);
// size_t world_chunk_index(const struct World* world, ivec3s offset);

// ivec3s world_blockpos_to_chunk_offset(ivec3s pos);
// ivec3s world_blockpos_to_chunk_local(ivec3s pos);
