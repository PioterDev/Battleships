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
#include "MainUI.h"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/epoll.h>

#include <clay.h>

#include "Renderer.h"

#define CLAY_DEFAULT_BUTTON_COLOR PCB_CLITERAL(Clay_Color){150, 150, 150, 255}
#define CLAY_DEFAULT_BUTTON_COLOR_HOVERED PCB_CLITERAL(Clay_Color){100, 100, 100, 255}

#define CLAY_DEFAULT_TEXT_CONFIG() CLAY_TEXT_CONFIG({.fontSize = 20, .textColor = CLAY_WHITE})

#define CLAY_WHITE   PCB_CLITERAL(Clay_Color){255, 255, 255, 255}
#define CLAY_GREEN   PCB_CLITERAL(Clay_Color){0,   255, 0,   255}
#define CLAY_RED     PCB_CLITERAL(Clay_Color){255, 0,   0,   255}
#define CLAY_MAGENTA PCB_CLITERAL(Clay_Color){255, 0,   255, 255}
#define CLAY_CYAN    PCB_CLITERAL(Clay_Color){0,   255, 255, 255}

#define DEBUG false

static const Clay_String CLAY_DEFAULT_STRING = {
    .isStaticallyAllocated = true,
    .length = 11,
    .chars = "Placeholder"
};

