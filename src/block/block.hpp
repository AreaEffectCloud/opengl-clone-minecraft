#pragma once
#include <cstdint>

namespace ocm {
    enum class BlockID : uint8_t {
        AIR = 0,
        DIRT = 1,
        GRASS = 2,
        SAND = 3,
        STONE = 4,
        COBBLESTONE = 5,
        WATER = 6,
        LOG = 7,
        LEAVES = 8,
        CACTUS = 9,
        COAL_ORE = 10,
        IRON_ORE = 11,
    };
}