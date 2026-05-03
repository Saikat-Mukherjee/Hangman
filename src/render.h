#ifndef RENDER_H
#define RENDER_H

#include <SDL3/SDL.h>
#include <SDL3/SDL_ttf.h>
#include "game.h"

/* ── Window dimensions ──────────────────────────────────────────────── */
#define WIN_W   800
#define WIN_H   600

/* ── Layout zones ───────────────────────────────────────────────────── */
#define HEADER_H        55      /* top bar height                       */
#define PANEL_DIV_X     400     /* x that divides gallows / info panels */
#define GAME_AREA_BOT   420     /* y where game area ends               */

/* ── On-screen keyboard constants ───────────────────────────────────── */
#define KB_KEY_W    48
#define KB_KEY_H    40
#define KB_KEY_GAP  8
#define KB_ROW0_Y   428
#define KB_ROW1_Y   476
#define KB_ROW2_Y   524
/* Row start X: centres each row within 800px */
#define KB_ROW0_X   124   /* QWERTYUIOP: 10 keys */
#define KB_ROW1_X   152   /* ASDFGHJKL:  9 keys  */
#define KB_ROW2_X   208   /* ZXCVBNM:    7 keys  */

/* ── Renderer context ───────────────────────────────────────────────── */
typedef struct {
    SDL_Renderer *renderer;
    TTF_Font     *font_large;    /* ~40 pt */
    TTF_Font     *font_medium;   /* ~26 pt */
    TTF_Font     *font_small;    /* ~18 pt */
} Renderer;

/* ── Colour palette ─────────────────────────────────────────────────── */
extern SDL_Color COL_BG;          /* #1a1a2e  deep navy     */
extern SDL_Color COL_PANEL;       /* #16213e  panel bg      */
extern SDL_Color COL_WHITE;       /* #f0f0f0                */
extern SDL_Color COL_ACCENT;      /* #e94560  red accent    */
extern SDL_Color COL_GREEN;       /* #2ecc71  correct guess */
extern SDL_Color COL_WRONG_KEY;   /* #7f1a1a  wrong key bg  */
extern SDL_Color COL_GRAY;        /* #646478  muted text    */
extern SDL_Color COL_YELLOW;      /* #f1c40f  hint/warning  */
extern SDL_Color COL_KEY_BG;      /* #2d2d44  unguessed key */

/* ── Init / destroy ─────────────────────────────────────────────────── */
int  render_init(Renderer *r, SDL_Renderer *sdl_r, const char *font_path);
void render_destroy(Renderer *r);

/* ── Frame helpers ──────────────────────────────────────────────────── */
void render_clear(Renderer *r);
void render_present(Renderer *r);

/* ── Text helpers ───────────────────────────────────────────────────── */
void render_text(Renderer *r, TTF_Font *font, const char *text,
                 int x, int y, SDL_Color col);
void render_text_centered(Renderer *r, TTF_Font *font, const char *text,
                          int cx, int y, SDL_Color col);
void render_text_right(Renderer *r, TTF_Font *font, const char *text,
                       int rx, int y, SDL_Color col);

/* ── Game-screen drawing ────────────────────────────────────────────── */
void render_header(Renderer *r, const Game *g);
void render_gallows(Renderer *r, int stage);
void render_word_display(Renderer *r, const Game *g);
void render_status(Renderer *r, const Game *g);
void render_keyboard(Renderer *r, const Game *g);

/* ── Hit-test: returns 'A'-'Z' if (mx,my) is on a keyboard key, else 0 */
char render_keyboard_key_at(int mx, int my);

/* ── Utility rect / fill ────────────────────────────────────────────── */
void render_filled_rect(Renderer *r, int x, int y, int w, int h, SDL_Color col);
void render_rect_outline(Renderer *r, int x, int y, int w, int h, SDL_Color col);

#endif /* RENDER_H */
