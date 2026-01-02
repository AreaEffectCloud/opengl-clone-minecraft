#include "chunk.hpp"
#include <cstring>

namespace ocm {
    Chunk::Chunk(int cx, int cz)
        : m_cx(cx), m_cz(cz), m_blocks(static_cast<size_t>(CHUNK_SIZE_X) * CHUNK_SIZE_Y * CHUNK_SIZE_Z, 0) {
    }
    
    Chunk::~Chunk() {
        destroy_gl_resources();
    };

    void Chunk::destroy_gl_resources() {
        if (vao != 0) {
            glDeleteVertexArrays(1, &vao);
            glDeleteBuffers(1, &vbo);
            glDeleteBuffers(1, &ebo);
            vao = vbo = ebo = 0;
        }
    }

    uint8_t Chunk::get_block(int x, int y, int z) const noexcept {
        if (x < 0 || x >= CHUNK_SIZE_X || y < 0 || y >= CHUNK_SIZE_Y || z < 0 || z >= CHUNK_SIZE_Z) return 0; // AIR
        return m_blocks[index(x, y, z)];
    }
    
    void Chunk::set_block(int x, int y, int z, uint8_t id) noexcept {
        if (x < 0 || x >= CHUNK_SIZE_X || y < 0 || y >= CHUNK_SIZE_Y || z < 0 || z >= CHUNK_SIZE_Z) return;
        m_blocks[index(x, y, z)] = id;
        isDirty = true;
    }

    void Chunk::add_face(
        std::vector<gfx::ChunkVertex>& vertices, 
        std::vector<uint32_t>& indices, 
        int x, int y, int z, 
        FaceDirection dir, 
        uint32_t& vertex_offset,
        uint8_t blockID
    ) {
        float fx = static_cast<float>(x);
        float fy = static_cast<float>(y);
        float fz = static_cast<float>(z);

        // Determine the faceID
        float fID = static_cast<float>(dir);
        float fBlockID = static_cast<float>(blockID);

        // Define the 4 vertices for each face based on direction
        switch (dir) {
            case TOP: // Y+
                vertices.push_back({fx,   fy+1, fz+1, 0, 0, fID, fBlockID});
                vertices.push_back({fx+1, fy+1, fz+1, 1, 0, fID, fBlockID});
                vertices.push_back({fx+1, fy+1, fz,   1, 1, fID, fBlockID});
                vertices.push_back({fx,   fy+1, fz,   0, 1, fID, fBlockID});
                break;
            case BOTTOM: // Y-
                vertices.push_back({fx,   fy, fz,   0, 0, fID, fBlockID});
                vertices.push_back({fx+1, fy, fz,   1, 0, fID, fBlockID});
                vertices.push_back({fx+1, fy, fz+1, 1, 1, fID, fBlockID});
                vertices.push_back({fx,   fy, fz+1, 0, 1, fID, fBlockID});
                break;
            case SIDE_FRONT: // Z+
                vertices.push_back({fx,   fy,   fz+1, 0, 0, fID, fBlockID});
                vertices.push_back({fx+1, fy,   fz+1, 1, 0, fID, fBlockID});
                vertices.push_back({fx+1, fy+1, fz+1, 1, 1, fID, fBlockID});
                vertices.push_back({fx,   fy+1, fz+1, 0, 1, fID, fBlockID});
                break;
            case SIDE_BACK: // Z-
                vertices.push_back({fx+1, fy,   fz,   0, 0, fID, fBlockID});
                vertices.push_back({fx,   fy,   fz,   1, 0, fID, fBlockID});
                vertices.push_back({fx,   fy+1, fz,   1, 1, fID, fBlockID});
                vertices.push_back({fx+1, fy+1, fz,   0, 1, fID, fBlockID});
                break;
            case SIDE_RIGHT: // X+
                vertices.push_back({fx+1, fy,   fz+1, 0, 0, fID, fBlockID});
                vertices.push_back({fx+1, fy,   fz,   1, 0, fID, fBlockID});
                vertices.push_back({fx+1, fy+1, fz,   1, 1, fID, fBlockID});
                vertices.push_back({fx+1, fy+1, fz+1, 0, 1, fID, fBlockID});
                break;
            case SIDE_LEFT: // X-
                vertices.push_back({fx,   fy,   fz,   0, 0, fID, fBlockID});
                vertices.push_back({fx,   fy,   fz+1, 1, 0, fID, fBlockID});
                vertices.push_back({fx,   fy+1, fz+1, 1, 1, fID, fBlockID});
                vertices.push_back({fx,   fy+1, fz,   0, 1, fID, fBlockID});
                break;
        }

        // Add indexes
        indices.push_back(vertex_offset + 0);
        indices.push_back(vertex_offset + 1);
        indices.push_back(vertex_offset + 2);
        indices.push_back(vertex_offset + 2);
        indices.push_back(vertex_offset + 3);
        indices.push_back(vertex_offset + 0);

        vertex_offset += 4;
    }

    uint8_t get_safe_block(const Chunk& chunk, int x, int y, int z) {
        if (x < 0 || x >= CHUNK_SIZE_X || 
            y < 0 || y >= CHUNK_SIZE_Y || 
            z < 0 || z >= CHUNK_SIZE_Z)  {
                return 0; // チャンク外は空気とみなす
        }
        return chunk.get_block(x, y, z);
    }
} // namespace ocm