#pragma once

#include <vector>
#include <cstdint>
#include "../gfx/cube_renderer.hpp"
#include "chunk.hpp"
#include "world.hpp"

namespace ocm {

    class WorldRenderer {
        public:
            WorldRenderer();
            ~WorldRenderer();

            bool init();

            void upload_world(const World& world);

            void draw(const float* viewProj4x4);

        private:
            gfx::CubeRenderer m_renderer;
            std::vector<gfx::Vec3f> m_positions;
    };
} // namespace ocm