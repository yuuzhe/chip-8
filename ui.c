#include "ui.h"

int ui_init(SDL_Window **window, SDL_Renderer **renderer, SDL_Texture **texture)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        fprintf(stderr, "Failed to initialize SDL library\n");
        return -1;
    }

    *window = SDL_CreateWindow("CHIP-8 EMULATOR",
                               SDL_WINDOWPOS_UNDEFINED,
                               SDL_WINDOWPOS_UNDEFINED,
                               WINDOW_W,
                               WINDOW_H,
                               SDL_WINDOW_OPENGL);
    if (window == NULL) {
        fprintf(stderr, "Failed to initialize window\n");
        return -2;
    }

    *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        fprintf(stderr, "Failed to initialize renderer\n");
        return -3;
    }

    *texture = SDL_CreateTexture(*renderer, 
                                 SDL_PIXELFORMAT_RGBA8888,
                                 SDL_TEXTUREACCESS_TARGET, 
                                 WINDOW_W, 
                                 WINDOW_H);
    if (texture == NULL) {
        fprintf(stderr, "Failed to initialize texture\n");
        return -4;
    }

    SDL_SetRenderDrawColor(*renderer, 0, 0, 0, 255);
	SDL_RenderClear(*renderer);
    return 0;
}

void ui_draw(chip8_t *c8, SDL_Renderer *renderer, SDL_Texture *texture)
{
    size_t i, j;
    uint32_t pixels[WINDOW_H * WINDOW_W];

    for (i = 0; i < WINDOW_H; i++)
        for (j = 0; j < WINDOW_W; j++)
            pixels[i*WINDOW_W + j] = c8->graph[i/15*GRAPH_W + j/15] ? 
                                     0x00ff0000 : 0x000000ff;

    c8->draw_now = false;
    SDL_UpdateTexture(texture, NULL, pixels, WINDOW_W * sizeof(uint32_t));
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}
