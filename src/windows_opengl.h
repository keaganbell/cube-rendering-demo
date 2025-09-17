#pragma once

typedef HGLRC WINAPI wgl_create_context_attribs_arb(HDC DC, HGLRC SharedContext, i32 *Attribs);
typedef void WINAPI wgl_swap_interval_ext(u32 interval);
//typedef BOOL WINAPI wgl_get_pixel_format_attrib_iv_arb(HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, const int *piAttributes, int *piValues);
//typedef BOOL WINAPI wgl_get_pixel_format_attrib_fv_arb(HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, const int *piAttributes, FLOAT *pfValues);
//typedef BOOL WINAPI wgl_choose_pixel_format_arb(HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);

typedef GLuint WINAPI type_glCreateShader(GLenum type);
typedef void WINAPI type_glDeleteShader (GLuint shader);
typedef void WINAPI type_glShaderSource(GLuint shader, GLsizei count, GLchar **string, GLint *length);
typedef void WINAPI type_glCompileShader(GLuint shader);
typedef void WINAPI type_glGetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void WINAPI type_glGetShaderiv(	GLuint shader, GLenum pname, GLint *params);

typedef GLuint WINAPI type_glCreateProgram(void);
typedef void WINAPI type_glDeleteProgram (GLuint program);
typedef void WINAPI type_glAttachShader(GLuint program, GLuint shader);
typedef void WINAPI type_glLinkProgram(GLuint program);
typedef void WINAPI type_glUseProgram(GLuint program);
typedef void WINAPI type_glValidateProgram(GLuint program);
typedef void WINAPI type_glGetProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void WINAPI type_glGetProgramiv(GLuint program, GLenum pname, GLint *params);

typedef GLint WINAPI type_glGetUniformLocation(GLuint program, const GLchar *name);
typedef void WINAPI type_glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void WINAPI type_glUniform1i(GLint location, GLint v0);
typedef void WINAPI type_glUniform1f(GLint location, GLfloat v0);
typedef void WINAPI type_glUniform2fv(GLint location, GLsizei count, const GLfloat *value);
typedef void WINAPI type_glUniform3fv(GLint location, GLsizei count, const GLfloat *value);
typedef void WINAPI type_glUniform4fv(GLint location, GLsizei count, const GLfloat *value);

typedef void WINAPI type_glGenVertexArrays(GLsizei n, GLuint *arrays);
typedef void WINAPI type_glBindVertexArray(GLuint array);
typedef void WINAPI type_glGenBuffers (GLsizei n, GLuint *buffers);
typedef void WINAPI type_glBindBuffer (GLenum target, GLuint buffer);
typedef void WINAPI type_glBufferData (GLenum target, GLsizeiptr size, const void *data, GLenum usage);
typedef void WINAPI type_glBufferSubData (GLenum target, GLintptr offset, GLsizeiptr size, const void *data);

typedef GLint WINAPI type_glGetAttribLocation(GLuint program, const GLchar *name);
typedef void WINAPI type_glBindAttribLocation(GLuint program, GLuint index, const GLchar *name);
typedef void WINAPI type_glEnableVertexAttribArray(GLuint index);
typedef void WINAPI type_glDisableVertexAttribArray(GLuint index);
typedef void WINAPI type_glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
typedef void WINAPI type_glVertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer);

typedef void WINAPI type_glGenFramebuffers(GLenum target, GLuint *buffers);
typedef void WINAPI type_glBindFramebuffer(GLenum target, GLuint buffer);
typedef void WINAPI type_glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum, GLuint buffer, void *thing);
typedef void WINAPI type_glBlitFramebuffer(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
typedef GLenum WINAPI type_glCheckFramebufferStatus(GLenum target);

typedef void WINAPI type_glGenRenderbuffers(GLsizei n, GLuint *renderbuffers);
typedef void WINAPI type_glBindRenderbuffer(GLenum target, GLuint renderbuffer);
typedef void WINAPI type_glRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
typedef void WINAPI type_glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
typedef void WINAPI type_glRenderbufferStorageMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);

typedef void WINAPI type_glTexImage2DMultisample(GLenum target, GLsizei samples, GLint internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
typedef void WINAPI type_glActiveTexture(GLenum texture);

typedef void WINAPI type_glDrawElementsBaseVertex(GLenum mode, GLsizei count, GLenum type, GLvoid *indices, GLint basevertex);

typedef void DEBUGPROC(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);
typedef void WINAPI type_glDebugMessageCallback(DEBUGPROC callback, void * userParam);
