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

#if 0
GLuint debug_program_handle;
GLuint debug_vao;
GLuint debug_vbo;
GLuint debug_ibo;

GLuint debug_mvp_loc;

GLuint create_debug_program(opengl *OpenGL) {
    char *VertexSource = R"FOO(
    #version 430 core
    layout(location = 0) in vec4 _Color;
    layout(location = 1) in vec3 _Position;
    layout(location = 2) in vec3 _Normal;
    layout(location = 3) in vec2 _UV;
    layout(location = 4) in int _TexID;
    uniform mat4 PVM;
    out vec4 Color;
    out vec3 Normal;
    out vec2 UV;
    out vec3 FragPos;
    flat out int TexID;
    void main() {
        Color = _Color;
        UV = _UV;
        TexID = _TexID;
        FragPos = _Position;
        Normal = _Normal;
        gl_Position = PVM*vec4(_Position, 1.0);
    }
    )FOO";

    char *FragmentSource = R"FOO(
    #version 430 core
    in vec4 Color;
    in vec3 Normal;
    in vec2 UV;
    in vec3 FragPos;
    flat in int TexID;
    out vec4 FragColor;
    void main() {
        FragColor = Color*vec4(UV + FragPos.xy*vec2(0.0f), 0.0f*Normal.x + 0.0f*TexID, 1.0f);
    }
    )FOO";

    GLuint handle = CompileShaderProgram(OpenGL, VertexSource, FragmentSource);
    OpenGL->glUseProgram(handle);
    OpenGL->glBindAttribLocation(handle, 0, "_Color");
    OpenGL->glBindAttribLocation(handle, 1, "_Position");
    OpenGL->glBindAttribLocation(handle, 2, "_Normal");
    OpenGL->glBindAttribLocation(handle, 3, "_UV");
    OpenGL->glBindAttribLocation(handle, 4, "_TexID");
    debug_mvp_loc = OpenGL->glGetUniformLocation(handle, "PVM");
    return handle;
}

void DEBUGInitOpenGL(opengl *OpenGL, platform_api *Platform, opengl_info Info) {
    (void)Platform; (void)Info;
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    OpenGL->glDebugMessageCallback(DebugCallback, NULL);
    debug_program_handle = create_debug_program(OpenGL);
    OpenGL->glGenVertexArrays(1, &debug_vao);
    OpenGL->glBindVertexArray(debug_vao);

    vertex vertices[] = {
        { { 1.0f, 1.0f, 1.0f, 1.0f }, { -0.5f,  0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f }, 0 }, // top left
        { { 1.0f, 1.0f, 1.0f, 1.0f }, {  0.5f,  0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f }, 0 }, // top right
        { { 1.0f, 1.0f, 1.0f, 1.0f }, {  0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f }, 0 }, // bottom right
        { { 1.0f, 1.0f, 1.0f, 1.0f }, {  0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f }, 0 }, // bottom right
        { { 1.0f, 1.0f, 1.0f, 1.0f }, { -0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f }, 0 }, // bottom left
        { { 1.0f, 1.0f, 1.0f, 1.0f }, { -0.5f,  0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f }, 0 }, // top left
    };                                                                                          
    OpenGL->glGenBuffers(1, &debug_vbo);
    OpenGL->glBindBuffer(GL_ARRAY_BUFFER, debug_vbo);
    OpenGL->glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    OpenGL->glVertexAttribPointer( 0, 4, GL_FLOAT, false, sizeof(vertex), (void *)OFFSETOF(vertex, Color));
    OpenGL->glVertexAttribPointer( 1, 3, GL_FLOAT, false, sizeof(vertex), (void *)OFFSETOF(vertex, Position));
    OpenGL->glVertexAttribPointer( 2, 3, GL_FLOAT, false, sizeof(vertex), (void *)OFFSETOF(vertex, Normal));
    OpenGL->glVertexAttribPointer( 3, 2, GL_FLOAT, false, sizeof(vertex), (void *)OFFSETOF(vertex, UV));
    OpenGL->glVertexAttribIPointer(4, 1, GL_INT, sizeof(vertex), (void *)OFFSETOF(vertex, TextureID));
    OpenGL->glEnableVertexAttribArray(0);
    OpenGL->glEnableVertexAttribArray(1);
    OpenGL->glEnableVertexAttribArray(2);
    OpenGL->glEnableVertexAttribArray(3);
    OpenGL->glEnableVertexAttribArray(4);
}



INITIALIZE(Initialize) {
    GlobalPlatform = Platform;
    program_state *State = (program_state *)Memory->PermanentStorage;

    State->PermanentArena = InitializeArena((u8 *)Memory->PermanentStorage, Memory->PermanentSize);
    State->PermanentArena.Used = sizeof(*State);
    State->ScratchArena = InitializeArena((u8 *)Memory->FrameScratchStorage, Memory->FrameScratchSize);

    opengl_info Info = GetOpenGLInfo();
    DEBUGInitOpenGL(OpenGL, Platform, Info);
    InitializeWorld(&State->World);
}

UPDATE_AND_RENDER(UpdateAndRender) {
    (void)OpenGL;
    GlobalPlatform = Platform;
    program_state *State = (program_state *)Memory->PermanentStorage;
    State->ScratchArena = InitializeArena((u8 *)Memory->FrameScratchStorage, Memory->FrameScratchSize);

    mat4 ProjectionView = CalculateCameraProjectionView(&State->World.Camera, (f32)Input->ClientWidth/Input->ClientHeight);
    OpenGL->glUniformMatrix4fv(debug_mvp_loc, 1, GL_TRUE, ProjectionView.Elements);
    glViewport(0, 0, Input->ClientWidth, Input->ClientHeight);
    glClearColor(0.1f, 0.12f, 0.14f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

// debug
#else //////////////////////////////////////////////////////////////////////////////
// original

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

    #if 0
    vec3 P = {};
    f32 SideLength = 1.0f;
    f32 Height = 1.0f;
    f32 R = .5f*SideLength;
    f32 H = .5f*Height;
    vec2 HorizontalExtent = vec2(SideLength);
    vec2 VerticalExtent = vec2(SideLength, Height);
    P.x += SideLength/2.f;
    P.y += SideLength/2.f;
    vec3 PFRONT = P + vec3(0.f, -R, 0.f);
    PushQuad(RenderCommands->RenderGroup, PFRONT, VerticalExtent, QUAD_FACING_FRONT, Color);
    #else
    UpdateAndRenderWorld(&State->World, Input, RenderCommands);
    #endif
    //if (State->World.DebugMode) {
    //    UpdateAndRenderDebugGizmos(State, Input, RenderCommands);
    //}

    OpenGLEndFrame(OpenGL);
}
#endif

}
