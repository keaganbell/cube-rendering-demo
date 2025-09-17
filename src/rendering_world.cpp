#pragma once

static inline vec2i WorldPositionToChunkIndex(vec3 P) {
    // TODO: handle z
    vec3 HalfMapDim = vec3(GRID_WIDTH*CHUNK_HEIGHT*.5f, GRID_HEIGHT*CHUNK_HEIGHT*.5f, 0.f);
    vec3 ChunkSpaceP = (P + HalfMapDim)/vec3(GRID_WIDTH, GRID_HEIGHT, 0.f);
    return vec2i((i32)ChunkSpaceP.x, (i32)ChunkSpaceP.y);
}

static inline vec3 ChunkIndexToWorldPosition(vec2i Index) {
    // TODO: handle z
    vec3 HalfMapDim = vec3(GRID_WIDTH*CHUNK_HEIGHT*.5f, GRID_HEIGHT*CHUNK_HEIGHT*.5f, 0.f);
    vec3 ChunkSpaceP = vec3(Index.x, Index.y, 0.f)*vec3(CHUNK_WIDTH, CHUNK_HEIGHT, 0.f) - HalfMapDim;
    return ChunkSpaceP + vec3((f32)CHUNK_WIDTH/2.f, (f32)CHUNK_HEIGHT/2.f, 0.f);
}

static inline world_chunk *GetChunkAt(world *World, vec3 P) {
    vec2i Index = WorldPositionToChunkIndex(P);
    return &World->Chunks[Index.y*GRID_WIDTH + Index.x];
}

static inline world_chunk *GetChunkAt(world *World, vec2i Index) {
    return &World->Chunks[Index.y*GRID_WIDTH + Index.x];
}

static inline world_chunk *GetChunkAt(world *World, i32 x, i32 y) {
    return &World->Chunks[y*GRID_WIDTH + x];
}

static void InitializeCamera(camera *Camera, vec3 WorldUp) {
    Camera->Orthographic = false;
    Camera->FOV = (f32)PI/2.0f; // in radians
    Camera->Near = -1.f;
    Camera->Far = -100.f;
    Camera->Aspect = 16.f/9.f;
    Camera->Position = vec3(0.f, -3.f, 10.f);
    LookAt(Camera, vec3(), WorldUp);
}

static void InitializeWorld(world *World) {
    World->Initialized = true;
    World->Series = InitRandom(12);
    World->Up = vec3(0.f, 0.f, 1.f);
    World->CurrentCamera = &World->Camera;
    InitializeCamera(&World->Camera, World->Up);
    InitializeCamera(&World->DebugCamera, World->Up);

    vec4 Colors[3] = {};
    Colors[0] = vec4(1.f, 0.f, 1.f, 1.f);
    Colors[1] = vec4(.22f, .4f, 0.2f, 1.f);
    Colors[2] = vec4(.2f, .17f, .06f, 1.f);

    for (u32 Chunkj = 0; Chunkj < GRID_HEIGHT; ++Chunkj) {
        for (u32 Chunki = 0; Chunki < GRID_WIDTH; ++Chunki) {
            world_chunk *Chunk = GetChunkAt(World, Chunki, Chunkj);
            Chunk->Index = vec2i(Chunki, Chunkj);
            for (u32 j = 0; j < CHUNK_HEIGHT; ++j) {
                for (u32 i = 0; i < CHUNK_WIDTH; ++i) {
                    grid_cell *Cell = &Chunk->Ground.Cells[j*CHUNK_WIDTH + i];
                    Cell->Type = GRID_CELL_TYPE_GRASS;
                    Cell->Color = Colors[1];

                    f32 X = (f32)i - (f32)CHUNK_WIDTH/2.f;
                    f32 Y = (f32)j - (f32)CHUNK_HEIGHT/2.f;
                    f32 Z = 0.15f*RandomBilateral(&World->Series);
                    Cell->Position = vec3(X, Y, Z);
                }
            }
        }
    }
}

