#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "../gfx/vertex.hpp"

namespace ocm {
    enum FaceDirection {
        SIDE_FRONT,
        SIDE_BACK,
        TOP,
        BOTTOM,
        SIDE_RIGHT,
        SIDE_LEFT
    };

    constexpr int CHUNK_SIZE_X = 16;
    constexpr int CHUNK_SIZE_Y = 128;
    constexpr int CHUNK_SIZE_Z = 16;
    class Chunk {
        public:
            Chunk(int cx, int cz);
            ~Chunk();

            // OpenGLのリソースID
            uint32_t vao = 0, vbo = 0, ebo = 0;
            int indexCount = 0;

            uint32_t trans_vao = 0, trans_vbo = 0, trans_ebo = 0;
            int trans_indexCount = 0;
 
            // メッシュの再構築が必要か
            bool is_dirty = true;
            void set_dirty(bool dirty) { is_dirty = dirty; }
            
            // スレッドプールでメッシュ計算中か
            bool is_meshing = false;

            // 座標取得
            int cx() const { return m_cx; }
            int cz() const { return m_cz; }
            
            // ブロック操作
            uint8_t get_block(int x, int y, int z) const;
            void set_block(int x, int y, int z, uint8_t id);

            // 面を追加するヘルパー関数
            static void add_face(
                std::vector<gfx::ChunkVertex>& vertices, 
                std::vector<uint32_t>& indices, 
                int x, int y, int z, 
                FaceDirection dir, 
                uint32_t& vertex_offset,
                uint8_t blockID
            );
    
        private:
            int m_cx, m_cz;
            // メモリ効率のため1次元配列
            uint8_t m_blocks[CHUNK_SIZE_X * CHUNK_SIZE_Y * CHUNK_SIZE_Z];
    
            // インデックス計算用のヘルパー
            inline int get_index(int x, int y, int z) const {
                // return x + CHUNK_SIZE_X * (z + CHUNK_SIZE_Z * y);
                return x + (y * CHUNK_SIZE_X) + (z * CHUNK_SIZE_X * CHUNK_SIZE_Y);
            }
    };
    
    using ChunkPtr = std::unique_ptr<Chunk>;
} // namespace ocm