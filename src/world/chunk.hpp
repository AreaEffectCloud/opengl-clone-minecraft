#pragma once

#include "../gfx/cube_renderer.hpp"
#include <cstdint>
#include <vector>
#include <memory>

namespace ocm {
    enum FaceDirection {
        SIDE_FRONT,
        SIDE_BACK,
        TOP,
        BOTTOM,
        SIDE_RIGHT,
        SIDE_LEFT
    };

    constexpr int CHUNK_SIZE_X = 16;
    constexpr int CHUNK_SIZE_Z = 16;
    constexpr int CHUNK_SIZE_Y = 16;
    
    class Chunk {
        public:
            Chunk(int cx, int cz);
            ~Chunk();
    
            int cx() const noexcept { return m_cx; }
            int cz() const noexcept { return m_cz; }
            
            uint8_t get_block(int x, int y, int z) const noexcept;
            void set_block(int x, int y, int z, uint8_t id) noexcept;

            static void addFace(
                std::vector<gfx::ChunkVertex>& vertices, 
                std::vector<uint32_t>& indices, 
                int x, int y, int z, 
                FaceDirection dir, 
                uint32_t& vertex_offset,
                uint8_t blockID
            );

            uint8_t get_safe_block(const Chunk& chunk, int x, int y, int z) const noexcept;
    
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