static void UpdateCamera(camera *Camera, vec3 WorldUp, program_input *Input) {
    f32 MoveSpeed = .7f;
    f32 LookSpeed = .06f;

    b32 Moving = false;
    vec3 CameraSpaceMoveDirection = vec3();
    if (ButtonDown(Input, BUTTON_KEY_W)) {
        CameraSpaceMoveDirection.y += 1.f;
        Moving = true;
    }
    if (ButtonDown(Input, BUTTON_KEY_S)) {
        CameraSpaceMoveDirection.y -= 1.f;
        Moving = true;
    }
    if (ButtonDown(Input, BUTTON_KEY_A)) {
        CameraSpaceMoveDirection.x -= 1.f;
        Moving = true;
    }
    if (ButtonDown(Input, BUTTON_KEY_D)) {
        CameraSpaceMoveDirection.x += 1.f;
        Moving = true;
    }
    if (Moving) {
        vec3 WorldSpaceMoveDirection = GetCameraToWorldRotation(Camera)*CameraSpaceMoveDirection;
        WorldSpaceMoveDirection.z = 0.f;
        Camera->Position = Camera->Position + MoveSpeed*Normalized(WorldSpaceMoveDirection);
    }


    vec3 UpDirection = vec3();
    if (ButtonDown(Input, BUTTON_KEY_SHIFT)) {
        UpDirection.z -= 1.f;
    }
    if (ButtonDown(Input, BUTTON_KEY_SPACE)) {
        UpDirection.z += 1.f;
    }
    Camera->Position = Camera->Position + MoveSpeed*Normalized(UpDirection);

    if (ButtonDown(Input, BUTTON_KEY_RIGHT)) {
        vec3 CameraSpaceUp = GetWorldToCameraRotation(Camera)*WorldUp;
        RotateCamera(Camera, -LookSpeed, CameraSpaceUp);
    }
    if (ButtonDown(Input, BUTTON_KEY_LEFT)) {
        vec3 CameraSpaceUp = GetWorldToCameraRotation(Camera)*WorldUp;
        RotateCamera(Camera, LookSpeed, CameraSpaceUp);
    }
    if (ButtonDown(Input, BUTTON_KEY_UP)) {
        vec3 CameraSpaceRight = Cross(Camera->Target, Camera->Up);
        vec3 WorldSpaceRight = GetCameraToWorldRotation(Camera)*CameraSpaceRight;
        RotateCamera(Camera, 0.7f*LookSpeed, Normalized(WorldSpaceRight));
    }
    if (ButtonDown(Input, BUTTON_KEY_DOWN)) {
        vec3 CameraSpaceRight = Cross(Camera->Target, Camera->Up);
        vec3 WorldSpaceRight = GetCameraToWorldRotation(Camera)*CameraSpaceRight;
        RotateCamera(Camera, -0.7f*LookSpeed, Normalized(WorldSpaceRight));
    }
}

#if 0
static world_chunk *GetNextChunk(world *World, cull_chunk_itarator *Iterator) {
    // figure out the next chunk based on the current index
    // and the 4 corners. return NULL when done "scan lining"
    world_chunk *Result = NULL;
    if (Iterator->CurrentIndex.x <= Iterator->BottomRight.x) {
        Result = GetChunkAt(World, Iterator->CurrentIndex);
        ++Iterator->CurrentIndex.x;
    }
    return Result;
}

