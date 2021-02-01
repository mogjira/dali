#include "painter.h"
#include "game.h"
#include "common.h"
#include "render.h"
#include "obsidian/f_file.h"
#include "obsidian/r_geo.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <stdbool.h>
#include <obsidian/v_video.h>
#include <obsidian/d_display.h>
#include <obsidian/r_render.h>
#include <obsidian/r_raytrace.h>
#include <obsidian/t_utils.h>
#include <obsidian/i_input.h>
#include <obsidian/u_ui.h>

#define NS_TARGET 16666666 // 1 / 60 seconds
//#define NS_TARGET 500000000
#define NS_PER_S  1000000000

void painter_Init(void)
{
    obdn_v_config.rayTraceEnabled = true;
#ifndef NDEBUG
    obdn_v_config.validationEnabled = true;
#else
    obdn_v_config.validationEnabled = false;
#endif
    parms.copySwapToHost = false;
    const VkImageLayout finalUILayout = parms.copySwapToHost ? VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    if (!parms.copySwapToHost)
        obdn_d_Init(1000, 1000, NULL);
    else
    {
        OBDN_WINDOW_WIDTH = 1000;
        OBDN_WINDOW_HEIGHT = 1000;
    }
    obdn_v_Init();
    if (!parms.copySwapToHost)
        obdn_v_InitSurfaceXcb(d_XcbWindow.connection, d_XcbWindow.window);
    obdn_r_Init(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, parms.copySwapToHost);
    obdn_i_Init();
    obdn_u_Init(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, finalUILayout);
    obdn_i_Subscribe(g_Responder);
    r_InitRenderer();
    g_Init();
}

void painter_LoadFprim(Obdn_F_Primitive* fprim)
{
    Obdn_R_Primitive prim = obdn_f_CreateRPrimFromFPrim(fprim);
    obdn_f_FreePrimitive(fprim);
    r_LoadPrim(prim);
}

void painter_ReloadPrim(Obdn_F_Primitive* fprim)
{
    vkDeviceWaitIdle(device);
    r_ClearPrim();

    painter_LoadFprim(fprim);
    painter_StartLoop();
}

void painter_StartLoop(void)
{
    Obdn_LoopData loopData = obdn_CreateLoopData(NS_TARGET, 0, 0);

    // initialize matrices
    Mat4* xformProj    = r_GetXform(R_XFORM_PROJ);
    Mat4* xformProjInv = r_GetXform(R_XFORM_PROJ_INV);
    *xformProj    = m_BuildPerspective(0.01, 30);
    *xformProjInv = m_Invert4x4(xformProj);

    parms.shouldRun = true;

    while( parms.shouldRun ) 
    {
        obdn_FrameStart(&loopData);

        if (!parms.copySwapToHost)
            obdn_d_DrainEventQueue();
        obdn_i_ProcessEvents();

        g_Update();
        r_Render();

        obdn_FrameEnd(&loopData);
    }

    if (parms.reload)
    {
        parms.reload = false;
        vkDeviceWaitIdle(device);

        r_ClearPrim();
        Obdn_R_Primitive cube = obdn_r_CreateCubePrim(true);
        r_LoadPrim(cube);
        printf("RELOAD!\n");

        painter_StartLoop();
    }
    else if (parms.restart)
    {
        parms.restart = false;
        vkDeviceWaitIdle(device);

        painter_ShutDown();
        painter_Init();
        Obdn_R_Primitive cube = obdn_r_CreateCubePrim(true);
        r_LoadPrim(cube);
        printf("RESTART!\n");

        painter_StartLoop();
    }
}

void painter_StopLoop(void)
{
    parms.shouldRun = false;
}

void painter_ShutDown(void)
{
    vkDeviceWaitIdle(device);
    r_CleanUp();
    obdn_i_CleanUp();
    obdn_r_CleanUp();
    obdn_d_CleanUp();
    obdn_u_CleanUp();
    obdn_v_CleanUp();
}

bool painter_ShouldRun(void)
{
    return gameState.shouldRun;
}

void painter_SetColor(const float r, const float g, const float b)
{
    g_SetColor(r, g, b);
}

void painter_SetRadius(const float r)
{
    g_SetRadius(r);
}