static void setnonblock(int fd) {
    int flags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

static inline Clay_ChildAlignment Clay_childAlignment_center(void) {
    return PCB_CLITERAL(Clay_ChildAlignment){
        .x = CLAY_ALIGN_X_CENTER,
        .y = CLAY_ALIGN_Y_CENTER
    };
}

PCB_Enum(Battleships_Status, uint8_t) {
    BATTLESHIPS_STATUS_NOT_CONNECTED,
    BATTLESHIPS_STATUS_IN_MENU,
    BATTLESHIPS_STATUS_IN_EDIT_BOARD,
    BATTLESHIPS_STATUS_LOBBY,
    BATTLESHIPS_STATUS_IN_GAME,
    BATTLESHIPS_STATUS_COUNT,
};

PCB_Enum(Battleships_Status_Lobby, uint8_t) {
    BATTLESHIPS_STATUS_LOBBY_GOT_NONE,
    BATTLESHIPS_STATUS_LOBBY_GOT_HEADER,
    BATTLESHIPS_STATUS_LOBBY_GOT_END,
    BATTLESHIPS_STATUS_LOBBY_COUNT,
};

PCB_Enum(Battleships_Status_Edit_Board, uint8_t) {
    BATTLESHIPS_STATUS_EDIT_BOARD_GOT_NONE,
    BATTLESHIPS_STATUS_EDIT_BOARD_GOT_BOARD,
    BATTLESHIPS_STATUS_EDIT_BOARD_GOT_END,
    BATTLESHIPS_STATUS_EDIT_BOARD_COUNT,
};

PCB_Enum(Battleships_Status_Game, uint8_t) {
    BATTLESHIPS_STATUS_GAME_MESSAGE_GOT_NONE,
    BATTLESHIPS_STATUS_GAME_MESSAGE_GOT_OWN_BOARD,
    BATTLESHIPS_STATUS_GAME_MESSAGE_GOT_SHOOTING_BOARD,
    BATTLESHIPS_STATUS_GAME_OUR_TURN,
    BATTLESHIPS_STATUS_GAME_ENEMY_TURN,
    BATTLESHIPS_STATUS_GAME_WON,
    BATTLESHIPS_STATUS_GAME_LOST,
    BATTLESHIPS_STATUS_GAME_COUNT,
};

PCB_Enum(Battleships_Board_Tile, uint8_t) {
    BATTLESHIPS_BOARD_TILE_EMPTY,
    BATTLESHIPS_BOARD_TILE_OCCUPIED,
    BATTLESHIPS_BOARD_TILE_HIT,
    BATTLESHIPS_BOARD_TILE_MISS,
};

#if DEBUG
static const char *const Battleships_Status_hrn[BATTLESHIPS_STATUS_COUNT] = {
    [BATTLESHIPS_STATUS_NOT_CONNECTED] = "BATTLESHIPS_STATUS_NOT_CONNECTED",
    [BATTLESHIPS_STATUS_IN_MENU] = "BATTLESHIPS_STATUS_IN_MENU",
    [BATTLESHIPS_STATUS_IN_EDIT_BOARD] = "BATTLESHIPS_STATUS_IN_EDIT_BOARD",
    [BATTLESHIPS_STATUS_WAITING_FOR_OPPONENT] = "BATTLESHIPS_STATUS_WAITING_FOR_OPPONENT",
    [BATTLESHIPS_STATUS_LOBBY] = "BATTLESHIPS_STATUS_LOBBY",
};

static const char *const Battleships_Status_Lobby_hrn[BATTLESHIPS_STATUS_LOBBY_COUNT] = {
    [BATTLESHIPS_STATUS_LOBBY_GOT_NONE] = "BATTLESHIPS_STATUS_LOBBY_GOT_NONE",
    [BATTLESHIPS_STATUS_LOBBY_GOT_HEADER] = "BATTLESHIPS_STATUS_LOBBY_GOT_HEADER",
    [BATTLESHIPS_STATUS_LOBBY_GOT_END] = "BATTLESHIPS_STATUS_LOBBY_GOT_END",
};
#endif

typedef struct {
    PCB_String name;
} Lobby_Entry;

typedef struct {
    Lobby_Entry *data;
    size_t length, capacity;
} Lobby_Entries;


static void Lobby_Entries_reset(Lobby_Entries *l) {
#define destruct(ptr) PCB_String_destroy(&(ptr)->name)
    PCB_Vec_reset_d(l, destruct);
#undef destruct
}

static void Lobby_Entries_destroy(Lobby_Entries *l) {
    Lobby_Entries_reset(l);
    PCB_Vec_destroy(l);
}

typedef void (*Battleships_Board_Tile_onHover)(UI *ui, size_t x, size_t y);

typedef struct {
    Battleships_Board_Tile *data;
    size_t length, capacity;
} Battleships_Board_Row;

typedef struct {
    uint32_t size; uint32_t count;
} Battleships_Available_Ships_Of_Size;

typedef struct {
    Battleships_Available_Ships_Of_Size *data;
    size_t length, capacity;
} Battleships_Available_Ships;

typedef struct {
    Battleships_Board_Row *data;
    size_t length, capacity;
    //current row being edited
    uint32_t edited;
    //selected ship size
    uint32_t ship_size;
    //previously selected tile
    int32_t x, y;
    //ships not yet placed
    Battleships_Available_Ships ships;
} Battleships_Board;

typedef Clay_Color (*Battleships_Board_Tile_Color_pfn)(
    Battleships_Board *board, size_t x, size_t y, bool hovered
);

static void Battleships_Board_new_row(Battleships_Board *b, size_t width) {
    Battleships_Board_Row row = PCB_ZEROED;
    PCB_Vec_reserve(&row, width);
    row.length = width;
    memset(row.data, 0, row.length*sizeof(*row.data));
    row.length = width;
    PCB_Vec_append(b, row);
}

static void Battleships_Board_destroy(Battleships_Board *b) {
    PCB_Vec_destroy(&b->ships);
    PCB_Vec_reset_d(b, PCB_Vec_destroy);
    b->ship_size = 0;
    b->edited = 0;
    b->x = b->y = -1;
    PCB_Vec_destroy(b);
}

typedef struct {
    const char *server_addr;
    uint16_t server_port;
    int server_fd;
    PCB_String recv_buf;
    Lobby_Entries lobby;
    Battleships_Status status;
    struct {
        Battleships_Board own, enemy;
    } boards;
    union {
        Battleships_Status_Lobby lobby : 2;
        Battleships_Status_Edit_Board edit_board : 2;
        // Battleships_Status_Game game : 4;
        struct {
            Battleships_Status_Game status : 4;
            struct {
                float dt;
            } wait;
        } game;
    } menu_state;
    struct {
        uint8_t server_conn_failed : 1;
        uint8_t boards_initialized : 1;
        uint8_t waiting_for_opponent : 1;
    } flags;
} Battleships_Context;

static void Battleships_Context_change_state(Battleships_Context *b, Battleships_Status to) {
    switch(b->status) {
      case BATTLESHIPS_STATUS_NOT_CONNECTED:
        break;
      case BATTLESHIPS_STATUS_IN_MENU:
        //Ignore server hello
        PCB_String_reset(&b->recv_buf);
        break;
      case BATTLESHIPS_STATUS_IN_EDIT_BOARD:
        b->menu_state.edit_board = BATTLESHIPS_STATUS_EDIT_BOARD_GOT_NONE;
        b->boards.own.edited = 0;
        break;
      case BATTLESHIPS_STATUS_LOBBY:
        Lobby_Entries_reset(&b->lobby);
        b->menu_state.lobby = BATTLESHIPS_STATUS_LOBBY_GOT_NONE;
        break;
      case BATTLESHIPS_STATUS_IN_GAME:
        b->menu_state.game.status = BATTLESHIPS_STATUS_GAME_MESSAGE_GOT_NONE;
        break;
      case BATTLESHIPS_STATUS_COUNT: PCB_Unreachable;
    }
    switch(to) {
      case BATTLESHIPS_STATUS_NOT_CONNECTED: break;
      case BATTLESHIPS_STATUS_IN_MENU: break;
      // case BATTLESHIPS_STATUS_GET_BOARD: break;
      case BATTLESHIPS_STATUS_IN_EDIT_BOARD: break;
      case BATTLESHIPS_STATUS_LOBBY: break;
      case BATTLESHIPS_STATUS_IN_GAME:
        b->menu_state.game.status = BATTLESHIPS_STATUS_GAME_MESSAGE_GOT_NONE;
        break;
      case BATTLESHIPS_STATUS_COUNT: break;
    }
    b->status = to;
}

static void Battleships_Context_destroy(Battleships_Context *b) {
    PCB_String_destroy(&b->recv_buf);
    Lobby_Entries_destroy(&b->lobby);
    if(b->server_fd >= 0) close(b->server_fd);
    Battleships_Board_destroy(&b->boards.own);
    Battleships_Board_destroy(&b->boards.enemy);
}

typedef struct {
    PCB_Arena *arena;
    PCB_ArenaMark *mark;
    Renderer *r;
    PCB_String status_str;
    Battleships_Context battleships;
} MainUI_Context;

static void handle_clay_error(Clay_ErrorData e);
static Clay_Dimensions measure_text(
    Clay_StringSlice text,
    Clay_TextElementConfig *config,
    void *userData
);

#define Clay_StringSlice_2_PCB_StringView(ss) { ss.chars, (uint32_t)ss.length }

bool MainUI_init(UI *ui, Renderer *r, const char *server, uint16_t port) {
    const uint32_t S = Clay_MinMemorySize();
    PCB_log(PCB_LOGLEVEL_INFO, "clay_memsize = %u", S);

    PCB_Arena *arena = PCB_Arena_init(8*1024*1024);

    if(arena == NULL) goto error_arena;
    void *buf = PCB_Arena_alloc(arena, S);
    assert(buf != NULL);

    MainUI_Context *ctx = PCB_Arena_alloc(arena, sizeof(*ctx));
    assert(ctx != NULL);
    ctx->arena = arena;
    ctx->r = r;
    //Default to localhost.
    //This needs to live in dynamic memory, otherwise trying to connect after
    //reloading will segfault.
    //UI state outlives plugin's static memory (Rust would get a seizure).
    if(server == NULL) server = PCB_Arena_strdup(arena, "127.0.0.1");
    assert(server != NULL);
    ctx->battleships.server_addr = server;
    ctx->battleships.server_port = port;

    Clay_Arena a = Clay_CreateArenaWithCapacityAndMemory(S, buf);
    ui->ctx = Clay_Initialize(
        a,
        (Clay_Dimensions){ui->env.viewport.w, ui->env.viewport.h},
        (Clay_ErrorHandler){ .errorHandlerFunction = handle_clay_error }
    );
    Clay_SetMeasureTextFunction(measure_text, ui);
    if(!PCB_String_reserve_to(&ctx->status_str, 16*1024)) goto error_arena;
    ui->user_data = ctx;
    ctx->battleships.server_fd = -1;

    PCB_ArenaMark *mark = PCB_Arena_mark(arena);
    assert(mark != NULL);
    ctx->mark = mark;
    return true;
error_arena:
    PCB_Arena_destroy(arena);
    return false;
}

void MainUI_prereload(UI *ui) {
    (void)ui;
    Clay_ResetMeasureTextCache();
}

void MainUI_postreload(UI *ui) {
    Clay_SetCurrentContext(ui->ctx);
    Clay_SetMeasureTextFunction(measure_text, ui);
}

//0 -> left
//1 -> middle
//2 -> right
static inline bool left_clicked(UI *ui) {
    return (ui->env.mouse_btn_state & (1 << 0)) == 0 &&
           (ui->env.mouse_btn_state_previous & (1 << 0)) != 0;
}

static inline bool right_clicked(UI *ui) {
    return (ui->env.mouse_btn_state & (1 << 2)) == 0 &&
           (ui->env.mouse_btn_state_previous & (1 << 2)) != 0;
}

static void render_clay_rect(MainUI_Context *ctx, Clay_RenderCommand *cmd) {
    const Clay_BoundingBox *const bb = &cmd->boundingBox;
    Vec4 rect;
    rect.rect.x = bb->x; rect.rect.y = bb->y;
    rect.rect.w = bb->width; rect.rect.h = bb->height;

    const Clay_Color *const cc = &cmd->renderData.rectangle.backgroundColor;
    Vec4 c;
    c.color.r = cc->r/255.0f; c.color.g = cc->g/255.0f;
    c.color.b = cc->b/255.0f; c.color.a = cc->a/255.0f;
    Renderer_draw_rect(ctx->r, rect, c);
}

static void render_clay_text(MainUI_Context *ctx, Clay_RenderCommand *cmd) {
    Clay_TextRenderData *d = &cmd->renderData.text;
    PCB_StringView sv = Clay_StringSlice_2_PCB_StringView(d->stringContents);
    Vec2 pos;
    pos.x = cmd->boundingBox.x;
    pos.y = cmd->boundingBox.y;
    const Clay_Color *const cc = &d->textColor;
    Vec4 c;
    c.color.r = cc->r/255.0f; c.color.g = cc->g/255.0f;
    c.color.b = cc->b/255.0f; c.color.a = cc->a/255.0f;
    Renderer_TextDrawEx ex = { .underline = 0.0f, .flags = { .ascent = RENDERER_ONE } };
    Renderer_draw_text(ctx->r, sv, d->fontSize, pos, c, &ex);
}

static void render_clay(MainUI_Context *ctx) {
    Clay_RenderCommandArray renderCommands = Clay_EndLayout();
    for (int i = 0; i < renderCommands.length; i++) {
        Clay_RenderCommand *cmd = &renderCommands.internalArray[i];
        switch (cmd->commandType) {
          case CLAY_RENDER_COMMAND_TYPE_RECTANGLE:
            render_clay_rect(ctx, cmd);
            break;
          case CLAY_RENDER_COMMAND_TYPE_BORDER:
            break;
          case CLAY_RENDER_COMMAND_TYPE_TEXT:
            render_clay_text(ctx, cmd);
            break;
          case CLAY_RENDER_COMMAND_TYPE_IMAGE:
            break;
          case CLAY_RENDER_COMMAND_TYPE_SCISSOR_START:
            break;
          case CLAY_RENDER_COMMAND_TYPE_SCISSOR_END:
            break;
          case CLAY_RENDER_COMMAND_TYPE_CUSTOM:
            break;
          case CLAY_RENDER_COMMAND_TYPE_NONE: break;
        }
    }
}

static void error_text(MainUI_Context *ctx, uint32_t font_size, Clay_Color color) {
    Clay_String str = {
        .isStaticallyAllocated = false,
        .length = (int32_t)(uint32_t)ctx->status_str.length,
        .chars = ctx->status_str.data
    };

    Clay_TextElementConfig *cfg = PCB_Arena_calloc(ctx->arena, sizeof(*cfg));
    assert(cfg != NULL);
    cfg->textColor = color;
    cfg->fontSize = font_size;
    CLAY_TEXT(str, cfg);
}

static Clay_String alloc_clay_string(PCB_Arena *arena, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char *str = PCB_Arena_vasprintf(arena, fmt, args);
    va_end(args);
    assert(str != NULL);
    return (Clay_String){
        .length = strlen(str),
        .chars = str
    };
}

static bool connect_battleships_server(MainUI_Context *ctx) {
    uint16_t port = ctx->battleships.server_port;
    if(ctx->battleships.server_fd >= 0) {
        PCB_log(PCB_LOGLEVEL_WARN, "Already connected!");
        return true;
    }
    int fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(fd < 0) {
        PCB_String_reset(&ctx->status_str);
        PCB_String_appendf(
            &ctx->status_str,
            "Failed to create socket: %s",
            strerror(errno)
        );
        ctx->battleships.flags.server_conn_failed = true;
        return false;
    }
    struct sockaddr_in sa = PCB_ZEROED;
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    sa.sin_addr.s_addr = inet_addr(ctx->battleships.server_addr);
    if(connect(fd, (struct sockaddr*)&sa, sizeof(sa)) < 0) {
        PCB_String_reset(&ctx->status_str);
        PCB_String_appendf(
            &ctx->status_str,
            "Failed to connect to %s:%hu: %s",
            ctx->battleships.server_addr, port, strerror(errno)
        );
        ctx->battleships.flags.server_conn_failed = true;
        return false;
    }
    setnonblock(fd);
    ctx->battleships.flags.server_conn_failed = false;
    ctx->battleships.server_fd = fd;
    ctx->battleships.status = BATTLESHIPS_STATUS_IN_MENU;
    assert(PCB_String_reserve_to(&ctx->battleships.recv_buf, 1 << 16));
    ctx->battleships.boards.own.x = ctx->battleships.boards.own.y = -1;
    return true;
}

static void disconnect_battleships_server(MainUI_Context *ctx) {
    Battleships_Context *b = &ctx->battleships;
    if(b->server_fd >= 0) close(b->server_fd);
    b->server_fd = -1;
    b->status = BATTLESHIPS_STATUS_NOT_CONNECTED;
    b->flags.server_conn_failed = false;

    PCB_String *s = &b->recv_buf;
    if(!PCB_String_isEmpty(s))
        s->data[s->length = 0] = '\0';

    Lobby_Entries_destroy(&b->lobby);
    b->menu_state.lobby = BATTLESHIPS_STATUS_LOBBY_GOT_NONE;
    Battleships_Board_destroy(&b->boards.own);
    Battleships_Board_destroy(&b->boards.enemy);
    b->flags.boards_initialized = false;
}

static void tile_hover_edit(UI *ui, size_t y, size_t x) {
    bool left = left_clicked(ui), right = right_clicked(ui);
    if(!left && !right) return;
    MainUI_Context *ctx = ui->user_data;
    Battleships_Context *b = &ctx->battleships;
    Battleships_Board *board = &b->boards.own;
    if(board->data[y].data[x] != BATTLESHIPS_BOARD_TILE_EMPTY) return;
    if(board->ship_size == 0) return;
    if((board->x < 0 || board->y < 0) && left) {
        board->x = (int32_t)(uint32_t)x;
        board->y = (int32_t)(uint32_t)y;
        PCB_log(PCB_LOGLEVEL_DEBUG, "Selected tile (%zu, %zu)", x, y);
        return;
    }
    if((uint32_t)board->y > board->length || (uint32_t)board->x > board->data[0].length) {
        PCB_log(PCB_LOGLEVEL_WARN, "Previously selected tile went out of bounds! Resetting.");
        board->x = board->y = -1;
        return;
    }
    uint32_t size_index = 0;
    for(; size_index < board->ships.length; size_index++) {
        if(board->ships.data[size_index].size == board->ship_size) break;
    }
    if(size_index == board->ships.length) {
        PCB_log(PCB_LOGLEVEL_WARN, "Selected size (%u) does not exist!", board->ship_size);
        return;
    }
    PCB_log(
        PCB_LOGLEVEL_DEBUG,
        "Clicked tile (%zu, %zu), selected = (%d, %d)",
        x, y, board->x, board->y
    );
    int32_t dx = (int32_t)(ssize_t)x - board->x,
            dy = (int32_t)(ssize_t)y - board->y;
    if(dx == 0 && dy == 0) {
        if(right) {
            board->x = board->y = -1;
            return;
        }
        if(board->ship_size != 1) return;
        //NOTE: Server uses column major, we use row major
        char *msg = PCB_Arena_asprintf(ctx->arena, "%d %d 1 0\n", board->x, board->y);
        assert(msg != NULL);
        write(b->server_fd, msg, strlen(msg));
        goto end;
    } else if(dx != 0 && dy != 0) {
        PCB_log(PCB_LOGLEVEL_WARN, "Cannot put ship on a diagonal! (%d %d)", dx, dy);
        return;
    }
    uint32_t len = (uint32_t)(dx == 0 ? (dy < 0 ? -dy : dy) : (dx < 0 ? -dx : dx));
    if(board->ship_size != len+1) {
        PCB_log(PCB_LOGLEVEL_WARN, "Distance between tiles doesn't match selected ship size");
        return;
    }
    //   0
    //3     1
    //   2
    enum {
        UP = 0,
        RIGHT = 1,
        DOWN = 2,
        LEFT = 3
    };
    //NOTE: Server uses column major, we use row major
    int direction = dx == 0 ? (dy > 0 ? DOWN : UP) : (dx > 0 ? RIGHT : LEFT);
    // int direction = dx == 0 ? (dy > 0 ? RIGHT : LEFT) : (dx > 0 ? DOWN : UP);

    char *msg = PCB_Arena_asprintf(
        //NOTE: Server uses column major, we use row major
        ctx->arena, "%d %d %u %d\n", board->x, board->y, board->ship_size, direction
    );
    assert(msg != NULL);
    PCB_log(PCB_LOGLEVEL_DEBUG_NL, PCB_LOC": msg = %s", msg);
    write(b->server_fd, msg, strlen(msg));
end:
    board->x = board->y = -1;
    if(--board->ships.data[size_index].count == 0) { //no more can be placed
        board->ship_size = 0; //deselect size
    }
    //we expect a message back from the server
    b->menu_state.edit_board = BATTLESHIPS_STATUS_EDIT_BOARD_GOT_NONE;
    b->boards.own.edited = 0;
}

static void tile_hover_game(UI *ui, size_t y, size_t x) {
    MainUI_Context *ctx = ui->user_data;
    if(!left_clicked(ui)) return;
    Battleships_Context *b = &ctx->battleships;
    Battleships_Board *board = &b->boards.enemy;
    PCB_log(PCB_LOGLEVEL_DEBUG, PCB_LOC": Clicked at (%zu, %zu)", x, y);
    if(board->data[y].data[x] != BATTLESHIPS_BOARD_TILE_EMPTY) {
        PCB_log(PCB_LOGLEVEL_DEBUG, PCB_LOC": Empty (%d)", board->data[y].data[x]);
        return;
    }

    //NOTE: Server uses column major, we use row major
    char *msg = PCB_Arena_asprintf(ctx->arena, "%zu %zu\n", x, y);
    assert(msg != NULL);
    PCB_log(PCB_LOGLEVEL_DEBUG_NL, PCB_LOC": msg = %s", msg);
    write(b->server_fd, msg, strlen(msg));
    //we expect a message back from the server
    b->menu_state.game.status = BATTLESHIPS_STATUS_GAME_MESSAGE_GOT_NONE;
}

static Clay_Color tile_color_edit(
    Battleships_Board *board, size_t y, size_t x, bool hovered
) {
    //NOTE: Server uses column major, we use row major
    Battleships_Board_Tile tile = board->data[y].data[x];
    //previously selected
    if((int32_t)(ssize_t)x == board->x && (int32_t)(ssize_t)y == board->y)
        return CLAY_MAGENTA;
    if(tile != BATTLESHIPS_BOARD_TILE_EMPTY) return CLAY_GREEN;
    return hovered ? CLAY_DEFAULT_BUTTON_COLOR_HOVERED : CLAY_WHITE;
}

static Clay_Color tile_color_game(
    Battleships_Board *board, size_t y, size_t x, bool hovered
) {
    if(board == NULL) return CLAY_WHITE;
    if(board->data == NULL) return CLAY_WHITE;
    if(board->data[y].data == NULL) return CLAY_WHITE;
    switch(board->data[y].data[x]) {
      case BATTLESHIPS_BOARD_TILE_OCCUPIED:
        return CLAY_MAGENTA;
      case BATTLESHIPS_BOARD_TILE_HIT:
        return CLAY_RED;
      case BATTLESHIPS_BOARD_TILE_MISS:
        return (Clay_Color){50, 50, 50, 255};
        break;
      case BATTLESHIPS_BOARD_TILE_EMPTY:
        return hovered ? CLAY_DEFAULT_BUTTON_COLOR_HOVERED : CLAY_WHITE;
    }
    return CLAY_WHITE;
}

static void draw_board(
    UI *ui,
    Battleships_Board *board,
    Clay_String sid,
    Battleships_Board_Tile_onHover on_hover,
    Battleships_Board_Tile_Color_pfn tile_color
) {
    MainUI_Context *ctx = ui->user_data;
    CLAY(CLAY_SID(sid), {
        .layout = {
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .childAlignment = Clay_childAlignment_center(),
            .padding = CLAY_PADDING_ALL(16), .childGap = 16,
        },
        .backgroundColor = {0, 0, 0, 255},
    }) {
        PCB_Vec_enumerate(board, i, v, it, Battleships_Board_Row) {
            char *row_id = PCB_Arena_asprintf(
                ctx->arena,
                "%.*s_R%zu",
                sid.length, sid.chars, it.i
            );
            assert(row_id != NULL);
            Clay_String row_sid = {
                .length = strlen(row_id),
                .chars = row_id
            };
            CLAY(CLAY_SID(row_sid), {
                .layout = {
                    .layoutDirection = CLAY_LEFT_TO_RIGHT,
                    .childAlignment = Clay_childAlignment_center(),
                    // .padding = CLAY_PADDING_ALL(16),
                    .childGap = 16,
                },
                .backgroundColor = {0, 0, 0, 255},
                // .backgroundColor = {180, 180, 180, 255}
            }) {
                bool hovered;
                PCB_Vec_enumerate(it.v, i, v, jt, Battleships_Board_Tile) {
                    char *tile_id = PCB_Arena_asprintf(
                        ctx->arena,
                        "%.*s_T%zux%zu",
                        sid.length, sid.chars, it.i, jt.i
                    );
                    assert(tile_id != NULL);
                    Clay_String tile_sid = {
                        .length = strlen(tile_id),
                        .chars = tile_id
                    };
                    CLAY(CLAY_SID_LOCAL(tile_sid), {
                        .layout = {
                            .sizing = {
                                .width  = CLAY_SIZING_FIXED(25),
                                .height = CLAY_SIZING_FIXED(25)
                            },
                            .childAlignment = Clay_childAlignment_center()
                        },
                        .backgroundColor = tile_color != NULL
                            //NOTE: Server uses column major, we use row major
                            ? tile_color(board, it.i, jt.i, (hovered = Clay_Hovered()))
                            : CLAY_DEFAULT_BUTTON_COLOR
                    }) {
                        if(!hovered) continue;
                        if(on_hover == NULL) continue;
                        on_hover(ui, it.i, jt.i);
                    }
                }
            }
        }
    }
}

static void draw_connect_screen(UI *ui) {
    MainUI_Context *ctx = (MainUI_Context*)ui->user_data;
    bool hovered;
    CLAY(CLAY_ID("ConnectToServerOuter"), {
        .layout = {
            .sizing = {
                .width  = CLAY_SIZING_PERCENT(1.0f),
                .height = CLAY_SIZING_PERCENT(1.0f)
            },
            .childAlignment = Clay_childAlignment_center(),
            .padding = CLAY_PADDING_ALL(16), .childGap = 16,
        },
    }) {
        CLAY(CLAY_ID("ConnectToServer"), {
            .layout = {
                .sizing = {
                    .width  = CLAY_SIZING_FIT((0.25f*ui->env.viewport.w)),
                    .height = CLAY_SIZING_FIT((0.25f*ui->env.viewport.h))
                },
                .childAlignment = Clay_childAlignment_center(),
                .padding = CLAY_PADDING_ALL(16),
            },
            .backgroundColor = (hovered = Clay_Hovered())
                ? CLAY_DEFAULT_BUTTON_COLOR_HOVERED
                : CLAY_DEFAULT_BUTTON_COLOR,

        }) {
            if(left_clicked(ui) && hovered) {
                connect_battleships_server(ctx);
            }
            if(ctx->battleships.flags.server_conn_failed) {
                error_text(ctx, 20, CLAY_WHITE);
            } else {
                CLAY_TEXT(CLAY_STRING("Click to connect!"), CLAY_DEFAULT_TEXT_CONFIG());
            }
        }
    }
}

static void draw_menu(UI *ui) {
    MainUI_Context *ctx = ui->user_data;
    bool hovered;
    CLAY(CLAY_ID("MenuButtons"), {
        .layout = {
            .sizing = {
                .width  = CLAY_SIZING_PERCENT(1.0f),
                .height = CLAY_SIZING_PERCENT(1.0f)
            },
            .childAlignment = Clay_childAlignment_center(),
            .padding = CLAY_PADDING_ALL(16), .childGap = 16,
        },
    }) {
        Clay_LayoutConfig button_layout = {
            .sizing = {
                .height = CLAY_SIZING_PERCENT(0.1f)
            },
            .childAlignment = Clay_childAlignment_center(),
            .padding = CLAY_PADDING_ALL(16),
        };
        Battleships_Context *b = &ctx->battleships;
#define BUTTON_COLOR ((hovered = Clay_Hovered()) \
    ? CLAY_DEFAULT_BUTTON_COLOR_HOVERED \
    : CLAY_DEFAULT_BUTTON_COLOR)
        CLAY(CLAY_IDI("MenuButton", 1), {
            button_layout, .backgroundColor = BUTTON_COLOR
        }) {
            CLAY_TEXT(CLAY_STRING("Create a game"), CLAY_DEFAULT_TEXT_CONFIG());
            if(left_clicked(ui) && hovered) {
                //NOTE: Assumes that we receive the entire message in-between this
                //and previous state change.
                write(b->server_fd, "1\n", 2);
                b->flags.waiting_for_opponent = true;
                Battleships_Context_change_state(b, BATTLESHIPS_STATUS_IN_GAME);
            }
        }
        CLAY(CLAY_IDI("MenuButton", 2), {
            button_layout, .backgroundColor = BUTTON_COLOR
        }) {
            CLAY_TEXT(CLAY_STRING("Join a game"), CLAY_DEFAULT_TEXT_CONFIG());
            if(left_clicked(ui) && hovered) {
                //NOTE: Assumes that we receive the entire message in-between this
                //and previous state change.
                write(b->server_fd, "2\n", 2);
                Battleships_Context_change_state(b, BATTLESHIPS_STATUS_LOBBY);
            }
        }
        CLAY(CLAY_IDI("MenuButton", 3), {
            button_layout, .backgroundColor = BUTTON_COLOR
        }) {
            CLAY_TEXT(CLAY_STRING("Edit board"), CLAY_DEFAULT_TEXT_CONFIG());
            if(left_clicked(ui) && hovered) {
                //NOTE: Assumes that we receive the entire message in-between this
                //and previous state change.
                write(b->server_fd, "3\n", 2);
                Battleships_Context_change_state(b, BATTLESHIPS_STATUS_IN_EDIT_BOARD);
            }
        }
#undef BUTTON_COLOR
    }
}

