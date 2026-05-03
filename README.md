# Hangman

A production-grade, SDL3-powered Hangman game written in C99 for Windows. Features a full graphical UI, multiple word categories, three difficulty levels, a persistent leaderboard, hints, and a two-player mode — all built from scratch with no game engine.

---

## Quick Start

### Prerequisites

| Requirement | Version |
|---|---|
| GCC (MinGW-W64 x86_64-ucrt-posix-seh) | 15.x |
| mingw32-make | bundled with MinGW-W64 |
| SDL3 development libraries (MinGW) | 3.4.8 |
| SDL3_ttf development libraries (MinGW) | 3.x |
| Roboto-Regular.ttf | any (OFL license) |

### 1 — Download SDL3

From **https://github.com/libsdl-org/SDL/releases/tag/release-3.4.8**, download `SDL3-devel-3.4.8-mingw.zip` and copy:

```
x86_64-w64-mingw32/include/SDL3/  →  include/SDL3/
x86_64-w64-mingw32/lib/*.a        →  lib/
x86_64-w64-mingw32/bin/SDL3.dll   →  ./ (project root)
```

### 2 — Download SDL3_ttf

From **https://github.com/libsdl-org/SDL_ttf/releases/latest**, download the MinGW zip and copy:

```
x86_64-w64-mingw32/include/SDL3/SDL_ttf.h  →  include/SDL3/
x86_64-w64-mingw32/lib/*.a                 →  lib/
x86_64-w64-mingw32/bin/SDL3_ttf.dll        →  ./
(copy any bundled DLLs, e.g. libfreetype*.dll)
```

### 3 — Download font

From **https://fonts.google.com/specimen/Roboto**, click *Download family*, extract, and copy:

```
Roboto-Regular.ttf  →  assets/fonts/Roboto-Regular.ttf
```

### 4 — Build

Open a terminal (PowerShell, Git Bash, or MSYS2) in the project root:

```sh
mingw32-make
```

Output binary: `hangman.exe`

### 5 — Run

```sh
./hangman.exe
```

> `SDL3.dll` and `SDL3_ttf.dll` must be in the same folder as `hangman.exe`.

---

## Features

- **4 word categories** — Animals, Countries, Technology, Mixed  
- **3 difficulty levels** — Easy (8 chances, 40% masked), Medium (6, 60%), Hard (4, 80%)  
- **Hint system** — reveals one blank letter at a 2-wrong-guess penalty  
- **2-player mode** — Player 1 types the secret word (hidden), Player 2 guesses  
- **Persistent leaderboard** — top scores saved to `data/scores.dat` (CSV), sorted by score  
- **6-stage gallows** — Bresenham circle head, proportional limb progression  
- **On-screen keyboard** — colour-coded: correct (green), wrong (red), unused (grey)  

---

## Project Structure

```
Hangman/
├── src/
│   ├── main.c          # State machine & SDL3 event loop
│   ├── game.c/h        # Pure game logic (no SDL dependency)
│   ├── render.c/h      # All SDL3 drawing (gallows, text, keyboard)
│   ├── ui.c/h          # Button & text-input components
│   ├── words.c/h       # Word loading & difficulty-filtered selection
│   ├── scores.c/h      # Leaderboard read/write/sort
│   └── winmain.c       # WinMain → main bridge (replaces libSDL3main)
├── data/
│   ├── scores.dat      # Leaderboard (CSV, auto-created)
│   └── words/
│       ├── animals.txt
│       ├── countries.txt
│       ├── tech.txt
│       └── mixed.txt
├── assets/
│   └── fonts/
│       └── Roboto-Regular.ttf
├── include/SDL3/        # SDL3 headers (you copy here)
├── lib/                 # SDL3 .a import libs (you copy here)
├── obj/                 # Compiled object files (auto-generated)
├── Makefile
└── hangman.exe          # Output binary
```

---

## Build Targets

```sh
mingw32-make          # build hangman.exe
mingw32-make clean    # remove obj/ and hangman.exe
mingw32-make run      # build + run
```

---

## Architecture

The game runs as a 9-state machine:

```
MAIN_MENU → CATEGORY_SELECT → DIFFICULTY_SELECT → PLAYING
PLAYING   → WIN | LOSE → HIGH_SCORES
MAIN_MENU → MULTIPLAYER_INPUT → PLAYING → WIN | LOSE
any state → MAIN_MENU (back button)
any state → QUIT
```

Each state has its own render and event handler. Game logic (`game.c`), word loading (`words.c`), and score persistence (`scores.c`) are pure C with no SDL dependency — they can be unit-tested or reused independently.

---

## Scoring

$$\text{score} = (\text{chances remaining} + 1) \times \text{difficulty multiplier} \times 10$$

| Difficulty | Multiplier | Starting chances |
|---|---|---|
| Easy | 1× | 8 |
| Medium | 2× | 6 |
| Hard | 3× | 4 |

---

## Dependencies

| Library | License | Purpose |
|---|---|---|
| SDL3 3.4.8 | zlib | Window, renderer, events |
| SDL3_ttf | zlib | TrueType font rendering |
| Roboto | OFL | UI font |

All other code is original C99, compiled with GCC 15.2.0 MinGW-W64.

---

## Troubleshooting

| Symptom | Fix |
|---|---|
| `SDL3.dll not found` | Copy `SDL3.dll` next to `hangman.exe` |
| `TTF_OpenFont failed` | Check `assets/fonts/Roboto-Regular.ttf` exists |
| `cannot find -lSDL3` | Check `lib/libSDL3.dll.a` exists |
| `SDL.h: No such file` | Check `include/SDL3/SDL.h` exists |
| Blank window / crash | Check `data/words/*.txt` are not empty |

