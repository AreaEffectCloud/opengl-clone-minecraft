#include "world_renderer.hpp"
#include <cstdio>

namespace ocm {
    WorldRenderer::WorldRenderer() = default;
    WorldRenderer::~WorldRenderer() = default;

    bool WorldRenderer::init() {
        return m_renderer.init();
    }

    void WorldRenderer::render(const World& world, const glm::vec3& camPos, const glm::mat4& viewProj, int viewDistance) {
        std::vector<Chunk*> visibleChunks = const_cast<World&>(world).get_visible_chunks(camPos, viewDistance);

        for (Chunk* chunk : visibleChunks) {
            if (chunk->isDirty) {
                update_single_chunk_mesh(world, *chunk);
                chunk->isDirty = false;
            }
        }
        m_renderer.setup_frame(glm::value_ptr(viewProj), camPos);

        // 各チャンクを個別に描画
        for (Chunk* chunk : visibleChunks) {
            m_renderer.draw_chunk(*chunk);
        }
    }

    void WorldRenderer::update_single_chunk_mesh(const World& world, Chunk& chunk) {
        std::vector<gfx::ChunkVertex> vertices;
        std::vector<uint32_t> indices;
        uint32_t vertex_offset = 0;

        int cx = chunk.cx();;
        int cz = chunk.cz();

        for (int y = 0; y < CHUNK_SIZE_Y; y++) {
            for (int z = 0; z < CHUNK_SIZE_Z; z++) {
                for (int x = 0; x < CHUNK_SIZE_X; x++) {

                    int wx = cx * CHUNK_SIZE_X + x;
                    int wz = cz * CHUNK_SIZE_Z + z;

                    uint8_t blockID = static_cast<uint8_t>(world.get_block(wx, y, wz));
                    if (blockID == 0) continue;
    
                    if (y + 1 >= CHUNK_SIZE_Y || world.get_block(wx, y + 1, wz) == BlockID::AIR) {
                        Chunk::add_face(vertices, indices, x, y, z, FaceDirection::TOP, vertex_offset, blockID);
                    }
                    if (y - 1 < 0 || world.get_block(wx, y - 1, wz) == BlockID::AIR) {
                        Chunk::add_face(vertices, indices, x, y, z, FaceDirection::BOTTOM, vertex_offset, blockID);
                    }
                    if (world.get_block(wx, y, wz + 1) == BlockID::AIR) {
                        Chunk::add_face(vertices, indices, x, y, z, FaceDirection::SIDE_FRONT, vertex_offset, blockID);
                    }
                    if (world.get_block(wx, y, wz - 1) == BlockID::AIR) {
                        Chunk::add_face(vertices, indices, x, y, z, FaceDirection::SIDE_BACK, vertex_offset, blockID);
                    }
                    if (world.get_block(wx + 1, y, wz) == BlockID::AIR) {
                        Chunk::add_face(vertices, indices, x, y, z, FaceDirection::SIDE_RIGHT, vertex_offset, blockID);
                    }
                    if (world.get_block(wx - 1, y, wz) == BlockID::AIR) {
                        Chunk::add_face(vertices, indices, x, y, z, FaceDirection::SIDE_LEFT, vertex_offset, blockID);
                    }
                }
            }
        }
        m_renderer.update_chunk_mesh(chunk, vertices, indices);
        // std::printf("[WorldRenderer] Total Mesh: %zu vertices, %zu faces\n", vertices.size(), (int)(indices.size() / 6));
    }
} // namespace ocm