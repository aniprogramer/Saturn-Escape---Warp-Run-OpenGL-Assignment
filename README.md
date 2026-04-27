# Saturn Escape: Warp Run

A 5-minute automated 3D cinematic experience implemented in C++ using OpenGL and freeglut.

This repository contains several experiment files; `main6.cpp` is the full cinematic: Saturn, rings, dogfights, an ethereal portal, a hyperspace tunnel, and a final "The End" title card.

## Features

- Procedural starfields, nebulae and hyperspace tunnel effects
- Detailed Saturn body + multi-ring rendering
- Particle-based sparks and explosions
- Automated timeline (strictly 5 minutes / 300s) with camera choreography
- Interactive keys for basic playback control

## Prerequisites

- A C++ compiler with C++11 support (g++, clang, or MSVC)
- OpenGL (system GPU drivers)
- freeglut (recommended) or GLUT

On Debian/Ubuntu (example):

```bash
sudo apt-get install build-essential freeglut3-dev libglu1-mesa-dev
```

On Windows, install MinGW-w64/MSYS2 or use Visual Studio; place `freeglut.dll` next to the executable if using prebuilt binaries.

## Build

**Windows (MinGW / MSYS2)**

```bash
# If using MSYS2, install freeglut first:
# pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-freeglut

g++ -std=c++11 main6.cpp -o SaturnEscape -lfreeglut -lglu32 -lopengl32 -lwinmm -lgdi32
```

**Visual Studio (MSVC)**

- Create a new empty console or Win32 project, add `main6.cpp` to the project.
- Link against `freeglut.lib`, `opengl32.lib`, and `glu32.lib` (add additional system libs if needed).
- Ensure `freeglut.dll` is available next to the built executable or in your PATH.

**Linux**

```bash
g++ -std=c++11 main6.cpp -o SaturnEscape -lGL -lGLU -lglut -lm
```

## Run

- Execute the produced binary (`./SaturnEscape` or `SaturnEscape.exe`).
- The application starts in fullscreen (calls `glutFullScreen()` in `main()`).
- Press `Esc` to quit.

## Controls

- Space: Pause / Play
- L (or l): Skip +10s
- J (or j): Back -10s
- Esc: Quit

These match the key handling in `main6.cpp`.

## Timeline (high-level)

```
0-25s:   Intro - Top-down Isometric view of Saturn
25-35s:  SEAMLESS TRANSITION - Fly-by over Saturn
35-90s:  SPACE WARS DOGFIGHT - Dynamic Action Camera & Combat
90-150s: Saturn's Rings entry & Asteroid Field Action
150-210s: Damage & Engine Flicker under Saturn's Shadow
210-230s: ETHEREAL PORTAL FORMATION (Matches new reference)
230-270s: HYPERSPACE TUNNEL
270-290s: NEW GALAXY ARRIVAL
290-300s: Whiteout Fade & "The End" (Strictly 5 Minutes)
```

## Important constants (edit in `main6.cpp`)

- `REFRESH_MS` — frame timer interval in milliseconds
- `TOTAL_TIME` — total timeline duration (300.0f by default)
- `MAX_PARTICLES`, `ASTEROID_COUNT` — particle and asteroid counts (adjust for performance)

## Customization & Notes

- To reduce CPU/GPU load: lower particle counts, reduce star/tunnel point counts, or run in a smaller windowed resolution.
- The code uses immediate-mode OpenGL (glBegin/glEnd) and fixed-function pipeline; porting to modern OpenGL (VBOs / shaders) will significantly improve performance and visual flexibility.
- Many visual parameters are hard-coded for a single cinematic timeline—search for time-based conditionals in `display()` to tweak behavior.

## Known issues / Tips

- On some Windows setups you may need to copy `freeglut.dll` to the executable directory.
- Build in Release mode for best performance.

## Credits

- Author: (project or student author — update as needed)
- Uses: OpenGL, freeglut

## License

No license is included. Add a `LICENSE` file if you want to make this code reusable under a specific license.