static void draw_board_edit(UI *ui) {
    MainUI_Context *ctx = ui->user_data;
    Battleships_Context *b = &ctx->battleships;
    CLAY(CLAY_ID("BoardEdit_Wrapper"), {
        .layout = {
            .sizing = {
                .width  = CLAY_SIZING_PERCENT(1.0f),
                .height = CLAY_SIZING_PERCENT(1.0f)
            },
            .childAlignment = Clay_childAlignment_center(),
            .padding = CLAY_PADDING_ALL(16), .childGap = 16,
        },
    }) {
        CLAY(CLAY_ID("BoardEdit_Buttons"), {
            .layout = {
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
                .padding = CLAY_PADDING_ALL(16), .childGap = 16,
            }
        }) {
            bool hovered;
            PCB_Vec_enumerate(
                &b->boards.own.ships, i, v, it, 
                Battleships_Available_Ships
            ) {
                CLAY(CLAY_SID(
                    alloc_clay_string(ctx->arena, "BoardEdit_Button_%u", it.v->size)
                ), {
                    .layout = {
                        .sizing = { .width = CLAY_SIZING_GROW(0) },
                        .padding = CLAY_PADDING_ALL(8),
                        .childAlignment = Clay_childAlignment_center(),
                    },
                    .backgroundColor =
                        it.v->size == b->boards.own.ship_size
                        ? (Clay_Color){200, 0, 200, 255}
                        : (it.v->count > 0
                            ? ((hovered = Clay_Hovered())
                                ? CLAY_DEFAULT_BUTTON_COLOR_HOVERED
                                : CLAY_DEFAULT_BUTTON_COLOR)
                            : CLAY_RED)
                }) {
                    CLAY_TEXT(
                        alloc_clay_string(
                            ctx->arena, "%u: %u left", it.v->size, it.v->count
                        ),
                        CLAY_DEFAULT_TEXT_CONFIG()
                    );
                    if(left_clicked(ui) && hovered && it.v->count > 0) {
                        if(b->boards.own.ship_size == it.v->size) {
                            b->boards.own.ship_size = 0; //deselect
                        } else {
                            b->boards.own.ship_size = it.v->size;
                        }
                    }
                }
            }
            Clay_LayoutConfig lCfg = {
                .sizing = { .width = CLAY_SIZING_GROW(0) },
                .padding = CLAY_PADDING_ALL(8),
                .childAlignment = Clay_childAlignment_center(),
            };
            CLAY(CLAY_ID("BoardEdit_Button_clear"), {
                .layout = lCfg,
                .backgroundColor = (hovered = Clay_Hovered())
                    ? CLAY_DEFAULT_BUTTON_COLOR_HOVERED
                    : CLAY_DEFAULT_BUTTON_COLOR
            }) {
                CLAY_TEXT(CLAY_STRING("Clear board"), CLAY_DEFAULT_TEXT_CONFIG());
                if(left_clicked(ui) && hovered) {
                    write(b->server_fd, "clear\n", 6);
                    b->boards.own.edited = 0;
                    b->menu_state.edit_board = BATTLESHIPS_STATUS_EDIT_BOARD_GOT_NONE;
                }
            }
            CLAY(CLAY_ID("BoardEdit_Button_exit"), {
                .layout = lCfg,
                .backgroundColor = (hovered = Clay_Hovered())
                    ? CLAY_DEFAULT_BUTTON_COLOR_HOVERED
                    : CLAY_DEFAULT_BUTTON_COLOR
            }) {
                CLAY_TEXT(CLAY_STRING("Back to menu"), CLAY_DEFAULT_TEXT_CONFIG());
                if(left_clicked(ui) && hovered) {
                    write(b->server_fd, "exit\n", 5);
                    Battleships_Context_change_state(b, BATTLESHIPS_STATUS_IN_MENU);
                }
            }
        }
        draw_board(
            ui, &b->boards.own,
            CLAY_STRING("BoardEdit"),
            tile_hover_edit, tile_color_edit
        );
    }
}

