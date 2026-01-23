#pragma once
namespace gfx {
    struct ChunkVertex { 
        float x, y, z;
        float u, v;
        float faceID;
        float blockID;
    };

    struct MeshData {
        int cx, cz;
        std::vector<ChunkVertex> vertices;
        std::vector<uint32_t> indices;
    };
} // namespace gfx