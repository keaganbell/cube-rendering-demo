#define _CRT_SECURE_NO_WARNINGS
#include <math.h>
#include <stdlib.h>

#define GL_GLEXT_PROTOTYPES
#include "GL/glcorearb.h"

#include "defines.h"
#include "platform.h"
#include "log.h"
#include "arena.h"
#include "random.h"
#include "rendering_math.h"
#include "input.h"
#include "rendering_camera.h"
#include "rendering_world.h"
#include "rendering_renderer.h"
#include "windows_opengl.h" // has all the typedefs. TODO: abstract WINAPI away
#include "rendering_opengl.h"
#include "rendering.h"

#include "rendering_camera.cpp"
#include "rendering_renderer.cpp"
#include "rendering_opengl.cpp"
#include "rendering_world.cpp"
#include "rendering_debug.cpp"

extern "C"{

INITIALIZE(Initialize) {
    GlobalPlatform = Platform;
    program_state *State = (program_state *)Memory->PermanentStorage;

    State->PermanentArena = InitializeArena((u8 *)Memory->PermanentStorage, Memory->PermanentSize);
    State->PermanentArena.Used = sizeof(*State);
    State->ScratchArena = InitializeArena((u8 *)Memory->FrameScratchStorage, Memory->FrameScratchSize);

    opengl_info Info = GetOpenGLInfo();
    InitOpenGL(OpenGL, Platform, Info);
}

UPDATE_AND_RENDER(UpdateAndRender) {
    GlobalPlatform = Platform;
    program_state *State = (program_state *)Memory->PermanentStorage;
    State->ScratchArena = InitializeArena((u8 *)Memory->FrameScratchStorage, Memory->FrameScratchSize);
    //memory_arena *Scratch = &State->ScratchArena;

    OpenGLBeginFrame(OpenGL, Input->ClientWidth, Input->ClientHeight);
    render_commands *RenderCommands = &OpenGL->RenderCommands;

    UpdateAndRenderWorld(&State->World, Input, RenderCommands);

    if (State->World.DebugMode) {
        UpdateAndRenderDebugGizmos(State, Input, RenderCommands);
    }

    OpenGLEndFrame(OpenGL);
}

}
