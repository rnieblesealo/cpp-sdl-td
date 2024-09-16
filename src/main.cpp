#include "REntity.hpp"
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

const int TILE_WIDTH = 128;
const int TILE_HEIGHT = 128;

const int LEVEL_GRID_WIDTH = 12;
const int LEVEL_GRID_HEIGHT = 12;

const int LEVEL_WIDTH = TILE_WIDTH * LEVEL_GRID_WIDTH;
const int LEVEL_HEIGHT = TILE_HEIGHT * LEVEL_GRID_HEIGHT;

const int GUI_WIDTH = 128 * 3;

const int SCREEN_WIDTH = LEVEL_WIDTH + GUI_WIDTH;
const int SCREEN_HEIGHT = LEVEL_HEIGHT;

const int FONT_SIZE = 24;

// sdl/general
SDL_Window *gWindow = NULL;
SDL_Surface *gWindowSurface = NULL;
SDL_Renderer *gRenderer = NULL;
TTF_Font *gFont = NULL;

auto lastUpdateTime = std::chrono::high_resolution_clock::now();

float targetFps = 120;
float dt = 0;

// sfx/sound
Mix_Chunk *sfxShootEnemy;
Mix_Chunk *sfxShootTower; 

// map
RTexture tMap0;
const int MAP_0_PATH_LENGTH = 13;
SDL_Point map0Path[MAP_0_PATH_LENGTH];

void SetPoint(SDL_Point *path, int i, int x, int y) {
  // dangerous! can cause segfaults with this; there's no boundscheck
  path[i].x = x;
  path[i].y = y;
}

// enemies
RTexture tEnemy;
RTexture tEnemyWeapon;

SDL_Rect cEnemy[] = {{0, 0, 128, 128}};
SDL_Rect cEnemyWeapon[] = {{0 * 128, 0, 128, 128}, {1 * 128, 0, 128, 128},
                           {2 * 128, 0, 128, 128}, {3 * 128, 0, 128, 128},
                           {4 * 128, 0, 128, 128}, {5 * 128, 0, 128, 128},
                           {6 * 128, 0, 128, 128}, {7 * 128, 0, 128, 128}};

RSprite sEnemy(&tEnemy, cEnemy, 1);
RSprite sEnemyWeapon(&tEnemyWeapon, cEnemyWeapon, 8);

std::vector<REntity *> gEnemies;

int enemyTargetX = 0;
int enemyTargetY = 0;

void SpawnEnemy() {
  REntity *newEnemy = new REntity(&sEnemy, &sEnemyWeapon, sfxShootEnemy);

  // make enemy follow path
  newEnemy->SetPath(map0Path, MAP_0_PATH_LENGTH);
  newEnemy->SetPos(map0Path[0].x, map0Path[0].y);

  // add enemy to reg
  gEnemies.push_back(newEnemy);
}

// towers
RTexture tTowerBase;
RTexture tTowerWeapon;

SDL_Rect cTowerBase[] = {{0, 0, 128, 128}};
SDL_Rect cTowerWeapon[] = {
    {0 * 128, 0, 128, 128}, {1 * 128, 0, 128, 128}, {2 * 128, 0, 128, 128},
    {3 * 128, 0, 128, 128}, {4 * 128, 0, 128, 128}, {5 * 128, 0, 128, 128},
    {6 * 128, 0, 128, 128}, {7 * 128, 0, 128, 128}, {8 * 128, 0, 128, 128},
    {9 * 128, 0, 128, 128}, {10 * 128, 0, 128, 128}};

RSprite sTowerBase(&tTowerBase, cTowerBase, 1);
RSprite sTowerWeapon(&tTowerWeapon, cTowerWeapon, 11);

std::vector<REntity *> gTowers;

int towerTargetX = 0;
int towerTargetY = 0;

void SpawnTower(int gridX, int gridY) {
  if (gridX > LEVEL_GRID_WIDTH || gridX < 0 || gridY > LEVEL_GRID_HEIGHT ||
      gridY < 0) {
    printf("Invalid tower spawnpos\n");
    return;
  }

  // amplify pos to px scale
  // center pos over tile too; sprites render centered
  gridX = gridX * TILE_WIDTH + TILE_WIDTH / 2;
  gridY = gridY * TILE_HEIGHT + TILE_HEIGHT / 2;

  REntity *newTower = new REntity(&sTowerBase, &sTowerWeapon, sfxShootTower);

  // spawn tower in coords relative to grid
  newTower->SetPos(gridX, gridY);

  // add a small, random offset to the shoot timer
  newTower->AddToShootTimer((rand() % 100) / (float)100);

  gTowers.push_back(newTower);
}

