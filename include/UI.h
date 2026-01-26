//Copyright (c) 2026 Piotr Mikołajewski
#pragma once

#include <stdint.h>

typedef struct UI UI;

struct UI {
    //Implementation-specific UI context. This way we don't depend on a specific library.
    void *ctx;
    //Additional UI state, if necessary.
    void *user_data;
    //Current "environmental" state of the UI. Set by the caller of `update`.
    struct {
        struct { float w, h; } viewport;
        struct { float x, y; } cursor;
        struct { float x, y; } scroll;
        float dt;
        uint16_t mouse_btn_state;
        uint16_t mouse_btn_state_previous;
        struct {
            bool needs_reinit : 1;
        } flags;
        uint8_t _align1[1];
    } env;
};

