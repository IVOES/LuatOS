/* MIT License
 * 
 * Copyright (c) [2020] [Ryan Wendland]
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "luat_base.h"
#ifdef LUAT_USE_LVGL_SDL2
#include <stdio.h>
#include <assert.h>
#include "lvgl.h"
#include "lv_conf.h"
#include "lv_sdl_drv_display.h"

static lv_disp_buf_t disp_buf;
static lv_color_t *pixels;
static uint32_t *fb;
#ifdef NXDK
#include <hal/video.h>
void *framebuffer;
#else
#include <SDL2/SDL.h>
static lv_task_t *sdl_present_task;

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture *framebuffer = NULL;
#endif
static void sdl_fb_flush(lv_disp_drv_t *disp_drv,
                         const lv_area_t *area,
                         lv_color_t *color_p)
{
    //printf("disp flush %d %d %d %d\n", area->x1, area->y1, area->x2, area->y2);
    if(area->x2 < 0 || area->y2 < 0 ||
       area->x1 > disp_drv->hor_res - 1 || area->y1 > disp_drv->ver_res - 1) {
        lv_disp_flush_ready(disp_drv);
        return;
    }
    SDL_Rect r;
    r.x = area->x1;
    r.y = area->y1;
    r.w = area->x2 - area->x1 + 1;
    r.h = area->y2 - area->y1 + 1;

    uint32_t *tmp = fb;
    for (size_t i = 0; i < r.h; i++)
    {
        for (size_t j = 0; j < r.w; j++)
        {
            *tmp = lv_color_to32(*color_p);
            tmp ++;
            color_p ++;
        }
    }

    SDL_UpdateTexture(framebuffer, &r, fb, r.w * 4);
    lv_disp_flush_ready(disp_drv);
}

#ifndef NXDK
static void sdl_present(lv_task_t *task)
{
    if (renderer && framebuffer)
    {
        SDL_RenderCopy(renderer, framebuffer, NULL, NULL);
        SDL_RenderPresent(renderer);
    }
}
#endif

__attribute__((weak)) void display_wait_cb(lv_disp_drv_t *disp_drv)
{
    (void)disp_drv;
    //OPTIONAL: Called periodically while lvgl waits for an operation to be completed
    //          User can execute very simple tasks here or yield the task
}

lv_disp_t *lv_sdl_init_display(const char *win_name, int width, int height)
{
    //Setup the display buffer and driver
    if (width > LV_HOR_RES_MAX)
        width = LV_HOR_RES_MAX;
    if (height > LV_VER_RES_MAX)
        height = LV_VER_RES_MAX;
    size_t buff_line = 16;
    pixels = malloc(sizeof(lv_color_t) * width * buff_line);
    fb = malloc(sizeof(uint32_t) * width * buff_line);
    //printf("pixels %p\n", pixels);
    lv_disp_buf_init(&disp_buf, pixels, NULL, width * buff_line);
    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.buffer = &disp_buf;
    disp_drv.wait_cb = display_wait_cb;
    disp_drv.flush_cb = sdl_fb_flush;
    disp_drv.hor_res = width;
    disp_drv.ver_res = height;

    if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0)
        printf("SDL_InitSubSystem failed: %s\n", SDL_GetError());

    window = SDL_CreateWindow(win_name,
                              SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              width, height, 0);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    framebuffer = SDL_CreateTexture(renderer,
                                    SDL_PIXELFORMAT_ARGB8888,
                                    SDL_TEXTUREACCESS_STREAMING,
                                    width,
                                    height);

    sdl_present_task = lv_task_create(sdl_present, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_HIGHEST, NULL);

    return lv_disp_drv_register(&disp_drv);
}

void lv_sdl_deinit_display(void)
{
    #ifdef NXDK
    XVideoFlushFB();
    MmFreeContiguousMemory(framebuffer);
    #else
    lv_task_del(sdl_present_task);
    SDL_DestroyTexture(framebuffer);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
    #endif
}

#endif
