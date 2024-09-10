#ifndef R_GUI
#define R_GUI

#include "RTexture.hpp"
#include <SDL.h>
#include <SDL_ttf.h>
#include <vector>

class RGraphic {
public:
  RGraphic();

  SDL_Rect *GetArea();

  void SetDimensions(int w, int h);
  void SetPosition(int x, int y);
  void SetText(SDL_Renderer *renderer, TTF_Font *font, const char *text, SDL_Color color = {0, 0, 0, 255});
  void SetAreaColor(SDL_Color color);

  void Render(SDL_Renderer *renderer);

private:
  RTexture text;
  SDL_Rect area;

  SDL_Color areaColor;
  SDL_Color textColor;
};

class RVerticalLayoutGroup {
public:
  RVerticalLayoutGroup();

  void SetArea(SDL_Rect *base);
  void SetArea(int x, int y, int w, int h);
  void SetPadding(int w, int h);

  void AddElement(RGraphic *graphic);
  void Apply();

  void Render(SDL_Renderer *renderer);

private:
  SDL_Rect area;

  int wPad;
  int hPad;

  std::vector<RGraphic *> items;
};

#endif
