#ifndef RAT_H
#define RAT_H

#include "RSprite.hpp"
#include "RTexture.hpp"
#include <SDL_render.h>
#include <vector>

class Projectile {
public:
  Projectile(RTexture *projectileTexture);

  int GetPosX();
  int GetPosY();

  void SetPos(int x, int y);
  void SetVel(int vx, int vy);

  void Move();

  void Render(SDL_Renderer *renderer);

private:
  int posX, posY;
  int velX, velY;

  RTexture *texture;
};

class Enemy {
public:
  Enemy(RSprite *bodySprite, RSprite *weaponSprite);

  SDL_Rect *GetCollider();

  void SetPos(float x, float y);
  void SetVel(float vx, float vy);
  void SetTarget(float x, float y);
  void SetPath(SDL_Point *path, int pathLength);
  void SetSpeed(int speed);

  void Move();
  void MoveAlongPath();

  void Shoot(RTexture *projectileTexture, std::vector<Projectile *>& gProjectiles);

  void Render(SDL_Renderer *renderer, float dt);

private:
  RSprite *bodySprite;
  RSprite *weaponSprite;
  SDL_Rect collider;

  // these are stored as floats for calculation purposes
  // but should be rounded to ints for rendering
  float posX, posY;
  float velX, velY;
  float targetX, targetY;
  
  int speed;
  int projectileSpeed;

  SDL_Point *path;
  int pathLength;
  int nextPathPoint;

  // used for projectile motion, set by rendering
  float weaponAngle;
};

#endif
