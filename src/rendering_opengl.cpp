#pragma once

static opengl_info GetOpenGLInfo() {
    opengl_info Result = {};
    Result.Vendor = (char *)glGetString(GL_VENDOR);
    Result.Renderer = (char *)glGetString(GL_RENDERER);
    Result.Version = (char *)glGetString(GL_VERSION);
    //glGetIntegerv(GL_NUM_EXTENSIONS, &Result.ExtensionCount);
    //for (i32 i = 0; i < Result.ExtensionCount) {
    //    char *Extension = (char *)glGetStringi(GL_EXTENSIONS, i);
    //    if (StrEqual(Extension, ...)) {
    //        Result.(extenstion) = true;
    //    }
    //    else if (StrEqual(Extension, ...)) {
    //        ...
    //    }
    //}
    Result.ShadingLanguageVersion = (char *)glGetString(GL_SHADING_LANGUAGE_VERSION);
    
    return Result;
}

static GLuint CompileShaderProgram(opengl *OpenGL, char *VertexCode, char *FragmentCode) {
    
    GLuint VertexShader = OpenGL->glCreateShader(GL_VERTEX_SHADER);
    GLchar *VertexShaderSource[] = {
        // defines, header, pass these in through params
        VertexCode
    };
    OpenGL->glShaderSource(VertexShader, ArrayCount(VertexShaderSource), VertexShaderSource, 0);
    OpenGL->glCompileShader(VertexShader);
    GLint Success;
    OpenGL->glGetShaderiv(VertexShader, GL_COMPILE_STATUS, &Success);
    if (Success != GL_TRUE) {
        char Buffer[1024] = {};
        OpenGL->glGetShaderInfoLog(VertexShader, sizeof(Buffer), 0, Buffer);
        LERROR(GlobalPlatform->DebugPrint, "Vertex Shader: %s", Buffer);
    }

    GLuint FragmentShader = OpenGL->glCreateShader(GL_FRAGMENT_SHADER);
    GLchar *FragmentShaderSource[] = {
        // defines, header, pass these in through params
        FragmentCode
    };
    OpenGL->glShaderSource(FragmentShader, ArrayCount(FragmentShaderSource), FragmentShaderSource, 0);
    OpenGL->glCompileShader(FragmentShader);
    OpenGL->glGetShaderiv(FragmentShader, GL_COMPILE_STATUS, &Success);
    if (Success != GL_TRUE) {
        char Buffer[1024] = {};
        OpenGL->glGetShaderInfoLog(FragmentShader, sizeof(Buffer), 0, Buffer);
        LERROR(GlobalPlatform->DebugPrint, "Fragment Shader: %s", Buffer);
    }

    GLuint Result = OpenGL->glCreateProgram();
    OpenGL->glAttachShader(Result, VertexShader);
    OpenGL->glAttachShader(Result, FragmentShader);
    OpenGL->glLinkProgram(Result);
    OpenGL->glGetProgramiv(Result, GL_LINK_STATUS, &Success);
    if (Success != GL_TRUE) {
        char Buffer[1024] = {};
        OpenGL->glGetProgramInfoLog(Result, sizeof(Buffer), 0, Buffer);
        LERROR(GlobalPlatform->DebugPrint, "Program Link: %s", Buffer);
    }

    OpenGL->glValidateProgram(Result);
    OpenGL->glGetProgramiv(Result, GL_VALIDATE_STATUS, &Success);
    if (Success != GL_TRUE) {
        char Buffer[1024] = {};
        OpenGL->glGetProgramInfoLog(Result, sizeof(Buffer), 0, Buffer);
        LERROR(GlobalPlatform->DebugPrint, "Program Validation: %s", Buffer);
    }

    OpenGL->glDeleteShader(VertexShader);
    OpenGL->glDeleteShader(FragmentShader);

    return Result;
}

