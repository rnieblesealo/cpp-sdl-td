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
#include <filesystem>
#include <algorithm>
#include <chrono>
#include <string>

// Make the window itself smaller; multiply final screen width by scale
// Now everything is too big
// The window depends on the size we give things, so make those bigger/smaller instead

const float GLOB_SCALE = 0.5;

const int TILE_WIDTH = 128 * GLOB_SCALE;
const int TILE_HEIGHT = 128 * GLOB_SCALE;

const int LEVEL_GRID_WIDTH = 12;
const int LEVEL_GRID_HEIGHT = 12;

const int LEVEL_WIDTH = TILE_WIDTH * LEVEL_GRID_WIDTH;
const int LEVEL_HEIGHT = TILE_HEIGHT * LEVEL_GRID_HEIGHT;

const int GUI_WIDTH = (TILE_WIDTH * 3) * GLOB_SCALE;

const int SCREEN_WIDTH = (LEVEL_WIDTH + GUI_WIDTH); 
const int SCREEN_HEIGHT = LEVEL_HEIGHT; 

const int FONT_SIZE = 8;

// Files

const std::filesystem::path PATH_ASSETS =
    std::filesystem::current_path().parent_path() / "assets";
const std::filesystem::path PATH_PNG = PATH_ASSETS / "png";
const std::filesystem::path PATH_WAV = PATH_ASSETS / "wav";
const std::filesystem::path PATH_FONT = PATH_ASSETS / "font";

// SDL

SDL_Window *gWindow = NULL;
SDL_Surface *gWindowSurface = NULL;
SDL_Renderer *gRenderer = NULL;
TTF_Font *gFont = NULL;

void PrintError() { printf("%s\n", SDL_GetError()); }

// Debugging/Util

std::string IntToPaddedText(int value, int width) {
  // c++ handles std::string on its own so no manual mallocing needs to happen
  // here; this function works!
  std::string n_str = std::to_string(value);
  n_str =
      std::string(width - std::min(width, (int)n_str.length()), '0') + n_str;

  n_str = "x" + n_str;

  return n_str;
}

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

// Game Time

auto lastUpdateTime = std::chrono::high_resolution_clock::now();
float targetFps = 120;
float dt = 0;

// Event Handling

int mouseX = 0;
int mouseY = 0;

bool leftClick = false;
bool mouseWithinLevel = false;

int crosshairSnapX;
int crosshairSnapY;

// GUI

RTexture tHeart;
RTexture tDefenderHealth;
RTexture tCrosshair;

RGraphic graphicRedTank;
RButton buttonRedTank(&graphicRedTank, NULL);

RGraphic graphicGreenTank;
RButton buttonBlueTank(&graphicGreenTank, NULL);

RGraphic graphicYellowTank;
RButton buttonYellowTank(&graphicYellowTank, NULL);

RVerticalLayoutGroup vlGroup;

std::string defenderHealthText;
int defenderHealthTextWidth = 3; // used to pad with 0's when not triple digit

// Music

Mix_Music *songAutoDaFe;

// SFX

Mix_Chunk *sfxShootEnemy;
Mix_Chunk *sfxHitEnemy;
Mix_Chunk *sfxShootTower;

// Gameplay

int defenderMaxHealth = 5;
int defenderHealth = defenderMaxHealth;

// Maps

RTexture tMap0;
const int MAP_0_PATH_LENGTH = 13;
SDL_Point map0Path[MAP_0_PATH_LENGTH];

void SetPoint(SDL_Point *path, int i, int x, int y) {
  // dangerous! can cause segfaults with this; there's no boundscheck
  path[i].x = x;
  path[i].y = y;
}

// Projectiles

std::vector<RProjectile *> gProjectiles;

RTexture tBallRed;
RTexture tBallBlue;

// Entities

