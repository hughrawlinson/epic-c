#ifndef DRAW_GUI
int draw() { return 0; }
#else
#include "SDL.h"

int draw() {
  SDL_Window *window;
  SDL_Renderer *renderer;
  // SDL_Surface *surface;
  SDL_Event event;

  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s",
                 SDL_GetError());
    return 3;
  }

  if (!(window = SDL_CreateWindow("Hi!", SDL_WINDOWPOS_UNDEFINED,
                                  SDL_WINDOWPOS_UNDEFINED, 480, 480,
                                  SDL_WINDOW_RESIZABLE))) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window: %s",
                 SDL_GetError());
    return 3;
  }

  if (!(renderer = SDL_CreateRenderer(window, -1,
                                      SDL_RENDERER_PRESENTVSYNC |
                                          SDL_RENDERER_ACCELERATED))) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create renderer: %s",
                 SDL_GetError());
    return 3;
  }

  while (1) {
    SDL_PollEvent(&event);
    if (event.type == SDL_QUIT) {
      break;
    }

    SDL_Rect rect;

    rect.x = 400;
    rect.y = 400;
    rect.w = 200;
    rect.h = 250;

    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 0x00, 0x90, 0x00, 0x00);
    SDL_RenderFillRect(renderer, &rect);
    int result = SDL_RenderDrawRect(renderer, &rect);
    if (result) {
      fprintf(stderr, "%s", SDL_GetError());
    }
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
    SDL_RenderPresent(renderer);
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);

  SDL_Quit();

  return 0;
}

#endif