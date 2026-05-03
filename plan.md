# Hangman — Fix to Production SDL2 Game

## Overview
Transform the broken CLI `main.c` into a production-quality SDL2 windowed game
with categories, difficulty levels, a leaderboard, hints, and multiplayer.

## Tech Stack
- Language : C99
- GUI      : SDL2 + SDL2_ttf
- Build    : Makefile (MinGW-W64 GCC 15.2.0, Windows x86_64)

---

## Target File Structure
```
Hangman/
├── plan.md
├── SDL2_SETUP.txt          <- instructions for downloading SDL2
├── Makefile
├── README.md
├── main.c                  <- original broken file (kept for reference)
├── hang.txt                <- original word file (replaced by data/words/)
├── src/
│   ├── main.c              <- SDL2 entry point + state machine
│   ├── game.h / game.c     <- pure logic: init, guess, hint, score
│   ├── words.h / words.c   <- load category word lists, pick by difficulty
│   ├── render.h / render.c <- SDL2 drawing: gallows, word, keyboard, text
│   ├── ui.h / ui.c         <- Button struct, TextInput, hover detection
│   └── scores.h / scores.c <- CSV leaderboard: save, load, sort, top-10
├── assets/
│   └── fonts/
│       └── Roboto-Regular.ttf    <- OFL-licensed, see SDL2_SETUP.txt
├── data/
│   ├── words/
│   │   ├── animals.txt           <- 25 animal words (uppercase, one per line)
│   │   ├── countries.txt         <- 25 country names
│   │   ├── tech.txt              <- 25 tech terms
│   │   └── mixed.txt             <- 25 mixed words
│   └── scores.dat                <- CSV leaderboard (auto-created)
└── lib/                          <- SDL2.dll, SDL2_ttf.dll (see SDL2_SETUP.txt)
```

---

## Game States
```
STATE_MAIN_MENU
    -> [Play]          -> STATE_CATEGORY_SELECT
    -> [Multiplayer]   -> STATE_MULTIPLAYER_INPUT
    -> [High Scores]   -> STATE_HIGH_SCORES
    -> [Quit]          -> exit

STATE_CATEGORY_SELECT  -> STATE_DIFFICULTY_SELECT
STATE_DIFFICULTY_SELECT -> STATE_PLAYING
STATE_MULTIPLAYER_INPUT -> STATE_PLAYING  (word from P1, not file)

STATE_PLAYING -> STATE_WIN  (all letters guessed)
             -> STATE_LOSE (wrong_count >= max_chances)

STATE_WIN / STATE_LOSE -> STATE_HIGH_SCORES (after saving score)
                       -> STATE_CATEGORY_SELECT (play again)
                       -> STATE_MAIN_MENU

STATE_HIGH_SCORES -> STATE_MAIN_MENU
```

---

## Key Data Structures

### Game  (src/game.h)
| Field         | Type       | Description                               |
|---------------|------------|-------------------------------------------|
| word          | char[50]   | Original word (uppercase)                 |
| masked        | char[50]   | Current display state with '_' blanks     |
| guessed       | int[26]    | 1 = letter A-Z has been tried             |
| wrong_count   | int        | Number of wrong guesses so far            |
| max_chances   | int        | Max allowed wrongs (Easy=7, Med=5, Hard=3)|
| score         | int        | Computed on WIN                           |
| difficulty    | int        | 0=Easy 1=Medium 2=Hard                    |
| category      | char[20]   | "animals", "countries", "tech", "mixed"   |
| player_name   | char[32]   | Entered by player on win/lose screen      |
| is_multiplayer| int        | 1 if word came from P1 input              |

### Button  (src/ui.h)
`SDL_Rect rect`, `char label[64]`, `int hovered`, `SDL_Color bg`, `SDL_Color hover_bg`

### TextInput  (src/ui.h)
`char buffer[64]`, `int len`, `int hide_text` (1 = show as ***)

### ScoreEntry  (src/scores.h)
`char name[32]`, `char word[50]`, `char category[20]`, `int difficulty`, `int score`, `char date[12]`

---

## Difficulty Rules
| Level  | Chances | % Blanked (min 1) | Min Word Len | Score Multiplier |
|--------|---------|-------------------|--------------|------------------|
| Easy   | 7       | 40%               | 3            | ×1               |
| Medium | 5       | 60%               | 4            | ×2               |
| Hard   | 3       | 80%               | 5            | ×3               |

