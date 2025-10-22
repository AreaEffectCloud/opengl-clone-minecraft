#include "include/direction.h"
#include <string.h>
#include <assert.h>

const ivec3s DIRECTION_IVEC[6] = {
    {{ 0, 0, -1 }}, // NORTH
    {{ 0, 0,  1 }}, // SOUTH
    {{ 1, 0,  0 }}, // EAST
    {{-1, 0,  0 }}, // WEST
    {{ 0, 1,  0 }}, // UP
    {{ 0,-1,  0 }}  // DOWN
};

const vec3s DIRECTION_VEC[6] = {
    {{ 0, 0, -1}},
    {{ 0, 0,  1}},
    {{ 1, 0,  0}},
    {{-1, 0,  0}},
    {{ 0, 1,  0}},
    {{ 0,-1,  0}}
};

enum Direction _ivec3s2dir(ivec3s v) {
    for (size_t i = 0; i < 6; ++i) {
        if (!memcmp(&DIR2IVEC3S(i), &v, sizeof(ivec3s))) {
            return (enum Direction)i;
        }
    }
    assert(false);
    return (enum Direction)-1;
}
