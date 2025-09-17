#pragma once

static void DrawChunkBoundaries(render_group *Group, world *World) {
    (void)World;
    vec3 Offset = vec3(0.f, 0.f, 1.0f);
    for (u32 j = 0; j < GRID_HEIGHT+1; ++j) {
        f32 X = (f32)GRID_WIDTH*CHUNK_WIDTH/2.f;
        f32 Y = (f32)j*CHUNK_HEIGHT - (f32)GRID_HEIGHT*CHUNK_HEIGHT/2.f;
        vec3 P1 = vec3(-X, Y, 0.f) + Offset;
        vec3 P2 = vec3( X, Y, 0.f) + Offset;
        DEBUGDrawSegment(Group, P1, P2, 0, vec4(1.f));
    }
    for (u32 i = 0; i < GRID_WIDTH+1; ++i) {
        f32 X = (f32)i*CHUNK_WIDTH - (f32)GRID_WIDTH*CHUNK_WIDTH/2.f;
        f32 Y = (f32)GRID_HEIGHT*CHUNK_HEIGHT/2.f;
        vec3 P1 = vec3(X, -Y, 0.f) + Offset;
        vec3 P2 = vec3(X,  Y, 0.f) + Offset;
        DEBUGDrawSegment(Group, P1, P2, 0, vec4(1.f));
    }
}

// parameter is the main camera that will have their frustum drawn
static void DrawCameraFrustum(render_group *Group, camera *Camera) {
    f32 n = Camera->Near; // -1.f
    f32 h = -n*tanf(Camera->FOV/2.f); // half the height of the near plane
    f32 w = h*Camera->Aspect;

    vec3 P0 = Camera->Position;

    // WRONG? Why is z = -n and not positive n?
    vec3 CNBL = vec3(-w, -h, -n);
    vec3 CNBR = vec3( w, -h, -n);
    vec3 CNTL = vec3(-w,  h, -n);
    vec3 CNTR = vec3( w,  h, -n);

    mat3 CameraToWorld = GetCameraToWorldRotation(Camera);
    vec3 WNBL = P0 + CameraToWorld*CNBL;
    vec3 WNBR = P0 + CameraToWorld*CNBR;
    vec3 WNTL = P0 + CameraToWorld*CNTL;
    vec3 WNTR = P0 + CameraToWorld*CNTR;

#if 1
    DEBUGDrawSegment(Group, P0, WNBL, 8.f);
    DEBUGDrawSegment(Group, P0, WNBR, 8.f);
    DEBUGDrawSegment(Group, P0, WNTL, 8.f);
    DEBUGDrawSegment(Group, P0, WNTR, 8.f);

    DEBUGDrawSegment(Group, WNBR, WNBL);
    DEBUGDrawSegment(Group, WNTR, WNBR);
    DEBUGDrawSegment(Group, WNBL, WNTL);
    DEBUGDrawSegment(Group, WNTL, WNTR);
#else // add colors to the frustum lines
    DEBUGDrawSegment(Group, P0, WNBL, 8.f, COLOR_RED);
    DEBUGDrawSegment(Group, P0, WNBR, 8.f, COLOR_GREEN);
    DEBUGDrawSegment(Group, P0, WNTL, 8.f, COLOR_BLUE);
    DEBUGDrawSegment(Group, P0, WNTR, 8.f, COLOR_TEAL);

    DEBUGDrawSegment(Group, WNBR, WNBL, 0.f, vec4(0.2f, 0.2f, 1.f, 1.f));
    DEBUGDrawSegment(Group, WNTR, WNBR, 0.f, vec4(0.2f, 0.2f, 1.f, 1.f));
    DEBUGDrawSegment(Group, WNBL, WNTL, 0.f, vec4(0.2f, 0.2f, 1.f, 1.f));
    DEBUGDrawSegment(Group, WNTL, WNTR, 0.f, vec4(0.2f, 0.2f, 1.f, 1.f));
#endif
}

static void UpdateAndRenderDebugGizmos(program_state *State, program_input *Input, render_commands *RenderCommands) {
    world *World = &State->World;

    render_setup Setup = {};
    Setup.Camera = *World->CurrentCamera;
    Setup.TargetAspect = (f32)Input->ClientWidth/Input->ClientHeight;
    render_group DebugGroup_ = BeginRenderGroup(RenderCommands, &Setup);
    render_group *DebugGroup = &DebugGroup_;

    DrawCameraFrustum(DebugGroup, &World->Camera);
    DrawChunkBoundaries(DebugGroup, World);
    EndRenderGroup(DebugGroup);
}
