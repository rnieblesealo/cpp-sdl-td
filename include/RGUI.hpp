#ifndef R_GUI
#define R_GUI

#include "RTexture.hpp"
#include <SDL.h>
#include <SDL_events.h>
#include <SDL_mouse.h>
#include <SDL_ttf.h>
#include <vector>

class RGraphic {
public:
  RGraphic();

  SDL_Rect *GetArea();

  void SetDimensions(int w, int h);
  void SetPosition(int x, int y);
  void SetText(SDL_Renderer *renderer, TTF_Font *font, const char *text,
               Uint8 r, Uint8 g, Uint8 b);
  void SetAreaColor(Uint8 r, Uint8 g, Uint8 b);

  void Render(SDL_Renderer *renderer);

private:
  RTexture text;
  SDL_Rect area;

  SDL_Color areaColor;
  SDL_Color textColor;
};

class RButton {
public:
  RButton(RGraphic *graphic, void (*action)()) {
    this->graphic = graphic;
    this->onClick = action;
  }

  void SetAction(void (*action)()) { this->onClick = action; }

  void SetGraphic(RGraphic *graphic) { this->graphic = graphic; }

  void HandleEvent(SDL_Event *e) {

    // check if mouse clicking
    if (e->type == SDL_MOUSEBUTTONDOWN) {
      // check if mouse in bounds
      int mouseX = 0;
      int mouseY = 0;

      SDL_GetMouseState(&mouseX, &mouseY);

      SDL_Rect *area = graphic->GetArea();

      if (mouseX < area->x || mouseX > area->x + area->w || mouseY < 0 ||
          mouseY > area->y + area->h) {
        return;
      }

      // check if press; if so run action
      if (e->button.button == SDL_BUTTON_LEFT) {
        if (onClick != NULL) {
          onClick();
        }
      }
    }
  }

private:
  RGraphic *graphic;
  void (*onClick)();
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
