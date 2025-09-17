#define _CRT_SECURE_NO_WARNINGS
#include <math.h>

#define GL_GLEXT_PROTOTYPES
#include "GL/glcorearb.h"
#include "GL/wglext.h"

#include "defines.h"
#include "log.h"
#include "platform.h"
#include "arena.h"
#include "rendering_math.h"
#include "input.h"
#include "random.h"
#include "windows_opengl.h"
#include "rendering_camera.h"
#include "rendering_world.h"
#include "rendering_renderer.h"
#include "rendering_opengl.h" // opengl struct needs to be filled out by platform
#include "rendering.h"

#include "windows_platform.cpp"
#include "windows_opengl.cpp"

#ifndef APP_DLL_NAME
#define APP_DLL_NAME "app.dll"
#endif

struct windows_state {
    b32 Running;
    u32 ClientWidth;
    u32 ClientHeight;

    b32 IsActive;
    HWND Window;
};
static windows_state GlobalState;

struct program_code {
    b32 IsValid;
    FILETIME LastWriteTime;
    HMODULE CodeDLL;
    program_initialize *Initialize;
    program_update_and_render *UpdateAndRender;
};

static inline FILETIME GetLastWriteTime(char *Filename) {
    FILETIME LastWriteTime = {};
    WIN32_FIND_DATA FindData;
    HANDLE FindHandle = FindFirstFileA(Filename, &FindData);
    if (FindHandle != INVALID_HANDLE_VALUE) {
        LastWriteTime = FindData.ftLastWriteTime;
        FindClose(FindHandle);
    }
    return LastWriteTime;
}

static inline program_code GetProgramCode(char *SourceDLLName) {
    program_code Result = {};
    Result.LastWriteTime = GetLastWriteTime(SourceDLLName);
#ifndef RELEASE_BUILD
    CopyFile(SourceDLLName, "temp_" APP_DLL_NAME, false);
    Result.CodeDLL = LoadLibraryA("temp_" APP_DLL_NAME);
#else
    Result.CodeDLL = LoadLibraryA(APP_DLL_NAME);
#endif
    if (Result.CodeDLL) {
        Result.Initialize = (program_initialize *)GetProcAddress(Result.CodeDLL, "Initialize");
        Result.UpdateAndRender = (program_update_and_render *)GetProcAddress(Result.CodeDLL, "UpdateAndRender");
        if (Result.Initialize && Result.UpdateAndRender) {
            Result.IsValid = true;
        }
    }
    if (!Result.IsValid) {
        Result.Initialize = InitializeStub;
        Result.UpdateAndRender = UpdateAndRenderStub;
    }
    return Result;
}

#ifndef RELEASE_BUILD
static inline void UnloadProgramCode(program_code *Code) {
    Code->IsValid = false;
    Code->Initialize = InitializeStub;
    Code->UpdateAndRender = UpdateAndRenderStub;

    if (Code->CodeDLL) {
        FreeLibrary(Code->CodeDLL);
    }
}
#endif

LRESULT CALLBACK MainCallback(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam) {
    switch(Message) {

        case WM_CREATE: {
            RECT Client;
            GetClientRect(Window, &Client);
            GlobalState.ClientWidth = Client.right - Client.left;
            GlobalState.ClientHeight = Client.bottom - Client.top;
        } break;

        case WM_ACTIVATE: {
            if (WParam == WA_ACTIVE || WParam == WA_CLICKACTIVE) {
                GlobalState.IsActive = true;
            }
            else if (WParam == WA_INACTIVE) {
                // TODO: disable input stuff
                GlobalState.IsActive = false;
            }
        } break;

        case WM_SIZE: {
            GlobalState.ClientWidth = LOWORD(LParam);
            GlobalState.ClientHeight = HIWORD(LParam);
        } break;

        case WM_CLOSE: {
            GlobalState.Running = false;
        } break;

        //case WM_SYSKEYDOWN:
        //case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP: {
            Assert(!"Input came through callback");
        } break;

        case WM_DESTROY: {
            GlobalState.Running = false;
        } break;

        default:
            return DefWindowProc(Window, Message, WParam, LParam);
    };

    return 0;
}

