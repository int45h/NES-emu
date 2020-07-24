#include "interface.h"

/*
    Interface.c: Audio and Video interface for the NES emulator
*/

#define DEFAULT_DRIVER -1; /* index of default display driver */

/* Display window */
typedef struct Display
{
    SDL_Window      * window;
    SDL_Renderer    * renderer;
    SDL_Surface     * surface;
    SDL_Texture     * texture;
    const char      * title;
}
Display;

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
    disp->renderer  = SDL_CreateRenderer(window, DEFAULT_DRIVER, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);

    /* Other stuff goes here */
}

/* Update display */
void update_display(Display * disp)
{

}

/* Destroy display object */
void free_display(Display * disp)
{
    SDL_DestroyTexture(disp->texture);
    SDL_DestroyRenderer(disp->renderer);
    SDL_FreeSurface(disp->surface);
    SDL_DestroyWindow(disp->window);
    SDL_Quit();
}