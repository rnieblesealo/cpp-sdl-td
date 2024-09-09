#ifndef RAT_H
#define RAT_H

#include "RSprite.hpp"

class Enemy {
public:
  Enemy(RSprite *sprite);

  SDL_Rect *GetCollider();

  void SetPos(float x, float y);
  void SetVel(float vx, float vy);
  void SetPath(SDL_Point *path, int pathLength);
  void SetSpeed(int speed);

  void Move();
  void MoveAlongPath();
  void Render(SDL_Renderer *renderer, float dt);

private:
  RSprite *sprite;
  SDL_Rect collider;

  // these are stored as floats for calculation purposes
  // but should be rounded to ints for rendering
  float posX, posY;
  float velX, velY;
  int speed;

  SDL_Point *path;
  int pathLength;
  int nextPathPoint;
};

#endif