static inline void CreateMainProgram(opengl_main_program *Result, opengl *OpenGL) {

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

        gl_Position = PVM*vec4(_Position, 1.0f);
    }
    )FOO";

    char *FragmentSource = R"FOO(
    #version 430 core

    in vec4 Color;
    in vec3 Normal;
    in vec2 UV;
    in vec3 FragPos;

    flat in int TexID;

    uniform vec3 LightPos;
    uniform vec3 LightColor;
    uniform vec3 CameraPos;

    out vec4 FragColor;

    void main() {
        vec3 Norm = normalize(Normal);
        vec3 LightDir = normalize(LightPos - FragPos);
        vec3 ViewDir = normalize(CameraPos - FragPos);
        float LightDistance = distance(LightPos, FragPos);

        vec3 LightPos2 = vec3(3.0, 1.0, 2.0);
        vec3 LightDir2 = normalize(LightPos2 - FragPos);
        float LightDistance2 = distance(LightPos2, FragPos);
        
        float Ambient = 0.25;
        float Diffuse1 = 2.0*max(dot(Norm, LightDir), 0.0)/(LightDistance*LightDistance);
        float Diffuse2 = 4.0*max(dot(Norm, LightDir2), 0.0)/(LightDistance2*LightDistance2);
        float Specular = 0.1*pow(max(dot(ViewDir, reflect(-LightDir, Norm)), 0.0), 8);

        vec4 LitColor = vec4(LightColor*(Ambient + Diffuse1 + Diffuse2 + Specular), 1.0);
        if (TexID == 0) {
            FragColor = Color*LitColor;
        }
        else {
            FragColor = vec4(Color.rg*UV, 0.0, 1.0)*LitColor;
        }
    }
    )FOO";

    Result->Base.Handle = CompileShaderProgram(OpenGL, VertexSource, FragmentSource);
    GLuint Handle = Result->Base.Handle;
    OpenGL->glUseProgram(Result->Base.Handle);
    OpenGL->glBindAttribLocation(Handle, ATTRIBUTE_TYPE_COLOR, "_Color");
    OpenGL->glBindAttribLocation(Handle, ATTRIBUTE_TYPE_POSITION, "_Position");
    OpenGL->glBindAttribLocation(Handle, ATTRIBUTE_TYPE_NORMAL, "_Normal");
    OpenGL->glBindAttribLocation(Handle, ATTRIBUTE_TYPE_TEXCOORD, "_UV");
    OpenGL->glBindAttribLocation(Handle, ATTRIBUTE_TYPE_TEXTUREID, "_TexID");

    Result->PVM = OpenGL->glGetUniformLocation(Handle, "PVM");
    Result->LightPos = OpenGL->glGetUniformLocation(Handle, "LightPos");
    Result->LightColor = OpenGL->glGetUniformLocation(Handle, "LightColor");
    Result->CameraPos = OpenGL->glGetUniformLocation(Handle, "CameraPos");
}

static inline void CreateDebugProgram(opengl_debug_program *Result, opengl *OpenGL) {
    char *VertexSource = R"FOO(
    #version 430 core

    layout(location = 0) in vec4 _Color;
    layout(location = 1) in vec3 _Position;

    //uniform mat4 PVM;

    out vec4 Color;

    void main() {
        Color = _Color;
        //gl_Position = PVM*vec4(_Position, 1.0);
        gl_Position = vec4(_Position, 1.0);
    }

    )FOO";

    char *FragmentSource = R"FOO(
    #version 430 core

    in vec4 Color;
    out vec4 FragColor;

    void main() {
        FragColor = Color;
    }
    )FOO";

    Result->Base.Handle = CompileShaderProgram(OpenGL, VertexSource, FragmentSource);
    GLuint Handle = Result->Base.Handle;
    OpenGL->glUseProgram(Result->Base.Handle);
    OpenGL->glBindAttribLocation(Handle, ATTRIBUTE_TYPE_COLOR, "_Color");
    OpenGL->glBindAttribLocation(Handle, ATTRIBUTE_TYPE_POSITION, "_Position");

    Result->PVM = OpenGL->glGetUniformLocation(Handle, "PVM");
}

