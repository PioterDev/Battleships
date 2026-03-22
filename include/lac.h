/**
 * Copyright © 2026 Piotr Mikołajewski
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
//lac - linear algebra for C
#ifndef LAC_H
#define LAC_H

#define LACAPI static inline

#include <math.h>

typedef union {
    float data[2];
    struct { float x, y; };
    struct { float x, y; } point;
} Vec2;

typedef union {
    float data[3];
    struct { float x, y, z; };
    struct { float r, g, b; } color;
} Vec3;

typedef union {
    float data[4];
    struct { float x, y, z, w; };
    struct { float x, y, w, h; } rect;
    struct { float r, g, b, a; } color;
} Vec4;

typedef union {
    Vec4 rows[4];
    float data[4][4];
    float arr[16];
} Mat4;

LACAPI Vec2 Vec2_s(float s) { return (Vec2){.data = {s, s} }; }
LACAPI Vec2 Vec2_ss(float x, float y) { return (Vec2){ .data = {x, y} }; }

LACAPI Vec2 Vec2_add(Vec2 v1, Vec2 v2) { return (Vec2){ .data = {v1.x + v2.x, v1.y + v2.y} }; }
LACAPI Vec2 Vec2_sub(Vec2 v1, Vec2 v2) { return (Vec2){ .data = {v1.x - v2.x, v1.y - v2.y} }; }
LACAPI Vec2 Vec2_mul(Vec2 v1, Vec2 v2) { return (Vec2){ .data = {v1.x * v2.x, v1.y * v2.y} }; }
LACAPI Vec2 Vec2_div(Vec2 v1, Vec2 v2) { return (Vec2){ .data = {v1.x / v2.x, v1.y / v2.y} }; }

LACAPI Vec2 Vec2_adds(Vec2 v, float s) { return (Vec2){ .data = {v.x + s, v.y + s} }; }
LACAPI Vec2 Vec2_subs(Vec2 v, float s) { return (Vec2){ .data = {v.x - s, v.y - s} }; }
LACAPI Vec2 Vec2_muls(Vec2 v, float s) { return (Vec2){ .data = {v.x * s, v.y * s} }; }
LACAPI Vec2 Vec2_divs(Vec2 v, float s) { return (Vec2){ .data = {v.x / s, v.y / s} }; }

LACAPI Vec3 Vec3_s(float s) { return (Vec3){.data = {s, s, s} }; }
LACAPI Vec3 Vec3_ss(float x, float y, float z) { return (Vec3){ .data = {x, y, z} }; }

LACAPI Vec3 Vec3_add(Vec3 v1, Vec3 v2) { return (Vec3){ .data = {v1.x + v2.x, v1.y + v2.y, v1.z + v2.z} }; }
LACAPI Vec3 Vec3_sub(Vec3 v1, Vec3 v2) { return (Vec3){ .data = {v1.x - v2.x, v1.y - v2.y, v1.z - v2.z} }; }
LACAPI Vec3 Vec3_mul(Vec3 v1, Vec3 v2) { return (Vec3){ .data = {v1.x * v2.x, v1.y * v2.y, v1.z * v2.z} }; }
LACAPI Vec3 Vec3_div(Vec3 v1, Vec3 v2) { return (Vec3){ .data = {v1.x / v2.x, v1.y / v2.y, v1.z / v2.z} }; }

LACAPI Vec3 Vec3_adds(Vec3 v, float s) { return (Vec3){ .data = {v.x + s, v.y + s, v.z + s} }; }
LACAPI Vec3 Vec3_subs(Vec3 v, float s) { return (Vec3){ .data = {v.x - s, v.y - s, v.z - s} }; }
LACAPI Vec3 Vec3_muls(Vec3 v, float s) { return (Vec3){ .data = {v.x * s, v.y * s, v.z * s} }; }
LACAPI Vec3 Vec3_divs(Vec3 v, float s) { return (Vec3){ .data = {v.x / s, v.y / s, v.z / s} }; }

LACAPI float Vec3_dot(Vec3 v1, Vec3 v2) { return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z; }

LACAPI Vec3 Vec3_neg (Vec3 v) { v.x = -v.x; v.y = -v.y; v.z = -v.z; return v; }
LACAPI Vec3 Vec3_sqrt(Vec3 v) { for(int i = 0; i < 3; i++) { v.data[i] = sqrtf(v.data[i]); } return v; }
LACAPI Vec3 Vec3_norm(Vec3 v) { return Vec3_div(v, Vec3_sqrt(Vec3_s(Vec3_dot(v, v)))); }

LACAPI Vec3 Vec3_cross(Vec3 v1, Vec3 v2) { return (Vec3){ .data = { v1.y*v2.z - v2.y*v1.z, v1.z*v2.x - v2.z*v1.x, v1.x*v2.y - v2.x*v1.y }}; }

LACAPI Vec4 Vec4_s(float s) { return (Vec4){.data = {s, s, s, s} }; }
LACAPI Vec4 Vec4_ss(float x, float y, float z, float w) { return (Vec4){ .data = {x, y, z, w} }; }

LACAPI Vec4 Vec4_mulm(Vec4 v, Mat4 m) {
    Vec4 y = {0};
    y.data[0] = m.data[0][0] * v.x + m.data[0][1] * v.y + m.data[0][2] * v.z + m.data[0][3] * v.w;
    y.data[1] = m.data[1][0] * v.x + m.data[1][1] * v.y + m.data[1][2] * v.z + m.data[1][3] * v.w;
    y.data[2] = m.data[2][0] * v.x + m.data[2][1] * v.y + m.data[2][2] * v.z + m.data[2][3] * v.w;
    y.data[3] = m.data[3][0] * v.x + m.data[3][1] * v.y + m.data[3][2] * v.z + m.data[3][3] * v.w;
    return y;
}

LACAPI Mat4 Mat4_Id(float x) {
    Mat4 m = {
        .arr = {
            x, 0.0f, 0.0f, 0.0f,
            0.0f, x, 0.0f, 0.0f,
            0.0f, 0.0f, x, 0.0f,
            0.0f, 0.0f, 0.0f, x
        }
    };
    return m;
}

LACAPI Mat4 Mat4_ortho(float left, float right, float bottom, float top) {
    Mat4 m = Mat4_Id(1.0f);
    m.data[0][0] = 2.0f / (right - left);
    m.data[1][1] = 2.0f / (top - bottom);
    m.data[2][2] = -1.0f;
    m.data[3][0] = -(right + left) / (right - left);
    m.data[3][1] = -(top + bottom) / (top - bottom);
    return m;
}

LACAPI Mat4 Mat4_lookAt_lh(Vec3 eye, Vec3 center, Vec3 up) {
    Vec3 f = Vec3_norm(Vec3_sub(center, eye));
    Vec3 s = Vec3_norm(Vec3_cross(up, f));
    Vec3 u = Vec3_cross(f, s);

    Mat4 m = Mat4_Id(1.0f);
    m.data[0][0] =  s.x;
    m.data[1][0] =  s.y;
    m.data[2][0] =  s.z;
    m.data[0][1] =  u.x;
    m.data[1][1] =  u.y;
    m.data[2][1] =  u.z;
    m.data[0][2] =  f.x;
    m.data[1][2] =  f.y;
    m.data[2][2] =  f.z;
    m.data[3][0] = -Vec3_dot(s, eye);
    m.data[3][1] = -Vec3_dot(u, eye);
    m.data[3][2] = -Vec3_dot(f, eye);
    return m;
}
#endif //LAC_H