static void ProcessMessages(HWND Window, program_input *NewInput, program_input *OldInput) {
    MSG Message = {};
    while(PeekMessageA(&Message, Window, 0, 0, PM_REMOVE)) {
        if (Message.message == WM_QUIT) {
            GlobalState.Running = false;
        }

        switch (Message.message) {

            case WM_MOUSEMOVE: {
                NewInput->MousePosition = vec2i(LOWORD(Message.lParam), HIWORD(Message.lParam));
                NewInput->MouseDelta = NewInput->MousePosition - OldInput->MousePosition;
            }

            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP: {
                b8 AltKeyWasUp = (Message.lParam & (1 << 29));
                if (Message.wParam == VK_F4 && !AltKeyWasUp) {
                    // TODO: hitem with the "aRe YuO sURe!?"
                    GlobalState.Running = false;
                }
            }

            case WM_KEYDOWN:
            case WM_KEYUP: {
                b8 WasDown = ((Message.lParam & (1 << 30)) == 0);
                b8 IsUp = ((Message.lParam & (1 << 31)) != 0);
                if (WasDown != IsUp) {
                    if (Message.wParam == VK_SPACE) {
                        UpdateButton(BUTTON_KEY_SPACE, NewInput, IsUp);
                    }
                    else if (Message.wParam == VK_ESCAPE) {
                        UpdateButton(BUTTON_KEY_ESCAPE, NewInput, IsUp);
                    }
                    else if (Message.wParam == 'W') {
                        UpdateButton(BUTTON_KEY_W, NewInput, IsUp);
                    }
                    else if (Message.wParam == 'A') {
                        UpdateButton(BUTTON_KEY_A, NewInput, IsUp);
                    }
                    else if (Message.wParam == 'S') {
                        UpdateButton(BUTTON_KEY_S, NewInput, IsUp);
                    }
                    else if (Message.wParam == 'D') {
                        UpdateButton(BUTTON_KEY_D, NewInput, IsUp);
                    }
                    else if (Message.wParam == VK_LEFT) {
                        UpdateButton(BUTTON_KEY_LEFT, NewInput, IsUp);
                    }
                    else if (Message.wParam == VK_RIGHT) {
                        UpdateButton(BUTTON_KEY_RIGHT, NewInput, IsUp);
                    }
                    else if (Message.wParam == VK_UP) {
                        UpdateButton(BUTTON_KEY_UP, NewInput, IsUp);
                    }
                    else if (Message.wParam == VK_DOWN) {
                        UpdateButton(BUTTON_KEY_DOWN, NewInput, IsUp);
                    }
                }
            } break;

            case WM_LBUTTONDOWN: {
                UpdateButton(BUTTON_MOUSE_LEFT, NewInput, false);
            } break;

            case WM_LBUTTONUP: {
                UpdateButton(BUTTON_MOUSE_LEFT, NewInput, true);
            } break;

            case WM_RBUTTONDOWN: {
                UpdateButton(BUTTON_MOUSE_RIGHT, NewInput, false);
            } break;

            case WM_RBUTTONUP: {
                UpdateButton(BUTTON_MOUSE_RIGHT, NewInput, true);
            } break;

            default: {
                TranslateMessage(&Message);
                DispatchMessageA(&Message);
            }
        }
    }
}

