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

        // 1. 不透明ブロック
        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
        glEnable(GL_DEPTH_TEST);

        for (Chunk* chunk : visibleChunks) {
            // VAOが作成されていない(一度もメッシュ計算が終わっていない)チャンクはスキップ
            if (chunk->vao != 0 || chunk->indexCount != 0) {
                m_cubeRenderer.draw_chunk(*chunk);
            }
        }

        // 2. 半透明ブロック
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        // 水同士の重なりで消えないよう、深層書き込みを無効化
        glDepthMask(GL_FALSE);

        for (Chunk* chunk : visibleChunks) {
            if (chunk->trans_vao != 0 || chunk->trans_indexCount != 0) {
                m_cubeRenderer.draw_chunk_transparent(*chunk);
            }
        }

        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
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
                m_cubeRenderer.update_chunk_mesh(*chunk, data);
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
        uint32_t opaque_vertex_offset = 0;
        uint32_t transparent_vertex_offset = 0;

        // チャンクを取得
        Chunk* chunk = world.get_chunk_ptr(cx, cz);
        if (!chunk) return result;

        for (int y = 0; y < CHUNK_SIZE_Y; y++) {
            for (int z = 0; z < CHUNK_SIZE_Z; z++) {
                for (int x = 0; x < CHUNK_SIZE_X; x++) {
                    BlockID block = static_cast<BlockID>(chunk->get_block(x, y, z));
                    if (block == BlockID::AIR) continue; // AIR

                    int wx = cx * CHUNK_SIZE_X + x;
                    int wy = y;
                    int wz = cz * CHUNK_SIZE_Z + z;

                    bool is_water = (block == BlockID::WATER);

                    // 格納先の参照を切り替える
                    auto& target_vertices = is_water ? result.trans_vertices : result.opaque_vertices;
                    auto& target_indices = is_water ? result.trans_indices : result.opaque_indices;
                    auto& target_offset = is_water ? transparent_vertex_offset : opaque_vertex_offset;

                    auto shuold_add_face = [&](int nx, int ny, int nz, BlockID id) {
                        BlockID neighbor = world.get_block(nx, ny, nz);

                        // 隣が空気なら描画
                        if (neighbor == BlockID::AIR) return true; // AIR

                        // 自身がサボテン
                        if (id == BlockID::CACTUS) {
                            // 隣がサボテンなら描画しない
                            if (neighbor == BlockID::CACTUS) return false;
                            return true;
                        }

                        // 不透明ブロック
                        if (world.is_opaque(wx, wy, wz)) {
                            // 隣が空気・水・サボテン・葉なら描画
                            if (neighbor == BlockID::WATER || neighbor == BlockID::CACTUS || neighbor == BlockID::LEAVES) {
                                return true;
                            }
                            // 隣が不透明ブロックなら描画しない
                            return world.is_opaque(nx, ny, nz) ? false : true;
                        }

                        // 自身が葉
                        if (id == BlockID::LEAVES) {
                            // 隣が空気や水なら描画
                            return (neighbor == BlockID::AIR || neighbor == BlockID::WATER);
                        }


                        if (is_water) {
                            return false;
                        } else {
                            return (neighbor == BlockID::WATER);
                        }
                        
                        return world.is_opaque(nx, ny, nz);
                    };
    
                    if (shuold_add_face(wx, wy + 1, wz, block)) {
                        Chunk::add_face(target_vertices, target_indices, x, y, z, FaceDirection::TOP, target_offset, static_cast<uint8_t>(block));
                    }
                    if (wy > 0 && shuold_add_face(wx, wy - 1, wz, block)) {
                        Chunk::add_face(target_vertices, target_indices, x, y, z, FaceDirection::BOTTOM, target_offset, static_cast<uint8_t>(block));
                    }
                    if (shuold_add_face(wx, wy, wz + 1, block)) {
                        Chunk::add_face(target_vertices, target_indices, x, y, z, FaceDirection::SIDE_FRONT, target_offset, static_cast<uint8_t>(block));
                    }
                    if (shuold_add_face(wx, wy, wz - 1, block)) {
                        Chunk::add_face(target_vertices, target_indices, x, y, z, FaceDirection::SIDE_BACK, target_offset, static_cast<uint8_t>(block));
                    }
                    if (shuold_add_face(wx + 1, wy, wz, block)) {
                        Chunk::add_face(target_vertices, target_indices, x, y, z, FaceDirection::SIDE_RIGHT, target_offset, static_cast<uint8_t>(block));
                    }
                    if (shuold_add_face(wx - 1, wy, wz, block)) {
                        Chunk::add_face(target_vertices, target_indices, x, y, z, FaceDirection::SIDE_LEFT, target_offset, static_cast<uint8_t>(block));
                    }
                }
            }
        }
        return result;
    }
} // namespace ocm