static inline void CreateFinalCompositeProgram(opengl_final_composite_program *Result, opengl *OpenGL) {
    char *VertexSource = R"FOO(
    #version 430 core

    layout(location = 0) in vec2 _Position;
    layout(location = 1) in vec2 _UV;

    out vec2 UV;

    void main() {
        UV = _UV;
        gl_Position = vec4(_Position, 0.0, 1.0);
    }

    )FOO";

    char *FragmentSource = R"FOO(
    #version 430 core

    in vec2 UV;

    uniform sampler2D Tex;

    out vec4 FragColor;

    void main() {
        FragColor = texture(Tex, UV);
    }
    )FOO";

    Result->Base.Handle = CompileShaderProgram(OpenGL, VertexSource, FragmentSource);
    GLuint Handle = Result->Base.Handle;
    OpenGL->glUseProgram(Result->Base.Handle);
    OpenGL->glBindAttribLocation(Handle, 0, "_Position");
    OpenGL->glBindAttribLocation(Handle, 1, "_UV");
}

static void SetupBatchRenderer(opengl *OpenGL) {
    CreateMainProgram(&OpenGL->MainProgram, OpenGL);
    CreateDebugProgram(&OpenGL->DebugProgram, OpenGL);

    OpenGL->glGenVertexArrays(1, &OpenGL->VAO);
    OpenGL->glBindVertexArray(OpenGL->VAO);

    OpenGL->glGenBuffers(1, &OpenGL->VBO);
    OpenGL->glBindBuffer(GL_ARRAY_BUFFER, OpenGL->VBO);
    OpenGL->glBufferData(GL_ARRAY_BUFFER, OpenGL->MaxVertexCount*sizeof(vertex), 0, GL_DYNAMIC_DRAW);

    OpenGL->glGenBuffers(1, &OpenGL->IBO);
    OpenGL->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, OpenGL->IBO);
    OpenGL->glBufferData(GL_ELEMENT_ARRAY_BUFFER, OpenGL->MaxIndexCount*sizeof(u32), 0, GL_DYNAMIC_DRAW);

    OpenGL->glVertexAttribPointer (ATTRIBUTE_TYPE_COLOR, 4, GL_FLOAT, false, sizeof(vertex), (void *)OFFSETOF(vertex, Color));
    OpenGL->glVertexAttribPointer (ATTRIBUTE_TYPE_POSITION, 3, GL_FLOAT, false, sizeof(vertex), (void *)OFFSETOF(vertex, Position));
    OpenGL->glVertexAttribPointer (ATTRIBUTE_TYPE_NORMAL, 3, GL_FLOAT, false, sizeof(vertex), (void *)OFFSETOF(vertex, Normal));
    OpenGL->glVertexAttribPointer (ATTRIBUTE_TYPE_TEXCOORD, 2, GL_FLOAT, false, sizeof(vertex), (void *)OFFSETOF(vertex, UV));
    OpenGL->glVertexAttribIPointer(ATTRIBUTE_TYPE_TEXTUREID, 1, GL_INT, sizeof(vertex), (void *)OFFSETOF(vertex, TextureID));
    OpenGL->glEnableVertexAttribArray(0);
    OpenGL->glEnableVertexAttribArray(1);
    OpenGL->glEnableVertexAttribArray(2);
    OpenGL->glEnableVertexAttribArray(3);
    OpenGL->glEnableVertexAttribArray(4);

    OpenGL->glBindVertexArray(0);
}

