#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef int32_t s32;
typedef int64_t s64;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef float f32;

typedef struct { s32 x, y, z; } ivec3s;
typedef struct { s32 x, y; } ivec2s;
typedef struct { f32 x, y, z; } vec3s;

#define CHUNK_SIZE_X 16
#define CHUNK_SIZE_Y 2
#define CHUNK_SIZE_Z 16

#define CHUNK_VOLUME (CHUNK_SIZE_X * CHUNK_SIZE_Y * CHUNK_SIZE_Z)

#define CHUNK_LOCAL_INDEX(px, py, pz) ((px) + (pz) * CHUNK_SIZE_X + (py) * CHUNK_SIZE_X * CHUNK_SIZE_Z)

#endif