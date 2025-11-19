#pragma once

// #include "types.h"
// #include "block.h"
// #include "chunkmesh.h"
#include <cstdint>
#include <vector>
#include <memory>

namespace ocm {

    constexpr int CHUNK_SIZE_X = 16;
    constexpr int CHUNK_SIZE_Z = 16;
    constexpr int CHUNK_SIZE_Y = 2;
    
    class Chunk{
        public:
            Chunk(int cx, int cz);
            ~Chunk();
    
            int cx() const noexcept { return m_cx; }
            int cz() const noexcept { return m_cz; }
    
            uint8_t get_block(int x, int y, int z) const noexcept;
            void set_block(int x, int y, int z, uint8_t id) noexcept;
    
            size_t block_count() const noexcept { return m_blocks.size(); }
    
        private:
            int m_cx, m_cz;
            std::vector<uint8_t> m_blocks;
    
            static inline size_t index(int x, int y, int z) noexcept {
                return static_cast<size_t>(x + CHUNK_SIZE_X * (z + CHUNK_SIZE_Z * y));
            }
    };
    
    using ChunkPtr = std::unique_ptr<Chunk>;
} // namespace ocm



// struct World;

// struct Chunk {
//     struct World* world;
//     ivec3s offset;
//     ivec3s position;

//     BlockId* blocks;
//     size_t block_count;

//     struct {
//         bool empty:1;
//         bool generating:1;
//     } flags;

//     struct ChunkMesh* mesh;
// };

// void chunk_init(struct Chunk* chunk, struct World* world, ivec3s offset);
// void chunk_destroy(struct Chunk* chunk);

// BlockId chunk_get_block(const struct Chunk* chunk, ivec3s local);
// void chunk_set_block(struct Chunk* chunk, ivec3s local, BlockId block_id);

// BlockId chunk_get_block_world(const struct Chunk* chunk, ivec3s local);

// void chunk_build_mesh(struct Chunk* chunk);
