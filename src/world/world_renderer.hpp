#pragma once

#include <vector>
#include <cstdint>
#include "chunk.hpp"
#include "world.hpp"
#include "../gfx/cube_renderer.hpp"

namespace ocm {
    class WorldRenderer {
        public:
            WorldRenderer();
            ~WorldRenderer();

            bool init();

            // Dirtyなチャンクのメッシュ構築と描画を行う
            void render(const World& world, const glm::vec3& camPos, const glm::mat4& viewProj, int viewDistance);

        private:
            gfx::CubeRenderer m_renderer;
            // 特定のチャンクのメッシュを構築・更新
            void update_single_chunk_mesh(const World& world, Chunk& chunk);
    };
} // namespace ocm