#include "REntity.hpp"

#include <SDL_mixer.h>
#include <SDL_render.h>
#include <SDL_stdinc.h>
#include <time.h>

const double PI = 3.14159265358979323846;

RProjectile::RProjectile(REntity *issuer, RTexture *projectileTexture) {
  posX = 0;
  posY = 0;
  velX = 0;
  velY = 0;

  texture = projectileTexture;

  this->issuer = issuer;
}

int RProjectile::GetPosX() { return posX; }

int RProjectile::GetPosY() { return posY; }

REntity *RProjectile::GetIssuer() { return issuer; }

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

REntity::REntity(EntityKind kind, RSprite *bodySprite, RSprite *weaponSprite,
                 Mix_Chunk *shootSound) {
  this->kind = kind;

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
  projectileDamage = 1;

  path = NULL;
  pathLength = -1;
  nextPathPoint = -1;

  weaponAngle = 0;
  fireRate = 2;

  // start shoot timer right away
  shootTimer.Start();

  // start at full health
  maxHealth = 100;
  health = maxHealth;
}

bool REntity::IsAtEndOfPath() { return nextPathPoint >= pathLength; }

int REntity::GetPosX() { return posX; }

int REntity::GetPosY() { return posY; }

int REntity::GetProjectileDamage() { return projectileDamage; }

int REntity::GetHealth() { return health; }

float REntity::GetFireRate() { return fireRate; }

SDL_Rect *REntity::GetRect() {
  // use body sprite draw rect for now
  // probably going to use something else as collider in the future
  return bodySprite->GetRect();
}

void REntity::SetPos(float x, float y) {
  posX = x;
  posY = y;
}

void REntity::SetVel(float vx, float vy) {
  velX = vx;
  velY = vy;
}

void REntity::SetTarget(float x, float y) {
  targetX = x;
  targetY = y;
}

void REntity::SetProjectileSpeed(int speed) { projectileSpeed = speed; }

void REntity::SetFireRate(int rate) { fireRate = rate; }

void REntity::AddToShootTimer(float amt) { shootTimer.AddOffset(amt); }

void REntity::SetPath(SDL_Point *path, int pathLength) {
  this->path = path;
  this->pathLength = pathLength;

  // target the next point in the path
  nextPathPoint = 0;
}

void REntity::SetSpeed(int speed) { this->speed = speed; }

void REntity::TakeDamage(int amt) {
  if (health - amt < 0) {
    health = 0;
  }

  else {
    health = health - amt;
  }
}

void REntity::Heal(int amt) {
  if (health + amt > maxHealth) {
    health = maxHealth;
  }

  else {
    health = health + amt;
  }
}

void REntity::MoveAlongPath() {
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

bool REntity::CheckCollision(SDL_Rect *a, SDL_Rect *b) {
  // sides of both rects
  int leftA, leftB;
  int rightA, rightB;
  int topA, topB;
  int bottomA, bottomB;

  // calculate sides of a
  leftA = a->x;
  rightA = a->x + a->w;
  topA = a->y;
  bottomA = a->y + a->h;

  // calculate sides of b
  leftB = b->x;
  rightB = b->x + b->w;
  topB = b->y;
  bottomB = b->y + b->h;

  // if any sides from a are outside b
  if (bottomA <= topB) {
    return false;
  }

  if (topA >= bottomB) {
    return false;
  }

  if (rightA <= leftB) {
    return false;
  }

  if (leftA >= rightB) {
    return false;
  }

  // if no sides of a are outside b
  return true;
}

bool REntity::CheckCollision(SDL_Rect *a, int x, int y) {
  // check if (x, y) is inside a
  int leftA = a->x;
  int rightA = a->x + a->w;
  int topA = a->y;
  int bottomA = a->y + a->h;

  if (x < leftA) {
    return false;
  }

  if (x > rightA) {
    return false;
  }

  if (y < topA) {
    return false;
  }

  if (y > bottomA) {
    return false;
  }

  return true;
}

void REntity::Shoot(RTexture *projectileTexture,
                    std::vector<RProjectile *> &gProjectiles, float dt) {

  // this is the shoot timer
  shootTimer.Tick(dt);

  // fire on set interval
  if (shootTimer.GetTime() > 1 / fireRate) {
    // don't forget to handle this dynamic mem!
    RProjectile *n = new RProjectile(this, projectileTexture);

    // calculate target using weapon angle
    n->SetVel((int)(SDL_cosf(weaponAngle) * projectileSpeed),
              (int)(SDL_sinf(weaponAngle) * projectileSpeed));

    n->SetPos(this->posX, this->posY);

    // add this projectile to registry
    gProjectiles.push_back(n);

    // play shoot sound
    Mix_PlayChannel(-1, shootSound, 0);

    shootTimer.Reset();
  }
}

void REntity::Render(SDL_Renderer *renderer, float dt) {
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

  // draw the healthbar
  RenderHealthBar(renderer);
}

void REntity::RenderHealthBar(SDL_Renderer *renderer) {
  SDL_Color frameColor;

  frameColor.r = 18;
  frameColor.g = 18;
  frameColor.b = 18;
  frameColor.a = 255;

  SDL_Color fillColor;

  fillColor.a = 255;
  
  switch (kind) {
  case TANK:
    fillColor.r = 255;
    fillColor.g = 0;
    fillColor.b = 0;
    break;
  case TOWER:
    fillColor.r = 0;
    fillColor.g = 0;
    fillColor.b = 255;
    break;
  default:
    fillColor.r = 255;
    fillColor.g = 255;
    fillColor.b = 255;
    break;
  }

  int barPad = 3;
  int yCenterOffset = (float)bodySprite->GetHeight() / 2 - 15;

  SDL_Rect frame;

  frame.w = 120;
  frame.h = 15;
  frame.x = posX - (float)frame.w / 2;
  frame.y = posY + yCenterOffset;

  SDL_Rect bar;

  int maxBarWidth = frame.w - 2 * barPad;
  int currBarWidth = maxBarWidth * ((float)health / maxHealth);

  bar.x = frame.x + barPad;
  bar.y = frame.y + barPad;
  bar.w = currBarWidth;
  bar.h = frame.h - 2 * barPad;

  SDL_SetRenderDrawColor(renderer, frameColor.r, frameColor.g, frameColor.b,
                         frameColor.a);

  SDL_RenderFillRect(renderer, &frame);

  SDL_SetRenderDrawColor(renderer, fillColor.r, fillColor.g, fillColor.b,
                         fillColor.a);

  SDL_RenderFillRect(renderer, &bar);

  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
}
