//Copyright (c) 2026 Piotr Mikołajewski
#include "Render/Text.h"

#include <PCB.h>

#include "gl_fns.h"

#include <stb_image_write.h>

static void Character_none(Character *c, AtlasTextDataByHeight *d) {
    c->width = c->height = 0;
    c->bearing.x = c->bearing.y = 0;
    c->atlasCoords.x = c->atlasCoords.y = 0;
    //stubs so that the renderer shows empty space
    c->advance.x = d->fontHeight/2;
    c->advance.y = d->fontHeight;
    PCB_Arr_forEach_it(c->texCoords.data, it, float) {
        *it = 0.0f;
    }
}

static size_t AtlasTextRow_bytesize(uint32_t capacity) {
    PCB_static_assert(offsetof(AtlasTextRowHeader, data) == 6*sizeof(uint32_t),);
    return 6*sizeof(uint32_t) + capacity*sizeof(uint32_t);
}

static FT_Error load_glyph(
    AtlasText *a, uint32_t cp
) {
    FT_UInt glyph_idx = FT_Get_Char_Index(a->font, (FT_ULong)cp);
    if(glyph_idx == 0) return FT_Err_Invalid_Character_Code;
    FT_Error e = FT_Load_Glyph(a->font, glyph_idx, FT_LOAD_RENDER);
    if(e) {
        PCB_log(
            PCB_LOGLEVEL_ERROR,
            PCB_LOC": Failed to load codepoint U+%X from font '%s/%s': %s",
            cp, a->font->family_name, a->font->style_name, FT_Error_String(e)
        );
        return e;
    }
    return 0;
}

static uint32_t find_first_free_y(AtlasText *a) {
    AtlasTextRow r = NULL;
    PCB_Vec_forEach_it(&a->data, it, AtlasTextDataByHeight) {
        PCB_Vec_forEach_it(&it->rows, jt, AtlasTextRow) {
            if(r == NULL) { r = *jt; continue; }
            if((*jt)->y > r->y) { r = *jt; continue; }
            if((*jt)->height > r->height) r = *jt;
        }
    }
    //+1 so that floating point errors don't pick up pixels from adjacent glyphs
    uint32_t v = r != NULL ? r->y + r->height + 1 : 0;
    return v;
}

static bool AtlasText__expandTex(AtlasText *a, size_t w, size_t h) {
    PCB_assert(w > a->width && h > a->height);
#if RENDER_BACKEND == RENDER_BACKEND_OPENGL
    const size_t wxh = w*h;
    // const size_t wxh = a->width*a->height;
    if(wxh > SIZE_MAX/2) return false;
    //NOTE: OpenGL doesn't give easy way of checking if realloc failed
    GL(glBindBuffer(GL_PIXEL_PACK_BUFFER, a->pbo));

    GL(glBindTexture(GL_TEXTURE_2D, a->tex));
    GL(glBufferData(GL_PIXEL_PACK_BUFFER, (GLsizeiptr)wxh, NULL, GL_STREAM_COPY));
    GL(glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_BYTE, NULL));
    GL(glBindBuffer(GL_PIXEL_PACK_BUFFER, 0));

    GL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, NULL));
    GL(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, a->pbo));
    GL(glTexSubImage2D(
        GL_TEXTURE_2D, 0, 0, 0, (GLsizei)a->width, (GLsizei)a->height,
        GL_RED, GL_UNSIGNED_BYTE, NULL
    ));
    GL(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));

    GL(glBindTexture(GL_TEXTURE_2D, 0));

    Vec2 rescale = Vec2_ss((float)a->width/(float)w, (float)a->height/(float)h);
    PCB_Vec_forEach_it(&a->data, it, const AtlasTextDataByHeight) {
        const size_t N = it->length;
        for(size_t i = 0; i < N; i++) {
            Character *c = &it->cpData[i];
            PCB_Arr_forEach_it(c->texCoords.vs, v) {
                *v = Vec2_mul(*v, rescale);
            }
        }
    }
    a->width = w; a->height = h;
    return true;
