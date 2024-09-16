#ifndef RAT_H
#define RAT_H

#include "RSprite.hpp"
#include "RTexture.hpp"
#include "RTimer.hpp"
#include <SDL_mixer.h>
#include <SDL_render.h>
#include <vector>

// you can do this ???
class REntity;

class RProjectile {
public:
  RProjectile(REntity* issuer, RTexture *projectileTexture);

  int GetPosX();
  int GetPosY();
  REntity *GetIssuer();

  void SetPos(int x, int y);
  void SetVel(int vx, int vy);

  void Move();

  void Render(SDL_Renderer *renderer);

private:
  int posX, posY;
  int velX, velY;

  RTexture *texture;
  REntity *issuer;
};

class REntity {
public:
  REntity(RSprite *bodySprite, RSprite *weaponSprite, Mix_Chunk *shootSound);

  bool IsAtEndOfPath();

  int GetPosX();
  int GetPosY();
  int GetProjectileDamage();
  int GetHealth();
  float GetFireRate();
  SDL_Rect *GetRect();

  static bool CheckCollision(SDL_Rect *a, SDL_Rect *b);
  static bool CheckCollision(SDL_Rect *a, int x, int y);

  void SetPos(float x, float y);
  void SetVel(float vx, float vy);
  void SetTarget(float x, float y);
  void SetProjectileSpeed(int speed);
  void SetFireRate(int rate);
  void AddToShootTimer(float amt);
  void SetPath(SDL_Point *path, int pathLength);
  void SetSpeed(int speed);

  void Damage(int amt);
  void Heal(int amt);

  void Move();
  void MoveAlongPath();

  void Shoot(RTexture *projectileTexture,
             std::vector<RProjectile *> &gRProjectiles, float dt);

  void RenderHealthBar(SDL_Renderer *renderer);
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
  int projectileDamage;

  SDL_Point *path;
  int pathLength;
  int nextPathPoint;

  // used for projectile motion, set by rendering
  RTimer shootTimer;
  float weaponAngle;

  // in seconds
  float fireRate;

  // health stuff
  int maxHealth;
  int health;
};

#endif