static char *opengl_message_source_strings[] = {
    "GL_DEBUG_SOURCE_API",
    "GL_DEBUG_SOURCE_WINDOW_SYSTEM",
    "GL_DEBUG_SOURCE_SHADER_COMPILER",
    "GL_DEBUG_SOURCE_THIRD_PARTY",
    "GL_DEBUG_SOURCE_APPLICATION",
    "GL_DEBUG_SOURCE_OTHER"
};
static char *opengl_message_type_strings[] = {
    "GL_DEBUG_TYPE_ERROR",
    "GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR",
    "GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR",
    "GL_DEBUG_TYPE_PORTABILITY",
    "GL_DEBUG_TYPE_PERFORMANCE",
    "GL_DEBUG_TYPE_MARKER",
    "GL_DEBUG_TYPE_PUSH_GROUP",
    "GL_DEBUG_TYPE_POP_GROUP",
    "GL_DEBUG_TYPE_OTHER"
};
void DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
    (void)userParam; (void)length; (void)id;
    i32 source_index = sizeof(opengl_message_source_strings) - 1;
    switch (source) {
        case GL_DEBUG_SOURCE_API:                  { source_index = 0; } break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:        { source_index = 1; } break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER:      { source_index = 2; } break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:          { source_index = 3; } break;
        case GL_DEBUG_SOURCE_APPLICATION:          { source_index = 4; } break;
        default: {}
    }
    i32 type_index   = sizeof(opengl_message_type_strings) - 1;
    switch (type) {
        case GL_DEBUG_TYPE_ERROR:                      { type_index = 0; } break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:        { type_index = 1; } break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:         { type_index = 2; } break;
        case GL_DEBUG_TYPE_PORTABILITY:                { type_index = 3; } break;
        case GL_DEBUG_TYPE_PERFORMANCE:                { type_index = 4; } break;
        case GL_DEBUG_TYPE_MARKER:                     { type_index = 5; } break;
        case GL_DEBUG_TYPE_PUSH_GROUP:                 { type_index = 6; } break;
        case GL_DEBUG_TYPE_POP_GROUP:                  { type_index = 7; } break;
    }
    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH: {
            LERROR(GlobalPlatform->DebugPrint, "%s (%s): %s", opengl_message_source_strings[source_index], opengl_message_type_strings[type_index], message);
        } break;
        case GL_DEBUG_SEVERITY_MEDIUM: {
            LWARN(GlobalPlatform->DebugPrint, "%s (%s): %s", opengl_message_source_strings[source_index], opengl_message_type_strings[type_index], message);
        } break;
        case GL_DEBUG_SEVERITY_LOW:
        case GL_DEBUG_SEVERITY_NOTIFICATION:
        default: {}
    }
}

// TODO: if I want to delete and allocate Vulkan for example. I need to add
// a way to free all the opengl memory
static void InitOpenGL(opengl *OpenGL, platform_api *Platform, opengl_info Info) {
    (void)Info;
    OpenGL->TargetWidth = RENDER_TARGET_WIDTH;
    OpenGL->TargetHeight = RENDER_TARGET_HEIGHT;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_CULL_FACE);
    //glFrontFace(GL_CCW);
    //glCullFace(GL_BACK);
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

    u32 MaxQuadCount = (1<<20); // 1 Mi Quads
    OpenGL->MaxVertexCount = MaxQuadCount*4;
    OpenGL->MaxIndexCount = MaxQuadCount*6;
    OpenGL->Vertices = (vertex *)Platform->Allocate(OpenGL->MaxVertexCount*sizeof(vertex));
    OpenGL->Indices = (u32 *)Platform->Allocate(OpenGL->MaxIndexCount*sizeof(u32));
    Assert(OpenGL->Vertices && OpenGL->Indices);

    SetupBatchRenderer(OpenGL);
    
    OpenGL->glGenFramebuffers(1, &OpenGL->MultisampledFBO);
    OpenGL->glBindFramebuffer(GL_FRAMEBUFFER, OpenGL->MultisampledFBO);

    glGenTextures(1, &OpenGL->MultisampledTexture);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, OpenGL->MultisampledTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    OpenGL->glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, OpenGL->TargetWidth, OpenGL->TargetHeight, GL_TRUE);
    OpenGL->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, OpenGL->MultisampledTexture, 0);

    OpenGL->glGenRenderbuffers(1, &OpenGL->DepthRBO);
    OpenGL->glBindRenderbuffer(GL_RENDERBUFFER, OpenGL->DepthRBO);
    OpenGL->glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT24, OpenGL->TargetWidth, OpenGL->TargetHeight);
    OpenGL->glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, OpenGL->DepthRBO);

    GLenum Status = OpenGL->glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (Status != GL_FRAMEBUFFER_COMPLETE) {
        LERROR(GlobalPlatform->DebugPrint, "Framebuffer incomplete: 0x%x", Status);
    }
    OpenGL->glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // FINAL COMPOSITITE IMAGE
    CreateFinalCompositeProgram(&OpenGL->FinalCompositeProgram, OpenGL);

    OpenGL->glGenFramebuffers(1, &OpenGL->CompositeFBO);
    OpenGL->glBindFramebuffer(GL_FRAMEBUFFER, OpenGL->CompositeFBO);

    glGenTextures(1, &OpenGL->CompositeTexture);
    glBindTexture(GL_TEXTURE_2D, OpenGL->CompositeTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, OpenGL->TargetWidth, OpenGL->TargetHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    OpenGL->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, OpenGL->CompositeTexture, 0);
    Status = OpenGL->glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (Status != GL_FRAMEBUFFER_COMPLETE) {
        LERROR(GlobalPlatform->DebugPrint, "Framebuffer incomplete: 0x%x", Status);
    }

    f32 QuadVertices[] = {
        // Positions   // TexCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
    };

    OpenGL->glGenVertexArrays(1, &OpenGL->CompositeVAO);
    OpenGL->glBindVertexArray(OpenGL->CompositeVAO);

    OpenGL->glGenBuffers(1, &OpenGL->CompositeVBO);
    OpenGL->glBindBuffer(GL_ARRAY_BUFFER, OpenGL->CompositeVBO);
    OpenGL->glBufferData(GL_ARRAY_BUFFER, sizeof(QuadVertices), QuadVertices, GL_STATIC_DRAW);

    OpenGL->glEnableVertexAttribArray(0); // hard code this in the shader i guess
    OpenGL->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)0);
    OpenGL->glEnableVertexAttribArray(1);
    OpenGL->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(2 * sizeof(float)));
    
    OpenGL->glBindVertexArray(0);

    OpenGL->glDebugMessageCallback(DebugCallback, NULL);

    OpenGL->glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
    OpenGL->glBindVertexArray(0);
    OpenGL->glBindBuffer(GL_ARRAY_BUFFER, 0);
}

