#ifndef RAT_H
#define RAT_H

#include "RSprite.hpp"
#include "RTexture.hpp"
#include "RTimer.hpp"
#include <SDL_mixer.h>
#include <SDL_render.h>
#include <vector>

class RProjectile {
public:
  RProjectile(RTexture *projectileTexture);

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

class REnemy {
public:
  REnemy(RSprite *bodySprite, RSprite *weaponSprite, Mix_Chunk *shootSound);

  bool IsAtEndOfPath();

  int GetPosX();
  int GetPosY();

  void SetPos(float x, float y);
  void SetVel(float vx, float vy);
  void SetTarget(float x, float y);
  void SetProjectileSpeed(int speed);
  void SetFireRate(int rate);
  void SetPath(SDL_Point *path, int pathLength);
  void SetSpeed(int speed);

  void Move();
  void MoveAlongPath();

  void Shoot(RTexture *projectileTexture, std::vector<RProjectile *>& gRProjectiles, float dt);

  void Render(SDL_Renderer *renderer, float dt);

private:
  RSprite *bodySprite;
  RSprite *weaponSprite;
  Mix_Chunk *shootSound;

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
  RTimer shootTimer;
  float weaponAngle;
 
  // in seconds
  float fireRate;
};

#endif