void CheckProjectileCollisions(REntity *enemy,
                               std::vector<REntity *> &parentList) {
  REntity *self = enemy;

  // check for projectile collisions
  std::vector<RProjectile *>::iterator iter;
  for (iter = gProjectiles.begin(); iter != gProjectiles.end();) {
    RProjectile *projectile =
        gProjectiles.at(std::distance(gProjectiles.begin(), iter));
    REntity *projectileIssuer = projectile->GetIssuer();

    auto issuerIndex =
        std::find(parentList.begin(), parentList.end(), projectileIssuer);

    // if the issuer is another entity in the list this entity is in, it's
    // friendly fire
    bool issuerInParentList = (issuerIndex != parentList.end());

    if (projectileIssuer == NULL || projectileIssuer == self ||
        issuerInParentList) {
      iter++;
      continue;
    }

    int projectileX = projectile->GetPosX();
    int projectileY = projectile->GetPosY();

    // check if projectile pos is inside rect
    if (REntity::CheckCollision(self->GetRect(), projectileX, projectileY)) {
      self->TakeDamage(projectileIssuer->GetProjectileDamage());

      // TESTING
      // play enemy damage sound
      Mix_PlayChannel(0, sfxHitEnemy, 0);

      // if health reaches zero, die
      if (self->GetHealth() == 0) {

        // remove enemy from parent vector by value rather than index
        parentList.erase(
            std::remove(parentList.begin(), parentList.end(), self),
            parentList.end());
      }

      // erase colliding projectile
      iter = gProjectiles.erase(iter);
    }

    else {
      iter++;
    }
  }
}

// Enemies

std::vector<REntity *> gEnemies;

int amtRed = 999;
int amtGreen = 999;
int amtYellow = 999;

int enemyTargetX = 0;
int enemyTargetY = 0;

RTexture tEnemy;
RTexture tEnemyWeapon;

RTexture tEnemyGreen;
RTexture tEnemyWeaponGreen;

RTexture tEnemyYellow;
RTexture tEnemyWeaponYellow;

SDL_Rect cEnemy[] = {{0, 0, 128, 128}};
SDL_Rect cEnemyWeapon[] = {{0 * 128, 0, 128, 128}, {1 * 128, 0, 128, 128},
                           {2 * 128, 0, 128, 128}, {3 * 128, 0, 128, 128},
                           {4 * 128, 0, 128, 128}, {5 * 128, 0, 128, 128},
                           {6 * 128, 0, 128, 128}, {7 * 128, 0, 128, 128}};

RSprite sEnemy(&tEnemy, cEnemy, 1);
RSprite sEnemyWeapon(&tEnemyWeapon, cEnemyWeapon, 8);

void SpawnRedEnemy() {
  if (amtRed <= 0) {
    return;
  }

  REntity *newEnemy = new REntity(TANK, &sEnemy, &sEnemyWeapon, sfxShootEnemy);

  // give path, place at beginning
  newEnemy->SetPath(map0Path, MAP_0_PATH_LENGTH);
  newEnemy->SetPos(map0Path[0].x, map0Path[0].y);

  // set properties
  newEnemy->SetFireRate(8);
  newEnemy->SetSpeed(3);

  // add enemy to reg
  gEnemies.push_back(newEnemy);

  // now have one less enemy!
  amtRed--;
  graphicRedTank.SetText(gRenderer, gFont, IntToPaddedText(amtRed, 3).c_str());
}

// Towers

std::vector<REntity *> gTowers;

int towerTargetX = 0;
int towerTargetY = 0;

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

void SpawnTower(int gridX, int gridY) {
  if (gridX > LEVEL_GRID_WIDTH || gridX < 0 || gridY > LEVEL_GRID_HEIGHT ||
      gridY < 0) {
    return;
  }

  // amplify pos to px scale
  // center pos over tile too; sprites render centered
  gridX = gridX * TILE_WIDTH + TILE_WIDTH / 2;
  gridY = gridY * TILE_HEIGHT + TILE_HEIGHT / 2;

  REntity *newTower =

      new REntity(TOWER, &sTowerBase, &sTowerWeapon, sfxShootTower);

  // spawn tower in coords relative to grid
  newTower->SetPos(gridX, gridY);

  // add a small, random offset to the shoot timer
  newTower->AddToShootTimer((rand() % 100) / (float)100);

  newTower->SetFireRate(5);
  newTower->SetProjectileSpeed(14);

  gTowers.push_back(newTower);
}