static void OpenGLBeginFrame(opengl *OpenGL, u32 ClientWidth, u32 ClientHeight) {
    
    // reset the state for the beginning of the frame. resetting the push
    // buffer back to the base
    render_commands *RenderCommands = &OpenGL->RenderCommands;
    RenderCommands->PushBufferBase = OpenGL->PushBufferMemory;
    RenderCommands->PushBufferAt = OpenGL->PushBufferMemory;
    RenderCommands->Cap = sizeof(OpenGL->PushBufferMemory);

    RenderCommands->MaxVertexCount = OpenGL->MaxVertexCount;
    RenderCommands->VertexCount = 0;
    RenderCommands->Vertices = OpenGL->Vertices;

    RenderCommands->MaxIndexCount = OpenGL->MaxIndexCount;
    RenderCommands->IndexCount = 0;
    RenderCommands->Indices = OpenGL->Indices;

    RenderCommands->ClientWidth = ClientWidth;
    RenderCommands->ClientHeight = ClientHeight;

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

static inline void BeginUseProgram(opengl *OpenGL, opengl_debug_program *Program, render_setup *Setup) {
    (void)Setup;
    OpenGL->glUseProgram(Program->Base.Handle);
    OpenGL->glEnableVertexAttribArray(ATTRIBUTE_TYPE_COLOR);
    OpenGL->glEnableVertexAttribArray(ATTRIBUTE_TYPE_POSITION);
//    OpenGL->glEnableVertexAttribArray(ATTRIBUTE_TYPE_NORMAL);
//    OpenGL->glEnableVertexAttribArray(ATTRIBUTE_TYPE_TEXCOORD);
//    OpenGL->glEnableVertexAttribArray(ATTRIBUTE_TYPE_TEXTUREID);
}

static inline void BeginUseProgram(opengl *OpenGL, opengl_main_program *Program, render_setup *Setup) {
    (void)Setup;
    OpenGL->glUseProgram(Program->Base.Handle);
    OpenGL->glEnableVertexAttribArray(ATTRIBUTE_TYPE_COLOR);
    OpenGL->glEnableVertexAttribArray(ATTRIBUTE_TYPE_POSITION);
    OpenGL->glEnableVertexAttribArray(ATTRIBUTE_TYPE_NORMAL);
    OpenGL->glEnableVertexAttribArray(ATTRIBUTE_TYPE_TEXCOORD);
    OpenGL->glEnableVertexAttribArray(ATTRIBUTE_TYPE_TEXTUREID);
}

static inline void EndUseProgram(opengl *OpenGL, opengl_main_program *Program) {
    (void)Program;
    OpenGL->glUseProgram(0);
    OpenGL->glDisableVertexAttribArray(ATTRIBUTE_TYPE_COLOR);
    OpenGL->glDisableVertexAttribArray(ATTRIBUTE_TYPE_POSITION);
    OpenGL->glDisableVertexAttribArray(ATTRIBUTE_TYPE_NORMAL);
    OpenGL->glDisableVertexAttribArray(ATTRIBUTE_TYPE_TEXCOORD);
    OpenGL->glDisableVertexAttribArray(ATTRIBUTE_TYPE_TEXTUREID);
}

static inline void EndUseProgram(opengl *OpenGL, opengl_debug_program *Program) {
    (void)Program;
    OpenGL->glUseProgram(0);
    OpenGL->glDisableVertexAttribArray(ATTRIBUTE_TYPE_COLOR);
    OpenGL->glDisableVertexAttribArray(ATTRIBUTE_TYPE_POSITION);
//    OpenGL->glDisableVertexAttribArray(ATTRIBUTE_TYPE_NORMAL);
//    OpenGL->glDisableVertexAttribArray(ATTRIBUTE_TYPE_TEXCOORD);
//    OpenGL->glDisableVertexAttribArray(ATTRIBUTE_TYPE_TEXTUREID);
}

static inline void BeginUseProgram(opengl *OpenGL, opengl_final_composite_program *Program) {
    OpenGL->glUseProgram(Program->Base.Handle);
    OpenGL->glEnableVertexAttribArray(0);
    OpenGL->glEnableVertexAttribArray(1);
}

static inline void EndUseProgram(opengl *OpenGL, opengl_final_composite_program *Program) {
    (void)Program;
    OpenGL->glUseProgram(0);
    OpenGL->glEnableVertexAttribArray(0);
    OpenGL->glEnableVertexAttribArray(1);
}

static void ProcessTexturedQuadBatchCommand(opengl *OpenGL, textured_quad_batch_command *Batch) {
    //OpenGL->glBindFramebuffer(GL_FRAMEBUFFER, OpenGL->MultisampledFBO);
    OpenGL->glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, OpenGL->TargetWidth, OpenGL->TargetHeight);

    OpenGL->glBindVertexArray(OpenGL->VAO);
    OpenGL->glBindBuffer(GL_ARRAY_BUFFER, OpenGL->VBO);
    OpenGL->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, OpenGL->IBO);

    //BeginUseProgram(OpenGL, &OpenGL->MainProgram, &Batch->RenderSetup);
    opengl_main_program *Program = &OpenGL->MainProgram;
    OpenGL->glUseProgram(Program->Base.Handle);

    OpenGL->glBufferSubData(GL_ARRAY_BUFFER, 0, Batch->VertexCount*sizeof(vertex), Batch->Vertices);
    OpenGL->glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, Batch->IndexCount*sizeof(u32), Batch->Indices);

    mat4 ProjectionView = CalculateCameraProjectionView(&Batch->RenderSetup.Camera, Batch->RenderSetup.TargetAspect);
    OpenGL->glUniformMatrix4fv(Program->PVM, 1, GL_TRUE, ProjectionView.Elements);

    static f32 t;
    t += 0.0367f;
    vec3 LightWorldPos = vec3(-2.f, 1.f, 2.f);
    OpenGL->glUniform3fv(Program->LightPos, 1, LightWorldPos.Elements);
    OpenGL->glUniform3fv(Program->LightColor, 1, vec3(1.f, 1.f, 1.f).Elements);
    OpenGL->glUniform3fv(Program->CameraPos, 1, Batch->RenderSetup.Camera.Position.Elements);
    
    glDrawElements(GL_TRIANGLES, Batch->IndexCount, GL_UNSIGNED_INT, 0);

    //EndUseProgram(OpenGL, &OpenGL->MainProgram);
    OpenGL->glBindBuffer(GL_ARRAY_BUFFER, 0);
    OpenGL->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    OpenGL->glBindVertexArray(0);
    OpenGL->glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void ProcessLineBatchCommand(opengl *OpenGL, line_batch_command *Lines) {
    OpenGL->glBindFramebuffer(GL_FRAMEBUFFER, 0);
    BeginUseProgram(OpenGL, &OpenGL->DebugProgram, &Lines->RenderSetup);
    opengl_debug_program *Program = &OpenGL->DebugProgram;

    OpenGL->glBindVertexArray(OpenGL->VAO);
    OpenGL->glBindBuffer(GL_ARRAY_BUFFER, OpenGL->VBO);
    OpenGL->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, OpenGL->IBO);
    OpenGL->glBufferSubData(GL_ARRAY_BUFFER, 0, Lines->VertexCount*sizeof(vertex), Lines->Vertices);

    mat4 ProjectionView = CalculateCameraProjectionView(&Lines->RenderSetup.Camera, Lines->RenderSetup.TargetAspect);
    OpenGL->glUniformMatrix4fv(Program->PVM, 1, GL_TRUE, ProjectionView.Elements);

    glDrawArrays(GL_LINES, 0, Lines->VertexCount);

    EndUseProgram(OpenGL, &OpenGL->DebugProgram);
    OpenGL->glBindBuffer(GL_ARRAY_BUFFER, 0);
    OpenGL->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    OpenGL->glBindVertexArray(0);
}

