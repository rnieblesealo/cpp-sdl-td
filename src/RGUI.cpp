#include "RGUI.hpp"

RGraphic::RGraphic() {
  areaColor.r = 255;
  areaColor.g = 255;
  areaColor.b = 255;
  areaColor.a = 255;

  textColor.r = 0;
  textColor.g = 0;
  textColor.b = 0;
  textColor.a = 255;
}

SDL_Rect *RGraphic::GetArea() { return &area; }

void RGraphic::SetDimensions(int w, int h) {
  area.w = w;
  area.h = h;
}

void RGraphic::SetPosition(int x, int y) {
  area.x = x;
  area.y = y;
}

void RGraphic::SetText(SDL_Renderer *renderer, TTF_Font *font, const char *text,
                       Uint8 r, Uint8 g, Uint8 b) {
  this->text.LoadFromRenderedText(renderer, font, text, r, g, b);
}

void RGraphic::SetAreaColor(Uint8 r, Uint8 g, Uint8 b) {
  areaColor.r = r;
  areaColor.g = g;
  areaColor.b = b;
}

void RGraphic::Render(SDL_Renderer *renderer) {
  // draw area
  SDL_SetRenderDrawColor(renderer, areaColor.r, areaColor.g, areaColor.b,
                         areaColor.a);
  SDL_RenderFillRect(renderer, &area);
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);

  // draw text
  text.Render(renderer, area.x, area.y, area.w, area.h);
}

RVerticalLayoutGroup::RVerticalLayoutGroup() {
  wPad = 0;
  hPad = 0;
}

void RVerticalLayoutGroup::SetArea(SDL_Rect *base) {
  // set area from another rect
  area = *base;
}

void RVerticalLayoutGroup::SetArea(int x, int y, int w, int h) {
  // set area manually
  area.x = x;
  area.y = y;
  area.w = w;
  area.h = h;
}

void RVerticalLayoutGroup::SetPadding(int w, int h) {
  wPad = w;
  hPad = h;
}

void RVerticalLayoutGroup::AddElement(RGraphic *graphic) {
  items.push_back(graphic);
}

void RVerticalLayoutGroup::Apply() {
  // get w, h of one element
  int elemW = area.w;
  int elemH = area.h / items.size();

  for (int i = 0; i < items.size(); ++i) {
    // set elem position
    items[i]->SetPosition(area.x + wPad, hPad + elemH * i);

    // set elem dimension
    items[i]->SetDimensions(elemW - wPad * 2, elemH - hPad * 2);
  }
}

void RVerticalLayoutGroup::Render(SDL_Renderer *renderer) {
  for (int i = 0; i < items.size(); ++i) {
    items[i]->Render(renderer);
  }
}
