#include "RSprite.hpp"
#include "RTexture.hpp"
#include "Rat.hpp"

#include <SDL.h>
#include <SDL_error.h>
#include <SDL_events.h>
#include <SDL_image.h>
#include <SDL_keycode.h>
#include <SDL_mixer.h>
#include <SDL_render.h>
#include <SDL_stdinc.h>
#include <SDL_ttf.h>
#include <SDL_video.h>
#include <chrono>

const int SCREEN_TILE_WIDTH = 16;
const int SCREEN_TILE_HEIGHT = 16;
const int SCREEN_TILE_SCALE = 60;

const int SCREEN_WIDTH = SCREEN_TILE_WIDTH * SCREEN_TILE_SCALE; 
const int SCREEN_HEIGHT = SCREEN_TILE_HEIGHT * SCREEN_TILE_SCALE; 

SDL_Window *gWindow = NULL;
SDL_Surface *gWindowSurface = NULL;
SDL_Renderer *gRenderer = NULL;

std::chrono::time_point lastUpdateTime =
    std::chrono::high_resolution_clock::now();
float targetFps = 120;
float dt = 0;

RTexture tMap0;

RTexture tRat0;

SDL_Rect tRat0Clips[] = {{0, 0, 9, 9}, {0, 9, 9, 9}};

RSprite sRat0(&tRat0, tRat0Clips, 2);

Rat testRat(&sRat0);

// pathfollowing
const int MAP_0_PATH_LENGTH = 12;
SDL_Point map0Path[MAP_0_PATH_LENGTH];

void PrintError() { printf("%s\n", SDL_GetError()); }

void DrawPath(SDL_Renderer *renderer, SDL_Point *path, int pathLength) {
  // draw path nodes to ensure they're ok

  int hintSize = 16;
  SDL_Color hintColor = {255, 0, 0, 255};

  SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

  for (int i = 0; i < pathLength; ++i) {
    SDL_Rect point = {path[i].x - hintSize / 2, path[i].y - hintSize / 2,
                      hintSize, hintSize};
    SDL_RenderFillRect(renderer, &point);
  }

  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
}

bool Init() {
  bool success = true;

  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    PrintError();
    success = false;
  }

  gWindow = SDL_CreateWindow("Cats vs. Robo-Rats!", SDL_WINDOWPOS_CENTERED,
                             SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH,
                             SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

  if (!gWindow) {
    PrintError();
    success = false;
  }

  gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);

  if (gRenderer == NULL) {
    PrintError();
    success = false;
  }

  gWindowSurface = SDL_GetWindowSurface(gWindow);

  SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 0);

  int imageFlags = IMG_INIT_PNG;
  if (!(IMG_Init(imageFlags) & imageFlags)) {
    PrintError();
    success = false;
  }

  if (TTF_Init() == -1) {
    PrintError();
    success = false;
  }

  if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
    PrintError();
    success = false;
  }

  return success;
}

bool LoadMedia() {
  bool success = true;

  if (!tMap0.LoadFromFile(gRenderer, "../assets/map0.png")) {
    PrintError();
    success = false;
  }

  if (!tRat0.LoadFromFile(gRenderer, "../assets/rat0.png")) {
    PrintError();
    success = false;
  }

  tRat0.SetScale(10);

  // map points, in unscaled coords
  map0Path[0] = {10, 7};
  map0Path[1] = {10, 2};
  map0Path[2] = {6, 2};
  map0Path[3] = {6, 12};
  map0Path[4] = {2, 12};
  map0Path[5] = {2, 9};
  map0Path[6] = {12, 9};
  map0Path[7] = {12, 7};
  map0Path[8] = {14, 7};
  map0Path[9] = {14, 12};
  map0Path[10] = {10, 12};
  map0Path[11] = {10, 15};

  for (int i = 0; i < MAP_0_PATH_LENGTH; ++i) {
    int tileSizeX =
        (int)SDL_roundf((float)SCREEN_WIDTH / tMap0.GetWidthUnscaled());
    int tileSizeY =
        (int)SDL_roundf((float)SCREEN_HEIGHT / tMap0.GetHeightUnscaled());

    printf("%d\n", tileSizeX); 

    // scale up to match screen coords
    map0Path[i].x *= tileSizeX;
    map0Path[i].y *= tileSizeY;

    // center over tile size
    map0Path[i].x += (int)SDL_roundf((float)tileSizeX / 2);
    map0Path[i].y += (int)SDL_roundf((float)tileSizeY / 2);
  }

  // feed map to rat
  testRat.SetPath(map0Path, MAP_0_PATH_LENGTH);

  return success;
}

void Close() {
  tMap0.Free();

  SDL_DestroyRenderer(gRenderer);
  gRenderer = NULL;

  SDL_DestroyWindow(gWindow);
  gWindow = NULL;

  IMG_Quit();
  Mix_Quit();
  SDL_Quit();
}

int main() {
  if (!Init()) {
    return 1;
  }

  if (!LoadMedia()) {
    return 1;
  }

  // pre game loop code

  SDL_Event e;

  bool quit = false;
  while (!quit) {
    std::chrono::time_point currentTime =
        std::chrono::high_resolution_clock::now();

    dt = std::chrono::duration<float, std::chrono::seconds::period>(
             currentTime - lastUpdateTime)
             .count();

    // dt must meet or exceed duration of frame before flip
    if (dt < 1 / targetFps) {
      continue;
    }

    while (SDL_PollEvent(&e)) {
      // event handling
      if (e.type == SDL_QUIT) {
        quit = true;
      }

      else if (e.type == SDL_KEYDOWN) {
        if (e.key.keysym.sym == SDLK_ESCAPE) {
          quit = true;
        }
      }
    }

    // update code
    testRat.MoveAlongPath();

    SDL_RenderClear(gRenderer);

    // draw code
    tMap0.Render(gRenderer, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    testRat.Render(gRenderer, dt);

    DrawPath(gRenderer, map0Path, MAP_0_PATH_LENGTH);

    SDL_RenderPresent(gRenderer);

    // before flip
    lastUpdateTime = currentTime;
  }

  Close();

  return 1;
}