static void OpenGLEndFrame(opengl *OpenGL) {
    // scan through the render commands push buffer and do the actual rendering
    // also TODO: figure out a render pass pipeline. how to use multiple framebuffers etc

    render_commands *RenderCommands = &OpenGL->RenderCommands;

    for (u8 *CommandAt = RenderCommands->PushBufferBase; CommandAt < RenderCommands->PushBufferAt;) {
        render_command_header *Typeless = (render_command_header*)CommandAt;
        switch(Typeless->Type) {
            case TYPE_clear_color_command: {
                clear_color_command *Command = (clear_color_command *)Typeless;
                //glClearColor(Command->ClearColor.r, Command->ClearColor.g, Command->ClearColor.b, Command->ClearColor.a);
                //glClear(GL_COLOR_BUFFER_BIT);
                CommandAt += sizeof(*Command);
            } break;

            case TYPE_clear_depth_command: {
                clear_depth_command *Command = (clear_depth_command *)Typeless;
                //glClear(GL_DEPTH_BUFFER_BIT);
                CommandAt += sizeof(*Command);
            } break;

            case TYPE_textured_quad_batch_command: {
                textured_quad_batch_command *Batch = (textured_quad_batch_command *)Typeless;
                // TODO: Check that the batch is valid
                ProcessTexturedQuadBatchCommand(OpenGL, Batch);
                CommandAt += sizeof(*Batch);
            } break;

            case TYPE_line_batch_command: {
                line_batch_command *Lines = (line_batch_command *)Typeless;
                // TODO: Check that the batch is valid
                ProcessLineBatchCommand(OpenGL, Lines);
                CommandAt += sizeof(*Lines);
            } break;

            default:
                // INVALID
                Assert(0);
        }
    }

    //OpenGL->glBindFramebuffer(GL_READ_FRAMEBUFFER, OpenGL->MultisampledFBO); // Source (multisampled)
    //OpenGL->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, OpenGL->CompositeFBO); // Destination (regular)
    //#if 1
    //OpenGL->glBlitFramebuffer(0, 0, OpenGL->TargetWidth, OpenGL->TargetHeight,
    //                          0, 0, OpenGL->TargetWidth, OpenGL->TargetHeight,
    //                          GL_COLOR_BUFFER_BIT, GL_LINEAR);
    //#else
    //OpenGL->glBlitFramebuffer(0, 0, OpenGL->TargetWidth, OpenGL->TargetHeight,
    //                          0, 0, OpenGL->TargetWidth, OpenGL->TargetHeight,
    //                          GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    //#endif
    //OpenGL->glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //OpenGL->glBindVertexArray(OpenGL->CompositeVAO);
    //OpenGL->glBindBuffer(GL_ARRAY_BUFFER, OpenGL->CompositeVBO);
    //BeginUseProgram(OpenGL, &OpenGL->FinalCompositeProgram);

    //glViewport(0, 0, RenderCommands->ClientWidth, RenderCommands->ClientHeight);
    //glClearColor(1.f, 0.f, 1.f, 1.f);
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //OpenGL->glActiveTexture(GL_TEXTURE0);
    //glBindTexture(GL_TEXTURE_2D, OpenGL->CompositeTexture);
    //glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    //EndUseProgram(OpenGL, &OpenGL->FinalCompositeProgram);
    //OpenGL->glBindBuffer(GL_ARRAY_BUFFER, 0);
    //OpenGL->glBindVertexArray(0);
}
