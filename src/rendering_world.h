#pragma once

enum grid_cell_type {
    GRID_CELL_TYPE_EMPTY,
    GRID_CELL_TYPE_GRASS,
    GRID_CELL_TYPE_DIRT,

    GRID_CELL_TYPE_MAX_COUNT
};

struct world_room {
    // handle to chunk
    // position in chunk
    // dimension in chunk
    // type of room
};

//struct entity {
//};

struct grid_cell {
    vec4 Color;
    vec3 Position;
    f32 Height;
    grid_cell_type Type;
};

#define CHUNK_WIDTH 16
#define CHUNK_HEIGHT 16
struct ground {
    grid_cell Cells[CHUNK_WIDTH*CHUNK_HEIGHT];
};

struct world_chunk {
//    entity *Entities;
//    u32 EntityCount;
    vec2i Index;
    ground Ground;
    world_chunk *NextVisible;
};

#if 0
struct cull_chunk_itarator {
    vec2i BottomLeft;
    vec2i BottomRight;
    vec2i TopLeft;
    vec2i TopRight;

    vec2i CurrentIndex;
};
#endif

#define GRID_WIDTH 4
#define GRID_HEIGHT 4
struct world {
    b32 Initialized;
    random_series Series;

    vec3 Up;
    camera Camera;

    b32 DebugMode;
    camera DebugCamera;
    camera *CurrentCamera;

    // memory cache of "loaded" chunks. if this was minecraft
    // chunks that don't fit in this cache would have to be
    // dynamically evicted and loaded from disc
    world_chunk Chunks[GRID_WIDTH*GRID_HEIGHT];

    // pointer to the beginning of a doubly linked list
    world_chunk *VisibleChunks;
    u32 VisibleChunkCount;
    // if using free cam, limit the visible chunk count and
    // obscure the edge with fog.
};