static void draw_waiting(UI *ui) {
    MainUI_Context *ctx = ui->user_data;
    ctx->battleships.menu_state.game.wait.dt += ui->env.dt;
    CLAY(CLAY_ID("Waiting"), {
        .layout = {
            .sizing = {
                .width  = CLAY_SIZING_PERCENT(1.0f),
                .height = CLAY_SIZING_PERCENT(1.0f)
            },
            .childAlignment = Clay_childAlignment_center(),
        },
    }) {
        while(ctx->battleships.menu_state.game.wait.dt >= 6.f/3.f)
            ctx->battleships.menu_state.game.wait.dt -= 6.f/3.f;

        if(ctx->battleships.menu_state.game.wait.dt >= 4.f/3.f) {
            CLAY_TEXT(CLAY_STRING("Waiting for opponent..."), CLAY_DEFAULT_TEXT_CONFIG());
        }
        else if(ctx->battleships.menu_state.game.wait.dt >= 2.f/3.f) {
            CLAY_TEXT(CLAY_STRING("Waiting for opponent.."), CLAY_DEFAULT_TEXT_CONFIG());
        } else {
            CLAY_TEXT(CLAY_STRING("Waiting for opponent."), CLAY_DEFAULT_TEXT_CONFIG());
        }
    }
}

