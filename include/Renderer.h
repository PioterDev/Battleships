//Copyright (c) 2026 Piotr Mikołajewski
#pragma once

#include <PCB.h>

#include <lac.h>

#include "Render/Backend.h"
#include "Render/Text.h"

#define FONT_SIZE 32

PCB_Enum(Renderer_Command_Type, uint32_t) {
    RENDERER_COMMAND_NONE,
    RENDERER_COMMAND_RECT,
    RENDERER_COMMAND_TEXT,
};

typedef struct {
    Vec4 rect;
    union {
        Vec4 color;
        Texture tex;
    } data;
    bool use_tex;
} Renderer_Command_Rect;

PCB_Enum(Renderer_AddFactor, uint8_t) {
    RENDERER_ZERO = 0,  //f(x, a) = x
    RENDERER_ONE,       //f(x, a) = x + a
    RENDERER_MINUS_ONE  //f(x, a) = x - a
};

typedef struct {
    float underline;
    union {
        uint32_t all;
        struct {
            Renderer_AddFactor ascent  : 2;
            Renderer_AddFactor descent : 2;
        };
    } flags;
} Renderer_TextDrawEx;

typedef struct {
    PCB_StringView text;
    Vec4 color;
    Vec2 pos;
    uint32_t h;
    Renderer_TextDrawEx ex;
} Renderer_Command_Text;

typedef struct {
    union {
        Renderer_Command_Rect rect;
        Renderer_Command_Text text;
    } data;
    Renderer_Command_Type type;
} Renderer_Command;

typedef struct {
    Renderer_Command *data;
    size_t length, capacity;
} Renderer_Commands;

// #define TEST

//TODO: break it down
typedef struct {
    Viewport viewport;
    struct {
        GLuint sp;
#ifndef TEST
        GLuint vao;
#endif
        GLuint vbo;
    } rect;
    struct {
        GLuint sp;
#ifndef TEST
        GLuint vao;
        union {
            struct { GLuint pos, tex; };
            GLuint data[2];
        } vbo;
#else
        GLuint vbo;
#endif
    } text;
#ifdef TEST
    GLuint vao;
#endif
    GLuint ebo;
    Mat4 Porth;
    AtlasText fontAtlas;
    Renderer_Commands cmds;

} Renderer;

//arena is thread-local
PCB_Arena *Renderer_arena(void);

bool Renderer_init(Renderer *r, int window_width, int window_height, FT_Face font);
void Renderer_destroy(Renderer *r);
void Renderer_background_color(Renderer *r, Vec4 color);
void Renderer_clear(Renderer *r);
void Renderer_viewport(Renderer *r, Viewport v);

void Renderer_draw_rect(Renderer *r, Vec4 rect, Vec4 color);
void Renderer_draw_rect_tex(Renderer *r, Vec4 rect, Texture tex);

//TODO: We should probably default to `pos` being the upper left corner
//instead of a typographic baseline (from the caller's perspective, no changes
//to the implementation).
bool Renderer_draw_text(
    Renderer *r,
    PCB_StringView text,
    uint32_t h,
    Vec2 pos,
    Vec4 color,
    Renderer_TextDrawEx *ex
);