// projectiles
// TODO: we are modcoloring the same texture, implement correctly
RTexture tBallRed;
RTexture tBallBlue;
std::vector<RProjectile *> gProjectiles;

// event handling
int mouseX = 0;
int mouseY = 0;

// gui
RTexture tCrosshair;

RGraphic graphicA;
RButton buttonA(&graphicA, &SpawnEnemy);

RVerticalLayoutGroup vlGroup;

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
  SetPoint(map0Path, 0, 0, 5);
  SetPoint(map0Path, 1, 8, 5);
  SetPoint(map0Path, 2, 8, 2);
  SetPoint(map0Path, 3, 6, 2);
  SetPoint(map0Path, 4, 6, 10);
  SetPoint(map0Path, 5, 4, 10);
  SetPoint(map0Path, 6, 4, 7);
  SetPoint(map0Path, 7, 9, 7);
  SetPoint(map0Path, 8, 9, 6);
  SetPoint(map0Path, 9, 11, 6);
  SetPoint(map0Path, 10, 11, 10);
  SetPoint(map0Path, 11, 8, 10);
  SetPoint(map0Path, 12, 8, 12);

  for (int i = 0; i < MAP_0_PATH_LENGTH; ++i) {
    // scale up point coords to match screen coords
    map0Path[i].x *= TILE_WIDTH;
    map0Path[i].y *= TILE_HEIGHT;

    // center over tile size
    map0Path[i].x -= (int)SDL_roundf((float)TILE_WIDTH / 2);
    map0Path[i].y -= (int)SDL_roundf((float)TILE_HEIGHT / 2);
  }

  if (!tBallRed.LoadFromFile(gRenderer, "../assets/ball.png")) {
    PrintError();
    success = false;
  }

  tBallRed.ModColor(255, 0, 0);
  tBallRed.SetScale(4);

  if (!tBallBlue.LoadFromFile(gRenderer, "../assets/ball.png")) {
    PrintError();
    success = false;
  }

  tBallBlue.ModColor(0, 0, 255);
  tBallBlue.SetScale(4);

  if (!tTowerBase.LoadFromFile(gRenderer, "../assets/b-tower-base.png", 255,
                               255, 255)) {
    PrintError();
    success = false;
  }

  if (!tTowerWeapon.LoadFromFile(gRenderer, "../assets/b-tower-weapon.png")) {
    PrintError();
    success = false;
  }

  if (!tEnemy.LoadFromFile(gRenderer, "../assets/r-tank-body.png", 255, 255,
                           255)) {
    PrintError();
    success = false;
  }

  if (!tEnemyWeapon.LoadFromFile(gRenderer, "../assets/r-tank-turret1.png", 255,
                                 255, 255)) {
    PrintError();
    success = false;
  }

  if (!tCrosshair.LoadFromFile(gRenderer, "../assets/crosshair.png")) {
    PrintError();
    success = false;
  }

  tCrosshair.SetScale(7);

  sfxShootEnemy = Mix_LoadWAV("../assets/shoot.wav");
  if (!sfxShootEnemy) {
    PrintError();
    success = false;
  }

  sfxShootTower = Mix_LoadWAV("../assets/shoot1.wav");
  if (!sfxShootEnemy){
    PrintError();
    success = false;
  }

  return success;
}