static void draw_lobby(UI *ui) {
    MainUI_Context *ctx = ui->user_data;
    CLAY(CLAY_ID("MenuButtons"), {
        .layout = {
            .sizing = {
                .width  = CLAY_SIZING_PERCENT(1.0f),
                .height = CLAY_SIZING_PERCENT(1.0f)
            },
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .childAlignment = Clay_childAlignment_center(),
            .padding = CLAY_PADDING_ALL(16), .childGap = 16,
        },
        .clip = { .vertical = true, .childOffset = Clay_GetScrollOffset() }
    }) {
        bool hovered;
        PCB_Vec_enumerate(&ctx->battleships.lobby, i, v, it, Lobby_Entry) {
            CLAY(CLAY_IDI("Lobby", it.i), {
                .layout = {
                    .sizing = { .width = CLAY_SIZING_FIT(400) },
                    .childAlignment = Clay_childAlignment_center(),
                    .padding = CLAY_PADDING_ALL(8)
                },
                .backgroundColor = (hovered = Clay_Hovered())
                    ? CLAY_DEFAULT_BUTTON_COLOR_HOVERED
                    : CLAY_DEFAULT_BUTTON_COLOR
            }) {
                Clay_String str = {
                    .length = (int32_t)(uint32_t)it.v->name.length,
                    //Lobby entries are invalidated on state change.
                    //This is a hack.
                    .chars = PCB_Arena_strdup(ctx->arena, it.v->name.data)
                };
                assert(str.chars != NULL);
                Clay_TextElementConfig *cfg = PCB_Arena_calloc(ctx->arena, sizeof(*cfg));
                assert(cfg != NULL);
                cfg->textColor = CLAY_WHITE;
                cfg->fontSize = 20;
                CLAY_TEXT(str, cfg);
                if(left_clicked(ui) && hovered) {
                    char *msg = PCB_Arena_asprintf(ctx->arena, "%zu", it.i);
                    assert(msg != NULL);
                    write(ctx->battleships.server_fd, msg, strlen(msg));
                    Battleships_Context_change_state(
                        &ctx->battleships, BATTLESHIPS_STATUS_IN_GAME
                    );
                }
            }
        }
    }
}

