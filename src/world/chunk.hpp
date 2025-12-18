#pragma once

#include <cstdint>
#include <vector>
#include <memory>

namespace ocm {

    constexpr int CHUNK_SIZE_X = 16;
    constexpr int CHUNK_SIZE_Z = 16;
    constexpr int CHUNK_SIZE_Y = 8;
    
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