#include "world_renderer.hpp"
#include <algorithm>
#include <cstdio>
#include <future>
namespace ocm {
    WorldRenderer::WorldRenderer() {
        m_pool = std::make_unique<util::ThreadPool>(std::thread::hardware_concurrency());
    }
    WorldRenderer::~WorldRenderer() = default;

    bool WorldRenderer::init() {
        return m_cubeRenderer.init();
    }

    void WorldRenderer::render(const World& world, const glm::vec3& camPos, const glm::mat4& viewProj, int viewDistance) {
        // 描画対象のチャンクを取得
        std::vector<Chunk*> visibleChunks = const_cast<World&>(world).get_visible_chunks(camPos, viewDistance);
        // メッシュの非同期更新リクエストと結果の回収
        update_meshes(const_cast<World&>(world));
        // シェーダのグローバル設定
        m_cubeRenderer.setup_frame(glm::value_ptr(viewProj), camPos);
        // 各チャンクを描画
        for (Chunk* chunk : visibleChunks) {
            // VAOが作成されていない(一度もメッシュ計算が終わっていない)チャンクをスキップ
            if (chunk->vao == 0 || chunk->indexCount == 0) continue;

            // 描画
            m_cubeRenderer.draw_chunk(*chunk);
        }
    }

    void WorldRenderer::update_meshes(const World& world) {

        // メッシュ更新が必要なチャンクを探して更新
        std::queue<gfx::MeshData> resultsToUpload;
        {
            std::lock_guard<std::mutex> lock(m_resultMutex);
            while (!m_meshResults.empty()) {
                resultsToUpload.push(std::move(m_meshResults.front()));
                m_meshResults.pop();
            }
        }
        
        while (!resultsToUpload.empty()) {
            auto& data = resultsToUpload.front();
            Chunk* chunk = world.get_chunk_ptr(data.cx, data.cz);
            if (chunk) {
                m_cubeRenderer.update_chunk_mesh(*chunk, data.vertices, data.indices);
                chunk->is_meshing = false;
            }
            resultsToUpload.pop();
        }
        
        // 新しいタスクの発行
        for (auto* chunk : world.get_all_chunks_raw_ptr()) {
            if (chunk->is_dirty && !chunk->is_meshing) {
                chunk->is_dirty = false;
                chunk->is_meshing = true;

                int cx = chunk->cx();
                int cz = chunk->cz();

                m_pool->enqueue([this, &world, cx, cz]() {
                    gfx::MeshData result = this->build_mesh_data(world, cx, cz);

                    // 結果を安全に格納
                    std::lock_guard<std::mutex> lock(this->m_resultMutex);
                    this->m_meshResults.push(std::move(result));
                });
            }
        }
    }
    
    gfx::MeshData WorldRenderer::build_mesh_data(const World& world, int cx, int cz) {
        gfx::MeshData result;
        result.cx = cx;
        result.cz = cz;
        uint32_t vertex_offset = 0;

        // チャンクを取得
        Chunk* chunk = world.get_chunk_ptr(cx, cz);
        if (!chunk) return result;

        for (int y = 0; y < CHUNK_SIZE_Y; y++) {
            for (int z = 0; z < CHUNK_SIZE_Z; z++) {
                for (int x = 0; x < CHUNK_SIZE_X; x++) {
                    uint8_t block = chunk->get_block(x, y, z);
                    if (block == 0) continue; // AIR

                    int wx = cx * CHUNK_SIZE_X + x;
                    int wy = y;
                    int wz = cz * CHUNK_SIZE_Z + z;
    
                    if (!world.is_opaque(wx, wy + 1, wz)) {
                        Chunk::add_face(result.vertices, result.indices, x, y, z, FaceDirection::TOP, vertex_offset, block);
                    }
                    if (wy > 0 && !world.is_opaque(wx, wy - 1, wz)) {
                        Chunk::add_face(result.vertices, result.indices, x, y, z, FaceDirection::BOTTOM, vertex_offset, block);
                    }
                    if (!world.is_opaque(wx, wy, wz + 1)) {
                        Chunk::add_face(result.vertices, result.indices, x, y, z, FaceDirection::SIDE_FRONT, vertex_offset, block);
                    }
                    if (!world.is_opaque(wx, wy, wz - 1)) {
                        Chunk::add_face(result.vertices, result.indices, x, y, z, FaceDirection::SIDE_BACK, vertex_offset, block);
                    }
                    if (!world.is_opaque(wx + 1, wy, wz)) {
                        Chunk::add_face(result.vertices, result.indices, x, y, z, FaceDirection::SIDE_RIGHT, vertex_offset, block);
                    }
                    if (!world.is_opaque(wx - 1, wy, wz)) {
                        Chunk::add_face(result.vertices, result.indices, x, y, z, FaceDirection::SIDE_LEFT, vertex_offset, block);
                    }
                }
            }
        }
        return result;
    }
} // namespace ocm