// Initialization

bool Init() {
  bool success = true;

  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    PrintError();
    success = false;
  }

  // make window
  gWindow = SDL_CreateWindow("Dower Tefense", SDL_WINDOWPOS_CENTERED,
                             SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH,
                             SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
  if (!gWindow) {
    PrintError();
    success = false;
  }

  // make renderer
  gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
  if (gRenderer == NULL) {
    PrintError();
    success = false;
  }

  // store window surface for drawing
  gWindowSurface = SDL_GetWindowSurface(gWindow);

  // start image loader
  int imageFlags = IMG_INIT_PNG;
  if (!(IMG_Init(imageFlags) & imageFlags)) {
    PrintError();
    success = false;
  }

  // start font loader
  if (TTF_Init() == -1) {
    PrintError();
    success = false;
  }

  // start mixer w/2 channels
  // 0: sfx
  // 1: music
  if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
    PrintError();
    success = false;
  }

  // make sfx half as loud as music (trust me, your ears will love it)
  Mix_Volume(0, 32);
  Mix_Volume(1, MIX_MAX_VOLUME);

  // put mouse at window center
  SDL_WarpMouseInWindow(gWindow, LEVEL_WIDTH / 2, LEVEL_HEIGHT / 2);

  // because we need to poll the event handler to get new mouse position, set
  // the target to center manually
  enemyTargetX = LEVEL_WIDTH / 2;
  enemyTargetY = LEVEL_HEIGHT / 2;

  // scale screen
  // SDL_RenderSetLogicalSize(gRenderer, 1920, 1080);

  return success;
}

void MakeMapPaths() {
  // first set map points in unscaled coords (e.g. 10, 7 refers to 10 tiles x, 7
  // tiles y)
  // first int is the index of the point, second is the coords
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
    // scale up points to match level screen cords
    map0Path[i].x *= TILE_WIDTH;
    map0Path[i].y *= TILE_HEIGHT;

    // also make a tile's posistion its center; it's easier to deal with
    // positioning entities this way
    map0Path[i].x -= (int)SDL_roundf((float)TILE_WIDTH / 2);
    map0Path[i].y -= (int)SDL_roundf((float)TILE_HEIGHT / 2);
  }
}

void ConfigureGUI() {
  // Graphic Colors

  graphicRedTank.SetAreaColor(40, 40, 40);
  graphicGreenTank.SetAreaColor(40, 40, 40);
  graphicYellowTank.SetAreaColor(40, 40, 40);

  // Button Icons

  graphicRedTank.SetIcon(&tEnemy);
  graphicGreenTank.SetIcon(&tEnemyGreen);
  graphicYellowTank.SetIcon(&tEnemyYellow);

  // Button Text

  graphicRedTank.SetTextPadding(25);
  graphicRedTank.SetTextScale(8);
  graphicRedTank.SetText(gRenderer, gFont, IntToPaddedText(amtRed, 2).c_str());

  graphicGreenTank.SetTextPadding(25);
  graphicGreenTank.SetTextScale(8);
  graphicGreenTank.SetText(gRenderer, gFont,
                           IntToPaddedText(amtGreen, 2).c_str());

  graphicYellowTank.SetTextPadding(25);
  graphicYellowTank.SetTextScale(8);
  graphicYellowTank.SetText(gRenderer, gFont,
                            IntToPaddedText(amtYellow, 2).c_str());

  // Button Actions

  buttonRedTank.SetAction(&SpawnRedEnemy);
  buttonBlueTank.SetAction(NULL);
  buttonYellowTank.SetAction(NULL);

  // Layout Group
  // 1. Add elements
  // 2. Position & pad
  // 3. Apply (readies it for drawing)

  vlGroup.AddElement(&graphicRedTank);
  vlGroup.AddElement(&graphicGreenTank);
  vlGroup.AddElement(&graphicYellowTank);

  vlGroup.SetArea(LEVEL_WIDTH, tHeart.GetHeight(), GUI_WIDTH,
                  SCREEN_HEIGHT - tHeart.GetHeight());
  vlGroup.SetPadding(40, 40);

  vlGroup.Apply();
}

