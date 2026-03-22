//Copyright (c) 2026 Piotr Mikołajewski
#pragma once

#include <stdint.h>
#include <stddef.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <lac.h>

#include "Backend.h"

#define FROM_26_6(v) ((float)((v)/64) + (float)((v)%64)/64.0f)

typedef struct { //@sa https://freetype.org/freetype2/docs/glyphs/glyphs-3.html
    uint32_t width, height;
    struct {  int32_t x, y; } bearing; //NOTE: only horizontal is supported
    struct { uint32_t x, y; } advance; //NOTE: 26.6 fixed point
    struct { uint32_t x, y; } atlasCoords; //in pixels, upper left corner
    //NOTE: values are normalized
    union {
        struct {
            //ul    ur
            //
            //ll    lr
            Vec2 ul, ur, lr, ll;
        };
        Vec2 vs[4];
        float data[4*2];
    } texCoords;
} Character;

typedef struct {
    //Vertical offset from the start of the atlas.
    uint32_t y;
    //Of this row in the atlas. Bitmaps with height under this value may be stored here.
    uint32_t height;
    //Cached sum of widths of bitmaps stored here.
    //Compute from iterating `data` and summing `cpData[data[i]].width`.
    uint32_t currentWidth;
    //Number of codepoint indices stored.
    uint32_t length;
    //It's a dynamic array, you know the drill.
    uint32_t capacity;
    uint32_t _padding[1];
    //Codepoints indices are stored as a flexible array member in the same allocation.
    uint32_t data[1];
} AtlasTextRowHeader;

typedef AtlasTextRowHeader* AtlasTextRow;

typedef struct {
    AtlasTextRow *data;
    size_t length, capacity;
} AtlasTextRows;

typedef struct {
    //Codepoints available in the atlas for a given font height,
    //sorted in ascending order.
    //`data[0]` always contains U+FFFD, therefore search for a codepoint
    //should start from index 1.
    uint32_t *cps;
    //Data associated with each codepoint from `cps`.
    //NOTE: Points to an allocation shared with `cps` at an offset
    //`sizeof(*cps)*capacity`.
    Character *cpData;
    //`cps` and `cpData` are dynamic arrays with identical
    //`length` and `capacity`.
    size_t length, capacity;
    //Bitmap allocation is done row-wise.
    //This isn't super efficient, but it's much easier to reason about than
    //allocating individual rectangles.
    //Rows are per font height for simplicity.
    AtlasTextRows rows;
    //Identifies a particular set of bitmaps.
    //@sa https://freetype.org/freetype2/docs/glyphs/glyphs-3.html
    uint32_t fontHeight;
    uint32_t ascent;    //26.6
     int32_t descent;   //26.6
    uint32_t linespace; //26.6
    //Codepoints for which loading the glyph failed.
    //Stops attempts to add codepoints which would fail to load again in a loop,
    //shortcutting as fake "success" instead.
    //User code should ignore this field and use U+FFFD if a codepoint is not
    //present in `cps`.
    struct {
        uint32_t *data;
        size_t length, capacity;
    } cps_failed;
} AtlasTextDataByHeight;

typedef struct {
    AtlasTextDataByHeight *data;
    size_t length, capacity;
} AtlasTextData;

//All fields should be treated by user code as read-only.
//`font` may be shared by using `FT_(Reference|Done)_Face`.
typedef struct {
    //Holds per-font-height list of available codepoints and their
    //positions in the atlas.
    AtlasTextData data;
    //Font associated with this atlas
    FT_Face font;
    //Optional. May point to a GPU buffer mapped to CPU's address space.
    uint8_t *pixels;
    //Current size of the atlas.
    size_t width, height;
    //The actual atlas.
    Texture tex;
#ifdef RENDER_BACKEND_OPENGL
    //@sa https://songho.ca/opengl/gl_pbo.html
    GLuint pbo;
#endif
} AtlasText;

int AtlasText_init(AtlasText *a, FT_Face font, size_t w, size_t h);
void AtlasText_destroy(AtlasText *a);
//Returns an index to `a->data` with the appropiate `AtlasTextDataByHeight`
//structure or negative if not found.
ssize_t AtlasText_queryHeight(AtlasText *a, uint32_t h);
//Returns an index to a `AtlasTextDataByHeight` structure whose font height is
//closest to `h`, but not smaller, or negative if not found.
ssize_t AtlasText_queryHeight_gte(AtlasText *a, uint32_t h);
//Returns an index to `a->data` with the appropiate `AtlasTextDataByHeight`,
//creating it if necessary or
//-2 if initializing storage for the height failed,
//-3 if initializing the height failed.
ssize_t AtlasText_addHeight(AtlasText *a, uint32_t h);
bool AtlasText_addCodepoint(AtlasText *a, ssize_t hIdx, uint32_t cp);
//Returns an index to `a->data.data[hIdx].cps`, 0 if `cp` was not found,
//-2 if `hIdx` is invalid,
//0 is reserved for U+FFFD.
int32_t AtlasText_queryCodepoint(AtlasText *a, ssize_t hIdx, uint32_t cp);

bool AtlasText_dump(AtlasText *a, const char *filepath);

// bool AtlasText_addCodepoints(AtlasText *a, ssize_t hIdx, uint32_t cp);

// void AtlasText_removeCodepoint(AtlasText *a, ssize_t hIdx, int32_t cp);

//Compact rows as tightly as possible.
//This should be called after deleting a height or a large amount of codepoints.
//Currently unsupported.
// void AtlasText_defragment(AtlasText *a);
