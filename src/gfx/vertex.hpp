#pragma once

namespace gfx {
    struct ChunkVertex { 
        float x, y, z;
        float u, v;
        float faceID;
        float blockID;
    };
} // namespace gfx