bool LoadMedia() {
  bool success = true;

  // Preliminary: Make texture global scale match ours 
  RTexture::SetGlobalScale(GLOB_SCALE);

  // Fonts

  gFont = TTF_OpenFont((PATH_FONT / "better-font.ttf").c_str(), FONT_SIZE);
  if (gFont == NULL) {
    PrintError();
    success = false;
  }

  // Maps

  if (!tMap0.LoadFromFile(gRenderer, (PATH_PNG / "map0.png").c_str())) {
    PrintError();
    success = false;
  }

  // Projectiles

  if (!tBallRed.LoadFromFile(gRenderer, (PATH_PNG / "ball.png").c_str())) {
    PrintError();
    success = false;
  }

  tBallRed.ModColor(255, 0, 0);
  tBallRed.SetScale(4);

  if (!tBallBlue.LoadFromFile(gRenderer, (PATH_PNG / "ball.png").c_str())) {
    PrintError();
    success = false;
  }

  tBallBlue.ModColor(0, 0, 255);
  tBallBlue.SetScale(4);

  // Towers

  if (!tTowerBase.LoadFromFile(
          gRenderer, (PATH_PNG / "b-tower-base.png").c_str(), 255, 255, 255)) {
    PrintError();
    success = false;
  }

  if (!tTowerWeapon.LoadFromFile(gRenderer,
                                 (PATH_PNG / "b-tower-weapon.png").c_str())) {
    PrintError();
    success = false;
  }

  // Enemies

  if (!tEnemy.LoadFromFile(gRenderer, (PATH_PNG / "r-tank-body.png").c_str(),
                           255, 255, 255)) {
    PrintError();
    success = false;
  }

  if (!tEnemyWeapon.LoadFromFile(
          gRenderer, (PATH_PNG / "r-tank-weapon.png").c_str(), 255, 255, 255)) {
    PrintError();
    success = false;
  }

  if (!tEnemyGreen.LoadFromFile(
          gRenderer, (PATH_PNG / "g-tank-body.png").c_str(), 255, 255, 255)) {
    PrintError();
    success = false;
  }

  if (!tEnemyWeaponGreen.LoadFromFile(
          gRenderer, (PATH_PNG / "g-tank-weapon.png").c_str(), 255, 255, 255)) {
    PrintError();
    success = false;
  }

  if (!tEnemyYellow.LoadFromFile(
          gRenderer, (PATH_PNG / "y-tank-body.png").c_str(), 255, 255, 255)) {
    PrintError();
    success = false;
  }

  if (!tEnemyWeaponYellow.LoadFromFile(
          gRenderer, (PATH_PNG / "y-tank-weapon.png").c_str(), 255, 255, 255)) {
    PrintError();
    success = false;
  }

  // GUI

  if (!tCrosshair.LoadFromFile(gRenderer,
                               (PATH_PNG / "crosshair.png").c_str())) {
    PrintError();
    success = false;
  }

  tCrosshair.SetScale(7);

  // TODO make the default arg for colorkey not do a colorkey in the first place
  // set bogus modcolor for now bc we use both white and red in the heart's
  // actual sprite
  if (!tHeart.LoadFromFile(gRenderer, (PATH_PNG / "heart.png").c_str(), 1, 1,
                           1)) {
    PrintError();
    success = false;
  }

  tHeart.SetScale(12);
  tHeart.ModColor(255, 0, 0);

  defenderHealthText = std::to_string(defenderHealth);
  defenderHealthText =
      std::string(defenderHealthTextWidth -
                      std::min(defenderHealthTextWidth,
                               (int)defenderHealthText.length()),
                  '0') +
      defenderHealthText;

  tDefenderHealth.LoadFromRenderedText(
      gRenderer, gFont, defenderHealthText.c_str(), 255, 255, 255);

  // Music

  songAutoDaFe = Mix_LoadMUS((PATH_WAV / "auto-da-fe.mp3").c_str());
  if (!songAutoDaFe) {
    PrintError();
    success = false;
  }

  // SFX

  sfxShootEnemy = Mix_LoadWAV((PATH_WAV / "shoot0.wav").c_str());
  if (!sfxShootEnemy) {
    PrintError();
    success = false;
  }

  sfxShootTower = Mix_LoadWAV((PATH_WAV / "shoot1.wav").c_str());
  if (!sfxShootEnemy) {
    PrintError();
    success = false;
  }

  sfxHitEnemy = Mix_LoadWAV((PATH_WAV / "hit0.wav").c_str());
  if (!sfxHitEnemy) {
    PrintError();
    success = false;
  }

  return success;
}