static void draw_game(UI *ui) {
    MainUI_Context *ctx = ui->user_data;
    Battleships_Context *b = &ctx->battleships;
    if(b->flags.waiting_for_opponent) {
        draw_waiting(ui);
        return;
    }
    CLAY(CLAY_ID("BoardGame_Wrapper"), {
        .layout = {
            .sizing = {
                .width  = CLAY_SIZING_PERCENT(1.0f),
                .height = CLAY_SIZING_PERCENT(1.0f)
            },
            .childAlignment = Clay_childAlignment_center(),
            .padding = CLAY_PADDING_ALL(16), .childGap = 16,
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
    }) {
        CLAY(CLAY_ID("GameStatus"), {
            .layout = {
                .sizing = {
                    .height = CLAY_SIZING_PERCENT(0.1f)
                },
            }
        }) {
            Clay_String s = CLAY_DEFAULT_STRING;
            Clay_Color textColor = CLAY_WHITE;
            switch(b->menu_state.game.status) {
              case BATTLESHIPS_STATUS_GAME_OUR_TURN:
                s = alloc_clay_string(ctx->arena, "Your turn");
                textColor = CLAY_CYAN;
                break;
              case BATTLESHIPS_STATUS_GAME_ENEMY_TURN:
                s = alloc_clay_string(ctx->arena, "Enemy turn");
                textColor = CLAY_MAGENTA;
                break;
              case BATTLESHIPS_STATUS_GAME_WON:
                s = alloc_clay_string(ctx->arena, "You won");
                textColor = CLAY_GREEN;
                break;
              case BATTLESHIPS_STATUS_GAME_LOST:
                s = alloc_clay_string(ctx->arena, "You lost");
                textColor = CLAY_RED;
                break;
              case BATTLESHIPS_STATUS_GAME_MESSAGE_GOT_NONE:
              case BATTLESHIPS_STATUS_GAME_MESSAGE_GOT_OWN_BOARD:
              case BATTLESHIPS_STATUS_GAME_MESSAGE_GOT_SHOOTING_BOARD:
                s = CLAY_DEFAULT_STRING;
                break;
              case BATTLESHIPS_STATUS_GAME_COUNT:
                PCB_Unreachable;
            }
            CLAY_TEXT(s, CLAY_TEXT_CONFIG({.fontSize = 32, .textColor = textColor }));
        }
        bool game_ended =
            b->menu_state.game.status == BATTLESHIPS_STATUS_GAME_WON ||
            b->menu_state.game.status == BATTLESHIPS_STATUS_GAME_LOST;

        if(game_ended) {
            bool hovered;
            CLAY(CLAY_ID("GameFinishButton"), {
                .layout = {
                    .padding = CLAY_PADDING_ALL(8)
                },
                .backgroundColor = (hovered = Clay_Hovered())
                    ? CLAY_DEFAULT_BUTTON_COLOR_HOVERED
                    : CLAY_DEFAULT_BUTTON_COLOR
            }) {
                CLAY_TEXT(CLAY_STRING("Press to continue"), CLAY_DEFAULT_TEXT_CONFIG());
                if(left_clicked(ui) && hovered) {
                    write(b->server_fd, "", 1);
                    Battleships_Context_change_state(b, BATTLESHIPS_STATUS_IN_MENU);
                    PCB_log(PCB_LOGLEVEL_DEBUG, PCB_LOC": ships.length = %zu", b->boards.own.ships.length);
                }
            }
        }
        CLAY(CLAY_ID("Boards"), 0) {
            CLAY(CLAY_ID("BoardOursContainer"), {
                .layout = {
                    .childAlignment = Clay_childAlignment_center(),
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                }
            }) {
                CLAY_TEXT(CLAY_STRING("Your board"), CLAY_DEFAULT_TEXT_CONFIG());
                draw_board(
                    ui, &b->boards.own, CLAY_STRING("BoardOurs"),
                    NULL, tile_color_game
                );
            }
            CLAY(CLAY_ID("BoardEnemyContainer"), {
                .layout = {
                    .childAlignment = Clay_childAlignment_center(),
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                }
            }) {
                CLAY_TEXT(CLAY_STRING("Shooting board"), CLAY_DEFAULT_TEXT_CONFIG());
                draw_board(
                    ui, &b->boards.enemy, CLAY_STRING("BoardEnemy"),
                    game_ended ? NULL : tile_hover_game, tile_color_game
                );
            }
        }
    }
}

static void parse_board_row(PCB_StringView sv, Battleships_Board *board) {
    if(board->edited >= board->length) {
        PCB_log(
            PCB_LOGLEVEL_WARN,
            "Did server board schema change? Got too many rows (%u)",
            board->edited
        );
        PCB_log(PCB_LOGLEVEL_DEBUG, "Message was '" PCB_SV_Fmt "'", PCB_SV_Arg(sv));
        return;
    }
    Battleships_Board_Row *row = &board->data[board->edited];
    PCB_StringView sv_ = sv;
    for(size_t i = 0; i < row->length && sv.length > 1; i++) {
        //NOTE: Assumes a format of "* * * ..."
        switch(*sv_.data) {
          case '*':
            row->data[i] = BATTLESHIPS_BOARD_TILE_EMPTY;
            break;
          case '#':
            row->data[i] = BATTLESHIPS_BOARD_TILE_OCCUPIED;
            break;
          case '!':
            row->data[i] = BATTLESHIPS_BOARD_TILE_HIT;
            break;
          case '0':
            row->data[i] = BATTLESHIPS_BOARD_TILE_MISS;
            break;
          default:
            break;
        }
        sv_.data += 2; sv_.length -= 2;
    }
    ++board->edited;
}

static void parse_edit_board_message(MainUI_Context *ctx) {
    Battleships_Context *b = &ctx->battleships;
    PCB_String *s = &b->recv_buf;
continue_parsing_edit_board_msg:
    switch(b->menu_state.edit_board) {
      case BATTLESHIPS_STATUS_EDIT_BOARD_GOT_NONE:
        while(s->length > 0) {
            // PCB_log(PCB_LOGLEVEL_DEBUG, PCB_LOC": %s", s->data);
            if(!strncmp(s->data, "Available ships:\n----------\n", 28)) {
                assert(PCB_String_remove_range(s, 0, 28));
                b->menu_state.edit_board= BATTLESHIPS_STATUS_EDIT_BOARD_GOT_BOARD;
                goto continue_parsing_edit_board_msg;
            }
            PCB_StringView sv = PCB_String_subcstr(s, "\n");
            if(PCB_String_isEmpty(&sv)) break;
            sv = PCB_CLITERAL(PCB_StringView){s->data, (size_t)(sv.data - s->data)};
            if(!b->flags.boards_initialized) {
                //NOTE: Assumes a format of "* * * ..."
                Battleships_Board_new_row(&b->boards.own,   sv.length/2);
                Battleships_Board_new_row(&b->boards.enemy, sv.length/2);
            }
            parse_board_row(sv, &b->boards.own);
            assert(PCB_String_remove_range(s, 0, sv.length+1));
            // PCB_log(PCB_LOGLEVEL_DEBUG, PCB_LOC": Removed %zu bytes", sv.length+1);
            // PCB_log(PCB_LOGLEVEL_DEBUG, PCB_LOC": Left to parse: '%s'", s->data);
        }
        break;
      case BATTLESHIPS_STATUS_EDIT_BOARD_GOT_BOARD: {
        PCB_ArenaMark *m = PCB_Arena_mark(ctx->arena);
        while(s->length > 0) {
            // PCB_log(PCB_LOGLEVEL_DEBUG, PCB_LOC": %s", s->data);
            if(!strncmp(s->data, "----------\n", 11)) {
                assert(PCB_String_remove_range(s, 0, 11));
                b->menu_state.edit_board = BATTLESHIPS_STATUS_EDIT_BOARD_GOT_END;
                b->flags.boards_initialized = true;
                PCB_log(PCB_LOGLEVEL_DEBUG, PCB_LOC": Got end");
                break;
            }
            PCB_StringView sv = PCB_String_subcstr(s, "\n");
            if(PCB_String_isEmpty(&sv)) break;
            sv = PCB_CLITERAL(PCB_StringView){s->data, (size_t)(sv.data - s->data)};
            char *line = PCB_Arena_strndup(
                ctx->arena,
                sv.data   + (sizeof("Size ")-1),
                sv.length - (sizeof("Size ")-1)
            );
            assert(line != NULL);
            // PCB_log(PCB_LOGLEVEL_DEBUG, "line = %s", line);
            uint32_t size = (uint32_t)strtoul(line, &line, 10);
            line += sizeof(" number left ")-1;
            uint32_t count = (uint32_t)strtoul(line, NULL, 10);
            if(!b->flags.boards_initialized) {
                Battleships_Available_Ships_Of_Size avail = {
                    (uint32_t)size, (uint32_t)count
                };
                PCB_log(PCB_LOGLEVEL_DEBUG, "Appending avail");
                PCB_Vec_append(&b->boards.own.ships, avail);
            }
            // PCB_log(PCB_LOGLEVEL_DEBUG, "size = %u, count = %u", size, count);
            PCB_Vec_forEach_it(&b->boards.own.ships, it, Battleships_Available_Ships_Of_Size) {
                if(it->size != size) continue;
                it->count = count;
                break;
            }
            PCB_String_remove_range(s, 0, sv.length+1);
            PCB_Arena_restore_to(ctx->arena, m);
        }
        PCB_Arena_restore(ctx->arena, m);
      } break;
      case BATTLESHIPS_STATUS_EDIT_BOARD_GOT_END:
        b->boards.own.edited = 0;
        break;
      case BATTLESHIPS_STATUS_EDIT_BOARD_COUNT: PCB_Unreachable;
    }
}

static void parse_game_message(MainUI_Context *ctx) {
    Battleships_Context *b = &ctx->battleships;
    PCB_String *s = &b->recv_buf;
    if(b->flags.waiting_for_opponent) {
        if(!strcmp(s->data, "Waiting for opponent\n")) {
            assert(PCB_String_remove_range(s, 0, 21));
            return;
        }
        b->flags.waiting_for_opponent = false;
        b->menu_state.game.status = BATTLESHIPS_STATUS_GAME_MESSAGE_GOT_NONE;
        PCB_log(PCB_LOGLEVEL_DEBUG, "Waited for opponent");
    }
continue_parsing_game_msg:
    PCB_log(PCB_LOGLEVEL_DEBUG, PCB_LOC": Game state = %d", b->menu_state.game.status);
    switch(b->menu_state.game.status) {
      case BATTLESHIPS_STATUS_GAME_MESSAGE_GOT_NONE:
        while(s->length > 0) {
            // PCB_log(PCB_LOGLEVEL_DEBUG, PCB_LOC": %s", s->data);
            if(!strncmp(s->data, "Your board:\n", 12)) {
                assert(PCB_String_remove_range(s, 0, 12));
                continue;
            }
            if(!strncmp(s->data, "\nShooting board:\n", 17)) {
                assert(PCB_String_remove_range(s, 0, 17));
                b->menu_state.game.status = BATTLESHIPS_STATUS_GAME_MESSAGE_GOT_OWN_BOARD;
                b->flags.boards_initialized = true;
                goto continue_parsing_game_msg;
            }
            PCB_StringView sv = PCB_String_subcstr(s, "\n");
            if(PCB_String_isEmpty(&sv)) break;
            sv = PCB_CLITERAL(PCB_StringView){s->data, (size_t)(sv.data - s->data)};
            PCB_log(PCB_LOGLEVEL_DEBUG, PCB_LOC": '" PCB_SV_Fmt "'", PCB_SV_Arg(sv));
            if(!b->flags.boards_initialized) {
                //NOTE: Assumes a format of "* * * ..."
                Battleships_Board_new_row(&b->boards.own,   sv.length/2);
                Battleships_Board_new_row(&b->boards.enemy, sv.length/2);
            }
            parse_board_row(sv, &b->boards.own);
            PCB_String_remove_range(s, 0, sv.length+1);
        }
        break;
      case BATTLESHIPS_STATUS_GAME_MESSAGE_GOT_OWN_BOARD: {
        while(s->length > 0) {
            PCB_StringView sv = PCB_String_subcstr(s, "\n");
            if(PCB_String_isEmpty(&sv)) break;
            sv = PCB_CLITERAL(PCB_StringView){s->data, (size_t)(sv.data - s->data)};
            if(sv.length == 0) {
                assert(PCB_String_remove_range(s, 0, 1));
                b->menu_state.game.status = BATTLESHIPS_STATUS_GAME_MESSAGE_GOT_SHOOTING_BOARD;
                goto continue_parsing_game_msg;
            }
            parse_board_row(sv, &b->boards.enemy);
            PCB_String_remove_range(s, 0, sv.length+1);
        }
      } break;
      case BATTLESHIPS_STATUS_GAME_MESSAGE_GOT_SHOOTING_BOARD: {
        PCB_StringView sv = PCB_String_subcstr(s, "\n");
        if(PCB_String_isEmpty(&sv)) break;
        sv = PCB_CLITERAL(PCB_StringView){s->data, (size_t)(sv.data - s->data)};
        if(!strncmp(sv.data, "WAIT", sv.length)) {
            b->menu_state.game.status = BATTLESHIPS_STATUS_GAME_ENEMY_TURN;
        } else if(!strncmp(sv.data, "YOUR TURN", sv.length)) {
            b->menu_state.game.status = BATTLESHIPS_STATUS_GAME_OUR_TURN;
        } else if(!strncmp(sv.data, "YOU WON", sv.length)) {
            b->menu_state.game.status = BATTLESHIPS_STATUS_GAME_WON;
        } else if(!strncmp(sv.data, "YOU LOST", sv.length)) {
            b->menu_state.game.status = BATTLESHIPS_STATUS_GAME_LOST;
        } else {
            PCB_log(
                PCB_LOGLEVEL_WARN,
                "Received unknown game status: '" PCB_SV_Fmt "'",
                PCB_SV_Arg(sv)
            );
            disconnect_battleships_server(ctx);
        }
        b->boards.own.edited = b->boards.enemy.edited = 0;
        PCB_String_remove_range(s, 0, sv.length+1);
      } break;
      //If we're called with these set, a move was made.
      case BATTLESHIPS_STATUS_GAME_OUR_TURN:
      case BATTLESHIPS_STATUS_GAME_ENEMY_TURN:
        b->menu_state.game.status = BATTLESHIPS_STATUS_GAME_MESSAGE_GOT_NONE;
        goto continue_parsing_game_msg;
      case BATTLESHIPS_STATUS_GAME_WON:
      case BATTLESHIPS_STATUS_GAME_LOST:
        //server sends "Send anything to continue"; we don't care about it
        PCB_String_reset(s);
        break;
      case BATTLESHIPS_STATUS_GAME_COUNT:
      default:
        PCB_Unreachable;
    }
}

static void poll_and_parse_server_message(MainUI_Context *ctx) {
    ssize_t r;
    PCB_String *s = &ctx->battleships.recv_buf;
    while((r = read(
        ctx->battleships.server_fd,
        s->data + s->length,
        s->capacity - s->length - 1
    )) > 0) {
        PCB_log(PCB_LOGLEVEL_DEBUG, "Received '%.*s'", (int)r, s->data + s->length);
        s->data[s->length += (size_t)r] = '\0';
        if(s->length + 1 == s->capacity)
            assert(PCB_String_reserve(s, s->capacity));
    }
    if(r == 0) {
        disconnect_battleships_server(ctx);
        return;
    } else switch(errno) {
      case EWOULDBLOCK:
        if(s->length == 0) return; //saves some compute by not processing further
        break;
      default:
        PCB_logLatestError("Failed to read from battleships server socket");
        disconnect_battleships_server(ctx);
        return;
    }
    Battleships_Context *b = &ctx->battleships;
    // PCB_log(PCB_LOGLEVEL_DEBUG, PCB_LOC": %s", Battleships_Status_hrn[b->status]);
    switch(b->status) {
      case BATTLESHIPS_STATUS_NOT_CONNECTED:
      case BATTLESHIPS_STATUS_IN_MENU:
        break;
      case BATTLESHIPS_STATUS_IN_EDIT_BOARD:
        parse_edit_board_message(ctx);
        break;
      case BATTLESHIPS_STATUS_LOBBY: {
      continue_parsing_lobby_msg:
        switch(b->menu_state.lobby) {
          case BATTLESHIPS_STATUS_LOBBY_GOT_NONE:
            if(!strncmp(s->data, "Current games:\n----------\n", 26)) {
                assert(PCB_String_remove_range(s, 0, 26));
                b->menu_state.lobby = BATTLESHIPS_STATUS_LOBBY_GOT_HEADER;
                goto continue_parsing_lobby_msg;
            }
            break;
          case BATTLESHIPS_STATUS_LOBBY_GOT_HEADER: {
            PCB_ArenaMark *m = PCB_Arena_mark(ctx->arena);
            while(1) {
                PCB_StringView sv = PCB_String_subcstr(s, "\n");
                if(PCB_String_isEmpty(&sv)) break;
                sv = PCB_CLITERAL(PCB_StringView){s->data, (size_t)(sv.data - s->data)};
                if(!strncmp(sv.data, "----------\n", sv.length)) {
                    assert(PCB_String_remove_range(s, 0, 11));
                    b->menu_state.lobby = BATTLESHIPS_STATUS_LOBBY_GOT_END;
                    break;
                }
                Lobby_Entries *l = &b->lobby;
                Lobby_Entry ent = PCB_ZEROED;
                char *line = PCB_Arena_strndup(ctx->arena, sv.data, sv.length);
                assert(line != NULL);
                //skip ID, currently mapped to internal index, so the client
                //representation will be identical without it
                (void)strtoul(line, &line, 10);
                while(PCB_isspace(*line)) ++line;
                PCB_String_append_cstr(&ent.name, line);
                PCB_Vec_append(l, ent);
                PCB_String_remove_range(s, 0, sv.length+1);
                PCB_Arena_restore_to(ctx->arena, m);
            }
            if(DEBUG) PCB_log(
                PCB_LOGLEVEL_DEBUG,
                PCB_LOC": Got %zu lobby entries",
                b->lobby.length
            );
            PCB_Arena_restore(ctx->arena, m);
          } break;
          case BATTLESHIPS_STATUS_LOBBY_GOT_END:
            break;
          case BATTLESHIPS_STATUS_LOBBY_COUNT: PCB_Unreachable;
        } break;
      }
      case BATTLESHIPS_STATUS_IN_GAME:
        parse_game_message(ctx);
        break;
      case BATTLESHIPS_STATUS_COUNT: PCB_Unreachable;
    }
}

void MainUI_update(UI *ui) {
    static float tdt = 0.0f;
    MainUI_Context *ctx = (MainUI_Context*)ui->user_data;
    PCB_Arena_restore_to(ctx->arena, ctx->mark);

    Clay_SetCurrentContext((Clay_Context*)ui->ctx);
    Clay_SetLayoutDimensions(
        (Clay_Dimensions){ui->env.viewport.w, ui->env.viewport.h}
    );
    Clay_SetPointerState(
        (Clay_Vector2){ui->env.cursor.x, ui->env.cursor.y},
        ui->env.mouse_btn_state & 1
    );
    Clay_UpdateScrollContainers(
        true,
        (Clay_Vector2) {ui->env.scroll.x, ui->env.scroll.y},
        ui->env.dt
    );
    tdt += ui->env.dt;
    if(ctx->battleships.server_fd >= 0) poll_and_parse_server_message(ctx);

    Clay_BeginLayout();
    bool hovered;
    CLAY(CLAY_ID("root"), {
        .layout = {
            .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)},
            .padding = CLAY_PADDING_ALL(16),
            .childGap = 16
        },
        .backgroundColor = {30, 30, 30, 255}
    }) {
        CLAY(CLAY_ID("ResetButton"), {
            .floating = {
                .offset = {50, 50},
                .zIndex = 1,
                .attachPoints = {
                    .element = CLAY_ATTACH_POINT_LEFT_TOP,
                    .parent = CLAY_ATTACH_POINT_LEFT_TOP,
                },
                .attachTo = CLAY_ATTACH_TO_PARENT,
            },
            .layout = {
                .childAlignment = Clay_childAlignment_center(),
                .padding = CLAY_PADDING_ALL(16),
            },
            .backgroundColor = (hovered = Clay_Hovered())
                ? CLAY_DEFAULT_BUTTON_COLOR_HOVERED
                : CLAY_DEFAULT_BUTTON_COLOR,
        }) {
            CLAY_TEXT(CLAY_STRING("Reset"), CLAY_DEFAULT_TEXT_CONFIG());
            if(left_clicked(ui) && hovered) {
                disconnect_battleships_server(ctx);
            }
        }
        switch(ctx->battleships.status) {
          case BATTLESHIPS_STATUS_NOT_CONNECTED:
            draw_connect_screen(ui);
            break;
          case BATTLESHIPS_STATUS_IN_MENU:
            draw_menu(ui);
            break;
          case BATTLESHIPS_STATUS_IN_EDIT_BOARD:
            draw_board_edit(ui);
            break;
          case BATTLESHIPS_STATUS_LOBBY:
            draw_lobby(ui);
            break;
          case BATTLESHIPS_STATUS_IN_GAME:
            draw_game(ui);
            break;
          case BATTLESHIPS_STATUS_COUNT: PCB_Unreachable;
        }
    }
    render_clay(ctx);
}

