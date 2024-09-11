#include "RTimer.hpp"

RTimer::RTimer() {
  timer = 0;
  running = false;
}

float RTimer::GetTime() { return timer; }

void RTimer::Reset() { timer = 0; }

void RTimer::Start() { running = true; }

void RTimer::Stop() { running = false; }

void RTimer::Tick(float dt) {
  timer += dt;
}