void Close() {
  // TODO finish this!
  // there is a lot we aren't freeing

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

// Game Flow

// WARNING: what happens if projectile/enemy is popped from the middle?
// forloop might be dangerous

void ClearOffBoundsProjectiles() {
  for (int i = 0; i < gProjectiles.size(); ++i) {
    RProjectile *projectile = gProjectiles[i];
    if (projectile->GetPosX() < 0 || projectile->GetPosX() > LEVEL_WIDTH ||
        projectile->GetPosY() < 0 || projectile->GetPosY() > LEVEL_HEIGHT) {

      // remove ith element; vector erases this for us
      gProjectiles.erase(gProjectiles.begin() + i);
    }
  }
}

void ClearFinishedEnemies() {
  // clear enemies that have cleared the path
  for (int i = 0; i < gEnemies.size(); ++i) {
    REntity *enemy = gEnemies[i];

    if (enemy->IsAtEndOfPath()) {
      // enemies that clear the path also do damage to defender
      // keep it at units so its easier :)
      defenderHealth--;

      // update health text
      tDefenderHealth.LoadFromRenderedText(
          gRenderer, gFont, IntToPaddedText(defenderHealth, 3).c_str(), 255,
          255, 255);

      // clear enemy
      gEnemies.erase(gEnemies.begin() + i);
    }
  }
}

void UpdateProjectiles() {
  // upd projectiles
  for (int i = 0; i < gProjectiles.size(); ++i) {
    RProjectile *projectile = gProjectiles.at(i);

    projectile->Move();
    projectile->Render(gRenderer);
  }
}

void UpdateEnemies() {
  for (int i = 0; i < gEnemies.size(); ++i) {
    REntity *enemy = gEnemies.at(i);

    CheckProjectileCollisions(enemy, gEnemies);

    enemy->MoveAlongPath();
    enemy->SetTarget(enemyTargetX, enemyTargetY);
    enemy->Shoot(&tBallRed, gProjectiles, dt);
    enemy->Render(gRenderer, dt);
  }
}

void UpdateTowers() {
  bool didSetSnap = false;

  for (int i = 0; i < gTowers.size(); ++i) {
    REntity *tower = gTowers.at(i);

    if (!didSetSnap &&
        REntity::CheckCollision(tower->GetRect(), mouseX, mouseY)) {
      crosshairSnapX = tower->GetPosX();
      crosshairSnapY = tower->GetPosY();

      didSetSnap = true;
    }

    CheckProjectileCollisions(tower, gTowers);

    REntity *targetEnemy;

    if (gEnemies.size() > 0) {
      targetEnemy = gEnemies.at(0);

      float towerRange = 1000;
      float targetDistance =
          REntity::Distance(targetEnemy->GetPosX(), targetEnemy->GetPosY(),
                            tower->GetPosX(), tower->GetPosY());

      if (targetDistance < towerRange) {
        tower->SetTarget(targetEnemy->GetPosX(), targetEnemy->GetPosY());
        tower->Shoot(&tBallBlue, gProjectiles, dt);
      }
    }

    tower->Render(gRenderer, dt);
  }

  // if no towers collide with the cursor, we should set the crosshair
  // snap points to be invalid
  if (!didSetSnap) {
    crosshairSnapX = -1;
    crosshairSnapY = -1;
  }
}

void DrawUI() {
  // pos calculations are a mess and were eyeballed
  // TODO improve that
  vlGroup.Render(gRenderer);

  int heartPosX = LEVEL_WIDTH + 30;
  int heartPosY = 20;

  int tDefHealthW = 100 * 2;
  int tDefHealthH = 100;

  tHeart.Render(gRenderer, heartPosX, heartPosY, NULL);
  tDefenderHealth.Render(gRenderer, heartPosX + tHeart.GetWidth() + 15,
                         heartPosY - 12, tDefHealthW, tDefHealthH);
}

int main() {
  // Initialization

  if (!Init()) {
    return 1;
  }

  if (!LoadMedia()) {
    return 1;
  }

  // seed the randomizer
  srand(time(NULL));

  MakeMapPaths();
  ConfigureGUI();

  // TODO remove; spawn some towers for testing
  int nTowers = 24;
  for (int i = 0; i < nTowers; ++i) {
    SpawnTower(rand() % LEVEL_GRID_WIDTH, rand() % LEVEL_GRID_HEIGHT);
  }

  Mix_PlayMusic(songAutoDaFe, -1);

  // Main Loop

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
      // always get mouse position
      SDL_GetMouseState(&mouseX, &mouseY);

      // Quit Commands

      if (e.type == SDL_QUIT) {
        quit = true;
      }

      else if (e.type == SDL_KEYDOWN) {
        if (e.key.keysym.sym == SDLK_ESCAPE) {
          quit = true;
        }
      }

      // Left Click State

      else if (e.type == SDL_MOUSEBUTTONDOWN) {
        if (e.button.button == SDL_BUTTON_LEFT) {
          leftClick = true;
        }
      }

      else if (e.type == SDL_MOUSEBUTTONUP) {
        if (e.button.button == SDL_BUTTON_LEFT) {
          leftClick = false;
        }
      }

      // Individual Event Handling

      buttonRedTank.HandleEvent(&e);
    }

    // move target if holding left click somewhere within the level
    mouseWithinLevel = mouseX > 0 && mouseX <= LEVEL_WIDTH && mouseY > 0 &&
                       mouseY <= LEVEL_HEIGHT;

    if (leftClick && mouseWithinLevel) {
      bool shouldSnap = crosshairSnapX >= 0 && crosshairSnapY >= 0;

      if (shouldSnap) {
        enemyTargetX = crosshairSnapX;
        enemyTargetY = crosshairSnapY;
      }

      else {
        enemyTargetX = mouseX;
        enemyTargetY = mouseY;
      }
    }

    ClearOffBoundsProjectiles();
    ClearFinishedEnemies();

    // Drawing

    SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);

    SDL_RenderClear(gRenderer);

    // render map
    tMap0.Render(gRenderer, 0, 0, LEVEL_WIDTH, LEVEL_HEIGHT);

    // render crosshair
    tCrosshair.Render(gRenderer, enemyTargetX, enemyTargetY, NULL, true);

    UpdateProjectiles();
    UpdateEnemies();
    UpdateTowers();

    DrawUI();

    SDL_RenderPresent(gRenderer);

    // Before Next Frame

    lastUpdateTime = currentTime;
  }

  Close();

  return 0;
}
