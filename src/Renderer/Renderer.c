//Copyright (c) 2026 Piotr Mikołajewski
#include "Renderer.h"

#include <stdio.h>
#include <assert.h>

#define GL_FNS_IMPLEMENTATION
#include <gl_fns.h>

PCB_BeforeMain(gl_procs) {
    //Custom OpenGL loader. To hell with GLEW!
    if(!load_gl_procs()) exit(1);
}


PCB_Arena *Renderer_arena(void) {
    static PCB_thread_local PCB_Arena *arena;
    if(arena == NULL) arena = PCB_Arena_init(2*1024*1024);
    return arena;
}

const float default_texCoords[] = {
    0.0f, 0.0f,
    0.0f, 0.0f,
    0.0f, 0.0f,
    0.0f, 0.0f,
};

static const char* rectVertexShader =
    "#version 430 core\n"
    "layout(location = 0) uniform mat4 Porth;\n"
#ifndef TEST
    "layout(location = 0) in vec2 pos;\n"
#else
    "layout(location = 0) in vec4 pos;\n"
#endif
    "void main() {\n"
    "   gl_Position = Porth * vec4(pos.xy, 0.0, 1.0);\n"
    "   gl_Position.y = -gl_Position.y;\n"
    "}\n";
static const char* rectFragmentShader =
    "#version 430 core\n"
    "layout(location = 1) uniform vec4 Color;\n"
    "out vec4 color;\n"
    "void main() {\n"
    "   color = vec4(Color);\n"
    "}\n";

static const char* textVertexShader =
    "#version 430 core\n"
    "layout(location = 0) uniform mat4 Porth;\n"
#ifndef TEST
    "layout(location = 0) in vec2 pos;\n"
    "layout(location = 1) in vec2 tex;\n"
#else
    "layout(location = 0) in vec4 pos;\n"
#endif
    "out vec2 texc;\n"
    "void main() {\n"
    "   gl_Position = Porth * vec4(pos.xy, 0.0, 1.0);\n"
    "   gl_Position.y = -gl_Position.y;\n"
#ifndef TEST
    "   texc = tex;\n"
#else
    "   texc = pos.zw;\n"
#endif
    "}\n";
static const char* textFragmentShader =
    "#version 430 core\n"
    "layout(location = 1) uniform vec4 Color;\n"
    "layout(location = 2) uniform sampler2D tex;\n"
    "in vec2 texc;\n"
    "out vec4 color;\n"
    "void main() {\n"
    "   color = vec4(1.0, 1.0, 1.0, texture(tex, texc).r);\n"
    "   color = vec4(Color) * color;\n"
    "}\n";

static GLuint createShader(GLenum type, const char* src) {
    GLint tmp;
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &tmp);
    if(!(tmp > 0)) return shader;

    const size_t L = (size_t)(GLuint)tmp;
    PCB_Arena *a = Renderer_arena();
    PCB_Arena_scope(a) {
        char* log = PCB_Arena_alloc(a, L);
        if(log == NULL) goto checkStatus;
        glGetShaderInfoLog(shader, tmp, &tmp, log);
    checkStatus:
        glGetShaderiv(shader, GL_COMPILE_STATUS, &tmp);
        if(tmp == GL_FALSE) { glDeleteShader(shader); shader = 0; }
        if(log == NULL) continue;
        if(tmp == GL_FALSE)
            PCB_log(PCB_LOGLEVEL_ERROR, "Failed to compile shader:\n%s", log);
        else
            PCB_log(PCB_LOGLEVEL_INFO, "Shader log: \n%s", log);
    }
    return shader;
}

