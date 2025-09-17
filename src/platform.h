#pragma once

enum log_level;

#define PLATFORM_VIRTUAL_ALLOCATE(name) void *name(size_t Size)
typedef PLATFORM_VIRTUAL_ALLOCATE(platform_virtual_allocate);

#define PLATFORM_DEBUG_PRINT(name) void name(log_level Level, const char *Filename, u32 Line, const char *Message, ...)
typedef PLATFORM_DEBUG_PRINT(platform_debug_print);

struct platform_api {
    platform_debug_print *DebugPrint;
    platform_virtual_allocate *Allocate;
};

static platform_api *GlobalPlatform;
