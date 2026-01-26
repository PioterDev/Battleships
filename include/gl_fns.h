/**
 * Copyright © 2026 Piotr Mikolajewski
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
#ifndef GL_FNS_H
#define GL_FNS_H

#include <stdbool.h>
#include <GL/gl.h>


#define GL_FNS_STRS(...) #__VA_ARGS__
#define GL_FNS_STR_(x) #x
#define GL_FNS_STR(x) GL_FNS_STR_(x)

#ifdef DEBUG_GL
#define GL(...) do { \
    __VA_ARGS__; \
    GLenum err = glGetError(); \
    if(err != 0) printf( \
        __FILE__":"GL_FNS_STR(__LINE__)":"GL_FNS_STRS(__VA_ARGS__)": %u\n", err \
    ); \
} while(0)
#else
#define GL(...) __VA_ARGS__
#endif //DEBUG_OPENGL


typedef void (GLAPIENTRY *GLdebugproc)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);

//<function-name>, <return type>, <args...>
//To add a new thing here, either find its signature in OpenGL docs
//or yoink from glew.h by finding a variable `__glew<function-name>` export definition
//and jumping to its function pointer type definition.
//For `glCreateShader`: find "__glewCreateShader;" and find a typedef for "PFNGLCREATESHADERPROC"
#define GL_PROCS \
    GL_PROC(glCreateShader, GLuint, GLenum type) \
    GL_PROC(glShaderSource, void, GLuint shader, GLsizei count, const GLchar *const* string, const GLint* length) \
    GL_PROC(glCompileShader, void, GLuint shader) \
    GL_PROC(glGetShaderiv, void, GLuint shader, GLenum pname, GLint* param) \
    GL_PROC(glGetShaderInfoLog, void, GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog) \
    GL_PROC(glCreateProgram, GLuint, void) \
    GL_PROC(glAttachShader, void, GLuint program, GLuint shader) \
    GL_PROC(glLinkProgram, void, GLuint program) \
    GL_PROC(glGetProgramiv, void, GLuint program, GLenum pname, GLint* param) \
    GL_PROC(glGetProgramInfoLog, void, GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog) \
    GL_PROC(glDeleteShader, void, GLuint shader) \
    GL_PROC(glGenVertexArrays, void, GLsizei n, GLuint* arrays) \
    GL_PROC(glGenBuffers, void, GLsizei n, GLuint* buffers) \
    GL_PROC(glBindVertexArray, void, GLuint array) \
    GL_PROC(glBindBuffer, void, GLenum target, GLuint buffer) \
    GL_PROC(glBufferData, void, GLenum target, GLsizeiptr size, const void* data, GLenum usage) \
    GL_PROC(glBufferSubData, void, GLenum target, GLintptr offset, GLsizeiptr size, const void* data) \
    GL_PROC(glVertexAttribPointer, void, GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer) \
    GL_PROC(glEnableVertexAttribArray, void, GLuint index) \
    GL_PROC(glUseProgram, void, GLuint program) \
    GL_PROC(glDeleteVertexArrays, void, GLsizei n, const GLuint* arrays) \
    GL_PROC(glDeleteBuffers, void, GLsizei n, const GLuint* buffers) \
    GL_PROC(glDeleteProgram, void, GLuint program) \
    GL_PROC(glUniformMatrix4fv, void, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) \
    GL_PROC(glUniform3fv, void, GLint location, GLsizei count, const GLfloat* value) \
    GL_PROC(glUniform4fv, void, GLint location, GLsizei count, const GLfloat* value) \
    GL_PROC(glUniform1i, void, GLint location, GLint v0) \
    GL_PROC(glGetUniformLocation, GLint, GLuint program, const GLchar* name) \
    GL_PROC(glGetAttribLocation, GLint, GLuint program, const GLchar* name) \
    GL_PROC(glDebugMessageCallback, void, GLdebugproc callback, const void *userParam) \
    GL_PROC(glMapBuffer, void*, GLenum target, GLenum access) \
    GL_PROC(glUnmapBuffer, GLboolean, GLenum target) \
    GL_PROC(glVertexAttribFormat, void, GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset) \
    GL_PROC(glBindVertexBuffer, void, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride)


#define GL_PROC(fn, ret, ...) typedef ret (GLAPIENTRY *fn##_pfn) (__VA_ARGS__);

GL_PROCS
#undef GL_PROC

#define GL_PROC(fn, ret, ...) extern __attribute__((visibility("hidden"))) fn##_pfn fn;

GL_PROCS
#undef GL_PROC

bool load_gl_procs(void);

#ifdef GL_FNS_IMPLEMENTATION
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

#define GL_PROC(fn, ret, ...) fn##_pfn fn = NULL;
GL_PROCS
#undef GL_PROC

bool load_gl_procs(void) {
    bool result = false;
    // void* (*glXGetProcAddress)(const GLubyte*) = NULL;
    void* self = dlopen(NULL, RTLD_LAZY | RTLD_GLOBAL);
    if(self == NULL) {
        fprintf(stderr, "Failed to open binary object of itself: %s\n", dlerror());
        goto exit;
    }
    // *(void**)&glXGetProcAddress = dlsym(self, "glXGetProcAddress");
#define GL_PROC(fn, ret, ...) \
    *(void**)&fn = GL_getProcAddress(#fn); \
    if(fn == NULL) { \
        const char *e = dlerror(); \
        fprintf(stderr, "Failed to load " #fn ": %s", e != NULL ? e : "Success"); \
        goto defer; \
    }
    /* if(glXGetProcAddress != NULL) {
        printf("Using 'glXGetProcAddress'\n");
#define GL_getProcAddress(sym) glXGetProcAddress((const GLubyte*)sym)
GL_PROCS
#undef GL_getProcAddress
    } else */ {
#define GL_getProcAddress(sym) dlsym(self, sym)
GL_PROCS
#undef GL_getProcAddress
    }
#undef GL_PROC

    result = true;
defer:
    dlclose(self);
exit:
    return result;
}
#endif //GL_FNS_IMPLEMENTATION
#endif //GL_FNS_H
