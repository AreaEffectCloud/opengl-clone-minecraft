#include "chunk.hpp"
#include <cstring>

namespace ocm {

    Chunk::Chunk(int cx, int cz)
        : m_cx(cx), m_cz(cz), m_blocks(static_cast<size_t>(CHUNK_SIZE_X) * CHUNK_SIZE_Y * CHUNK_SIZE_Z, 0) {
    }
    
    Chunk::~Chunk() = default;
    
    uint8_t Chunk::get_block(int x, int y, int z) const noexcept {
        if (x < 0 || x >= CHUNK_SIZE_X || y < 0 || y >= CHUNK_SIZE_Y || z < 0 || z >= CHUNK_SIZE_Z) return 0; // AIR
        return m_blocks[index(x, y, z)];
    }
    
    void Chunk::set_block(int x, int y, int z, uint8_t id) noexcept {
        if (x < 0 || x >= CHUNK_SIZE_X || y < 0 || y >= CHUNK_SIZE_Y || z < 0 || z >= CHUNK_SIZE_Z) return;
        m_blocks[index(x, y, z)] = id;
    }
} // namespace ocm