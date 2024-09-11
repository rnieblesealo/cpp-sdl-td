#include "REnemy.hpp"
#include "RGUI.hpp"
#include "RSprite.hpp"
#include "RTexture.hpp"
#include <SDL.h>
#include <SDL_error.h>
#include <SDL_events.h>
#include <SDL_image.h>
#include <SDL_keycode.h>
#include <SDL_mixer.h>
#include <SDL_mouse.h>
#include <SDL_render.h>
#include <SDL_stdinc.h>
#include <SDL_ttf.h>
#include <SDL_video.h>
#include <chrono>

// these are set from a 12*12 tile level, with tile size being 128*128px
const int LEVEL_WIDTH = 1536;
const int LEVEL_HEIGHT = 1536;

const int GUI_WIDTH = 128 * 3;

const int SCREEN_WIDTH = LEVEL_WIDTH + GUI_WIDTH;
const int SCREEN_HEIGHT = LEVEL_HEIGHT;

const int TILE_W = 128;
const int TILE_H = 128;

// this is independent
const int FONT_SIZE = 24;

SDL_Window *gWindow = NULL;
SDL_Surface *gWindowSurface = NULL;
SDL_Renderer *gRenderer = NULL;
TTF_Font *gFont = NULL;

std::chrono::time_point lastUpdateTime =
    std::chrono::high_resolution_clock::now();
float targetFps = 120;
float dt = 0;

RTexture tEnemy0;
RTexture tEnemy0Weapon;

SDL_Rect tEnemy0Clips[] = {{0, 0, 128, 128}};
SDL_Rect tEnemy0WeaponClips[] = {
    {0 * 128, 0, 128, 128}, {1 * 128, 0, 128, 128}, {2 * 128, 0, 128, 128},
    {3 * 128, 0, 128, 128}, {4 * 128, 0, 128, 128}, {5 * 128, 0, 128, 128},
    {6 * 128, 0, 128, 128}, {7 * 128, 0, 128, 128}};

RSprite sEnemy0(&tEnemy0, tEnemy0Clips, 1);
RSprite sEnemy0Weapon(&tEnemy0Weapon, tEnemy0WeaponClips, 8);

REnemy enemy0(&sEnemy0, &sEnemy0Weapon);

// maps
RTexture tMap0;
const int MAP_0_PATH_LENGTH = 13;
SDL_Point map0Path[MAP_0_PATH_LENGTH];

// other
int mouseX = 0;
int mouseY = 0;

// layout group test
RGraphic graphicA;
RGraphic graphicB;
RGraphic graphicC;

RVerticalLayoutGroup vlGroup;

// projectiles test
RTexture tBall;
std::vector<RProjectile *> gProjectiles;

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

  gWindow = SDL_CreateWindow("Dower Tefense", SDL_WINDOWPOS_CENTERED,
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

  gFont = TTF_OpenFont("../assets/font.ttf", FONT_SIZE);
  if (gFont == NULL) {
    PrintError();
    success = false;
  }

  if (!tMap0.LoadFromFile(gRenderer, "../assets/map0.png")) {
    PrintError();
    success = false;
  }

  // set map points, in unscaled coords (e.g. 10, 7 refers to 10 tiles x, 7
  // tiles y)
  map0Path[0] = {0, 5};
  map0Path[1] = {8, 5};
  map0Path[2] = {8, 2};
  map0Path[3] = {6, 2};
  map0Path[4] = {6, 10};
  map0Path[5] = {4, 10};
  map0Path[6] = {4, 7};
  map0Path[7] = {9, 7};
  map0Path[8] = {9, 6};
  map0Path[9] = {11, 6};
  map0Path[10] = {11, 10};
  map0Path[11] = {8, 10};
  map0Path[12] = {8, 12};

  for (int i = 0; i < MAP_0_PATH_LENGTH; ++i) {
    // scale up point coords to match screen coords
    map0Path[i].x *= TILE_W;
    map0Path[i].y *= TILE_H;

    // center over tile size
    map0Path[i].x -= (int)SDL_roundf((float)TILE_W / 2);
    map0Path[i].y -= (int)SDL_roundf((float)TILE_H / 2);
  }

  if (!tBall.LoadFromFile(gRenderer, "../assets/ball.png")) {
    PrintError();
    success = false;
  }

  tBall.ModColor(255, 0, 0);
  tBall.SetScale(4);

  if (!tEnemy0.LoadFromFile(gRenderer, "../assets/r-tank-body.png",
                            {255, 255, 255})) {
    PrintError();
    success = false;
  }

  if (!tEnemy0Weapon.LoadFromFile(gRenderer, "../assets/r-tank-turret1.png",
                                  {255, 255, 255})) {
    PrintError();
    success = false;
  }

  // feed map to enemy
  enemy0.SetPath(map0Path, MAP_0_PATH_LENGTH);

  // place enemy at start pos
  enemy0.SetPos(map0Path[0].x, map0Path[0].y);

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
  graphicA.SetText(gRenderer, gFont, "A.");
  graphicB.SetText(gRenderer, gFont, "B?");
  graphicC.SetText(gRenderer, gFont, "C!");

  graphicA.SetAreaColor({255, 0, 0, 255});
  graphicB.SetAreaColor({0, 255, 0, 255});
  graphicC.SetAreaColor({0, 0, 255, 255});

  vlGroup.AddElement(&graphicA);
  vlGroup.AddElement(&graphicB);
  vlGroup.AddElement(&graphicC);

  vlGroup.SetArea(LEVEL_WIDTH, 0, GUI_WIDTH, SCREEN_HEIGHT);
  vlGroup.SetPadding(60, 60);
  vlGroup.Apply();

  SDL_Rect *x = graphicA.GetArea();

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

      // fire projectile when click
      else if (e.type == SDL_MOUSEBUTTONDOWN) {
        if (e.button.button == SDL_BUTTON_LEFT) {
          enemy0.Shoot(&tBall, gProjectiles);
        }
      }

      // set mouse coords
      else if (e.type == SDL_MOUSEMOTION) {
        SDL_GetMouseState(&mouseX, &mouseY);
      }
    }

    // update code
    enemy0.MoveAlongPath();
    enemy0.SetTarget(mouseX, mouseY);

    // clear offbounds projectiles
    for (int i = 0; i < gProjectiles.size(); ++i) {
      if (gProjectiles[i]->GetPosX() < 0 ||
          gProjectiles[i]->GetPosX() > LEVEL_WIDTH ||
          gProjectiles[i]->GetPosY() < 0 ||
          gProjectiles[i]->GetPosY() > LEVEL_HEIGHT) {
        RProjectile *delProjectile = gProjectiles[i];

        // remove ith element
        gProjectiles.erase(gProjectiles.begin() + i);

        // free memory (does vector erase do this for us?)
        delete delProjectile;
      }
    }

    SDL_RenderClear(gRenderer);

    // draw code
    tMap0.Render(gRenderer, 0, 0, LEVEL_WIDTH, LEVEL_HEIGHT);
    enemy0.Render(gRenderer, dt);
    vlGroup.Render(gRenderer);

    // render all projectiles
    for (int i = 0; i < gProjectiles.size(); ++i) {
      gProjectiles[i]->Move();
      gProjectiles[i]->Render(gRenderer);
    }

    SDL_RenderPresent(gRenderer);

    // before flip
    lastUpdateTime = currentTime;
  }

  Close();

  return 1;
}
