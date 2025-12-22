#pragma once
#include <cstdint>

namespace ocm {

    enum class BlockID : uint8_t {
        AIR = 0,
        DIRT = 1,
        GRASS = 2,
        STONE = 3,
        LOG = 4,
        PLANKS = 5,
        LEAVES = 6,
        WATER = 7,
        SAND = 8
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
