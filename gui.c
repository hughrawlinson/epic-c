#ifndef DRAW_GUI
int draw() { return 0; }
#else
#include "SDL.h"
#include "SDL_image.h"

SDL_Texture **prepare_images(SDL_Renderer *renderer, char **filenames,
                             size_t file_count) {
  SDL_Texture **earths;

  earths = malloc(file_count * sizeof(SDL_Texture *));

  for (size_t i = 0; i < file_count; i++) {
    SDL_Surface *surface = IMG_Load(filenames[i]);
    earths[i] = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
  }

  return earths;
}

int draw(char **filenames, size_t file_count) {
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

  SDL_Texture **earths = prepare_images(renderer, filenames, 1);

  while (1) {
    SDL_PollEvent(&event);
    if (event.type == SDL_QUIT) {
      break;
    }

    SDL_Rect rect;

    rect.x = 0;
    rect.y = 0;
    rect.w = 480;
    rect.h = 480;

    SDL_RenderClear(renderer);
    // SDL_SetRenderDrawColor(renderer, 0x00, 0x90, 0x00, 0x00);
    // SDL_RenderFillRect(renderer, &rect);
    // int result = SDL_RenderDrawRect(renderer, &rect);
    // if (result) {
    //   fprintf(stderr, "%s", SDL_GetError());
    // }

    SDL_RenderCopy(renderer, earths[0], NULL, &rect);

    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
    SDL_RenderPresent(renderer);
  }

  for (size_t i = 0; i < file_count; i++) {
    SDL_DestroyTexture(earths[i]);
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);

  SDL_Quit();

  return 0;
}

#endif