#else
    (void)a; (void)w; (void)h;
    return false;
#endif
}

static bool AtlasText__emplaceCp(
    AtlasText *a, AtlasTextDataByHeight *d,
    uint32_t cpIdx, size_t rIdx
) {
    AtlasTextRow r = d->rows.data[rIdx];
    if(r->length >= r->capacity) {
        const size_t new_bytesize = AtlasTextRow_bytesize(r->capacity*2);
        AtlasTextRow r_ = (AtlasTextRow)PCB_realloc(r, new_bytesize);
        if(r_ == NULL) return false;
        d->rows.data[rIdx] = r = r_;
        r->capacity *= 2;
    }
    const FT_GlyphSlot g = a->font->glyph;
    if(r->currentWidth + g->bitmap.width + 1 > a->width) {
        if(!AtlasText__expandTex(a, PCB_X1_5(a->width), PCB_X1_5(a->height)))
            return false;
    }
    Character *const c = &d->cpData[cpIdx];
    const uint32_t
        x = r->currentWidth, y = r->y,
        w = g->bitmap.width, h = g->bitmap.rows;
    PCB_assert(w <= INT32_MAX && h <= INT32_MAX);
    c->bearing.x     = g->bitmap_left;
    c->bearing.y     = g->bitmap_top;
    c->advance.x     = g->advance.x;
    c->advance.y     = g->advance.y;
    c->atlasCoords.x = x;
    c->atlasCoords.y = y;
    c->width         = w;
    c->height        = h;

    const float W = (float)a->width, H = (float)a->height;
    float x0 = (float)x/W,
          y0 = (float)y/H,
          x1 = (float)(x+w)/W,
          y1 = (float)(y+h+1)/H; //+1 fixes an issue with 1 pixel being cut off
                                 //for...reasons

    c->texCoords.ul = Vec2_ss(x0, y0);
    c->texCoords.ur = Vec2_ss(x1, y0);
    c->texCoords.ll = Vec2_ss(x0, y1);
    c->texCoords.lr = Vec2_ss(x1, y1);

    r->data[r->length++] = d->cps[cpIdx];
    r->currentWidth += w + 1; //again, spacing for floating point errors

#if RENDER_BACKEND == RENDER_BACKEND_OPENGL
    GL(glBindTexture(GL_TEXTURE_2D, a->tex)); {
        GL(glTexSubImage2D(
            GL_TEXTURE_2D, 0, (GLint)x, (GLint)y, (GLsizei)w, (GLsizei)h,
            GL_RED, GL_UNSIGNED_BYTE, g->bitmap.buffer
        ));
    } GL(glBindTexture(GL_TEXTURE_2D, 0));
    return true;
#else
#error "Atlas blitting not implemented for backends outside of OpenGL"
    return false;
#endif
}

static AtlasTextRow AtlasTextRow_new(uint32_t capacity, uint32_t y, uint32_t height) {
    AtlasTextRow r = (AtlasTextRow)PCB_realloc(NULL, AtlasTextRow_bytesize(capacity));
    if(r == NULL) return NULL;
    r->y = y;
    r->height = height;
    r->currentWidth = 0;
    r->length = 0;
    r->capacity = capacity;
    return r;
}

static bool AtlasTextDataByHeight_initStorage(AtlasTextDataByHeight *d) {
    void *mem = PCB_realloc(NULL, 64*(sizeof(*d->cps) + sizeof(*d->cpData)));
    if(mem == NULL) return false;
    d->capacity = 16;
    //[cps........][cpData........]
    d->cps = (uint32_t*)mem;
    d->cpData = (Character*)(void*)(d->cps + d->capacity);
    //TODO: use smaller initial capacity when PCB supports dynamically
    //setting it (currently 64)
    PCB_Vec_reserve(&d->rows, 16);
    return true;
}