int WinMain(HINSTANCE Instance, HINSTANCE PreviousInstance, PSTR CmdLine, int CmdShow) {
    (void)CmdShow; (void)CmdLine; (void)PreviousInstance;
    platform_api Platform = {};
    Platform.DebugPrint = PlatformDebugPrint;
    Platform.Allocate = PlatformAllocate;
    GlobalPlatform = &Platform;

    WNDCLASS WindowClass = {};
    WindowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = MainCallback;
    WindowClass.hInstance = Instance;
    WindowClass.hIcon = NULL;
    WindowClass.hCursor = NULL;
    WindowClass.lpszClassName = "MainWindowClass";

    RegisterClassA(&WindowClass);

    HWND Window = CreateWindowA(
        WindowClass.lpszClassName,
        "Rendering Practice",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL,
        NULL,
        Instance,
        NULL
    );

    if (Window) {
        GlobalState.Window = Window;
        HDC DC = GetDC(Window);
        opengl *OpenGL = WindowsSetupOpenGL(DC, GlobalPlatform);
        Assert(OpenGL);

        char *SourceDLLName = APP_DLL_NAME;
        program_code Program = GetProgramCode(SourceDLLName);

        program_memory Memory = {};
        Memory.PermanentSize = 64*MiB;
        Memory.FrameScratchSize = 1*GiB;
        Memory.PermanentStorage = VirtualAlloc(NULL, Memory.PermanentSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
        Memory.FrameScratchStorage = VirtualAlloc(NULL, Memory.FrameScratchSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

        if (Program.Initialize) {
            Program.Initialize(&Memory, &Platform, OpenGL);
        }

        program_input _InputA = {};
        program_input _InputB = {};
        program_input *NewInput = &_InputA;
        program_input *OldInput = &_InputB;

        LARGE_INTEGER LastFrameCounter;
        QueryPerformanceCounter(&LastFrameCounter);
        LARGE_INTEGER CounterFrequencyResult;
        QueryPerformanceFrequency(&CounterFrequencyResult);
        i64 CounterFrequency = CounterFrequencyResult.QuadPart;

        GlobalState.Running = true;
        while (GlobalState.Running) {
#ifndef RELEASE_BUILD
            FILETIME FileTime = GetLastWriteTime(SourceDLLName);
            if (CompareFileTime(&Program.LastWriteTime, &FileTime) != 0) {
                UnloadProgramCode(&Program);
                Program = GetProgramCode(SourceDLLName);
            }
#endif
            ProcessMessages(Window, NewInput, OldInput);
            if (GlobalState.IsActive) {
                b32 ShiftIsUp = GetKeyState(VK_SHIFT) & (1 << 15);
                UpdateButton(BUTTON_KEY_SHIFT, NewInput, !ShiftIsUp);
            }
            NewInput->ClientWidth = GlobalState.ClientWidth;
            NewInput->ClientHeight = GlobalState.ClientHeight;

            LARGE_INTEGER BeginFrameCounter;
            QueryPerformanceCounter(&BeginFrameCounter);
            i64 CounterElapsed = BeginFrameCounter.QuadPart - LastFrameCounter.QuadPart;
            NewInput->Frametime = (f64)CounterElapsed/CounterFrequency;
            LastFrameCounter = BeginFrameCounter;
            char Title[64] = {};
            sprintf(Title, "Rendering Practice | fps: %.0f", (f32)(1.f/NewInput->Frametime));
            SetWindowText(Window, Title);

            if (Program.UpdateAndRender) {
                Program.UpdateAndRender(&Memory, &Platform, OpenGL, NewInput);
            }

            SwapBuffers(DC); // TODO: maybe lock the framerate if vsync isnt supported

            // TODO: fix 
            program_input *Temp = NewInput;
            NewInput = OldInput;
            *NewInput = {};
            OldInput = Temp;
            NewInput->MousePosition = OldInput->MousePosition;
            for (u32 i = 0; i < BUTTON_TYPE_MAX_COUNT; ++i) {
                NewInput->Buttons[i].Down = OldInput->Buttons[i].Down;
            }
        }
    }
    else {
        LFATAL(Platform.DebugPrint, "Failed to create window.");
    }

    return 0;
}
