#pragma once
enum quad_facing_direction {
    QUAD_FACING_UP,
    QUAD_FACING_LEFT,
    QUAD_FACING_RIGHT,
    QUAD_FACING_DOWN,
    QUAD_FACING_FRONT,
    QUAD_FACING_BACK
};

#define PushCommand(group, type) (type *)_PushRenderCommand(group, sizeof(type), TYPE_##type)
static render_command_header *_PushRenderCommand(render_group *RenderGroup, size_t Size, render_command_type Type) {
    render_commands *Commands = RenderGroup->RenderCommands;
    render_command_header *Result = NULL;
    u8 *PushBufferEnd = Commands->PushBufferBase + Commands->Cap;
    if (Commands->PushBufferAt + Size <= PushBufferEnd) {
        Result = (render_command_header *)Commands->PushBufferAt;
        Result->Type = Type;
        Commands->PushBufferAt += Size;
    }
    return Result;
}

static inline void PushClearColor(render_group *RenderGroup, vec4 ClearColor = vec4(.1f, .1f, .1f, 1.f)) {
    clear_color_command *Command = PushCommand(RenderGroup, clear_color_command);
    if (Command) {
        Command->ClearColor = ClearColor;
    }
}

static inline void PushClearDepthBuffer(render_group *RenderGroup) {
    PushCommand(RenderGroup, clear_depth_command);
}

// TODO: make the clear boolean a packed boolean of different settings
static render_group BeginRenderGroup(render_commands *RenderCommands, render_setup *Setup) {
    render_group Result = {};
    Result.RenderCommands = RenderCommands;
    Result.RenderSetup = *Setup;
    return Result;
}

static void EndRenderGroup(render_group *RenderGroup) {
    (void)RenderGroup;
}

#if 0
static inline u32 *PushIndices(render_group *Group, u32 Count) {
    (void)Count;
    u32 *Result = NULL;
    textured_quad_batch_command *CurrentBatch = Group->CurrentQuads;
    return Result;
}
#endif

static inline void PushQuad(render_group *Group,
        vec3 P0, vec3 N0, vec2 UV0, vec4 C0,
        vec3 P1, vec3 N1, vec2 UV1, vec4 C1,
        vec3 P2, vec3 N2, vec2 UV2, vec4 C2,
        vec3 P3, vec3 N3, vec2 UV3, vec4 C3,
        b32 CCW = true,
        u32 TextureID = 0) {

    // TODO: still need to set Group->CurrentQuads to 0 if the batch gets full
    // so that I can start a new command to render a batch of quads
    if (!Group->CurrentQuads) {
        Group->CurrentQuads = PushCommand(Group, textured_quad_batch_command);
        Group->CurrentQuads->RenderSetup = Group->RenderSetup;
        Group->CurrentQuads->VertexCount = 0;
        Group->CurrentQuads->IndexCount = 0;
        Group->CurrentQuads->Vertices = Group->RenderCommands->Vertices + Group->RenderCommands->VertexCount;
        Group->CurrentQuads->Indices = Group->RenderCommands->Indices + Group->RenderCommands->IndexCount;
        Group->CurrentQuads->VertexArrayOffset = Group->RenderCommands->VertexCount;
    }
    textured_quad_batch_command *Batch = Group->CurrentQuads;

    vertex *Vertices = NULL;
    u32 MaxVertCount = Group->RenderCommands->MaxVertexCount;
    if (Group->RenderCommands->VertexCount + 4 < MaxVertCount) {
        Vertices = Batch->Vertices + Batch->VertexCount;
        Batch->VertexCount += 4;
        Group->RenderCommands->VertexCount += 4;
    }
    if (Vertices) {
        Vertices[0].Position = P0;
        Vertices[0].Normal = N0;
        Vertices[0].UV = UV0;
        Vertices[0].Color = C0;
        Vertices[0].TextureID = TextureID;

        Vertices[1].Position = P1;
        Vertices[1].Normal = N1;
        Vertices[1].UV = UV1;
        Vertices[1].Color = C1;
        Vertices[1].TextureID = TextureID;

        Vertices[2].Position = P2;
        Vertices[2].Normal = N2;
        Vertices[2].UV = UV2;
        Vertices[2].Color = C2;
        Vertices[2].TextureID = TextureID;

        Vertices[3].Position = P3;
        Vertices[3].Normal = N3;
        Vertices[3].UV = UV3;
        Vertices[3].Color = C3;
        Vertices[3].TextureID = TextureID;

        u32 *Indices = NULL;
        u32 Offset = (Batch->IndexCount/6)*4;
        u32 MaxIndexCount = Group->RenderCommands->MaxIndexCount;
        if (Group->RenderCommands->IndexCount + 6 < MaxIndexCount) {
            Indices = Batch->Indices + Batch->IndexCount;
            Batch->IndexCount += 6;
            Group->RenderCommands->IndexCount += 6;
        }
        if (Indices) {
            if (CCW) {
                Indices[0] = Offset + 0;
                Indices[1] = Offset + 1;
                Indices[2] = Offset + 2;
                Indices[3] = Offset + 2;
                Indices[4] = Offset + 3;
                Indices[5] = Offset + 0;
            }
            else {
                Indices[0] = Offset + 0;
                Indices[1] = Offset + 3;
                Indices[2] = Offset + 2;
                Indices[3] = Offset + 2;
                Indices[4] = Offset + 1;
                Indices[5] = Offset + 0;
            }
        }
    }
}