static GLuint createShaderProgram(const char* vert, const char* frag) {
    GLint tmp;
    GLuint p = 0, v, f;
    size_t L;

    v = createShader(GL_VERTEX_SHADER, vert);
    if(v == 0) goto exit;
    f = createShader(GL_FRAGMENT_SHADER, frag);
    if(f == 0) goto delete_v;

    p = glCreateProgram();
    glAttachShader(p, v);
    glAttachShader(p, f);
    glLinkProgram(p);
    glGetProgramiv(p, GL_INFO_LOG_LENGTH, &tmp);
    if(!(tmp > 0)) goto end;

    L = (size_t)(GLuint)tmp;
    PCB_Arena *a = Renderer_arena();
    PCB_Arena_scope(a) {
        char* log = PCB_Arena_alloc(a, L);
        if(log == NULL) goto checkStatus;
        glGetProgramInfoLog(p, tmp, &tmp, log);
    checkStatus:
        glGetProgramiv(p, GL_COMPILE_STATUS, &tmp);
        if(tmp == GL_FALSE) { glDeleteProgram(p); p = 0; }
        if(log == NULL) continue;
        if(tmp == GL_FALSE)
            PCB_log(PCB_LOGLEVEL_ERROR, "Failed to link shader program:\n%s", log);
        else
            PCB_log(PCB_LOGLEVEL_INFO, "Shader program log: \n%s", log);
    }
end:
    glDeleteShader(f);
delete_v:
    glDeleteShader(v);
exit:
    return p;
}

static void GLAPIENTRY glErrorMessageCallback(
    GLenum source, GLenum type, GLuint id, GLenum severity,
    GLsizei length, const GLchar* message, const void* userParam
) {
    (void)id; (void)userParam; (void)length;
    enum {
        INFO, WARN, ERR
    } level = INFO;
    static const char* levelStr[] = {
        [INFO] = "Info",
        [WARN] = "Warn",
        [ERR] = "Error"
    };
    switch(type) {
        case GL_DEBUG_TYPE_ERROR: level = ERR; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        case GL_DEBUG_TYPE_PORTABILITY:
        case GL_DEBUG_TYPE_PERFORMANCE:
            level = WARN; break;
    }
    const char* sourceStr = "Unknown";
    switch(source) {
        case GL_DEBUG_SOURCE_API:           sourceStr = "API"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM: sourceStr = "Window System"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: /* sourceStr = "Shader Compiler"; break; */return;
        case GL_DEBUG_SOURCE_THIRD_PARTY:   sourceStr = "Third Party"; break;
        case GL_DEBUG_SOURCE_APPLICATION:   sourceStr = "Application"; break;
        case GL_DEBUG_SOURCE_OTHER:         sourceStr = "Other"; break;
    }
    const char* typeStr = "Unknown";
    switch (type) {
        case GL_DEBUG_TYPE_ERROR:               typeStr = "";                    break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typeStr = "Deprecated Behavior"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  typeStr = "Undefined Behavior";  break;
        case GL_DEBUG_TYPE_PORTABILITY:         typeStr = "Portability";         break;
        case GL_DEBUG_TYPE_PERFORMANCE:         typeStr = "Performance";         break;
        case GL_DEBUG_TYPE_MARKER:              typeStr = "Marker";              break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          typeStr = "Push Group";          break;
        case GL_DEBUG_TYPE_POP_GROUP:           typeStr = "Pop Group";           break;
        case GL_DEBUG_TYPE_OTHER:               typeStr = "Other";               break;
    }
    const char* severityStr = NULL;
    switch(severity) {
        case GL_DEBUG_SEVERITY_HIGH:   severityStr = "High";    break;
        case GL_DEBUG_SEVERITY_MEDIUM: severityStr = "Medium";  break;
        case GL_DEBUG_SEVERITY_LOW:    severityStr = "Low";     break;
    }
    if(severityStr == NULL)
        printf("[%s][OpenGL][%s][%s] %s",     levelStr[level], sourceStr, typeStr, message);
    else
        printf("[%s][OpenGL][%s][%s][%s] %s", levelStr[level], severityStr, sourceStr, typeStr, message);
}

static void Renderer__initRectSp(Renderer *r) {
#ifndef TEST
    /*
     * NOTE: shader takes an orthographic projection matrix and will normalize
     * vertices accordingly. Therefore do NOT pass values between -1 and 1,
     * but rather from 0 to window size in the chosen axis.
     * This little thing wasted several hours of my life...
     */
    const float vertices[] = {
        0.0f, 0.0f,
        r->viewport.width, 0.0f,
        r->viewport.width, r->viewport.height,
        0.0f, r->viewport.height,
    };
#else
    const float vertices[] = {
        0.0f, 0.0f,
        0.0f, 0.0f, //tex
        r->viewport.width, 0.0f,
        0.0f, 0.0f, //tex
        r->viewport.width, r->viewport.height,
        0.0f, 0.0f, //tex
        0.0f, r->viewport.height,
        0.0f, 0.0f, //tex
    };
#endif
    GL(glUseProgram(r->rect.sp));
    GL(glUniformMatrix4fv(0, 1, false, r->Porth.arr));
#ifndef TEST
    GL(glGenVertexArrays(1, &r->rect.vao));
    GL(glBindVertexArray(r->rect.vao));
    GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r->ebo));
