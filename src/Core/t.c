/**
 * Copyright © 2026 Piotr Mikołajewski
 * This file is part of Battleships.
 *
 * Battleships is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Battleships is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with Battleships. If not, see <https://www.gnu.org/licenses/>.
 */
#include <PCB.h>

#include <dlfcn.h>

#define RGFW_IMPORT
#define RGFW_OPENGL
#define RGFW_NATIVE
#include <RGFW.h>

#include <lac.h>

#include "Renderer.h"
#include "MainUI.h"

enum { COUNTER_BASE = __COUNTER__ };
#define COUNTER (__COUNTER__ - COUNTER_BASE)

static uint64_t get_time(void) {
    struct timespec t = PCB_ZEROED;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (uint64_t)t.tv_sec*1000000000 + (uint64_t)t.tv_nsec;
}

static char arena_backing_store[1 << 26];

static struct {
    PCB_ShellCommand cmd;
    PCB_Process proc;
} rebuild_mainUI;

typedef struct {
    uint64_t start, end, delta, last_redraw;
} Timings;

typedef struct {
    uint8_t screen : 1;
    uint8_t mainUI : 1;
    uint8_t reload_mainUI : 1;
    uint8_t dump_atlas : 1;
    uint8_t window_resized : 1;
} Updates;

typedef struct {
    struct { float x, y; } cursor;
    struct { float x, y; } scroll;
    uint16_t button_state;
    uint16_t button_state_previous;
    uint8_t cursor_moved : 1;
    uint8_t scrolled : 1;
} Mouse_State;

typedef struct {
    const char *libpath;
    void *handle;
    MainUI_init_pfn init;
    MainUI_update_pfn update;
    MainUI_destroy_pfn destroy;
    MainUI_prereload_pfn prereload;
    MainUI_postreload_pfn postreload;
} MainUI_DLL;

static void MainUI_DLL_destroy(MainUI_DLL *dll) {
    if(dll->handle != NULL) dlclose(dll->handle);
    dll->libpath = NULL;
    dll->handle = NULL;
    dll->init = NULL;
    dll->update = NULL;
    dll->destroy = NULL;
}

static bool MainUI_reload(MainUI_DLL *dll) {
    if(dll->handle != NULL) {
        dlclose(dll->handle);
        dll->handle = NULL;
    }
    void *handle = dlopen(dll->libpath, RTLD_NOW);
    if(handle == NULL) {
        PCB_log(PCB_LOGLEVEL_ERROR, PCB_LOC": Failed to load %s: %s", dll->libpath, dlerror());
        return false;
    }
    void *init_fn = NULL,
         *update_fn = NULL,
         *destroy_fn = NULL,
         *prereload_fn = NULL,
         *postreload_fn = NULL;

#define LOAD(sym, pfn) do { \
    pfn = dlsym(handle, sym); \
    if(pfn == NULL) { \
        PCB_log(PCB_LOGLEVEL_ERROR, PCB_LOC": Failed to load '" sym "' symbol: %s", dlerror()); \
        goto error; \
    } \
} while(0)
    LOAD("MainUI_init", init_fn);
    LOAD("MainUI_update", update_fn);
    LOAD("MainUI_destroy", destroy_fn);
    LOAD("MainUI_prereload", prereload_fn);
    LOAD("MainUI_postreload", postreload_fn);
#undef LOAD
    dll->handle               = handle;
    *(void**)&dll->init       = init_fn;
    *(void**)&dll->update     = update_fn;
    *(void**)&dll->destroy    = destroy_fn;
    *(void**)&dll->prereload  = prereload_fn;
    *(void**)&dll->postreload = postreload_fn;
    return true;
error:
    return false;
}

static void MainUI_reload_if_proc_exited_successfully(
    UI *mainUI,
    MainUI_DLL *dll
) {
    if(!PCB_Process_isValid(&rebuild_mainUI.proc)) return;

    int result_ = PCB_Process_checkExit(&rebuild_mainUI.proc);
    if(result_ < 0) {
        PCB_logLatestError("%s", "");
        return;
    } else if(!result_) return;

    int code = PCB_Process_getExitCode(&rebuild_mainUI.proc);
    PCB_Process_destroy(&rebuild_mainUI.proc);
    if(code != 0) return;

    dll->prereload(mainUI);
    if(!MainUI_reload(dll)) return;
    dll->postreload(mainUI);
}

static void process_keyboard_pressed_event(RGFW_keyEvent *ev, Updates *u) {
    switch(ev->value) {
      case RGFW_g:
        u->reload_mainUI = true;
        break;
      case RGFW_d:
        u->dump_atlas = true;
        break;
      default:
        break;
    }
}

