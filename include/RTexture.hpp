#ifndef RTEXTURE_H
#define RTEXTURE_H

#include <SDL.h>
#include <SDL_ttf.h>

class RTexture {
public:
  RTexture();
  ~RTexture();

#if defined(SDL_TTF_MAJOR_VERSION)

  bool LoadFromRenderedText(SDL_Renderer *renderer, TTF_Font *font,
                            const char *text, Uint8 r = 255, Uint8 g = 255,
                            Uint8 b = 255);
#endif

  static void SetGlobalScale(float scale);

  bool LoadFromFile(SDL_Renderer *renderer, const char *path, Uint8 r = 0,
                    Uint8 g = 0, Uint8 b = 0);
  void Free();
  void SetBlendMode(SDL_BlendMode blendMode);
  void ModColor(Uint8 r, Uint8 g, Uint8 b);
  void ModAlpha(Uint8 a);

  void Render(SDL_Renderer *renderer, int x, int y, SDL_Rect *clip = NULL,
              bool center = false);
  void Render(SDL_Renderer *renderer, int x, int y, SDL_Rect *clip,
              double angle, SDL_Point *center, SDL_RendererFlip flip);
  void Render(SDL_Renderer *renderer, int x, int y, int w, int h,
              SDL_Rect *clip = NULL);

  int GetWidth();
  int GetHeight();
  int GetWidthUnscaled();
  int GetHeightUnscaled();
  float GetScale();
  SDL_Rect *GetRect();
  void SetScale(int nScale);

private:
  SDL_Texture *texture;
  SDL_Rect renderDest;

  int width;
  int height;
  int scale;

  static float globalScale;
};

#endif