#endif
    GL(glGenBuffers(1, &r->rect.vbo));
    GL(glBindBuffer(GL_ARRAY_BUFFER, r->rect.vbo));
    GL(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW));
#ifndef TEST
    GL(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0));
    GL(glEnableVertexAttribArray(0));
    GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
#endif

}

static void Renderer__initTextSp(Renderer *r) {
    GL(glUseProgram(r->text.sp));
    GL(glUniformMatrix4fv(0, 1, false, r->Porth.arr));
#ifndef TEST
    GL(glGenVertexArrays(1, &r->text.vao));
    GL(glBindVertexArray(r->text.vao));
    GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r->ebo));
#endif

#ifdef TEST
    GL(glGenBuffers(1, &r->text.vbo));
    GL(glBindBuffer(GL_ARRAY_BUFFER, r->text.vbo));
    GL(glBufferData(GL_ARRAY_BUFFER, sizeof(float)*4*4, NULL, GL_DYNAMIC_DRAW));
#else
    GL(glGenBuffers(2, r->text.vbo.data));

    GL(glBindBuffer(GL_ARRAY_BUFFER, r->text.vbo.pos));
    GL(glBufferData(GL_ARRAY_BUFFER, sizeof(float)*2*4, NULL, GL_DYNAMIC_DRAW));
    GL(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), 0));
    GL(glEnableVertexAttribArray(0));

    GL(glBindBuffer(GL_ARRAY_BUFFER, r->text.vbo.tex));
    GL(glBufferData(GL_ARRAY_BUFFER, sizeof(default_texCoords), default_texCoords, GL_DYNAMIC_DRAW));
    GL(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), 0));
    GL(glEnableVertexAttribArray(1));
    GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
#endif
}

bool Renderer_init(Renderer *r, int window_width, int window_height, FT_Face font) {
    PCB_CHECK_SELF(r, false);
    GL(glDebugMessageCallback(glErrorMessageCallback, NULL));
    GL(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));

    r->rect.sp = createShaderProgram(rectVertexShader, rectFragmentShader);
    if(r->rect.sp == 0) return false;
    r->text.sp = createShaderProgram(textVertexShader, textFragmentShader);
    if(r->text.sp == 0) return false;

    r->viewport.width = (float)window_width;
    r->viewport.height = (float)window_height;
    r->Porth = Mat4_ortho(0.0f, (float)window_width, 0.0f, (float)window_height);

    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
    };

#ifdef TEST
    GL(glGenVertexArrays(1, &r->vao));
    GL(glBindVertexArray(r->vao));
    GL(glEnableVertexAttribArray(0));
    GL(glVertexAttribFormat(0, 4, GL_FLOAT, GL_FALSE, 0));
#endif

    GL(glGenBuffers(1, &r->ebo));
    GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r->ebo));
    GL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW));

    Renderer__initRectSp(r);
    Renderer__initTextSp(r);
    GL(glUseProgram(0));
#ifndef TEST
    GL(glBindVertexArray(0));
#endif

    AtlasText *a = &r->fontAtlas;
    if(AtlasText_init(a, font, 512, 512) != 0) return false;

    return true;
}

void Renderer_destroy(Renderer *r) {
#ifndef TEST
    GL(glDeleteVertexArrays(1, &r->rect.vao));
#endif
    GL(glDeleteBuffers(1, &r->rect.vbo));

#ifndef TEST
    GL(glDeleteVertexArrays(1, &r->text.vao));
    GL(glDeleteBuffers(2, r->text.vbo.data));
#else
    GL(glDeleteBuffers(1, &r->text.vbo));
#endif

    GL(glDeleteBuffers(1, &r->ebo));

    GL(glDeleteProgram(r->rect.sp));
    GL(glDeleteProgram(r->text.sp));
    AtlasText_destroy(&r->fontAtlas);
}

void Renderer_background_color(Renderer *r, Vec4 c) {
    (void)r;
    GL(glClearColor(c.color.r, c.color.g, c.color.b, c.color.a));
}

