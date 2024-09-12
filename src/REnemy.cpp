#include "REnemy.hpp"

#include <SDL_mixer.h>
#include <SDL_render.h>
#include <SDL_stdinc.h>

const double PI = 3.14159265358979323846;

RProjectile::RProjectile(RTexture *projectileTexture) {
  posX = 0;
  posY = 0;
  velX = 0;
  velY = 0;

  texture = projectileTexture;
}

int RProjectile::GetPosX() { return posX; }

int RProjectile::GetPosY() { return posY; }

void RProjectile::SetPos(int x, int y) {
  posX = x;
  posY = y;
}

void RProjectile::SetVel(int vx, int vy) {
  velX = vx;
  velY = vy;
}

void RProjectile::Move() {
  posX += velX;
  posY += velY;
}

void RProjectile::Render(SDL_Renderer *renderer) {
  texture->Render(renderer, posX, posY, NULL, true);
}

REnemy::REnemy(RSprite *bodySprite, RSprite *weaponSprite, Mix_Chunk *shootSound) {
  this->bodySprite = bodySprite;
  this->weaponSprite = weaponSprite;
  this->shootSound = shootSound;

  posX = 0;
  posY = 0;

  velX = 0;
  velY = 0;

  targetX = -1;
  targetY = -1;

  speed = 2;
  projectileSpeed = 20;

  path = NULL;
  pathLength = -1;
  nextPathPoint = -1;

  weaponAngle = 0;
  fireRate = 2;

  // start shoot timer right away
  shootTimer.Start();
}

bool REnemy::IsAtEndOfPath() { return nextPathPoint >= pathLength; }

int REnemy::GetPosX(){
  return posX;
}

int REnemy::GetPosY(){
  return posY;
}

void REnemy::SetPos(float x, float y) {
  posX = x;
  posY = y;
}

void REnemy::SetVel(float vx, float vy) {
  velX = vx;
  velY = vy;
}

void REnemy::SetTarget(float x, float y) {
  targetX = x;
  targetY = y;
}

void REnemy::SetProjectileSpeed(int speed){
  projectileSpeed = speed;
}

void REnemy::SetFireRate(int rate){
  fireRate = rate;
}

void REnemy::SetPath(SDL_Point *path, int pathLength) {
  this->path = path;
  this->pathLength = pathLength;

  // target the next point in the path
  nextPathPoint = 0;
}

void REnemy::SetSpeed(int speed) { this->speed = speed; }

void REnemy::MoveAlongPath() {
  if (path == NULL || IsAtEndOfPath()) {
    printf("No path defined!\n");
    return;
  }

  float dx = path[nextPathPoint].x - posX;
  float dy = path[nextPathPoint].y - posY;
  float d = SDL_sqrtf(SDL_powf(dx, 2) + SDL_powf(dy, 2));

  // if distance is very small, snap pos to node and target next node
  // that 5 is picked arbitrarily
  if (d < 5) {
    SetPos(path[nextPathPoint].x, path[nextPathPoint].y);

    nextPathPoint++;

    return;
  }

  SetVel(dx / d, dy / d);

  posX += (velX * speed);
  posY += (velY * speed);
}

void REnemy::Shoot(RTexture *projectileTexture,
                   std::vector<RProjectile *> &gRProjectiles, float dt) {

  // this is the shoot timer
  shootTimer.Tick(dt);

  // fire on set interval
  if (shootTimer.GetTime() > 1 / fireRate) {
    // don't forget to handle this dynamic mem!
    RProjectile *n = new RProjectile(projectileTexture);

    // calculate target using weapon angle
    n->SetVel((int)(SDL_cosf(weaponAngle) * projectileSpeed),
              (int)(SDL_sinf(weaponAngle) * projectileSpeed));

    n->SetPos(this->posX, this->posY);

    // add this projectile to registry
    gRProjectiles.push_back(n);

    // play shoot sound
    Mix_PlayChannel(-1, shootSound, 0);

    shootTimer.Reset();
  }
}

void REnemy::Render(SDL_Renderer *renderer, float dt) {
  // round float pos to integer coords before rendering
  int rPosX = (int)SDL_roundf(posX);
  int rPosY = (int)SDL_roundf(posY);

  // point weapon to target if latter is ok (coords must be positive)
  if (targetX >= 0 && targetY >= 0) {
    double dx = targetX - posX;
    double dy = targetY - posY;

    // keep in radians, convert to deg when needed
    weaponAngle = SDL_atan2(dy, dx);
  }

  bodySprite->Render(renderer, dt, rPosX, rPosY, 0);

  // 90 accounts for initial rotation
  weaponSprite->Render(renderer, dt, rPosX, rPosY,
                       weaponAngle * (180 / PI) + 90);
}