**Score** = (chances_remaining + 1) × multiplier × 10

---

## Hangman Drawing — 6 Progressive Stages
| Stage | Body Part Added                  |
|-------|----------------------------------|
| 0     | Gallows frame only (always shown)|
| 1     | Head (circle)                    |
| 2     | Body (vertical line)             |
| 3     | Left arm                         |
| 4     | Right arm                        |
| 5     | Left leg                         |
| 6     | Right leg → GAME OVER            |

Rendered stage = `(wrong_count * 6) / max_chances` (proportional to difficulty)

---

## Screen Layout (800 × 600)
```
[0,0]──────────────────────────────────[800,0]
│          HEADER BAR  (h=55)                 │
│  Category | Difficulty | Score              │
[0,55]──────────────────────────────────[800,55]
│ LEFT PANEL (w=400)   │ RIGHT PANEL (w=400) │
│ Gallows drawing      │ "GUESS THE WORD:"   │
│                      │  _ A _ _ _ _        │
│                      │                     │
│                      │ Chances: ████░░░    │
│                      │ Wrong: E, R, T      │
│                      │ [  HINT  ]          │
[0,420]─────────────────────────────────[800,420]
│         KEYBOARD (QWERTY layout)            │
│   Q W E R T Y U I O P   (row y=428)        │
│    A S D F G H J K L    (row y=476)        │
│      Z X C V B N M      (row y=524)        │
[0,570]─────────────────────────────────[800,570]
│  [MENU]                        [score]      │
[0,600]─────────────────────────────────[800,600]
```

---

## Hint System
- Reveals one random still-blanked letter
- Penalty: +2 to `wrong_count`
- Disabled if `(max_chances - wrong_count) <= 2` (prevents instant loss)

---

## Multiplayer Mode
1. `STATE_MULTIPLAYER_INPUT`: P1 types a word; shown as `***` on screen
2. Screen clears + shows "P2 — your turn" transition
3. Enters `STATE_PLAYING` with P1's word (no file lookup)
4. Normal game flow (but category shown as "Multiplayer")

---

## Leaderboard (data/scores.dat)
CSV format:
```
name,word,category,difficulty,score,date
Alice,ELEPHANT,animals,1,60,2026-05-04
```
- Loaded at startup, saved after every game ending
- Top 10 displayed on `STATE_HIGH_SCORES` screen

---

## Implementation Phases

### Phase 1 — Core Logic (no SDL2 dependency)
- [x] `src/game.h` + `src/game.c`   — init, guess, hint, score
- [x] `src/words.h` + `src/words.c` — load files, pick word by difficulty
- [x] `src/scores.h` + `src/scores.c` — CSV read/write/sort
- [x] `data/words/*.txt`             — 4 word-list files (25 words each)
- [x] `data/scores.dat`              — CSV header line

### Phase 2 — SDL2 Rendering
- [x] `src/render.h` + `src/render.c` — gallows, word, keyboard, text helpers
- [x] `src/ui.h` + `src/ui.c`         — Button, TextInput components

### Phase 3 — Application Shell
- [x] `src/main.c`   — SDL2 init, event loop, state machine

### Phase 4 — Build System
- [x] `Makefile`        — all/clean, obj/ intermediates
- [x] `SDL2_SETUP.txt`  — download + install instructions

---

## Known Bugs Fixed vs Original main.c
| # | Original Bug                  | Fix                                        |
|---|-------------------------------|--------------------------------------------|
| 1 | `while(!feof(fp))`            | `while(fscanf(...)==1)` loop               |
| 2 | `rand()%0` crash on empty file| Validate word count before `rand()`        |
| 3 | Random blanking (could blank 0)| Deterministic % with minimum-1 guarantee   |
| 4 | `rand()%(1-0+1)` magic number | Named constants for blank percentage       |
| 5 | `fflush(stdin)` UB            | SDL event-driven input (no scanf at all)   |
| 6 | `arr[10][50]` fixed 10-word cap| Dynamic WordList with 100-word capacity    |

---

## External Dependencies to Download (see SDL2_SETUP.txt)
1. SDL2 MinGW development library  — https://libsdl.org/download-2.0.php
2. SDL2_ttf MinGW development library — https://github.com/libsdl-org/SDL_ttf/releases
3. Roboto-Regular.ttf (OFL license) — https://fonts.google.com/specimen/Roboto
