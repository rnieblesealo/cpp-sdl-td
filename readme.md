# Dower Tefense

![Dower Tefense illustration](.github/gameplay.gif)

Welcome to Dower Tefense! This is a WIP TD game with some RTS elements made using C++ and SDL2, mostly as a learning exercise.

Instead of being the defender, *you are the attacker!* Place down your troops, aim at the towers, and watch the action!

You win if you destroy all defenses or reach the end enough times, but if you run out of troops, you're outta here.

Good luck!

[Pixel art by zintoki](https://zintoki.itch.io/ground-shaker)

Music and SFX by me :)

## To-Do

- [X] Damage/health system
    - [x] Enemy/tower healthbars
    - [x] Tower/defender health
- [ ] Enemy colors
    - [x] Button icons
- [x] Music
- [x] Click to set target
    - [x] Snap to clicked enemy
- [x] Towers!
- [ ] Make all assets match 128x128 so that we don't have to do resizing
- [x] Make the UI not god awful
- [ ] Win condition
- [ ] `REFACTOR` UI code is a mess! Fix :)
- [ ] `OPTIMIZATION` Move all entities to one vector and typecheck using an enum
- [ ] `BUG` Hitting one target may damage another, very likely it's caused by list iteration; check warning comment
- [ ] `BUG` Are we freeing the texture in GUI graphics correctly? 

## Notes

### General/Uncategorized
- Sprite postions are relative to their center; everything else's position isn't
- If we get random segfault, check NULL assignments for R classes

### Idea 
Tower defense game, but you control the invaders instead!

- You can spawn in tanks of different colors, from a budget given at the beginning of the level
- The game spawns towers in as well from a budget too; it'll also tell you what tower it's spawning next, when, and where
- You can select an individual tank and tell it to hit a specific tower 
    - Click on the tank, then on the tower 
- You can also command each overall color to target a specific structure
    - Double-click on one tank of the desired color, then click on the tower
- If a tank destroyed its structure, it'll auto-target the closest tower
- **You win when enough tanks have reached the end, or when the game is out of towers!**
- There will be power-ups which can be applied to tanks, giving them extra damage, resistance, etc.

### Maps
1. Make maps by hand in image editor using tile editor
2. Hard-code paths by hand

This is the least confusing + most painless way to do things!

### Display Scaling
Exact multiples of 128 only, since this is the tilesize used throughout; use a single global scale factor applied to everything
