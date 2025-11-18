#ifndef MESHBUILDER_H
#define MESHBUILDER_H

#include "world.h"
#include "block.h"
#include "direction.h"
#include <vector>
#include <glm/glm.hpp>
#include <cglm/types-struct.h>
#include <cmath>

#include <iostream>

#define CHUNK_X CHUNK_SIZE_X
#define CHUNK_Y CHUNK_SIZE_Y
#define CHUNK_Z CHUNK_SIZE_Z

struct MeshData {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
};

class MeshBuilder {
    public:
        static MeshData build(const World& world) {
            MeshData mesh;
            unsigned int indexOffset = 0;

            for (int x = 0; x < CHUNK_SIZE_X; ++x) {
                for (int y = 0; y < CHUNK_SIZE_Y; ++y) {
                    for (int z = 0; z < CHUNK_SIZE_Z; ++z) {
                        BlockType currentBlock = world.getBlock(x, y, z);
                        if (currentBlock != BlockType::AIR) {
                            // face culling
                            for (int d = 0; d < 6; ++d) {
                                ivec3s dirVec = DIR2IVEC3S(d);
                                int nx = x + dirVec.x;
                                int ny = y + dirVec.y;
                                int nz = z + dirVec.z;

                                if (world.getBlock(nx, ny, nz) == BlockType::AIR) {
                                    addFace(mesh, currentBlock, (Direction)d, x, y, z, indexOffset);
                                }
                            }
                        }
                    }
                }
            }
            std::cout << "Mesh built: " << (mesh.vertices.size() / 8) << " vertices, " 
                << mesh.indices.size() << " indices generated (Culling applied)." << std::endl;
            return mesh;
        }
    private:
        static void addFace(MeshData& mesh, BlockType type, Direction dir, int x, int y, int z, unsigned int& offset) {
            // この頂点配列は、ブロックのテクスチャUVW座標と一致するよう定義
            // (x, y, z, r, g, b, u, v) の形式で8要素
            float faceVertices[32];

            // テクスチャがない場合の予備
            glm::vec3 color;
            if (type == BlockType::STONE) color = glm::vec3(0.5f, 0.5f, 0.5f);
            else if (type == BlockType::DIRT) color = glm::vec3(0.65f, 0.45f, 0.25f);
            else color = glm::vec3(1.0f, 0.0f, 1.0f);

            // 頂点とUVの配列の初期化
            float v[32];
            
            // counter clock wise
            switch (dir) { // -Z (奥面)
                case NORTH: {
                    float verts[] = {
                         0.5f,  0.5f, -0.5f,   color.r, color.g, color.b,   0.0f, 0.0f, // 0: 右下奥 (0,0)
                         0.5f, -0.5f, -0.5f,   color.r, color.g, color.b,   0.0f, 1.0f, // 1: 右上奥 (0,1)
                        -0.5f, -0.5f, -0.5f,   color.r, color.g, color.b,   1.0f, 1.0f, // 2: 左上奥 (1,1)
                        -0.5f,  0.5f, -0.5f,   color.r, color.g, color.b,   1.0f, 0.0f  // 3: 左下奥 (1,0)
                    };
                    memcpy(v, verts, sizeof(verts));
                    break;
                }
                case SOUTH: { // +Z (手前面)
                    float verts[] = {
                        -0.5f,  0.5f,  0.5f,   color.r, color.g, color.b,   0.0f, 0.0f, // 0: 左下前 (0,0)
                        -0.5f, -0.5f,  0.5f,   color.r, color.g, color.b,   0.0f, 1.0f, // 1: 左上前 (0,1)
                         0.5f, -0.5f,  0.5f,   color.r, color.g, color.b,   1.0f, 1.0f, // 2: 右上前 (1,1)
                         0.5f,  0.5f,  0.5f,   color.r, color.g, color.b,   1.0f, 0.0f  // 3: 右下前 (1,0)
                    };
                    memcpy(v, verts, sizeof(verts));
                    break;
                }
                case EAST: { // +X (右面)
                    float verts[] = {
                         0.5f,  0.5f,  0.5f,   color.r, color.g, color.b,   0.0f, 0.0f, // 0: 下前
                         0.5f, -0.5f,  0.5f,   color.r, color.g, color.b,   0.0f, 1.0f, // 1: 上前
                         0.5f, -0.5f, -0.5f,   color.r, color.g, color.b,   1.0f, 1.0f, // 2: 上奥
                         0.5f,  0.5f, -0.5f,   color.r, color.g, color.b,   1.0f, 0.0f  // 3: 下奥
                    };
                    memcpy(v, verts, sizeof(verts));
                    break;
                }
                case WEST: { // -X (左面)
                    float verts[] = {
                        -0.5f,  0.5f, -0.5f,   color.r, color.g, color.b,   0.0f, 0.0f, // 0: 下奥
                        -0.5f, -0.5f, -0.5f,   color.r, color.g, color.b,   0.0f, 1.0f, // 1: 上奥
                        -0.5f, -0.5f,  0.5f,   color.r, color.g, color.b,   1.0f, 1.0f, // 2: 上前
                        -0.5f,  0.5f,  0.5f,   color.r, color.g, color.b,   1.0f, 0.0f  // 3: 下前
                    };
                    memcpy(v, verts, sizeof(verts));
                    break;
                }
                case UP: { // +Y (天井面)
                    float verts[] = {
                        -0.5f,  0.5f, -0.5f,   color.r, color.g, color.b,   0.0f, 0.0f, // 0: 左奥
                        -0.5f,  0.5f,  0.5f,   color.r, color.g, color.b,   0.0f, 1.0f, // 1: 左前
                         0.5f,  0.5f,  0.5f,   color.r, color.g, color.b,   1.0f, 1.0f, // 2: 右前
                         0.5f,  0.5f, -0.5f,   color.r, color.g, color.b,   1.0f, 0.0f  // 3: 右奥
                    };
                    memcpy(v, verts, sizeof(verts));
                    break;
                }
                case DOWN: { // -Y (底面)
                    float verts[] = {
                        -0.5f, -0.5f,  0.5f,   color.r, color.g, color.b,   0.0f, 0.0f, // 0: 左前
                        -0.5f, -0.5f, -0.5f,   color.r, color.g, color.b,   0.0f, 1.0f, // 1: 左奥
                         0.5f, -0.5f, -0.5f,   color.r, color.g, color.b,   1.0f, 1.0f, // 2: 右奥
                         0.5f, -0.5f,  0.5f,   color.r, color.g, color.b,   1.0f, 0.0f  // 3: 右前
                    };
                    memcpy(v, verts, sizeof(verts));
                    break;
                }
            }

            // (x, y, z)のオフセットを適用
            for (int i = 0; i < 4; ++i) {
                v[i * 8 + 0] += (float)x;
                v[i * 8 + 1] += (float)y;
                v[i * 8 + 2] += (float)z;
            }

            // 生成した頂点データをメッシュに格納
            for (int i = 0; i < 32; ++i) {
                mesh.vertices.push_back(v[i]);
            }

            unsigned int indicesToAdd[] = {
                offset + 0, offset + 1, offset + 2,
                offset + 2, offset + 3, offset + 0
            };
            for (int i = 0; i <6; ++i) {
                mesh.indices.push_back(indicesToAdd[i]);
            }

            offset += 4;
        }
};

#endif