void MainUI_destroy(UI *ui) {
    MainUI_Context *ctx = (MainUI_Context*)ui->user_data;
    PCB_String_destroy(&ctx->status_str);
    Battleships_Context_destroy(&ctx->battleships);
    PCB_Arena_destroy(ctx->arena);
    Clay_SetCurrentContext(NULL);
}

static void handle_clay_error(Clay_ErrorData e) {
    PCB_log(PCB_LOGLEVEL_ERROR, "[Clay] %s", e.errorText.chars);
    __asm__("int3");
    switch(e.errorType) {
      case CLAY_ERROR_TYPE_TEXT_MEASUREMENT_FUNCTION_NOT_PROVIDED:
        break;
      case CLAY_ERROR_TYPE_ARENA_CAPACITY_EXCEEDED:
        break;
      case CLAY_ERROR_TYPE_ELEMENTS_CAPACITY_EXCEEDED:
        break;
      case CLAY_ERROR_TYPE_TEXT_MEASUREMENT_CAPACITY_EXCEEDED:
        break;
      case CLAY_ERROR_TYPE_DUPLICATE_ID:
        break;
      case CLAY_ERROR_TYPE_FLOATING_CONTAINER_PARENT_NOT_FOUND:
        break;
      case CLAY_ERROR_TYPE_PERCENTAGE_OVER_1:
        break;
      case CLAY_ERROR_TYPE_INTERNAL_ERROR:
        break;
      case CLAY_ERROR_TYPE_UNBALANCED_OPEN_CLOSE:
        break;
    }
}

