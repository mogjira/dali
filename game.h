#ifndef G_GAME_H
#define G_GAME_H

#include "common.h"
#include <coal/coal.h>

typedef struct {
    bool shouldRun;
} G_GameState;

struct Obdn_I_Event;

extern G_GameState gameState;

void g_Init(void);
void g_CleanUp(void);
bool g_Responder(const struct Obdn_I_Event *event);
void g_Update(void);
void g_SetColor(const float r, const float g, const float b);
void g_SetRadius(const float r);
void g_UpdateView(const Mat4* m);

#endif /* end of include guard: G_GAME_H */
