#include "world_renderer.hpp"
#include <cstdio>

namespace ocm {
    WorldRenderer::WorldRenderer() = default;
    WorldRenderer::~WorldRenderer() = default;

    bool WorldRenderer::init() {
        return m_renderer.init();
    }

    void WorldRenderer::upload_world(const World& world) {
        std::vector<gfx::ChunkVertex> vertices;
        std::vector<uint32_t> indices;
        uint32_t vertex_offset = 0; 

        // For simplicity, only upload blocks from the spawn chunk
        for (int y = 0; y < CHUNK_SIZE_Y; ++y) {
            for (int z = 0; z < CHUNK_SIZE_Z; ++z) {
                for (int x = 0; x < CHUNK_SIZE_X; ++x) {
                    uint8_t blockID = static_cast<uint8_t>(world.get_block(x, y, z));

                    if (blockID == 0) continue;

                    if (y + 1 >= CHUNK_SIZE_Y || world.get_block(x, y + 1, z) == BlockID::AIR) {
                        Chunk::addFace(vertices, indices, x, y, z, FaceDirection::TOP, vertex_offset, blockID);
                    }
                    if (y - 1 < 0 || world.get_block(x, y - 1, z) == BlockID::AIR) {
                        Chunk::addFace(vertices, indices, x, y, z, FaceDirection::BOTTOM, vertex_offset, blockID);
                    }
                    if (z + 1 >= CHUNK_SIZE_Z || world.get_block(x, y, z + 1) == BlockID::AIR) {
                        Chunk::addFace(vertices, indices, x, y, z, FaceDirection::SIDE_FRONT, vertex_offset, blockID);
                    }
                    if (z - 1 < 0 || world.get_block(x, y, z - 1) == BlockID::AIR) {
                        Chunk::addFace(vertices, indices, x, y, z, FaceDirection::SIDE_BACK, vertex_offset, blockID);
                    }
                    if (x + 1 >= CHUNK_SIZE_X || world.get_block(x + 1, y, z) == BlockID::AIR) {
                        Chunk::addFace(vertices, indices, x, y, z, FaceDirection::SIDE_RIGHT, vertex_offset, blockID);
                    }
                    if (x - 1 < 0 || world.get_block(x - 1, y, z) == BlockID::AIR) {
                        Chunk::addFace(vertices, indices, x, y, z, FaceDirection::SIDE_LEFT, vertex_offset, blockID);
                    }
                }
            }
        }
        m_renderer.update_mesh(vertices, indices);
        std::printf("[WorldRenderer] Generated mesh with %zu vertices and %zu indices\n", vertices.size(), indices.size());
    }

    void WorldRenderer::draw(const float* viewProj4x4) {
        m_renderer.draw(viewProj4x4);
    }
} // namespace ocm