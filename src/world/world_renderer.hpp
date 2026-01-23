#pragma once

#include <vector>
#include <cstdint>
#include <future>
#include <queue>
#include <mutex>
#include "chunk.hpp"
#include "world.hpp"
#include "../gfx/cube_renderer.hpp"
#include "../util/thread_pool.hpp"

namespace ocm {
    class WorldRenderer {
        public:
            WorldRenderer();
            ~WorldRenderer();

            bool init();

            // Dirtyなチャンクのメッシュ構築と描画を行う
            void render(const World& world, const glm::vec3& camPos, const glm::mat4& viewProj, int viewDistance);
            // 特定のチャンクのメッシュを構築・更新
            void update_meshes(const World& world);
            // void update_single_chunk_mesh(const World& world, Chunk& chunk);

        private:
            gfx::CubeRenderer m_cubeRenderer;

            // 実行中の非同期タスクを保持
            std::unique_ptr<util::ThreadPool> m_pool;

            std::queue<gfx::MeshData> m_meshResults; // 計算済みデータの待ち行列
            std::mutex m_resultMutex;                // キュー操作の排他制御

            // 実際に頂点データを組み立てる
            gfx::MeshData build_mesh_data(const World& world, int cx, int cz);
    };
} // namespace ocm