static void process_keyboard_released_event(RGFW_keyEvent *ev) {
    switch(ev->value) {
      default:
        break;
    }
}

static void process_events(
    RGFW_window *win,
    Mouse_State *mouse_state,
    Updates *u
) {
    RGFW_event event;
    while (RGFW_window_checkEvent(win, &event)) {
        switch(event.type) {
          case RGFW_quit: break;
          case RGFW_keyPressed:
            process_keyboard_pressed_event(&event.key, u);
            break;
          case RGFW_keyReleased:
            process_keyboard_released_event(&event.key);
            break;
          case RGFW_mouseButtonPressed:
            mouse_state->button_state |= 1 << event.button.value;
            break;
          case RGFW_mouseButtonReleased:
            mouse_state->button_state &= ~(1 << event.button.value);
            break;
          case RGFW_mouseScroll:
            mouse_state->scroll.x = event.scroll.x;
            mouse_state->scroll.y = event.scroll.y;
            mouse_state->scrolled = true;
            break;
          case RGFW_mousePosChanged:
            mouse_state->cursor.x = event.mouse.x;
            mouse_state->cursor.y = event.mouse.y;
            mouse_state->cursor_moved = true;
            break;
          case RGFW_windowMoved:
            break;
          case RGFW_windowResized:
            u->window_resized = true;
            break;
          case RGFW_focusIn:
            break;
          case RGFW_focusOut:
            break;
          case RGFW_mouseEnter:
            break;
          case RGFW_mouseLeave:
            break;
          case RGFW_windowRefresh:
            break;
          case RGFW_dataDrop:
            break;
          case RGFW_dataDrag:
            break;
          case RGFW_windowMaximized:
            break;
          case RGFW_windowMinimized:
            break;
          case RGFW_windowRestored:
            break;
          case RGFW_scaleUpdated:
            break;
        default: break; //ignore
      }
    }
}

static bool UI_maybe_update_env(UI *ui, Mouse_State *ms) {
    bool update = false;
    if(ms->cursor_moved) {
        ui->env.cursor.x = ms->cursor.x;
        ui->env.cursor.y = ms->cursor.y;
        update = true;
    }
    if(ms->scrolled) {
        ui->env.scroll.x = ms->scroll.x;
        ui->env.scroll.y = ms->scroll.y;
        update = true;
    }
    if(ms->button_state != ms->button_state_previous) {
        ui->env.mouse_btn_state = ms->button_state;
        ui->env.mouse_btn_state_previous = ms->button_state_previous;
        update = true;
    }
    return update;
}