static inline void PushQuad(render_group *Group,
        vec3 P, vec2 Extent,
        quad_facing_direction D = QUAD_FACING_UP,
        vec4 Color = vec4(1.f),
        b32 Invert = false,
        u32 TextureID = 0) {

    f32 HalfWidth = Extent.x/2.f;
    f32 HalfHeight = Extent.y/2.f;

    vec3 P0;
    vec3 P1;
    vec3 P2;
    vec3 P3;
    vec3 N0;
    vec3 N1;
    vec3 N2;    
    vec3 N3;

    vec2 UV0 = vec2(0.f, 0.f);
    vec2 UV1 = vec2(1.f, 0.f);
    vec2 UV2 = vec2(1.f, 1.f);
    vec2 UV3 = vec2(0.f, 1.f);

    b32 CCW = !Invert;

    switch (D) {
        case QUAD_FACING_UP: {
            P0 = P + vec3(-HalfWidth, -HalfHeight, 0.f);
            P1 = P + vec3( HalfWidth, -HalfHeight, 0.f);
            P2 = P + vec3( HalfWidth,  HalfHeight, 0.f);
            P3 = P + vec3(-HalfWidth,  HalfHeight, 0.f);

            N0 = vec3(0.f, 0.f, 1.f);
            N1 = vec3(0.f, 0.f, 1.f);
            N2 = vec3(0.f, 0.f, 1.f);
            N3 = vec3(0.f, 0.f, 1.f);
        } break;

        case QUAD_FACING_LEFT: {
            P0 = P + vec3(0.f,  HalfWidth, -HalfHeight);
            P1 = P + vec3(0.f, -HalfWidth, -HalfHeight);
            P2 = P + vec3(0.f, -HalfWidth,  HalfHeight);
            P3 = P + vec3(0.f,  HalfWidth,  HalfHeight);

            N0 = vec3(-1.f, 0.f, 0.f);
            N1 = vec3(-1.f, 0.f, 0.f);
            N2 = vec3(-1.f, 0.f, 0.f);
            N3 = vec3(-1.f, 0.f, 0.f);
        } break;

        case QUAD_FACING_RIGHT: {
            P0 = P + vec3(0.f, -HalfWidth, -HalfHeight);
            P1 = P + vec3(0.f,  HalfWidth, -HalfHeight);
            P2 = P + vec3(0.f,  HalfWidth,  HalfHeight);
            P3 = P + vec3(0.f, -HalfWidth,  HalfHeight);

            N0 = vec3(1.f, 0.f, 0.f);
            N1 = vec3(1.f, 0.f, 0.f);
            N2 = vec3(1.f, 0.f, 0.f);
            N3 = vec3(1.f, 0.f, 0.f);
        } break;

        case QUAD_FACING_DOWN: {
            CCW = !CCW;
            P0 = P + vec3(-HalfWidth, -HalfHeight, 0.f);
            P1 = P + vec3( HalfWidth, -HalfHeight, 0.f);
            P2 = P + vec3( HalfWidth,  HalfHeight, 0.f);
            P3 = P + vec3(-HalfWidth,  HalfHeight, 0.f);

            N0 = vec3(0.f, 0.f, -1.f);
            N1 = vec3(0.f, 0.f, -1.f);
            N2 = vec3(0.f, 0.f, -1.f);
            N3 = vec3(0.f, 0.f, -1.f);
        } break;

        case QUAD_FACING_FRONT: {
            CCW = !CCW;
            P0 = P + vec3( HalfWidth, 0.f, -HalfHeight);
            P1 = P + vec3(-HalfWidth, 0.f, -HalfHeight);
            P2 = P + vec3(-HalfWidth, 0.f,  HalfHeight);
            P3 = P + vec3( HalfWidth, 0.f,  HalfHeight);

            N0 = vec3(0.f, -1.f, 0.f);
            N1 = vec3(0.f, -1.f, 0.f);
            N2 = vec3(0.f, -1.f, 0.f);
            N3 = vec3(0.f, -1.f, 0.f);
        } break;

        case QUAD_FACING_BACK: {
            CCW = !CCW;
            P0 = P + vec3(-HalfWidth, 0.f, -HalfHeight);
            P1 = P + vec3( HalfWidth, 0.f, -HalfHeight);
            P2 = P + vec3( HalfWidth, 0.f,  HalfHeight);
            P3 = P + vec3(-HalfWidth, 0.f,  HalfHeight);

            N0 = vec3(0.f, 1.f, 0.f);
            N1 = vec3(0.f, 1.f, 0.f);
            N2 = vec3(0.f, 1.f, 0.f);
            N3 = vec3(0.f, 1.f, 0.f);
        } break;
    }

    vec4 C0 = Color;
    vec4 C1 = Color;
    vec4 C2 = Color;
    vec4 C3 = Color;

    PushQuad(Group, 
            P0, N0, UV0, C0,
            P1, N1, UV1, C1,
            P2, N2, UV2, C2,
            P3, N3, UV3, C3,
            CCW,
            TextureID);

}

