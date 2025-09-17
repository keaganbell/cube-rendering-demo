#pragma once

struct memory_arena {
    u8 *Base;
    size_t Used;
    size_t Cap;
};

static inline memory_arena InitializeArena(u8 *Base, size_t Cap) {
    memory_arena Result = {};
    Result.Base = Base;
    Result.Cap = Cap;
    return Result;
}

#define PushArray(Arena, Type, Count) (Type *)_PushArena(Arena, sizeof(Type)*Count)
#define PushStruct(Arena, Type) (Type *)_PushArena(Arena, sizeof(Type))
static inline void *_PushArena(memory_arena *Arena, size_t Size) {
    u8 *Result = NULL;
    if (Arena->Used + Size < Arena->Cap) {
        Result = Arena->Base + Arena->Used;
        Arena->Used += Size;
    }
    else {
        LERROR(GlobalPlatform->DebugPrint, "Arena overflow!");
    }
    return Result;
}