static bool AtlasTextDataByHeight_reserveCps(AtlasTextDataByHeight *d, size_t howMany) {
    size_t newSize = d->length + howMany;
    if(newSize <= d->capacity) return true;

    size_t newCap = d->capacity;
    const size_t elemsize = sizeof(*d->cps) + sizeof(*d->cpData);
    while(newCap < newSize) newCap *= 2;
    //we need to be able to cast to ssize_t
    if(newCap*elemsize >= SIZE_MAX/2) return false;

    void *mem = PCB_realloc(d->cps, newCap * elemsize);
    if(mem == NULL) return false;
    uint32_t *cps = (uint32_t*)mem;
    Character *cpData = (Character*)(cps + newCap);
    PCB_memmove(cpData, cps + d->capacity, sizeof(*d->cpData)*d->length);

    d->capacity = newCap;
    d->cps = cps;
    d->cpData = cpData;
    return true;
}

static bool AtlasTextDataByHeight_init(AtlasTextDataByHeight *d, AtlasText *a) {
    FT_Set_Pixel_Sizes(a->font, 0, d->fontHeight);
    const FT_GlyphSlot g = a->font->glyph;
    if(load_glyph(a, 0xFFFD) == 0) {
        const uint32_t h = g->bitmap.rows;
        uint32_t y = find_first_free_y(a);
        if(y + h > a->height) {
            if(!AtlasText__expandTex(a, PCB_X1_5(a->width), PCB_X1_5(a->height)))
                return false;
        }
        AtlasTextRow r = AtlasTextRow_new(64, y, h);
        if(r == NULL) return false;
        d->rows.data[d->rows.length++] = r;
        AtlasText__emplaceCp(a, d, 0, 0);
    } else {
        Character_none(&d->cpData[0], d);
    }
    d->cps[0] = 0xFFFD;
    ++d->length;
    FT_Size_Metrics *m = &a->font->size->metrics; //3 indirections?!
    d->ascent = m->ascender;
    d->descent = m->descender;
    d->linespace = m->height;
    return true;
}

int AtlasText_init(AtlasText *a, FT_Face font, size_t w, size_t h) {
    a->data.data = NULL; a->data.length = a->data.capacity = 0;
    a->font = font; FT_Reference_Face(font);
    a->width = w; a->height = h;
    const size_t wxh = w*h;
    if(wxh > SIZE_MAX/2) return -1;
#ifdef RENDER_BACKEND_OPENGL
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, &a->tex);
    glBindTexture(GL_TEXTURE_2D, a->tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenBuffers(1, &a->pbo);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, a->pbo);
    glBufferData(GL_PIXEL_PACK_BUFFER, (GLsizeiptr)wxh, NULL, GL_STREAM_COPY);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    return 0;
#else
    //TODO: create texture (currently unabstracted)
#error "Creating atlas texture outside of OpenGL not implemented"
    return -1;
#endif
}

void AtlasText_destroy(AtlasText *a) {
    PCB_Vec_forEach_it(&a->data, it, AtlasTextDataByHeight) {
        PCB_Vec_forEach_it(&it->rows, jt, AtlasTextRow) {
            PCB_free(*jt);
        }
        PCB_Vec_destroy(&it->cps_failed);
        PCB_Vec_destroy(&it->rows);
        PCB_free(it->cps);
    }
    PCB_Vec_destroy(&a->data);
    FT_Done_Face(a->font); a->font = NULL;
    if(a->pixels != NULL) { //TODO
        PCB_log(
            PCB_LOGLEVEL_WARN,
            "Don't know what to do with a non-NULL atlas pixels pointer"
            " on destroy, setting to NULL anyway!"
        );
        a->pixels = NULL;
    }
    a->width = a->height = 0;
#if RENDER_BACKEND == RENDER_BACKEND_OPENGL
    glDeleteTextures(1, &a->tex);
    glDeleteBuffers(1, &a->pbo);
#else
#error "Destroying atlas texture outside of OpenGL not implemented"
#endif
}

