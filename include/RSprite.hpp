#ifndef RSPRITE_H
#define RSPRITE_H

#include "RTexture.hpp"
#include <SDL_rect.h>

class RSprite {
public:
  RSprite(RTexture *spriteSheet, SDL_Rect *spriteClips, int nFrames);

  int GetFPS();
  int GetWidth();
  int GetHeight();
  int GetWidthUnscaled();
  int GetHeightUnscaled();
  bool GetMovedFrame();
  float GetFrameTimer();
  SDL_Point GetCenter();

  void SetFPS(int fps);
  void SetFrame(int f);

  bool Render(SDL_Renderer *renderer, float dt, int x, int y,
              bool center = false);

private:
  RTexture *spriteSheet;
  SDL_Rect *spriteClips;

  int currentFrame;
  int nFrames;
  int fps;
  bool movedFrame;
  float fTimer;
};

#endif
