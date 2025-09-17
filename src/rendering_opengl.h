#pragma once

// NOTE: HH uses a separate program for what maybe could be called
// a render pass: 
//  - opengl_program_common
//      * handle
//      * attrib locations (position, normal, uv, ...)
//      * samplers[16]
//
//  - zbias_program
//      * opengl_program_common
//      * transform id
//      * camera position
//      * fog direction, color, distance...
//      * clipping alpha stuff (i think this was for moving up and down stairs)
//  
//  IDEA
//  - postprocess_program
//      * opengl_program_common
//      * raytracing data...

// Basically create a program for each stage of the game's render pipeline.
// Maybe could say that there are 5 render passes per frame? Render to each
// render target (framebuffer) and then composite them on the screen.
//
// struct opengl {
//     opengl_framebuffer ResolveFramebuffer;
//     opengl_framebuffer DepthPeelBuffer; //[16];
//     opengl_framebuffer DepthPeelResolveBuffer[16];
//
//     zbias_program ZBiasNoDepthPeel; // NOTE(casey): Pass 0
//     zbias_program ZBiasDepthPeel; // NOTE(casey): Passes 1 through n
//     opengl_program_common PeelComposite; // NOTE(casey): Composite all passes
//     opengl_program_common FinalStretch;
//     opengl_program_common ResolveMultisample;
// }

struct opengl_info {
    char *Vendor;
    char *Renderer;
    char *Version;
    char *ShadingLanguageVersion;
    // TODO: add more info about which extensions are supported.
    //b32 blah ARB_Framebuffer_sRGB;
    //b32 blah blah sRGB;
    //...
};

// these are the layout locations in the shaders
enum attribute_type {
    ATTRIBUTE_TYPE_COLOR,
    ATTRIBUTE_TYPE_POSITION,
    ATTRIBUTE_TYPE_NORMAL,
    ATTRIBUTE_TYPE_TEXCOORD,
    ATTRIBUTE_TYPE_TEXTUREID,

    ATTRIBUTE_TYPE_MAX_COUNT
};

struct opengl_base_program {
    GLuint Handle;
};

struct opengl_main_program {
    opengl_base_program Base;

    // Uniforms
    GLuint PVM;
    GLuint LightPos;
    GLuint LightColor;
    GLuint CameraPos;
};

struct opengl_debug_program {
    opengl_base_program Base;
    
    // Uniforms
    GLuint PVM;
};

struct opengl_final_composite_program {
    opengl_base_program Base;
};

#define RENDERER_PUSH_BUFFER_MAX_SIZE (1 << 16) // 64 kiB
struct opengl {
    // platform_renderer header;
    // game_render_settings CurrentSettings;
    // GLuint TextureHandles...;

    u32 MaxVertexCount;
    vertex *Vertices;

    u32 MaxIndexCount;
    u32 *Indices;

    // BATCH BUFFER
    GLuint VAO;
    GLuint VBO;
    GLuint IBO;
    GLint Attributes[ATTRIBUTE_TYPE_MAX_COUNT];
    GLuint MultisampledFBO;
    GLuint MultisampledTexture;
    GLuint DepthRBO;

    // FINAL COMPOSITED IMAGE
    GLuint CompositeFBO;
    GLuint CompositeVAO;
    GLuint CompositeVBO;
    GLuint CompositeTexture;

#define RENDER_TARGET_WIDTH 1920
#define RENDER_TARGET_HEIGHT 1080
    u32 TargetWidth;
    u32 TargetHeight;

    u8 PushBufferMemory[RENDERER_PUSH_BUFFER_MAX_SIZE];
    render_commands RenderCommands;

    opengl_main_program MainProgram;
    opengl_debug_program DebugProgram;
    opengl_final_composite_program FinalCompositeProgram;

#define OpenGLFunction(Name) type_##Name *Name

    OpenGLFunction(glCreateShader);
    OpenGLFunction(glShaderSource);
    OpenGLFunction(glCompileShader);
    OpenGLFunction(glGetShaderInfoLog);
    OpenGLFunction(glGetShaderiv);
    OpenGLFunction(glDeleteShader);

    OpenGLFunction(glCreateProgram);
    OpenGLFunction(glAttachShader);
    OpenGLFunction(glLinkProgram);
    OpenGLFunction(glUseProgram);
    OpenGLFunction(glValidateProgram);
    OpenGLFunction(glGetProgramInfoLog);
    OpenGLFunction(glGetProgramiv);
    OpenGLFunction(glDeleteProgram);

    OpenGLFunction(glGetUniformLocation);
    OpenGLFunction(glUniformMatrix4fv);
    OpenGLFunction(glUniform4fv);
    OpenGLFunction(glUniform1i);
    OpenGLFunction(glUniform1f);
    OpenGLFunction(glUniform2fv);
    OpenGLFunction(glUniform3fv);

    OpenGLFunction(glGenVertexArrays);
    OpenGLFunction(glBindVertexArray);
    OpenGLFunction(glGenBuffers);
    OpenGLFunction(glBindBuffer);
    OpenGLFunction(glBufferData);
    OpenGLFunction(glBufferSubData);

    OpenGLFunction(glGetAttribLocation);
    OpenGLFunction(glBindAttribLocation);
    OpenGLFunction(glVertexAttribPointer);
    OpenGLFunction(glVertexAttribIPointer);
    OpenGLFunction(glEnableVertexAttribArray);
    OpenGLFunction(glDisableVertexAttribArray);

    OpenGLFunction(glGenFramebuffers);
    OpenGLFunction(glBindFramebuffer);
    OpenGLFunction(glFramebufferTexture2D);
    OpenGLFunction(glBlitFramebuffer);
    OpenGLFunction(glGenRenderbuffers);
    OpenGLFunction(glBindRenderbuffer);
    OpenGLFunction(glRenderbufferStorage);
    OpenGLFunction(glRenderbufferStorageMultisample);
    OpenGLFunction(glFramebufferRenderbuffer);
    OpenGLFunction(glCheckFramebufferStatus);

    OpenGLFunction(glTexImage2DMultisample);
    OpenGLFunction(glActiveTexture);

    OpenGLFunction(glDrawElementsBaseVertex);

    OpenGLFunction(glDebugMessageCallback);
};