ssize_t AtlasText_queryHeight(AtlasText *a, uint32_t h) {
    PCB_CHECK_SELF(a, -1);
    PCB_Vec_enumerate(&a->data, i, v, it, AtlasTextDataByHeight) {
        if(it.v->fontHeight == h) return (ssize_t)it.i;
    }
    return -2;
}

ssize_t AtlasText_queryHeight_gte(AtlasText *a, uint32_t h) {
    PCB_CHECK_SELF(a, -1);
    ssize_t closest = -2;
    PCB_Vec_enumerate(&a->data, i, v, it, AtlasTextDataByHeight) {
        if(it.v->fontHeight < h) continue;
        if(closest < 0) { closest = (ssize_t)it.i; continue; }
        int32_t diff_cur = (int32_t)a->data.data[closest].fontHeight - (int32_t)h,
                diff     = (int32_t)it.v->fontHeight - (int32_t)h;
        if(diff < diff_cur) closest = (ssize_t)it.i;
    }
    return closest;
}

ssize_t AtlasText_addHeight(AtlasText *a, uint32_t h) {
    PCB_CHECK_SELF(a, -1);
    PCB_Vec_enumerate(&a->data, i, v, it, AtlasTextDataByHeight) {
        if(it.v->fontHeight == h) return (ssize_t)it.i;
    }
    AtlasTextDataByHeight d = PCB_ZEROED;
    d.fontHeight = h;
    if(!AtlasTextDataByHeight_initStorage(&d)) return -2;
    if(!AtlasTextDataByHeight_init(&d, a)) return -3;
    PCB_Vec_append(&a->data, d);
    return (ssize_t)a->data.length - 1;
}

bool AtlasText_addCodepoint(AtlasText *a, ssize_t hIdx, uint32_t cp) {
    PCB_CHECK_SELF(a, false);
    PCB_CHECK(hIdx < 0, false);
    const size_t idx = (size_t)hIdx;
    PCB_CHECK(idx >= a->data.length, false);

    AtlasTextDataByHeight *d = &a->data.data[idx];
    PCB_assert(d->cps != NULL);
    PCB_assert(d->length > 0);

    int32_t L = 1, R = (int32_t)d->length - 1;
    while(L <= R) { //find index for `cp` using binary search
        int32_t m = L + (R-L)/2;
        if(d->cps[m] < cp) L = m + 1;
        else if(d->cps[m] > cp) R = m - 1;
    }
    if(L == R) return true; //`cp` already present

    {
        int32_t l = 0, r = (int32_t)d->cps_failed.length - 1;
        while(l <= r) {
            int32_t m = l + (r-l)/2;
            if(d->cps_failed.data[m] < cp) l = m + 1;
            else if(d->cps_failed.data[m] > cp) r = m - 1;
        }
        if(l == r) return true; //tried to add, failed, will be stubbed with U+FFFD
    }
    FT_Set_Pixel_Sizes(a->font, 0, d->fontHeight);
    FT_Error e = load_glyph(a, cp);
    if(e != 0) {
        int32_t L = 0, R = (int32_t)d->cps_failed.length - 1;
        while(L <= R) {
            int32_t m = L + (R-L)/2;
            if(d->cps_failed.data[m] < cp) L = m + 1;
            else if(d->cps_failed.data[m] > cp) R = m - 1;
        }
        assert(L != R); //the bsearch above should find it and shortcut
        const size_t l = (size_t)(uint32_t)L;
        PCB_Vec_insert(&d->cps_failed, cp, l);
        return false;
    }
    const FT_GlyphSlot g = a->font->glyph;
    const uint32_t h = g->bitmap.rows;
    size_t rIdx = d->rows.length;
    PCB_Vec_enumerate(&d->rows, i, v, it, AtlasTextRow) {
        if((*it.v)->height < h) //won't fit vertically
            continue;
        bool fits_horizontally = (*it.v)->currentWidth + g->bitmap.width+1 <= a->width;
        if(rIdx == d->rows.length) { //not set
            if(fits_horizontally) {
                rIdx = it.i;
            }
            continue;
        }
        //smaller height -> less wasted space
        bool smaller_height = (*it.v)->height < d->rows.data[rIdx]->height;
        if(smaller_height && fits_horizontally) {
            rIdx = it.i;
        }
    }
    if(rIdx == d->rows.length) {
        uint32_t y = find_first_free_y(a);
        if(y + h > a->height) {
            if(!AtlasText__expandTex(a, PCB_X1_5(a->width), PCB_X1_5(a->height)))
                return false;
        }
        AtlasTextRow r = AtlasTextRow_new(64, y, h);
        if(r == NULL) return false;
        PCB_Vec_append(&d->rows, r);
    }
    if(!AtlasTextDataByHeight_reserveCps(d, 1)) return false;

    const size_t l = (uint32_t)L;
    if(l < d->length) { //shift elements 1 position if necessary
        PCB_memmove(d->cps    + l+1, d->cps    + l, (d->length-l)*sizeof(*d->cps));
        PCB_memmove(d->cpData + l+1, d->cpData + l, (d->length-l)*sizeof(*d->cpData));
    }
    d->cps[l] = cp;
    ++d->length;
    if(!AtlasText__emplaceCp(a, d, l, rIdx)) { //unshift element on error
        --d->length;
        if(l < d->length) {
            PCB_memmove(d->cps    + l, d->cps    + l+1, (d->length-l)*sizeof(*d->cps));
            PCB_memmove(d->cpData + l, d->cpData + l+1, (d->length-l)*sizeof(*d->cpData));
        }
        return false;
    }
    return true;
}