static Clay_Dimensions measure_text(
    Clay_StringSlice text,
    Clay_TextElementConfig *config,
    void *userData
) {
    UI *ui = (UI*)userData;
    MainUI_Context *ctx = (MainUI_Context*)ui->user_data;
    AtlasText *a = &ctx->r->fontAtlas;
    Clay_Dimensions dims = PCB_ZEROED;

    ssize_t hIdx = AtlasText_queryHeight(a, config->fontSize);
    if(hIdx < 0) {
        PCB_log(PCB_LOGLEVEL_INFO, PCB_LOC": Font height %hu not in atlas", config->fontSize);
        hIdx = AtlasText_addHeight(a, config->fontSize);
        if(hIdx < 0) {
            PCB_log(
                PCB_LOGLEVEL_ERROR,
                PCB_LOC":%s: Failed to add font height %u to atlas",
                __func__, config->fontSize
            ); return dims;
        }
        // return dims;
    }
    const AtlasTextDataByHeight *const d = &a->data.data[hIdx];
    const float linespace = (float)(d->linespace/64) + (float)(d->linespace%64)/64.0f;

    dims.height += linespace;
    float x = 0.0f;
    // float y = 0.0f; //unused unless we decide to render text vertically (nah bro)

    PCB_StringView sv = Clay_StringSlice_2_PCB_StringView(text);
    // PCB_log(
    //     PCB_LOGLEVEL_DEBUG,
    //     PCB_LOC":%s: Clay text = '" PCB_SV_Fmt "'",
    //     __func__, PCB_SV_Arg(sv)
    // );
    PCB_Codepoint cp; cp.code = 0; cp.length = 1;
    for(; sv.length > 0; sv.data += cp.length, sv.length -= cp.length) {
        cp = PCB_StringView_GetCodepoint(sv, 0);
        if(cp.code < 0) {
            switch(cp.code) {
              case -2: goto defer;
              case -1: case -3: case -4: case -5: case -6:
                continue;
              case -16: case -17: case -18:
              default:
                PCB_Unreachable;
            }
        }
        switch(cp.code) {
          case '\n':
            dims.height += linespace;
            if(x > dims.width) dims.width = x;
            x = 0.0f;
            continue;
          //other control characters would go here
          default:
            break;
        }
    find_char_in_atlas:;
        const uint32_t cp_ = (uint32_t)cp.code;
        int32_t L = 1, R = (int32_t)d->length - 1;
        //find index of `cp` using binary search
        while(L <= R) {
            int32_t m = L + (R-L)/2;
            if(d->cps[m] < cp_) L = m + 1;
            else if(d->cps[m] > cp_) R = m - 1;
            else { L = R = m; break; }
        }
        if(d->cps[L] != cp_) { //not in current height
            L = 0; R = (int32_t)(uint32_t)d->cps_failed.length - 1;
            while(L <= R) {
                int32_t m = L + (R-L)/2;
                if(d->cps_failed.data[m] < cp_) L = m + 1;
                else if(d->cps_failed.data[m] > cp_) R = m - 1;
                else { L = R = m; break; }
            }
            if(L != R) { //not found in "failed" ones
                //TODO: Should be scheduled outside of drawing text
                //and executed after swapping buffers
                //though on 2nd thought, Clay caches values returned from here,
                //so we'd need to force it to remeasure
                if(!AtlasText_addCodepoint(a, hIdx, cp_)) {
                    PCB_log(PCB_LOGLEVEL_ERROR, "Failed to add U+%X to atlas", cp_);
                    L = 0;
                } else goto find_char_in_atlas;
            }
            // PCB_log(PCB_LOGLEVEL_DEBUG, PCB_LOC": U+%X not found in atlas", cp_);
        }
        Character *c = &d->cpData[L];
        x += (float)(c->advance.x/64) + (float)(c->advance.x%64)/64.0f;
    }
    if(x > dims.width) dims.width = x;
defer:
    return dims;
}