static void PushCube(render_group *Group, vec3 P, f32 SideLength, f32 Height, vec4 Color = vec4(1.f), i32 TexID = 0) {
    (void)TexID;
    f32 R = .5f*SideLength;
    f32 H = .5f*Height;

    vec2 HorizontalExtent = vec2(SideLength);
    vec2 VerticalExtent = vec2(SideLength, Height);
    P.x += SideLength/2.f;
    P.y += SideLength/2.f;

    // TOP
    vec3 PTOP = P + vec3(0.f, 0.f, H);
    PushQuad(Group, PTOP, HorizontalExtent, QUAD_FACING_UP, Color);

    // LEFT
    vec3 PLEFT = P + vec3(-R, 0.f, 0.f);
    PushQuad(Group, PLEFT, VerticalExtent, QUAD_FACING_LEFT, Color);

    // RIGHT
    vec3 PRIGHT = P + vec3(R, 0.f, 0.f);
    PushQuad(Group, PRIGHT, VerticalExtent, QUAD_FACING_RIGHT, Color);

    // FRONT
    vec3 PFRONT = P + vec3(0.f, -R, 0.f);
    PushQuad(Group, PFRONT, VerticalExtent, QUAD_FACING_FRONT, Color);

    // BACK
    vec3 PBACK = P + vec3(0.f, R, 0.f);
    PushQuad(Group, PBACK, VerticalExtent, QUAD_FACING_BACK, Color);

    // BOT
    vec3 PBOT = P + vec3(0.f, 0.f, -H);
    PushQuad(Group, PBOT, HorizontalExtent, QUAD_FACING_DOWN, Color);
}

static inline void DEBUGDrawSegment(render_group *Group, vec3 Point1, vec3 Point2, f32 Length = 0.f, vec4 Color = vec4(1.f, 1.f, 0.f, 1.f)) {
    if (!Group->CurrentLines) {
        Group->CurrentLines = PushCommand(Group, line_batch_command);
        Group->CurrentLines->RenderSetup = Group->RenderSetup;
        Group->CurrentLines->Vertices = Group->RenderCommands->Vertices + Group->RenderCommands->VertexCount;
        Group->CurrentLines->VertexCount = 0;
        Group->CurrentLines->VertexArrayOffset = Group->RenderCommands->VertexCount;
    }
    line_batch_command *Lines = Group->CurrentLines;

    vertex *Vertices = NULL;
    u32 MaxCount = Group->RenderCommands->MaxVertexCount;
    if (Group->RenderCommands->VertexCount + 2 < MaxCount) {
        Vertices = Lines->Vertices + Lines->VertexCount;
        Lines->VertexCount += 2;
        Group->RenderCommands->VertexCount += 2;
    }

    if (Vertices) {
        if (Length > 0.f) {
            vec3 Dir = Normalized(Point2 - Point1);
            Point2 = Point1 + Dir*Length;
        }

        Vertices[0].Position = Point1;
        Vertices[0].Color = Color;

        Vertices[1].Position = Point2;
        Vertices[1].Color = Color;
    }
}


