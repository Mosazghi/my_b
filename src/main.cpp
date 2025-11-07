#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <assert.h>
#include <iostream>
#include <string>
#include "url.h"

static Logger* logger = new Logger("main");
int main(int argc, char* argv[]) {
  logger->inf("Initializing...");
  // if (argc < 2) {
  //   std::cout << "Usage: " << argv[0] << " <url>\n";
  //   return 1;
  // }
  //
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
    return 1;
  }
  if (!TTF_Init()) {
    logger->err("TTF_Init: %s", SDL_GetError());
  }

  SDL_Window* window =
      SDL_CreateWindow("My Browser", 800, 600, SDL_WINDOW_RESIZABLE);
  if (!window) {
    logger->err("Failed to create window");
    SDL_Quit();
    exit(EXIT_FAILURE);
  }

  SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
  if (!renderer) {
    std::cerr << "Failed to create renderer: " << SDL_GetError() << std::endl;
    SDL_DestroyWindow(window);
    SDL_Quit();
    exit(EXIT_FAILURE);
  }

  bool running = true;

  SDL_Event event;

  URL url("https://portfolio.mostes.no");
  auto response = url.request();
  TTF_Font* font =
      TTF_OpenFont("/home/mosa/Projects/my_b/src/Heebo-Regular.ttf", 32);

  if (!font) {
    std::cerr << "Failed to load font: " << SDL_GetError() << '\n';
    TTF_Quit();
    SDL_Quit();
    exit(EXIT_FAILURE);
  }

  // Create a text surface
  SDL_Color color = {255, 255, 255, 255};  // white
  SDL_Surface* textSurface =
      TTF_RenderText_Solid(font, "Hello, SDL3!", 13, color);

  if (!textSurface) {
    logger->err("Render text failed: %s", SDL_GetError());
    exit(EXIT_FAILURE);
  }

  SDL_Texture* textTexture =
      SDL_CreateTextureFromSurface(renderer, textSurface);

  // Get size for rendering
  float textW, textH;
  // SDL_GetSurfaceSize(textSurface, &textW, &textH);
  SDL_DestroySurface(textSurface);

  SDL_FRect destRect = {50, 50, 20.4, 20.4};
  // const char* text = "Hello!";

  SDL_Event e;
  while (running) {
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_EVENT_QUIT) running = false;
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 50, 255);
    SDL_RenderClear(renderer);
    SDL_RenderTexture(renderer, textTexture, nullptr, &destRect);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    // SDL_RenderDebugText(renderer, 0, 0, response.c_str());

    SDL_RenderPresent(renderer);
  }

  SDL_DestroyTexture(textTexture);
  TTF_CloseFont(font);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  TTF_Quit();
  SDL_Quit();

  // std::string url = argv[1];
  return 0;
}
