#pragma once

/*
    Interface.h: Audio and Video interface for the NES emulator
*/

#include <SDL2/SDL.h>
#include <errno.h>

/* Game display */
typedef struct Display
{
    SDL_Window      * window;
    SDL_Renderer    * renderer;
    SDL_Texture     * texture;
    SDL_Surface     * surface;
    SDL_PixelFormat * format;

    const char      * title;
    size_t          width, height, pitch;
    uint8_t         * framebuffer;
}
Display;

const int8_t DEFAULT_DRIVER = -1;

void init_display(Display*, const char*, size_t, size_t);
void create_display(Display*, const char*, size_t, size_t);

/* Start SDL, init the first display */
void init_display(Display * disp, const char * window_title, size_t width, size_t height)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        fprintf(stderr, "error: Failed to start display: %s. Exiting!\n", SDL_GetError());
        return;
    }

    create_display(disp, window_title, width, height);
}

/* Create a display object */
void create_display(Display * disp, const char * window_title, size_t width, size_t height)
{
    /* Init window and renderer of display object, render to texture using HW acceleration */
    disp->window    = SDL_CreateWindow(window_title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, 0);
    disp->renderer  = SDL_CreateRenderer(disp->window, DEFAULT_DRIVER, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);

    /* Store width and height of window */
    disp->width     = width; 
    disp->height    = height;

    /* Set surface, texture, colordepth */
    disp->surface   = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_ARGB8888);
    disp->texture   = SDL_CreateTextureFromSurface(disp->renderer, disp->surface);

    /* Set render target and draw color (black) */
    SDL_SetRenderTarget(disp->renderer, disp->texture);
    SDL_SetRenderDrawColor(disp->renderer, 0, 0, 0, 0xFF);

    /* Get pitch */
    disp->format    = SDL_AllocFormat(SDL_PIXELFORMAT_ARGB8888);
    disp->pitch     = disp->surface->pitch;

    /* Set framebuffer */
    disp->framebuffer = (uint8_t *) disp->surface->pixels;

    /* Clear screen */
    SDL_RenderClear(disp->renderer);
}

/* Write pixel to display */
void write_ARGB8888_pixel_to_display(Display * disp, size_t x, size_t y, uint32_t pixel)
{
    x %= disp->width;
    y %= disp->height;
    
    *((uint32_t*)&disp->framebuffer[(y * disp->pitch) + (x * sizeof(uint32_t))]) = pixel;
    disp->framebuffer = (uint8_t*)disp->surface->pixels;
}

/* Write pixel array to display */
void write_ARGB8888_arr_to_display(Display * disp, size_t x, size_t y, uint32_t * pixel_stream, size_t w, size_t h)
{
    x %= disp->width;
    y %= disp->height;

    for (size_t i = 0; i < h; i++)
        memcpy((void *)&disp->framebuffer[((y + i) * disp->pitch) + (x * sizeof(uint32_t))], (void *)&pixel_stream[i * w], w * sizeof(uint32_t));

}

/* Push any changes to display */
void push_to_display(Display * disp)
{
    disp->texture = SDL_CreateTextureFromSurface(disp->renderer, disp->surface);
    SDL_RenderCopy(disp->renderer, disp->texture, NULL, NULL);
}

/* Update display */
void update_display(Display * disp)
{
    SDL_RenderPresent(disp->renderer);
}

/* Destroy display object */
void free_display(Display * disp)
{
    disp->framebuffer = NULL;

    SDL_DestroyTexture(disp->texture);
    SDL_DestroyRenderer(disp->renderer);
    SDL_FreeSurface(disp->surface);
    SDL_FreeFormat(disp->format);
    SDL_DestroyWindow(disp->window);
}

/* Event handling loop */
void on_event(int * i)
{
    SDL_Event ev;
    while (SDL_PollEvent(&ev))
    {
        switch (ev.type)
        {
            case SDL_KEYDOWN:
                if ( ev.key.keysym.scancode == SDL_SCANCODE_ESCAPE )
                {
                    *i = -1;
                    return;
                }
                break;
            case SDL_QUIT:
                *i = -2;
                return;
        }
    }
}

/* Main loop (TO-DO: allow for real time texture modification) */
void main_loop(Display * disp)
{
    int exit_code = 0;
    while( exit_code == 0 )
    {
        on_event(&exit_code);
        update_display(disp);
    }
    free_display(disp);
    SDL_Quit();
}

/* A simple wait function (TO-DO: don't use SDL_Delay) */
void wait(uint32_t msec)
{
    SDL_Delay(msec);
}
