//Copyright (c) 2026 Piotr Mikołajewski
#pragma once

#include <PCB.h>

#include "UI.h"
#include "Renderer.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef bool (*MainUI_init_pfn)(UI *ui, Renderer *r, const char *server);
typedef void (*MainUI_update_pfn)(UI *ui);
typedef void (*MainUI_destroy_pfn)(UI *ui);
typedef void (*MainUI_prereload_pfn)(UI *ui);
typedef void (*MainUI_postreload_pfn)(UI *ui);

bool MainUI_init(UI *ui, Renderer *r, const char *server);
void MainUI_update(UI *ui);
void MainUI_destroy(UI *ui);
void MainUI_prereload(UI *ui);
void MainUI_postreload(UI *ui);

#ifdef __cplusplus
}
#endif

