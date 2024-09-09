#include "Enemy.hpp"

#include <SDL_stdinc.h>

Enemy::Enemy(RSprite *sprite) {
  this->sprite = sprite;

  collider = {0, 0, 0, 0};

  posX = 0;
  posY = 0;
  velX = 0;
  velY = 0;
  speed = 2;

  path = NULL;
  pathLength = -1;
  nextPathPoint = -1;
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

void Enemy::Render(SDL_Renderer *renderer, float dt) {
  // round float pos to integer coords before rendering
  int rPosX = (int)SDL_roundf(posX);
  int rPosY = (int)SDL_roundf(posY);

  sprite->Render(renderer, dt, rPosX, rPosY, true);
}
