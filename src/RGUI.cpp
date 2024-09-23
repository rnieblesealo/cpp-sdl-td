#include "RGUI.hpp"
#include <SDL_render.h>

RGraphic::RGraphic() {
  areaColor.r = 15;
  areaColor.g = 15;
  areaColor.b = 15;
  areaColor.a = 255;

  textColor.r = 255;
  textColor.g = 255;
  textColor.b = 255;
  textColor.a = 255;

  textAnchor = R_BOTTOM;
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

void RGraphic::SetTextColor(Uint8 r, Uint8 g, Uint8 b){
  textColor.r = r;
  textColor.g = g;
  textColor.b = b;
}

void RGraphic::SetTextPadding(int padding){
  textPadding = padding;
}

void RGraphic::SetTextScale(int scale){
  text.SetScale(scale);
}

void RGraphic::SetText(SDL_Renderer *renderer, TTF_Font *font, const char *text){
  this->text.LoadFromRenderedText(renderer, font, text, textColor.r, textColor.g, textColor.b); 
}

void RGraphic::SetIcon(RTexture *icon) { this->icon = icon; }

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

  // get text pos based on anchor
  int textX = 0;
  int textY = 0;

  switch (textAnchor) {
  case R_TOP:
    textX = area.x + area.w / 2;
    textY = area.y + text.GetHeight() / 2 + textPadding;
    break;
  case R_BOTTOM:
    textX = area.x + area.w / 2;
    textY = area.y + area.h - text.GetHeight() / 2 - textPadding; 
    break;
  case R_LEFT:
    textX = area.x + text.GetWidth() / 2 + textPadding;
    textY = area.y + area.h / 2;
    break;
  case R_RIGHT:
    textX = area.x + area.w - text.GetWidth() / 2 - textPadding;
    textY = area.y + area.h / 2;
    break;
  default:
    // place at center by default
    textX = area.x + area.w / 2;
    textY = area.y + area.h  / 2;
  }

  // draw icon at center if there is one
  if (icon != NULL) {
    icon->Render(renderer, area.x + area.w / 2, area.y + area.h / 2, NULL,
                 true);
  }

  this->text.Render(renderer, textX, textY, NULL, true); 
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
    items[i]->SetPosition(area.x + wPad, area.y + hPad + elemH * i);

    // set elem dimension
    items[i]->SetDimensions(elemW - wPad * 2, elemH - hPad * 2);
  }
}

void RVerticalLayoutGroup::Render(SDL_Renderer *renderer) {
  for (int i = 0; i < items.size(); ++i) {
    items[i]->Render(renderer);
  }
}
