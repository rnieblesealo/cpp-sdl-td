#include "RTexture.hpp"

#include <SDL.h>
#include <SDL_events.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_render.h>
#include <SDL_ttf.h>
#include <SDL_video.h>

const int SCREEN_WIDTH = 1000;
const int SCREEN_HEIGHT = 1000;

SDL_Window *gWindow = NULL;
SDL_Surface* gWindowSurface = NULL;
SDL_Renderer *gRenderer = NULL;

RTexture tMap0;

bool Init(){
  bool success = true;

  if (SDL_Init(SDL_INIT_VIDEO) < 0){
    success = false;
  }

  gWindow = SDL_CreateWindow("Cats vs. Robo-Rats!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

  if (!gWindow){
    success = false;
  }

  gWindowSurface = SDL_GetWindowSurface(gWindow); 

  gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);

  if (gRenderer == NULL){
    success = false;
  }

  SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 0);

  int imageFlags = IMG_INIT_PNG; 
  if (!(IMG_Init(imageFlags) & imageFlags)){
    success = false;
  }

  if (TTF_Init() == -1){
    success = false;
  }

  if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0){
    success = false;
  }

  return success;
}

bool LoadMedia(){
  bool success = true;

  if (!tMap0.LoadFromFile(gRenderer, "../assets/map0.png")){
    success = false;
  }

  return success;
}

void Close(){
  tMap0.Free();
  
  SDL_DestroyRenderer(gRenderer);
  gRenderer = NULL;

  SDL_DestroyWindow(gWindow);
  gWindow = NULL;

  IMG_Quit();
  Mix_Quit();
  SDL_Quit();
}

int main(){
  if (!Init()){
    return 1;
  }

  if (!LoadMedia()){
    return 1;
  }

  SDL_Event e;

  bool quit = false;
  while (!quit){
    while (SDL_PollEvent(&e)){
      // event handling
    }

    // update code

    SDL_RenderClear(gRenderer);

    // draw code

    SDL_RenderPresent(gRenderer);
  }

  Close();

  return 0;
}
