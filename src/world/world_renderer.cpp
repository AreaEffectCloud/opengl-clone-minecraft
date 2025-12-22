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

        printf("[WorldRenderer] Generating combined mesh for world...\n");
        for (int cx = 0; cx < 4; cx++) {
            for (int cz = 0; cz < 4; cz++) {

                // For simplicity, only upload blocks from the spawn chunk
                for (int y = 0; y < CHUNK_SIZE_Y; y++) {
                    for (int z = 0; z < CHUNK_SIZE_Z; z++) {
                        for (int x = 0; x < CHUNK_SIZE_X; x++) {

                            int wx = cx * CHUNK_SIZE_X + x;
                            int wz = cz * CHUNK_SIZE_Z + z;

                            uint8_t blockID = static_cast<uint8_t>(world.get_block(wx, y, wz));
                            if (blockID == 0) continue;
        
                            if (y + 1 >= CHUNK_SIZE_Y || world.get_block(wx, y + 1, wz) == BlockID::AIR) {
                                Chunk::addFace(vertices, indices, wx, y, wz, FaceDirection::TOP, vertex_offset, blockID);
                            }
                            if (y - 1 < 0 || world.get_block(wx, y - 1, wz) == BlockID::AIR) {
                                Chunk::addFace(vertices, indices, wx, y, wz, FaceDirection::BOTTOM, vertex_offset, blockID);
                            }
                            if (world.get_block(wx, y, wz + 1) == BlockID::AIR) {
                                Chunk::addFace(vertices, indices, wx, y, wz, FaceDirection::SIDE_FRONT, vertex_offset, blockID);
                            }
                            if (world.get_block(wx, y, wz - 1) == BlockID::AIR) {
                                Chunk::addFace(vertices, indices, wx, y, wz, FaceDirection::SIDE_BACK, vertex_offset, blockID);
                            }
                            if (world.get_block(wx + 1, y, wz) == BlockID::AIR) {
                                Chunk::addFace(vertices, indices, wx, y, wz, FaceDirection::SIDE_RIGHT, vertex_offset, blockID);
                            }
                            if (world.get_block(wx - 1, y, wz) == BlockID::AIR) {
                                Chunk::addFace(vertices, indices, wx, y, wz, FaceDirection::SIDE_LEFT, vertex_offset, blockID);
                            }
                        }
                    }
                }
            }
        }
        m_renderer.update_mesh(vertices, indices);
        std::printf("[WorldRenderer] Total Mesh: %zu vertices, %zu faces\n", vertices.size(), (int)(indices.size() / 6));
    }

    void WorldRenderer::draw(const float* viewProj4x4, const glm::vec3& camPos) {
        m_renderer.draw(viewProj4x4, camPos);
    }
} // namespace ocm