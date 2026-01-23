#include "chunk.hpp"
#include "../gfx/vertex.hpp"
#include "../block/block.hpp"
#include <cstring>
#include <algorithm>
#include <GLFW/glfw3.h>

namespace ocm {
    Chunk::Chunk(int cx, int cz)
        : m_cx(cx), m_cz(cz), is_dirty(true), is_meshing(false),
          vao(0), vbo(0), ebo(0), indexCount(0),
          trans_vao(0), trans_vbo(0), trans_ebo(0), trans_indexCount(0) {
        // ブロックデータを空気(0)で初期化
        std::memset(m_blocks, 0, sizeof(m_blocks));
    }
    
    Chunk::~Chunk() {
        if (vao != 0) {
            glDeleteVertexArrays(1, &vao);
            glDeleteBuffers(1, &vbo);
            glDeleteBuffers(1, &ebo);
        }
        if (trans_vao != 0) {
            glDeleteVertexArrays(1, &trans_vao);
            glDeleteBuffers(1, &trans_vbo);
            glDeleteBuffers(1, &trans_ebo);
        }
    };

    uint8_t Chunk::get_block(int x, int y, int z) const {
        if (x < 0 || x >= CHUNK_SIZE_X || y < 0 || y >= CHUNK_SIZE_Y || z < 0 || z >= CHUNK_SIZE_Z) return 0; // AIR
        return m_blocks[get_index(x, y, z)];
    }
    
    void Chunk::set_block(int x, int y, int z, uint8_t id) {
        if (x < 0 || x >= CHUNK_SIZE_X || y < 0 || y >= CHUNK_SIZE_Y || z < 0 || z >= CHUNK_SIZE_Z) return;

        int idx = get_index(x, y, z);
        if (m_blocks[idx] != id) {
            m_blocks[idx] = id;
            is_dirty = true;
        }
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

        // 水面を下げる
        float yoffset = 0.0f;
        if (blockID == static_cast<uint8_t>(BlockID::WATER)) {
            yoffset = -0.1f;
        }

        // Determine the faceID
        float fID = static_cast<float>(dir);
        
        float textureLayer = 0.0f;
        if (blockID == 1) { // DIRT
            textureLayer = 1.0f;
        } else if (blockID == 2) { // GRASS_BLOCK
            if (dir == TOP) textureLayer = 2.0f;
            else if (dir == BOTTOM) textureLayer = 1.0f;
            else textureLayer = 3.0f;
        } else if (blockID == 3) { // SAND
            textureLayer = 4.0f;
        } else if (blockID == 4) { // STONE
            textureLayer = 5.0f;
        } else if (blockID == 5) { // COBBLESTONE
            textureLayer = 6.0f;
        } else if (blockID == 6) { // WATER
            textureLayer = 7.0f;
        } else if (blockID == 7) { // LOG
            textureLayer = 8.0f;
        } else {
            textureLayer = 0.0f; // Placeholder
        }

        // Define the 4 vertices for each face based on direction
        switch (dir) {
            case TOP: // Y+
                vertices.push_back({fx,   fy+1 + yoffset, fz+1, 0, 0, fID, textureLayer});
                vertices.push_back({fx+1, fy+1 + yoffset, fz+1, 1, 0, fID, textureLayer});
                vertices.push_back({fx+1, fy+1 + yoffset, fz,   1, 1, fID, textureLayer});
                vertices.push_back({fx,   fy+1 + yoffset, fz,   0, 1, fID, textureLayer});
                break;
            case BOTTOM: // Y-
                vertices.push_back({fx,   fy, fz,   0, 0, fID, textureLayer});
                vertices.push_back({fx+1, fy, fz,   1, 0, fID, textureLayer});
                vertices.push_back({fx+1, fy, fz+1, 1, 1, fID, textureLayer});
                vertices.push_back({fx,   fy, fz+1, 0, 1, fID, textureLayer});
                break;
            case SIDE_FRONT: // Z+
                vertices.push_back({fx,   fy,   fz+1, 0, 1, fID, textureLayer});
                vertices.push_back({fx+1, fy,   fz+1, 1, 1, fID, textureLayer});
                vertices.push_back({fx+1, fy+1 + yoffset, fz+1, 1, 0, fID, textureLayer});
                vertices.push_back({fx,   fy+1 + yoffset, fz+1, 0, 0, fID, textureLayer});
                break;
            case SIDE_BACK: // Z-
                vertices.push_back({fx+1, fy,   fz,   0, 1, fID, textureLayer});
                vertices.push_back({fx,   fy,   fz,   1, 1, fID, textureLayer});
                vertices.push_back({fx,   fy+1 + yoffset, fz,   1, 0, fID, textureLayer});
                vertices.push_back({fx+1, fy+1 + yoffset, fz,   0, 0, fID, textureLayer});
                break;
            case SIDE_RIGHT: // X+
                vertices.push_back({fx+1, fy,   fz+1, 0, 1, fID, textureLayer});
                vertices.push_back({fx+1, fy,   fz,   1, 1, fID, textureLayer});
                vertices.push_back({fx+1, fy+1 + yoffset, fz,   1, 0, fID, textureLayer});
                vertices.push_back({fx+1, fy+1 + yoffset, fz+1, 0, 0, fID, textureLayer});
                break;
            case SIDE_LEFT: // X-
                vertices.push_back({fx,   fy,   fz,   0, 1, fID, textureLayer});
                vertices.push_back({fx,   fy,   fz+1, 1, 1, fID, textureLayer});
                vertices.push_back({fx,   fy+1 + yoffset, fz+1, 1, 0, fID, textureLayer});
                vertices.push_back({fx,   fy+1 + yoffset, fz,   0, 0, fID, textureLayer});
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
} // namespace ocm