int main(int argc, char **argv) {
    const char *font_path = NULL;
    const char *server = NULL;
    uint16_t port = 1101;
    for(int i = 1; i < argc; i++) {
        char *arg = argv[i];
        if(!strncmp(arg, "--font=", 7)) {
            font_path = arg + 7;
            continue;
        }
        if(!strncmp(arg, "--server=", 9)) {
            server = arg + 9;
            continue;
        }
        if(!strncmp(arg, "--port=", 7)) {
            port = (uint16_t)strtoul(arg + 7, NULL, 10);
        }
    }
    PCB_ShellCommand_append_args(&rebuild_mainUI.cmd, "./b", "--target=MainUI");
    rebuild_mainUI.proc = PCB_Process_init();

    int result = 0;
    PCB_Arena *arena = PCB_Arena_init_in(
        arena_backing_store, sizeof(arena_backing_store)
    );
    FT_Library ft;
    FT_Error ft_e = FT_Init_FreeType(&ft);
    if(ft_e != 0) {
        PCB_log(
            PCB_LOGLEVEL_ERROR, PCB_LOC": Failed to initialize FreeType: %s",
            FT_Error_String(ft_e)
        );
        return COUNTER;
    }
    FT_Face font;
    if(font_path == NULL) {
        font_path = "/usr/share/fonts/Adwaita/AdwaitaMono-Regular.ttf";
        PCB_log(PCB_LOGLEVEL_INFO, "No font provided, will default to '%s'", font_path);
    }
    // const char *font_path = "/usr/share/fonts/noto-cjk/NotoSansCJK-Regular.ttc";
    ft_e = FT_New_Face(ft, font_path, 0, &font);
    if(ft_e != 0) {
        PCB_log(
            PCB_LOGLEVEL_ERROR, PCB_LOC": Failed to load %s: %s",
            font_path, FT_Error_String(ft_e)
        );
        result = COUNTER; goto ft_destroy;
    }

    int window_w = 1280, window_h = 720;
    RGFW_window win = PCB_ZEROED;
    RGFW_glHints* hints = RGFW_getGlobalHints_OpenGL();
    hints->major = 4;
    hints->minor = 3;
    RGFW_setGlobalHints_OpenGL(hints);
    {
        RGFW_window* win_ = RGFW_createWindowPtr(
            "Window",
            window_w, window_h,
            window_w, window_h,
            RGFW_windowOpenGL, &win
        );
        if (win_ == NULL) {
            PCB_log(PCB_LOGLEVEL_ERROR, "Failed to create RGFW window");
            result = COUNTER; goto ft_destroy;
        }
    }
    Renderer r = PCB_ZEROED;
    if(!Renderer_init(&r, window_w, window_h, font)) {
        result = COUNTER; goto close_window;
    }
    UI mainUI = PCB_ZEROED;
    MainUI_DLL mainUI_dll = { .libpath = "bin/libMainUI.so" };
    if(!MainUI_reload(&mainUI_dll)) goto renderer_close;
    mainUI.env.viewport.w = window_w;
    mainUI.env.viewport.h = window_h;

    // if(!MainUI_init(&mainUI, arena, &r)) {
    if(!mainUI_dll.init(&mainUI, &r, server, port)) {
        PCB_log(PCB_LOGLEVEL_ERROR, "Failed to initialize the main UI");
        result = COUNTER; goto mainUI_unload_dll;
    }

    // RGFW_window_setExitKey(window, RGFW_escape);
    RGFW_window_makeCurrentContext_OpenGL(&win);
    RGFW_window_swapInterval_OpenGL(&win, 1);
    Renderer_background_color(&r, (Vec4){0});

    Mouse_State mouse_state = PCB_ZEROED;
    Timings t = PCB_ZEROED;
    t.start = t.end = t.last_redraw = get_time();
    Updates updates = PCB_ZEROED;
    updates.screen = true;
    while(RGFW_window_shouldClose(&win) == RGFW_FALSE) {
        MainUI_reload_if_proc_exited_successfully(&mainUI, &mainUI_dll);
        t.start = get_time();
        mouse_state.cursor_moved          = false;
        mouse_state.scrolled              = false;
        mouse_state.button_state_previous = mainUI.env.mouse_btn_state_previous
                                          = mouse_state.button_state;

        process_events(&win, &mouse_state, &updates);

        updates.mainUI = UI_maybe_update_env(&mainUI, &mouse_state);
        if(updates.window_resized) {
            updates.window_resized = false;
            Viewport v = r.viewport;
            updates.mainUI = true;
            v.width = (float)win.w;
            v.height = (float)win.h;
            mainUI.env.viewport.w = v.width;
            mainUI.env.viewport.h = v.height;
            Renderer_viewport(&r, v);
        }
        //for testing UI animations
        //TODO: signal this from within UI itself via platform callback that can be
        //(de)registered on init/(pre|post)reload
        updates.mainUI |= true;
        if(updates.mainUI) {
            updates.mainUI = false;
            t.end = get_time();
            mainUI.env.dt = (float)(t.end - t.last_redraw)/1e9f;
            t.last_redraw = t.end;
            updates.screen = true;
        }

        if(!updates.screen) {
            struct timespec t = PCB_ZEROED;
            t.tv_nsec = 2e7;
            nanosleep(&t, NULL);
            goto end;
        };
        updates.screen = false;
        Renderer_clear(&r);
        // MainUI_update(&mainUI);
        mainUI_dll.update(&mainUI);
        RGFW_window_swapBuffers_OpenGL(&win);
        if(updates.reload_mainUI) {
            updates.reload_mainUI = false;
            if(PCB_Process_isValid(&rebuild_mainUI.proc)) goto end;
            rebuild_mainUI.proc = PCB_ShellCommand_runBg(&rebuild_mainUI.cmd);
            if(!PCB_Process_isValid(&rebuild_mainUI.proc)) {
                PCB_log(PCB_LOGLEVEL_ERROR, "Failed to start rebuild of main UI");
            }
        }
        if(updates.dump_atlas) {
            updates.dump_atlas = false;
            const char *filepath = "build/atlas.png";
            AtlasText_dump(&r.fontAtlas, filepath);
            PCB_log(PCB_LOGLEVEL_DEBUG, "Dumped atlas to %s", filepath);
        }
    end:
        t.end = get_time();
        t.delta = t.end - t.start;
        // PCB_log(PCB_LOGLEVEL_DEBUG_NL, "%lf ms\r", (double)t.delta / 1e6);
        // fflush(stdout);
    }
// MainUI_destroy:
    mainUI_dll.destroy(&mainUI);
    // MainUI_destroy(&mainUI);
mainUI_unload_dll:
    MainUI_DLL_destroy(&mainUI_dll);
renderer_close:
    Renderer_destroy(&r);
close_window:
    RGFW_window_closePtr(&win);
ft_destroy:
    FT_Done_FreeType(ft);
    return result;
}
