#include "RSprite.hpp"

#include <SDL_rect.h>
#include <SDL_render.h>

RSprite::RSprite(RTexture *spriteSheet, SDL_Rect *spriteClips, int nFrames) {
  this->spriteSheet = spriteSheet;
  this->spriteClips = spriteClips;
  this->nFrames = nFrames;

  currentFrame = 0;
  fTimer = 0;
  fps = 4;
}

float RSprite::GetFrameTimer() { return this->fTimer; }

bool RSprite::GetMovedFrame() { return movedFrame; }

int RSprite::GetFPS() { return this->fps; }

int RSprite::GetWidth() { return spriteSheet->GetWidth(); }

int RSprite::GetHeight() { return spriteSheet->GetHeight(); }

void RSprite::SetFPS(int fps) {
  if (fps < 0) {
    printf("Could not set FPS! Out of bounds.\n");
    return;
  }

  this->fps = fps;
}

void RSprite::SetFrame(int f) {
  if (f < 0 || f >= nFrames) {
    printf("Could not set frame! Out of bounds.\n");
    return;
  }

  this->currentFrame = f;
}

bool RSprite::Render(SDL_Renderer *renderer, float dt, int x, int y,
                     double angle) {
  movedFrame = false;

  if (fps > 0) {
    fTimer += dt;

    if (fTimer > (1.0 / fps)) {
      movedFrame = true;

      if (currentFrame + 1 == nFrames) {
        currentFrame = 0;
      } else {
        currentFrame++;
      }

      fTimer = 0;
    }
  }

  // null rotates around center
  spriteSheet->Render(renderer, x - spriteSheet->GetWidth() / 2,
                      y - spriteSheet->GetHeight() / 2,
                      &spriteClips[currentFrame], angle, NULL, SDL_FLIP_NONE);

  return movedFrame;
}
