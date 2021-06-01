#ifndef __UI_H__
#define __UI_H__

#include <SDL2/SDL.h>

#include "chip8.h"

#define WINDOW_W 960
#define WINDOW_H 480

#define ui_CLEANUP(window, renderer, texture) do {  \
    SDL_DestroyWindow(window);                      \
    SDL_DestroyRenderer(renderer);                  \
    SDL_DestroyTexture(texture);                    \
    SDL_Quit();                                     \
} while (0)

int  ui_init(SDL_Window **window, SDL_Renderer **renderer, SDL_Texture **texture);
void ui_draw(chip8_t *c8, SDL_Renderer *renderer, SDL_Texture *texture);

#endif
