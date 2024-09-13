#ifndef R_TIMER_H
#define R_TIMER_H

class RTimer{
public:
  RTimer();

  float GetTime();
  
  void Start();
  void Stop();
  void Reset();
  void AddOffset(float amt);
  void Tick(float dt);

private:
  float timer;
  bool running;
};

#endif
