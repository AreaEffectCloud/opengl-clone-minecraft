#include "include/chunkmesh.h"
#include "include/chunk.h"
#include "include/block.h"
#include <stdlib.h>
#include <string.h>

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <glad/glad.h>
#endif

extern "C" {

static const f32 FACE_VERTS[6][3] = {
    {0,0,0}, {1,0,0}, {1,1,0}, {0,1,0}, {0,0,0}, {1,1,0}
};

static void* reserve_ptr(void* ptr, size_t element_size, size_t* capacity, size_t needed) {
    if (needed <= *capacity) return ptr;
    size_t ncap = (*capacity == 0) ? 1024 : (*capacity * 2);
    while (ncap < needed) ncap *= 2;
    void* np = realloc(ptr, ncap * element_size);
    *capacity = ncap;
    return np;
}

struct ChunkMesh* chunkmesh_create(void) {
    struct ChunkMesh* mesh = (struct ChunkMesh*)calloc(1, sizeof(struct ChunkMesh));
    mesh->vertices = NULL;
    mesh->indices = NULL;
    mesh->vertex_count = 0;
    mesh->index_count = 0;
    mesh->vao = 0;
    mesh->vbo = 0;
    mesh->ebo = 0;
    mesh->uploaded = false;
    return mesh;
}

void chunkmesh_destroy(struct ChunkMesh* mesh) {
    if (!mesh) return;
    if (mesh->uploaded) {
        if (mesh->vbo) glDeleteBuffers(1, &mesh->vbo);
        if (mesh->ebo) glDeleteBuffers(1, &mesh->ebo);
        if (mesh->vao) glDeleteVertexArrays(1, &mesh->vao);
    }
    if (mesh->vertices) free(mesh->vertices);
    if (mesh->indices) free(mesh->indices);
    free(mesh);
}

static const f32 CUBE_FACE_POS[6][3] = {
    {0,0,0}, {1,0,0}, {1,1,0}, {0,1,0}, {0,0,0}, {1,1,0}
};

void chunkmesh_build_from_chunk(struct ChunkMesh* mesh, struct Chunk *chunk) {
    if (!chunk || !mesh) return;

    if (mesh->vertices) { 
        free(mesh->vertices);
        mesh->vertices = NULL;
        mesh->vertex_count = 0;
    }
    if (mesh->indices) {
        free(mesh->indices);
        mesh->indices = NULL;
        mesh->index_count = 0;
    }
    mesh->uploaded = false;

    size_t vcap = 0, icap = 0;
    struct Vertex* verts = NULL;
    u32* inds = NULL;

    for (s32 x = 0; x < CHUNK_SIZE_X; ++x) {
        for (s32 z = 0; z < CHUNK_SIZE_Z; ++z) {
            for (s32 y = 0; y < CHUNK_SIZE_Y; ++y) {
                ivec3s local = {x, y, z};
                BlockId id = chunk_get_block(chunk, local);
                if (id == AIR) continue;

                const ivec3s neighbors[6] = {
                    {x-1, y, z}, {x+1, y, z}, {x, y-1, z}, {x, y+1, z}, {x, y, z-1}, {x, y, z+1}
                };
                const int face_direction[6] = {0, 1, 2, 3, 4, 5};

                for (int f = 0; f < 6; ++f) {
                    ivec3s nlocal = neighbors[f];
                    BlockId nid = chunk_get_block_world(chunk, nlocal);
                    if (!block_is_transparent(nid)) continue;

                    size_t base_v = (size_t)verts == 0 && vcap == 0 ? 0 : mesh->vertex_count;
                    // ensure capacity
                    verts = (struct Vertex*)reserve_ptr(verts, sizeof(struct Vertex), &vcap, (mesh->vertex_count + 4));
                    inds = (u32*)reserve_ptr(inds, sizeof(u32), &icap, (mesh->index_count + 6));

                    // compute vertex positions depending on face f
                    f32 vx = (f32)chunk->position.x + (f32)x;
                    f32 vy = (f32)chunk->position.y + (f32)y;
                    f32 vz = (f32)chunk->position.z + (f32)z;

                    // define quad vertices for each face
                    struct Vertex vquad[4];
                    switch (f) {
                        case 0:
                            vquad[0] = { vx,   vy,   vz+1, 0, 0 };
                            vquad[1] = { vx,   vy,   vz,   1, 0 };
                            vquad[2] = { vx,   vy+1, vz,   1, 1 };
                            vquad[3] = { vx,   vy+1, vz+1, 0, 1 };
                            break;
                        case 1:
                            vquad[0] = { vx+1, vy,   vz  , 0, 0 };
                            vquad[1] = { vx+1, vy,   vz+1, 1, 0 };
                            vquad[2] = { vx+1, vy+1, vz+1, 1, 1 };
                            vquad[3] = { vx+1, vy+1, vz  , 0, 1 };
                            break;
                        case 2:
                            vquad[0] = { vx,   vy,   vz,   0, 0 };
                            vquad[1] = { vx+1, vy,   vz,   1, 0 };
                            vquad[2] = { vx+1, vy,   vz+1, 1, 1 };
                            vquad[3] = { vx,   vy,   vz+1, 0, 1 };
                            break;
                        case 3:
                            vquad[0] = { vx,   vy+1, vz+1, 0, 0 };
                            vquad[1] = { vx+1, vy+1, vz+1, 1, 0 };
                            vquad[2] = { vx+1, vy+1, vz,   1, 1 };
                            vquad[3] = { vx,   vy+1, vz,   0, 1 };
                            break;
                        case 4:
                            vquad[0] = { vx+1, vy,   vz,   0, 0 };
                            vquad[1] = { vx,   vy,   vz,   1, 0 };
                            vquad[2] = { vx,   vy+1, vz,   1, 1 };
                            vquad[3] = { vx+1, vy+1, vz,   0, 1 };
                            break;
                        case 5:
                            vquad[0] = { vx,   vy,   vz+1, 0, 0 };
                            vquad[1] = { vx+1, vy,   vz+1, 1, 0 };
                            vquad[2] = { vx+1, vy+1, vz+1, 1, 1 };
                            vquad[3] = { vx,   vy+1, vz+1, 0, 1 };
                            break;
                        default:
                            continue;
                    }
                    // append vertices
                    for (int vi = 0; vi < 4; ++vi) {
                        verts[mesh->vertex_count + vi] = vquad[vi];
                    }

                    inds[mesh->index_count + 0] = (u32)(mesh->vertex_count + 0);
                    inds[mesh->index_count + 1] = (u32)(mesh->vertex_count + 1);
                    inds[mesh->index_count + 2] = (u32)(mesh->vertex_count + 2);
                    inds[mesh->index_count + 3] = (u32)(mesh->vertex_count + 0);
                    inds[mesh->index_count + 4] = (u32)(mesh->vertex_count + 2);
                    inds[mesh->index_count + 5] = (u32)(mesh->vertex_count + 3);

                    mesh->vertex_count += 4;
                    mesh->index_count += 6;
                }
            }
        }
    }
    // shrink to fit
    if (mesh->vertex_count > 0) {
        verts = (struct Vertex*)realloc(verts, sizeof(struct Vertex) * mesh->vertex_count);
    } else {
        if (verts) free(verts);
        verts = NULL;
    }
    if (mesh->index_count > 0) {
        inds =(u32*)realloc(inds, sizeof(u32) * mesh->index_count);
    } else {
        if (inds) free(inds);
        inds = NULL;
    }

    mesh->vertices = verts;
    mesh->indices = inds;
}

// GPU upload: create VAO/VBO/EBO and upload data
void chunkmesh_upload(struct ChunkMesh* mesh) {
    if (!mesh) return;
    if (mesh->uploaded) return;
    if (!mesh->vertices || !mesh->indices) return;

    glGenVertexArrays(1, &mesh->vao);
    glBindVertexArray(mesh->vao);

    glGenBuffers(1, &mesh->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBufferData(GL_ARRAY_BUFFER, mesh->vertex_count * sizeof(struct Vertex), mesh->vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &mesh->ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->index_count * sizeof(u32), mesh->indices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void*)offsetof(struct Vertex, px));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void*)offsetof(struct Vertex, u));

    glBindVertexArray(0);
    mesh->uploaded = true;
}

void chunkmesh_render(struct ChunkMesh* mesh, enum ChunkMeshPart part) {
    (void)part;
    if (!mesh || !mesh->uploaded) return;
    glBindVertexArray(mesh->vao);
    glDrawElements(GL_TRIANGLES, (GLsizei)mesh->index_count, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

}