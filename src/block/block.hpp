#pragma once
#include <cstdint>

namespace ocm {

    enum class BlockID : uint8_t {
        AIR = 0,
        STONE = 1,
        DIRT = 2,
        GRASS = 3,
        WATER = 4,
        SAND = 5
    };
}


// struct BlockType {
//     BlockId id;
//     const char* name;
//     bool transparent; // 面を省略する際に参照
//     bool is_liquid;
//     int texture_index; // テクスチャアトラス内のインデックス
// };

// extern const struct BlockType BLOCK_TYPES[];

// static inline bool block_is_transparent(BlockId id) {
//     return BLOCK_TYPES[id].transparent;
// }
