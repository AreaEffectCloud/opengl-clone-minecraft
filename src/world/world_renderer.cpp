#include "world_renderer.hpp"
#include <cstdio>

namespace ocm {
    WorldRenderer::WorldRenderer() = default;
    WorldRenderer::~WorldRenderer() = default;

    bool WorldRenderer::init() {
        return m_renderer.init();
    }

    void WorldRenderer::upload_world(const World& world) {
        m_positions.clear();
        // For simplicity, only upload blocks from the spawn chunk
        int base_cx = 0;
        int base_cz = 0;
        for (int z = 0; z < CHUNK_SIZE_Z; ++z) {
            for (int x = 0; x < CHUNK_SIZE_X; ++x) {
                for (int y = 0; y < CHUNK_SIZE_Y; ++y) {
                    int world_x = base_cx * CHUNK_SIZE_X + x;
                    int world_z = base_cz * CHUNK_SIZE_Z + z;
                    auto id = world.get_block(world_x, y, world_z);
                    if (id != BlockID::AIR) {
                        gfx::Vec3f pos;
                        pos.x = (float)world_x;
                        pos.y = (float)y;
                        pos.z = (float)world_z;
                        m_positions.push_back(pos);
                    }
                }
            }
        }
        std::printf("[WorldRenderer] uploaded %zu instances\n", m_positions.size());
        m_renderer.update_instances(m_positions);
    }

    void WorldRenderer::draw(const float* viewProj4x4) {
        m_renderer.draw(viewProj4x4);
    }
} // namespace ocm