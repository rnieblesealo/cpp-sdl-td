#include "Enemy.hpp"

#include <SDL_render.h>
#include <SDL_stdinc.h>

const double PI = 3.14159265358979323846;

Projectile::Projectile(RTexture *projectileTexture) {
  posX = 0;
  posY = 0;
  velX = 0;
  velY = 0;

  texture = projectileTexture;
}

int Projectile::GetPosX(){
  return posX;
}

int Projectile::GetPosY(){
  return posY;
}

void Projectile::SetPos(int x, int y) {
  posX = x;
  posY = y;
}

void Projectile::SetVel(int vx, int vy) {
  velX = vx;
  velY = vy;
}

void Projectile::Move() {
  posX += velX;
  posY += velY;
}

void Projectile::Render(SDL_Renderer *renderer) {
  texture->Render(renderer, posX, posY, NULL, true);
}

Enemy::Enemy(RSprite *bodySprite, RSprite *weaponSprite) {
  this->bodySprite = bodySprite;
  this->weaponSprite = weaponSprite;

  collider = {0, 0, 0, 0};

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
}

SDL_Rect *Enemy::GetCollider() {
  if (collider.x + collider.y + collider.w + collider.h == 0) {
    printf("Warning: Getting zero-size collider!\n");
  }

  return &collider;
}

void Enemy::SetPos(float x, float y) {
  posX = x;
  posY = y;
}

void Enemy::SetVel(float vx, float vy) {
  velX = vx;
  velY = vy;
}

void Enemy::SetTarget(float x, float y) {
  targetX = x;
  targetY = y;
}

void Enemy::SetPath(SDL_Point *path, int pathLength) {
  this->path = path;
  this->pathLength = pathLength;

  // target the next point in the path
  nextPathPoint = 0;
}

void Enemy::SetSpeed(int speed) { this->speed = speed; }

void Enemy::MoveAlongPath() {
  if (path == NULL) {
    printf("No path defined!\n");
    return;
  }

  if (nextPathPoint >= pathLength) {
    printf("Reached end of path\n");

    SetVel(0, 0);

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

  Move();
}

void Enemy::Move() {
  posX += (velX * speed);
  posY += (velY * speed);
}

void Enemy::Shoot(RTexture *projectileTexture,
                  std::vector<Projectile *> &gProjectiles) {
  // don't forget to handle this dynamic mem!
  Projectile *n = new Projectile(projectileTexture);

  // calculate target using weapon angle
  n->SetVel((int)(SDL_cosf(weaponAngle) * projectileSpeed),
            (int)(SDL_sinf(weaponAngle) * projectileSpeed));

  n->SetPos(this->posX, this->posY);

  // add this projectile to registry
  gProjectiles.push_back(n);
}

void Enemy::Render(SDL_Renderer *renderer, float dt) {
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
