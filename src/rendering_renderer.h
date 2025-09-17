#pragma once

#define COLOR_RED     vec4(1.f, 0.f, 0.f, 1.f)
#define COLOR_GREEN   vec4(0.f, 1.f, 0.f, 1.f)
#define COLOR_BLUE    vec4(0.f, 0.f, 1.f, 1.f)
#define COLOR_TEAL    vec4(0.f, 1.f, 1.f, 1.f)
#define COLOR_MAGENTA vec4(1.f, 0.f, 1.f, 1.f)
#define COLOR_YELLOW  vec4(1.f, 1.f, 0.f, 1.f)

enum render_command_type {
    TYPE_clear_color_command,
    TYPE_clear_depth_command,
    TYPE_textured_quad_batch_command,
    TYPE_line_batch_command,
};

struct vertex {
    vec4 Color;
    vec3 Position;
    vec3 Normal;
    vec2 UV;
    u32 TextureID;
    //u8 Emission;
    //u8 Tone;
};

struct render_command_header {
    render_command_type Type;
};

struct clear_color_command {
    render_command_header Header;
    vec4 ClearColor;
};

struct clear_depth_command {
    render_command_header Header;
};

struct render_setup {
    // Everything necessary to pass in to the shader.
    //  - Lighting info
    //  - Fog info
    //  - emissiveness

    camera Camera;
    f32 TargetAspect;
};

// NOTE: Any billboards would need z-bias so that it doesnt clip into
// surrounding 3D objects.
struct textured_quad_batch_command {
    render_command_header Header;
    render_setup RenderSetup;

    u32 VertexArrayOffset;
    u32 VertexCount; // Count this batch
    vertex *Vertices;

    // NOTE: MaxQuadsPerBatch for u32 is 1 Mi Quads, which is also renderer's
    // limit. If I should use u16, then the MaxQuadsPerBatch is 16 Ki Quads.
    // If I use u16, then it would be less memory submitting to the GPU, but
    // I would need to do more CPU logic to push multiple batches per frame.
    u32 *Indices; 
    u32 IndexCount; // Count this batch
};

struct line_batch_command {
    render_command_header Header;
    render_setup RenderSetup;
    f32 LineWidth;

    u32 VertexArrayOffset;
    u32 VertexCount;
    vertex *Vertices;
};

struct render_commands {
    // render settings
    u32 ClientWidth;
    u32 ClientHeight;

    // texture handles
    // light atlas, other data handles

    u8 *PushBufferBase;
    u8 *PushBufferAt;
    size_t Cap;

    u32 MaxVertexCount;
    u32 VertexCount; // Total count
    vertex *Vertices;


    u32 MaxIndexCount;
    u32 IndexCount; // Total count
    u32 *Indices;
};

struct render_group {
    // assets *Assets;
    // flags and setup for the entire group

    memory_arena *Scratch;
    render_commands *RenderCommands;

    render_setup RenderSetup;

    textured_quad_batch_command *CurrentQuads;
    line_batch_command *CurrentLines;

    // TODO: default white texture? is this really necessary?
};

static render_group BeginRenderGroup(render_commands *RenderCommands, render_setup *Setup);
static void EndRenderGroup(render_group *RenderGroup);
static inline void PushClearColor(render_group *RenderGroup, vec4 ClearColor);
static inline void PushClearDepthBuffer(render_group *RenderGroup);
