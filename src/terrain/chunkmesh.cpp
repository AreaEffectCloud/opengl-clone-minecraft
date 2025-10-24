#include "terrain/chunkmesh.h"
#include "terrain/chunk.h"
#include "terrain/block.h"
#include <stdlib.h>
#include <string.h>

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/glew.h>
#endif

extern "C" {

/* helper: ensure capacity for arrays (simple doubling) */
static void *reserve_ptr(void *ptr, size_t element_size, size_t *capacity, size_t needed) {
    if (needed <= *capacity) return ptr;
    size_t ncap = (*capacity == 0) ? 1024 : (*capacity * 2);
    while (ncap < needed) ncap *= 2;
    void *np = realloc(ptr, ncap * element_size);
    *capacity = ncap;
    return np;
}

struct ChunkMesh *chunkmesh_create(void) {
    struct ChunkMesh *m = (struct ChunkMesh*)calloc(1, sizeof(struct ChunkMesh));
    m->vertices = NULL;
    m->indices = NULL;
    m->vertex_count = 0;
    m->index_count = 0;
    m->vao = 0;
    m->vbo = 0;
    m->ebo = 0;
    m->uploaded = false;
    return m;
}

void chunkmesh_destroy(struct ChunkMesh *m) {
    if (!m) return;
    if (m->uploaded) {
        if (m->vbo) glDeleteBuffers(1, &m->vbo);
        if (m->ebo) glDeleteBuffers(1, &m->ebo);
        if (m->vao) glDeleteVertexArrays(1, &m->vao);
    }
    if (m->vertices) free(m->vertices);
    if (m->indices) free(m->indices);
    free(m);
}

/* For simplicity, we will add quads for each visible face with a simple UV mapping.
   This is not optimized for merging vertices; it's straightforward and correct. */
void chunkmesh_build_from_chunk(struct ChunkMesh *m, struct Chunk *chunk) {
    if (!m || !chunk) return;

    // free previous cpu-side arrays
    if (m->vertices) { free(m->vertices); m->vertices = NULL; m->vertex_count = 0; }
    if (m->indices) { free(m->indices); m->indices = NULL; m->index_count = 0; }
    m->uploaded = false;

    size_t vcap = 0, icap = 0;
    struct Vertex *verts = NULL;
    uint32_t *inds = NULL;

    // For each block, if non-air, consider 6 faces
    for (s32 x = 0; x < CHUNK_SIZE_X; x++) {
        for (s32 z = 0; z < CHUNK_SIZE_Z; z++) {
            for (s32 y = 0; y < CHUNK_SIZE_Y; y++) {
                ivec3s local = { .x = x, .y = y, .z = z };
                BlockId id = chunk_get_block(chunk, local);
                if (id == BLOCK_AIR) continue;

                // check six neighbors: -x, +x, -y, +y, -z, +z
                const ivec3s neighs[6] = {
                    {x-1, y, z}, {x+1, y, z}, {x, y-1, z}, {x, y+1, z}, {x, y, z-1}, {x, y, z+1}
                };

                for (int f = 0; f < 6; f++) {
                    ivec3s nlocal = neighs[f];
                    BlockId nid = chunk_get_block_world(chunk, nlocal);
                    if (!block_is_transparent(nid)) {
                        // neighbor is solid => face hidden
                        continue;
                    }

                    // ensure capacity
                    verts = (struct Vertex*)reserve_ptr(verts, sizeof(struct Vertex), &vcap, (m->vertex_count + 4));
                    inds = (uint32_t*)reserve_ptr(inds, sizeof(uint32_t), &icap, (m->index_count + 6));

                    // compute vertex positions depending on face f
                    f32 vx = (f32)chunk->position.x + (f32)x;
                    f32 vy = (f32)chunk->position.y + (f32)y;
                    f32 vz = (f32)chunk->position.z + (f32)z;

                    // Define quad vertices for each face
                    struct Vertex vquad[4];
                    switch (f) {
                        case 0: // -x
                            vquad[0] = { vx,   vy,   vz+1, 0.0f, 0.0f };
                            vquad[1] = { vx,   vy,   vz,   1.0f, 0.0f };
                            vquad[2] = { vx,   vy+1, vz,   1.0f, 1.0f };
                            vquad[3] = { vx,   vy+1, vz+1, 0.0f, 1.0f };
                            break;
                        case 1: // +x
                            vquad[0] = { vx+1, vy,   vz,   0.0f, 0.0f };
                            vquad[1] = { vx+1, vy,   vz+1, 1.0f, 0.0f };
                            vquad[2] = { vx+1, vy+1, vz+1, 1.0f, 1.0f };
                            vquad[3] = { vx+1, vy+1, vz,   0.0f, 1.0f };
                            break;
                        case 2: // -y
                            vquad[0] = { vx,   vy,   vz,   0.0f, 0.0f };
                            vquad[1] = { vx+1, vy,   vz,   1.0f, 0.0f };
                            vquad[2] = { vx+1, vy,   vz+1, 1.0f, 1.0f };
                            vquad[3] = { vx,   vy,   vz+1, 0.0f, 1.0f };
                            break;
                        case 3: // +y
                            vquad[0] = { vx,   vy+1, vz+1, 0.0f, 0.0f };
                            vquad[1] = { vx+1, vy+1, vz+1, 1.0f, 0.0f };
                            vquad[2] = { vx+1, vy+1, vz,   1.0f, 1.0f };
                            vquad[3] = { vx,   vy+1, vz,   0.0f, 1.0f };
                            break;
                        case 4: // -z
                            vquad[0] = { vx+1, vy,   vz,   0.0f, 0.0f };
                            vquad[1] = { vx,   vy,   vz,   1.0f, 0.0f };
                            vquad[2] = { vx,   vy+1, vz,   1.0f, 1.0f };
                            vquad[3] = { vx+1, vy+1, vz,   0.0f, 1.0f };
                            break;
                        case 5: // +z
                            vquad[0] = { vx,   vy,   vz+1, 0.0f, 0.0f };
                            vquad[1] = { vx+1, vy,   vz+1, 1.0f, 0.0f };
                            vquad[2] = { vx+1, vy+1, vz+1, 1.0f, 1.0f };
                            vquad[3] = { vx,   vy+1, vz+1, 0.0f, 1.0f };
                            break;
                        default:
                            continue;
                    }

                    // append vertices
                    for (int vi = 0; vi < 4; vi++) {
                        verts[m->vertex_count + vi] = vquad[vi];
                    }

                    // append indices (two triangles)
                    inds[m->index_count + 0] = (uint32_t)(m->vertex_count + 0);
                    inds[m->index_count + 1] = (uint32_t)(m->vertex_count + 1);
                    inds[m->index_count + 2] = (uint32_t)(m->vertex_count + 2);
                    inds[m->index_count + 3] = (uint32_t)(m->vertex_count + 0);
                    inds[m->index_count + 4] = (uint32_t)(m->vertex_count + 2);
                    inds[m->index_count + 5] = (uint32_t)(m->vertex_count + 3);

                    m->vertex_count += 4;
                    m->index_count += 6;
                }
            }
        }
    }

    // shrink to fit
    if (m->vertex_count > 0) {
        verts = (struct Vertex*)realloc(verts, sizeof(struct Vertex) * m->vertex_count);
    } else {
        if (verts) free(verts);
        verts = NULL;
    }
    if (m->index_count > 0) {
        inds = (uint32_t*)realloc(inds, sizeof(uint32_t) * m->index_count);
    } else {
        if (inds) free(inds);
        inds = NULL;
    }

    m->vertices = verts;
    m->indices = inds;
}

/* GPU upload: create VAO/VBO/EBO and upload data */
void chunkmesh_upload(struct ChunkMesh *m) {
    if (!m) return;
    if (m->uploaded) return;
    if (!m->vertices || !m->indices) return;

    glGenVertexArrays(1, &m->vao);
    glBindVertexArray(m->vao);

    glGenBuffers(1, &m->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
    glBufferData(GL_ARRAY_BUFFER, m->vertex_count * sizeof(struct Vertex), m->vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &m->ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m->index_count * sizeof(uint32_t), m->indices, GL_STATIC_DRAW);

    /* vertex attribs: 0 = position (3f), 1 = uv (2f) */
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void*)offsetof(struct Vertex, px));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void*)offsetof(struct Vertex, u));

    glBindVertexArray(0);
    m->uploaded = true;
}

/* 描画：ここでは全インデックスを描画する簡易実装 */
void chunkmesh_render(struct ChunkMesh *m, enum ChunkMeshPart part) {
    (void)part;
    if (!m || !m->uploaded) return;
    glBindVertexArray(m->vao);
    glDrawElements(GL_TRIANGLES, (GLsizei)m->index_count, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

} // extern "C"