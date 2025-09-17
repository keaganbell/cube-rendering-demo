#pragma once

static opengl *WindowsSetupOpenGL(HDC DC, platform_api *Platform) {
    opengl *OpenGL = NULL;

    PIXELFORMATDESCRIPTOR DesiredPixelFormat = {};
    DesiredPixelFormat.nSize      = sizeof(PIXELFORMATDESCRIPTOR);
    DesiredPixelFormat.nVersion   = 1;
    DesiredPixelFormat.dwFlags    = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    DesiredPixelFormat.iPixelType = PFD_TYPE_RGBA;
    DesiredPixelFormat.cColorBits = 24;
    DesiredPixelFormat.cAlphaBits = 8;
    DesiredPixelFormat.iLayerType = PFD_MAIN_PLANE;

    i32 SuggestedPixelFormatIndex = ChoosePixelFormat(DC, &DesiredPixelFormat);
    PIXELFORMATDESCRIPTOR SuggestedPixelFormat = {};
    DescribePixelFormat(DC, SuggestedPixelFormatIndex, sizeof(SuggestedPixelFormat), &SuggestedPixelFormat);
    SetPixelFormat(DC, SuggestedPixelFormatIndex, &SuggestedPixelFormat);

    HGLRC RC = wglCreateContext(DC);
    if (wglMakeCurrent(DC, RC)) {
        wgl_create_context_attribs_arb *wglCreateContextAttribsARB = (wgl_create_context_attribs_arb *)wglGetProcAddress("wglCreateContextAttribsARB");
        if (wglCreateContextAttribsARB) {
            i32 Attribs[] = {
                WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
                WGL_CONTEXT_MINOR_VERSION_ARB, 3,
                WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB, WGL_CONTEXT_PROFILE_MASK_ARB,
                WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
                0
            };
            // INFO: ShareContext can be used to communicate texture handles 
            // accross different contexts for multi-threaded texture loading
            HGLRC ShareContext = 0;
            HGLRC ModernRC = wglCreateContextAttribsARB(DC, ShareContext, Attribs);
            if (ModernRC) {
                LINFO(GlobalPlatform->DebugPrint, "Modern OpenGL rendering context created.");
                wglMakeCurrent(DC, ModernRC);
                wglDeleteContext(RC);
                RC = ModernRC;

                wgl_swap_interval_ext *wglSwapIntervalEXT = (wgl_swap_interval_ext *)wglGetProcAddress("wglSwapIntervalEXT");
                if (wglSwapIntervalEXT) {
                    wglSwapIntervalEXT(1);
                }
                else {
                    LINFO(GlobalPlatform->DebugPrint, "No VSync support.");
                }

#define Win32GetOpenGLFunction(Name) OpenGL->Name = (type_##Name *)wglGetProcAddress(#Name)
                OpenGL = (opengl *)Platform->Allocate(sizeof(opengl));

                Win32GetOpenGLFunction(glCreateShader);
                Win32GetOpenGLFunction(glShaderSource);
                Win32GetOpenGLFunction(glCompileShader);
                Win32GetOpenGLFunction(glGetShaderInfoLog);
                Win32GetOpenGLFunction(glGetShaderiv);
                Win32GetOpenGLFunction(glDeleteShader);

                Win32GetOpenGLFunction(glCreateProgram);
                Win32GetOpenGLFunction(glAttachShader);
                Win32GetOpenGLFunction(glLinkProgram);
                Win32GetOpenGLFunction(glUseProgram);
                Win32GetOpenGLFunction(glValidateProgram);
                Win32GetOpenGLFunction(glGetProgramInfoLog);
                Win32GetOpenGLFunction(glGetProgramiv);
                Win32GetOpenGLFunction(glDeleteProgram);

                Win32GetOpenGLFunction(glGetUniformLocation);
                Win32GetOpenGLFunction(glUniformMatrix4fv);
                Win32GetOpenGLFunction(glUniform4fv);
                Win32GetOpenGLFunction(glUniform1i);
                Win32GetOpenGLFunction(glUniform1f);
                Win32GetOpenGLFunction(glUniform2fv);
                Win32GetOpenGLFunction(glUniform3fv);

                Win32GetOpenGLFunction(glGenVertexArrays);
                Win32GetOpenGLFunction(glBindVertexArray);
                Win32GetOpenGLFunction(glGenBuffers);
                Win32GetOpenGLFunction(glBindBuffer);
                Win32GetOpenGLFunction(glBufferData);
                Win32GetOpenGLFunction(glBufferSubData);

                Win32GetOpenGLFunction(glGetAttribLocation);
                Win32GetOpenGLFunction(glBindAttribLocation);
                Win32GetOpenGLFunction(glVertexAttribPointer);
                Win32GetOpenGLFunction(glVertexAttribIPointer);
                Win32GetOpenGLFunction(glEnableVertexAttribArray);
                Win32GetOpenGLFunction(glDisableVertexAttribArray);

                Win32GetOpenGLFunction(glGenFramebuffers);
                Win32GetOpenGLFunction(glBindFramebuffer);
                Win32GetOpenGLFunction(glFramebufferTexture2D);
                Win32GetOpenGLFunction(glBlitFramebuffer);
                Win32GetOpenGLFunction(glGenRenderbuffers);
                Win32GetOpenGLFunction(glBindRenderbuffer);
                Win32GetOpenGLFunction(glRenderbufferStorage);
                Win32GetOpenGLFunction(glRenderbufferStorageMultisample);
                Win32GetOpenGLFunction(glFramebufferRenderbuffer);
                Win32GetOpenGLFunction(glCheckFramebufferStatus);

                Win32GetOpenGLFunction(glTexImage2DMultisample);
                Win32GetOpenGLFunction(glActiveTexture);

                Win32GetOpenGLFunction(glDrawElementsBaseVertex);
                Win32GetOpenGLFunction(glDebugMessageCallback);

                // TODO: check for extra extensions and put them here. e.g.,
                // if(Info.OpenGL_ARB_framebuffer_object)
                // {
                //     Win32GetOpenGLFunction(glBindFramebuffer);
                //     Win32GetOpenGLFunction(glGenFramebuffers);
                //     Win32GetOpenGLFunction(glFramebufferTexture2D);
                //     Win32GetOpenGLFunction(glCheckFramebufferStatus);
                // }
            }
            else {
                LERROR(GlobalPlatform->DebugPrint, "Could not create modern OpenGL rendering context.");
            }
        }
        else {
            LERROR(GlobalPlatform->DebugPrint, "wglCreateContextAttribsARB not found.");
        }
    }
    else {
        LERROR(GlobalPlatform->DebugPrint, "Failed to create OpenGL context.");
    }
    return OpenGL;
}
