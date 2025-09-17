#pragma once

PLATFORM_VIRTUAL_ALLOCATE(PlatformAllocate) {
    return VirtualAlloc(0, Size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
}

PLATFORM_DEBUG_PRINT(PlatformDebugPrint) {
    char Buffer[1024];
    i32 PrefixLength = 0;

    switch(Level) {
        case LOG_TRACE: {
            PrefixLength = snprintf(Buffer, sizeof(Buffer), "trace: %s:%u: ", Filename, Line);
        } break;

        case LOG_DEBUG: {
            PrefixLength = snprintf(Buffer, sizeof(Buffer), "debug: %s:%u: ", Filename, Line);
        } break;

        case LOG_INFO: {
            PrefixLength = snprintf(Buffer, sizeof(Buffer), "info:  %s:%u: ", Filename, Line);
        } break;

        case LOG_WARN: {
            PrefixLength = snprintf(Buffer, sizeof(Buffer), "warn:  %s:%u: ", Filename, Line);
        } break;

        case LOG_ERROR: {
            PrefixLength = snprintf(Buffer, sizeof(Buffer), "error: %s:%u: ", Filename, Line);
        } break;

        case LOG_FATAL: {
            PrefixLength = snprintf(Buffer, sizeof(Buffer), "FATAL: %s:%u: ", Filename, Line);
        } break;
    }
    
    va_list Args;
    va_start(Args, Message);
    vsnprintf(Buffer + PrefixLength, sizeof(Buffer) - PrefixLength, Message, Args);
    va_end(Args);
    snprintf(Buffer + strlen(Buffer), sizeof(Buffer) - strlen(Buffer), "\n");

    OutputDebugString(Buffer);
}

