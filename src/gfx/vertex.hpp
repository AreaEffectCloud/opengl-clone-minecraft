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
        std::vector<ChunkVertex> opaque_vertices;
        std::vector<uint32_t> opaque_indices;
        // for transparent blocks (e.g. water)
        std::vector<ChunkVertex> trans_vertices;
        std::vector<uint32_t> trans_indices;
    };
} // namespace gfx