void Renderer_clear(Renderer *r) {
    (void)r;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer_viewport(Renderer *r, Viewport v) {
    r->viewport = v;
    r->Porth = Mat4_ortho(0.f, v.width, 0.f, v.height);

    GL(glUseProgram(r->text.sp));
    GL(glUniformMatrix4fv(0, 1, false, r->Porth.arr));
    GL(glUseProgram(r->rect.sp));
    GL(glUniformMatrix4fv(0, 1, false, r->Porth.arr));
    GL(glUseProgram(0));

    glViewport((GLint)v.x, (GLint)v.y, (GLsizei)v.width, (GLsizei)v.height);
}

void Renderer_draw_rect(Renderer *r, Vec4 rect, Vec4 c) {
    GL(glUseProgram(r->rect.sp));
    GL(glUniform4fv(1, 1, c.data));
    const float x = rect.rect.x, y = rect.rect.y;
    const float w = rect.rect.w, h = rect.rect.h;
#ifndef TEST
    const float vertices[] = {
        x,     /* r->viewport.height - */ y,
        x + w, /* r->viewport.height - */ y,
        x + w, /* r->viewport.height - */ (y + h),
        x,     /* r->viewport.height - */ (y + h),
    };
#else
    const float vertices[] = {
        x,     y,
        0.0f, 0.0f,
        x + w, y,
        0.0f, 0.0f,
        x + w, (y + h),
        0.0f, 0.0f,
        x,     (y + h),
        0.0f, 0.0f,
    };
#endif
#ifndef TEST
    GL(glBindVertexArray(r->rect.vao));
#endif
    GL(glBindBuffer(GL_ARRAY_BUFFER, r->rect.vbo));
    GL(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices));
#ifdef TEST
    GL(glBindVertexBuffer(0, r->rect.vbo, 0, sizeof(float)*4));
#endif
    GL(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0));
}

static void Renderer__draw_character(
    Renderer *r, const Character *c, Vec2 pos, float rescale
) {
    const Vec2 bearing = Vec2_muls(Vec2_ss((float)c->bearing.x, -(float)c->bearing.y), rescale);
    const Vec2 size = Vec2_muls(Vec2_ss((float)c->width, (float)c->height), rescale);
    const Vec2 ul = Vec2_add(pos, bearing);
#ifndef TEST
    const float vertices[] = {
        ul.x,          ul.y,
        ul.x + size.x, ul.y,
        ul.x + size.x, ul.y + size.y,
        ul.x,          ul.y + size.y,
    };
#else
    const Vec2 *vs = c->texCoords.vs;
    const float vertices[] = {
        ul.x,          ul.y, vs[0].x, vs[0].y,
        ul.x + size.x, ul.y, vs[1].x, vs[1].y,
        ul.x + size.x, ul.y + size.y, vs[2].x, vs[2].y,
        ul.x,          ul.y + size.y, vs[3].x, vs[3].y,
    };
#endif
#ifndef TEST
    GL(glBindBuffer(GL_ARRAY_BUFFER, r->text.vbo.pos));
    GL(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices));
    GL(glBindBuffer(GL_ARRAY_BUFFER, r->text.vbo.tex));
    GL(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(c->texCoords.data), c->texCoords.data));
#else
    GL(glBindBuffer(GL_ARRAY_BUFFER, r->text.vbo));
    GL(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices));
    GL(glBindVertexBuffer(0, r->text.vbo, 0, sizeof(float)*4));
#endif
    GL(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0));
}