void Close() {
  tMap0.Free();
  tEnemy.Free();
  tEnemyWeapon.Free();
  tBallRed.Free();

  Mix_FreeChunk(sfxShootEnemy);

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

  // setup button graphics
  graphicA.SetAreaColor(255, 0, 0);

  // setup button

  // setup layout group
  vlGroup.AddElement(&graphicA);

  vlGroup.SetArea(LEVEL_WIDTH, 0, GUI_WIDTH, SCREEN_HEIGHT);
  vlGroup.SetPadding(60, 60);
  vlGroup.Apply();

  // TESTING
  // spawn towers
  srand(time(NULL));

  int nTowers = 12;

  for (int i = 0; i < nTowers; ++i) {
    SpawnTower(rand() % LEVEL_GRID_WIDTH, rand() % LEVEL_GRID_HEIGHT);
  }

  SDL_Event e;

  bool quit = false;
  while (!quit) {
    auto currentTime = std::chrono::high_resolution_clock::now();

    dt = std::chrono::duration<float, std::chrono::seconds::period>(
             currentTime - lastUpdateTime)
             .count();

    // dt must meet or exceed duration of frame before flip
    if (dt < 1 / targetFps) {
      continue;
    }

    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT) {
        quit = true;
      }

      else if (e.type == SDL_KEYDOWN) {
        if (e.key.keysym.sym == SDLK_ESCAPE) {
          quit = true;
        }
      }

      // set target mouse coords; will be changed
      else if (e.type == SDL_MOUSEBUTTONDOWN) {
        if (e.button.button == SDL_BUTTON_LEFT) {
          SDL_GetMouseState(&enemyTargetX, &enemyTargetY);
        }
      }

      buttonA.HandleEvent(&e);
    }

    // WARNING: what happens if projectile/enemy is popped from the middle?
    // forloop might be dangerous

    // clear offbounds projectiles
    for (int i = 0; i < gProjectiles.size(); ++i) {
      RProjectile *projectile = gProjectiles[i];
      if (projectile->GetPosX() < 0 || projectile->GetPosX() > LEVEL_WIDTH ||
          projectile->GetPosY() < 0 || projectile->GetPosY() > LEVEL_HEIGHT) {

        // remove ith element
        gProjectiles.erase(gProjectiles.begin() + i);

        // free memory (does vector erase do this for us?)
        delete projectile;
      }
    }

    // clear offbounds enemies
    for (int i = 0; i < gEnemies.size(); ++i) {
      REntity *enemy = gEnemies[i];

      if (enemy->IsAtEndOfPath()) {
        gEnemies.erase(gEnemies.begin() + i);

        delete enemy;
      }
    }

    // drawing begins here
    SDL_RenderClear(gRenderer);

    // render map
    tMap0.Render(gRenderer, 0, 0, LEVEL_WIDTH, LEVEL_HEIGHT);

    // upd enemies
    for (int i = 0; i < gEnemies.size(); ++i) {
      SDL_SetRenderDrawColor(gRenderer, 255, 0, 0, 255);
      SDL_RenderDrawRect(gRenderer, gEnemies[i]->GetRect()); 
      SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);


      gEnemies[i]->MoveAlongPath();
      gEnemies[i]->SetTarget(enemyTargetX, enemyTargetY);

      // this is kinda misleading; it seems like a call-once but it needs to run
      // on update
      gEnemies[i]->Shoot(&tBallRed, gProjectiles, dt);

      gEnemies[i]->Render(gRenderer, dt);
    }

    // TESTING
    // make towers shoot at first enemy
    if (gEnemies.size() > 0) {
      towerTargetX = gEnemies[0]->GetPosX();
      towerTargetY = gEnemies[0]->GetPosY();
    }

    // render crosshair
    tCrosshair.Render(gRenderer, enemyTargetX, enemyTargetY, NULL, true);

    // upd tower
    for (int i = 0; i < gTowers.size(); ++i) {
      SDL_SetRenderDrawColor(gRenderer, 0, 0, 255, 255);
      SDL_RenderDrawRect(gRenderer, gTowers[i]->GetRect());
      SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);

      gTowers[i]->SetTarget(towerTargetX, towerTargetY);
      gTowers[i]->Shoot(&tBallBlue, gProjectiles, dt);
      gTowers[i]->Render(gRenderer, dt);
    }

    // upd projectiles
    for (int i = 0; i < gProjectiles.size(); ++i) {
      gProjectiles[i]->Move();
      gProjectiles[i]->Render(gRenderer);
    }

    // render ui
    vlGroup.Render(gRenderer);

    // debug drawing
    // DrawPath(gRenderer, map0Path, MAP_0_PATH_LENGTH);

    SDL_RenderPresent(gRenderer);

    // before flip
    lastUpdateTime = currentTime;
  }

  Close();

  return 1;
}
