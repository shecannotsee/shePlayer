//
// Created by shecannotsee on 23-6-6.
//

#ifndef SHEPLAYER_TEST_M3_SDL2_SHOW_TEST_H_
#define SHEPLAYER_TEST_M3_SDL2_SHOW_TEST_H_

#include <SDL2/SDL.h>

namespace m3_sdl2_show_test {

int main() {
  // 初始化SDL
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf("SDL 初始化失败: %s\n", SDL_GetError());
    return 1;
  }

  // 创建窗口
  SDL_Window* window = SDL_CreateWindow("SDL2 窗口", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_SHOWN);
  if (window == NULL) {
    printf("窗口创建失败: %s\n", SDL_GetError());
    return 1;
  }

  // 创建渲染器
  SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (renderer == NULL) {
    printf("渲染器创建失败: %s\n", SDL_GetError());
    return 1;
  }

  // 清空渲染器
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);
  SDL_RenderPresent(renderer);

  // 主循环
  SDL_Event event;
  bool quit = false;
  while (!quit) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        quit = true;
      }
    }
  }

  // 清理资源
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;

};

};// namespace m3_sdl2_show_test

#endif //SHEPLAYER_TEST_M3_SDL2_SHOW_TEST_H_