bool Renderer_draw_text(
    Renderer *r,
    PCB_StringView text,
    uint32_t h,
    Vec2 pos,
    Vec4 color,
    Renderer_TextDrawEx *ex
) {
    ssize_t hIdx = AtlasText_queryHeight_gte(&r->fontAtlas, h);
    if(hIdx < 0) {
        hIdx = AtlasText_addHeight(&r->fontAtlas, h);
        if(hIdx < 0) {
            PCB_log(
                PCB_LOGLEVEL_ERROR,
                PCB_LOC":%s: Failed to add font height %u to atlas",
                __func__, h
            ); return false;
        }
    }
    AtlasTextDataByHeight *d = &r->fontAtlas.data.data[hIdx];
    GL(glUseProgram(r->text.sp));
    GL(glEnable(GL_BLEND));
    GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    GL(glActiveTexture(GL_TEXTURE0));
    GL(glBindTexture(GL_TEXTURE_2D, r->fontAtlas.tex));
    GL(glUniform4fv(1, 1, color.data));
    GL(glUniform1i(2, 0));
#ifndef TEST
    GL(glBindVertexArray(r->text.vao));
#endif

    const Character *c = &d->cpData[0];
    const float rescale = (float)h/(float)d->fontHeight;
    const float linespace = FROM_26_6(d->linespace) * rescale;
    const float ascent    = FROM_26_6(d->ascent)    * rescale;
    const float descent   = FROM_26_6(d->descent)   * rescale;
#if 0
    PCB_log(PCB_LOGLEVEL_DEBUG, PCB_LOC":%s: text = %.*s", __func__, (int)text.length, text.data);
    PCB_log(PCB_LOGLEVEL_DEBUG, PCB_LOC":%s: pos = (%f, %f)", __func__, pos.x, pos.y);
    PCB_log(PCB_LOGLEVEL_DEBUG, PCB_LOC":%s: ascent = %f, descent = %f", __func__, ascent, descent);
#endif
    if(ex != NULL) {
        switch(ex->flags.ascent) {
            case RENDERER_ZERO:                       break;
            case RENDERER_ONE:       pos.y += ascent; break;
            case RENDERER_MINUS_ONE: pos.y -= ascent; break;
        }
        switch(ex->flags.descent) {
            case RENDERER_ZERO:                        break;
            case RENDERER_ONE:       pos.y += descent; break;
            case RENDERER_MINUS_ONE: pos.y -= descent; break;
        }
    }
    //offset to make `pos` as center point instead of baseline
    // pos.y += (ascent-descent)/2.0f;
    // pos.y += ascent;

    float x = pos.x;
    PCB_Codepoint cp; cp.code = 0; cp.length = 1;
    for(; text.length > 0; text.data += cp.length, text.length -= cp.length) {
        int32_t L = 1, R = (int32_t)d->length - 1;
        cp = PCB_StringView_GetCodepoint(text, 0);
        if(cp.code < 0) {
            switch(cp.code) {
              case -1: case -2: case -3: case -4: case -5: case -6:
                c = &d->cpData[0];
                goto draw_char;
                continue;
              case -16: case -17: case -18:
              default:
                PCB_Unreachable;
            }
        }
        uint32_t cp_ = (uint32_t)cp.code;
        switch(cp.code) {
          case '\n':
            pos.y += linespace;
            x = pos.x;
            continue;
          //other control characters would go here
          default:
            break;
        }
        while(L <= R) {
            int32_t m = L + (R-L)/2;
            if(d->cps[m] < cp_) L = m + 1;
            else if(d->cps[m] > cp_) R = m - 1;
            else { L = R = m; break; }
        }
        if(d->cps[L] == cp_) c = &d->cpData[L];
        else {
            L = 0; R = (int32_t)(uint32_t)d->cps_failed.length - 1;
            while(L <= R) {
                int32_t m = L + (R-L)/2;
                if(d->cps_failed.data[m] < cp_) L = m + 1;
                else if(d->cps_failed.data[m] > cp_) R = m - 1;
                else { L = R = m; break; }
            }
            //not found in "failed" ones and adding failed
            //TODO: Should be scheduled outside of drawing text
            //and executed after swapping buffers
            if(L != R && !AtlasText_addCodepoint(&r->fontAtlas, hIdx, cp_)) {
                PCB_log(PCB_LOGLEVEL_ERROR, "Failed to add U+%X to atlas", cp_);
            }
            L = 0;
            // PCB_log(PCB_LOGLEVEL_WARN, "U+%X not in atlas!", cp_);
        }
    draw_char:
        Renderer__draw_character(r, c, Vec2_ss(x, pos.y), rescale);
        x += FROM_26_6(c->advance.x)*rescale;
        if(d->cps[L] == cp_) c = &d->cpData[0];
    }

    GL(glBindTexture(GL_TEXTURE_2D, 0));
    // pos.y -= ascent/2.0f;
    if(ex != NULL) {
        if(ex->underline > 0.0f) {
            Renderer_draw_rect(r, Vec4_ss(pos.x, pos.y, x - pos.x, ex->underline), color);
        }
    }
    return true;
}
