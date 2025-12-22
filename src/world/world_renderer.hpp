#pragma once

#include <vector>
#include <cstdint>
#include "chunk.hpp"
#include "world.hpp"

namespace ocm {
    class WorldRenderer {
        public:
            WorldRenderer();
            ~WorldRenderer();

            bool init();

            void upload_world(const World& world);

            void draw(const float* viewProj4x4, const glm::vec3& camPos);

        private:
            gfx::CubeRenderer m_renderer;
    };
} // namespace ocm