static void CullChunks(world *World) {
    camera *Camera = &World->Camera;
    f32 n = Camera->Near; // -1.f
    f32 h = -n*tanf(Camera->FOV/2.f); // half the height of the near plane
    f32 w = h*Camera->Aspect;

    vec3 P0 = Camera->Position;
    vec3 CNBL = vec3(-w, -h, -n);
    vec3 CNBR = vec3( w, -h, -n);
    vec3 CNTL = vec3(-w,  h, -n);
    vec3 CNTR = vec3( w,  h, -n);

    mat3 CameraToWorld = GetCameraToWorldRotation(Camera);
    vec3 DirBL = Normalized(CameraToWorld*CNBL);
    vec3 DirBR = Normalized(CameraToWorld*CNBR);
    vec3 DirTL = Normalized(CameraToWorld*CNTL);
    vec3 DirTR = Normalized(CameraToWorld*CNTR);

    //vec3 Q = vec3(0.f, 0.f, 1.f); // point on the plane
    vec3 GroundNormal = vec3(0.f, 0.f, 1.f);
    f32 DenomBL = Dot(GroundNormal, DirBL);
    f32 DenomBR = Dot(GroundNormal, DirBR);
    f32 DenomTL = Dot(GroundNormal, DirTL);
    f32 DenomTR = Dot(GroundNormal, DirTR);

    // NOTE: if Denominator == 0, then the default index is 0,0. is this a problem?
    cull_chunk_itarator ChunkIterator = {};

    if (DenomBL != 0) {
        f32 t = Dot(GroundNormal, (/*Q*/ - P0))/DenomBL;
        if (t > 0.f) {
            vec3 Intersection = P0 + t*DirBL;
            ChunkIterator.BottomLeft = Clamp(WorldPositionToChunkIndex(Intersection), vec2i(0, GRID_WIDTH - 1), vec2i(0, GRID_HEIGHT - 1));
        }
    }
    if (DenomBR != 0) {
        f32 t = Dot(GroundNormal, (/*Q*/ - P0))/DenomBR;
        if (t > 0.f) {
            vec3 Intersection = P0 + t*DirBR;
            ChunkIterator.BottomRight = Clamp(WorldPositionToChunkIndex(Intersection), vec2i(0, GRID_WIDTH - 1), vec2i(0, GRID_HEIGHT - 1));
        }
    }
    if (DenomTL != 0) {
        f32 t = Dot(GroundNormal, (/*Q*/ - P0))/DenomTL;
        if (t > 0.f) {
            vec3 Intersection = P0 + t*DirTL;
            ChunkIterator.TopLeft = Clamp(WorldPositionToChunkIndex(Intersection), vec2i(0, GRID_WIDTH - 1), vec2i(0, GRID_HEIGHT - 1));
        }
    }
    if (DenomTR != 0) {
        f32 t = Dot(GroundNormal, (/*Q*/ - P0))/DenomTR;
        if (t > 0.f) {
            vec3 Intersection = P0 + t*DirTR;
            ChunkIterator.TopRight = Clamp(WorldPositionToChunkIndex(Intersection), vec2i(0, GRID_WIDTH - 1), vec2i(0, GRID_HEIGHT - 1));
        }
    }
    
    ChunkIterator.CurrentIndex = ChunkIterator.BottomLeft;
    world_chunk *Chunk = GetNextChunk(World, &ChunkIterator);
    Assert(Chunk);
    World->VisibleChunks = Chunk;
    while (Chunk) {
        ++World->VisibleChunkCount;
        Chunk->NextVisible = GetNextChunk(World, &ChunkIterator);
        Chunk = Chunk->NextVisible;
    }
}
#endif

static inline void PushWorldChunk(render_group *Group, world_chunk *Chunk) {
    vec3 ChunkP = ChunkIndexToWorldPosition(Chunk->Index);
    for (u32 i = 0; i < CHUNK_WIDTH*CHUNK_HEIGHT; ++i) {
        grid_cell *Cell = &Chunk->Ground.Cells[i];
        vec3 P = ChunkP + Cell->Position;
        f32 SideLength = 1.f;
        f32 Height = 1.f;
        vec4 Color = Cell->Color;
        PushCube(Group, P, SideLength, Height, Color);
    }
}

static void UpdateAndRenderWorld(world *World, program_input *Input, render_commands *RenderCommands) {

    if (!World->Initialized) {
        InitializeWorld(World);
    }

    if (ButtonPressed(Input, BUTTON_KEY_ESCAPE)) {
        World->DebugMode = !World->DebugMode;
        if (World->DebugMode) {
            World->CurrentCamera = &World->DebugCamera;
        }
        else {
            World->CurrentCamera = &World->Camera;
        }
    }

    UpdateCamera(World->CurrentCamera, World->Up, Input);

    // don't specify setup every frame lol
    render_setup Setup = {};
    Setup.Camera = *World->CurrentCamera;
    // TODO: maintain aspect?
    Setup.TargetAspect = (f32)Input->ClientWidth/Input->ClientHeight;
    render_group RenderGroup_ = BeginRenderGroup(RenderCommands, &Setup);
    render_group *RenderGroup = &RenderGroup_;
    PushClearColor(RenderGroup);
    PushClearDepthBuffer(RenderGroup);

    for (i32 j = 0; j < GRID_HEIGHT; ++j) {
        for (i32 i = 0; i < GRID_WIDTH; ++i) {
            world_chunk *Chunk = GetChunkAt(World, i, j);
            PushWorldChunk(RenderGroup, Chunk);
        }
    }

    EndRenderGroup(RenderGroup);
};
