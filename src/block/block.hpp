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
        COAL_ORE = 6,
        IRON_ORE = 7,
        LOG = 8,
        PLANKS = 9,
        LEAVES = 10,
        WATER = 11,
    };
}