int32_t AtlasText_queryCodepoint(AtlasText *a, ssize_t hIdx, uint32_t cp) {
    PCB_CHECK_SELF(a, -1);
    PCB_CHECK(hIdx < 0, -2);
    const size_t idx = (size_t)hIdx;
    PCB_CHECK(idx >= a->data.length, -2);

    AtlasTextDataByHeight *d = &a->data.data[idx];
    PCB_assert(d->cps != NULL);
    PCB_assert(d->length > 0);

    int32_t L = 1, R = (int32_t)d->length - 1;
    //find index of `cp` using binary search
    while(L <= R) {
        int32_t m = L + (R-L)/2;
        if(d->cps[m] < cp) L = m + 1;
        else if(d->cps[m] > cp) R = m - 1;
    }
    if(L == R) return (int32_t)L;
    return 0;
}

bool AtlasText_dump(AtlasText *a, const char *filepath) {
    bool result = false;
#if RENDER_BACKEND == RENDER_BACKEND_OPENGL
    const size_t wxh = a->width*a->height;
    if(wxh > SIZE_MAX/2) return false;
    //NOTE: OpenGL doesn't give easy way of checking if realloc failed
    GL(glBindTexture(GL_TEXTURE_2D, a->tex));

    GL(glBindBuffer(GL_PIXEL_PACK_BUFFER, a->pbo));
    GL(glBufferData(GL_PIXEL_PACK_BUFFER, (GLsizeiptr)wxh, NULL, GL_STREAM_COPY));
    GL(glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_BYTE, NULL));
    GL(glBindBuffer(GL_PIXEL_PACK_BUFFER, 0));

    GL(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, a->pbo));
    void *ptr;
    while(1) {
        GL(ptr = glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_READ_ONLY));
        if(ptr != NULL) break;
    }
    if(stbi_write_png(
        filepath,
        (ssize_t)a->width,
        (ssize_t)a->height,
        1,
        ptr,
        a->width
    )) {
        PCB_log(PCB_LOGLEVEL_INFO, PCB_LOC":%s: Dumped atlas to %s", __func__, filepath);
        result = true;
    } else {
        PCB_log(PCB_LOGLEVEL_ERROR, PCB_LOC":%s: Failed to dump atlas", __func__);
    }
    GL(glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER));
    GL(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));

    GL(glBindTexture(GL_TEXTURE_2D, 0));
#else
    (void)a; (void)w; (void)h;
#endif
    return result;
}
