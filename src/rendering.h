#pragma once

struct program_memory {
    void *PermanentStorage;
    size_t PermanentSize;

    void *FrameScratchStorage;
    size_t FrameScratchSize;
};

#define INITIALIZE(name) void name(program_memory *Memory, platform_api *Platform, opengl *OpenGL)
typedef INITIALIZE(program_initialize);
INITIALIZE(InitializeStub) {
    (void)Memory; (void)Platform; (void)OpenGL;
}

#define UPDATE_AND_RENDER(name) void name(program_memory *Memory, platform_api *Platform, opengl *OpenGL, program_input *Input)
typedef UPDATE_AND_RENDER(program_update_and_render);
UPDATE_AND_RENDER(UpdateAndRenderStub) {
    (void)Memory; (void)Platform; (void)OpenGL; (void)Input;
}

struct program_state {
    memory_arena PermanentArena;
    memory_arena ScratchArena;

    world World;

    random_series Random;
};
