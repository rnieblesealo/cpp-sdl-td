#include "RTexture.hpp"

#include <SDL_image.h>
#include <SDL_render.h>

RTexture::RTexture() {
  texture = NULL;
  width = 0;
  height = 0;
  scale = 1;
  renderDest.x = 0;
  renderDest.y = 0;
  renderDest.w = 0;
  renderDest.h = 0;

  // set global scale if instantiating new texture
  if (RTexture::globalScale <= 0) {
    RTexture::globalScale = 1;
  }
}

float RTexture::globalScale;

void RTexture::SetGlobalScale(float scale) { RTexture::globalScale = scale; }

RTexture::~RTexture() { Free(); }

#if defined(SDL_TTF_MAJOR_VERSION)

bool RTexture::LoadFromRenderedText(SDL_Renderer *renderer, TTF_Font *font,
                                    const char *text, Uint8 r, Uint8 g,
                                    Uint8 b) {

  Free();

  SDL_Color tSurfColor = {r, g, b, 255};
  SDL_Surface *tSurf = TTF_RenderText_Solid(font, text, tSurfColor);
  if (tSurf == NULL) {
    printf("Unable to render text: %s\n", SDL_GetError());
    return false;
  }

  texture = SDL_CreateTextureFromSurface(renderer, tSurf);
  if (tSurf == NULL) {
    printf("Unable to make texture from text: %s\n", SDL_GetError());
    return false;
  }

  width = tSurf->w;
  height = tSurf->h;

  SDL_FreeSurface(tSurf);

  return true;
}

#endif

bool RTexture::LoadFromFile(SDL_Renderer *renderer, const char *path, Uint8 r,
                            Uint8 g, Uint8 b) {

  Free();

  SDL_Surface *lSurf = IMG_Load(path);

  if (lSurf == NULL) {
    printf("Unable to load image: %s\n", SDL_GetError());
    return false;
  }

  SDL_SetColorKey(lSurf, SDL_TRUE, SDL_MapRGB(lSurf->format, r, g, b));

  SDL_Texture *nTexture = SDL_CreateTextureFromSurface(renderer, lSurf);

  if (nTexture == NULL) {
    printf("Could not create texture: %s\n", SDL_GetError());
    return false;
  }

  width = lSurf->w;
  height = lSurf->h;

  SDL_FreeSurface(lSurf);

  texture = nTexture;

  return true;
}

void RTexture::Free() {
  if (texture != NULL) {
    SDL_DestroyTexture(texture);
    texture = NULL;
    width = 0;
    height = 0;
  }
}

void RTexture::SetBlendMode(SDL_BlendMode blendMode) {

  SDL_SetTextureBlendMode(texture, blendMode);
}

void RTexture::ModColor(Uint8 r, Uint8 g, Uint8 b) {
  SDL_SetTextureColorMod(texture, r, g, b);
}

void RTexture::ModAlpha(Uint8 a) { SDL_SetTextureAlphaMod(texture, a); }

void RTexture::Render(SDL_Renderer *renderer, int x, int y, SDL_Rect *clip,
                      bool center) {

  renderDest.x = x;
  renderDest.y = y;

  if (clip != NULL) {
    renderDest.w = clip->w * scale * globalScale;
    renderDest.h = clip->h * scale * globalScale;
  }

  else {
    renderDest.w = width * scale * globalScale;
    renderDest.h = height * scale * globalScale;
  }

  if (center) {
    renderDest.x -= renderDest.w / 2;
    renderDest.y -= renderDest.h / 2;
  }

  SDL_RenderCopy(renderer, texture, clip, &renderDest);
}

void RTexture::Render(SDL_Renderer *renderer, int x, int y, SDL_Rect *clip,
                      double angle, SDL_Point *center, SDL_RendererFlip flip) {
  renderDest.x = x;
  renderDest.y = y;

  if (clip != NULL) {
    renderDest.w = clip->w * scale * globalScale;
    renderDest.h = clip->h * scale * globalScale;
  }

  else {
    renderDest.w = width * scale * globalScale;
    renderDest.h = height * scale * globalScale;
  }

  SDL_RenderCopyEx(renderer, texture, clip, &renderDest, angle, center, flip);
}

void RTexture::Render(SDL_Renderer *renderer, int x, int y, int w, int h,
                      SDL_Rect *clip) {

  renderDest.x = x;
  renderDest.y = y;
  renderDest.w = w;
  renderDest.h = h;

  SDL_RenderCopy(renderer, texture, clip, &renderDest);
}

int RTexture::GetWidth() {
  if (renderDest.w == width || renderDest.w == 0) {
    return width * scale * globalScale;
  }

  return renderDest.w;
}

int RTexture::GetHeight() {
  if (renderDest.h == height || renderDest.h == 0) {
    return height * scale * globalScale;
  }

  return renderDest.h;
}

int RTexture::GetWidthUnscaled() { return width; }

int RTexture::GetHeightUnscaled() { return height; }

float RTexture::GetScale(){
  return scale * globalScale;
}

SDL_Rect *RTexture::GetRect() { return &renderDest; }

void RTexture::SetScale(int nScale) { scale = nScale; }
