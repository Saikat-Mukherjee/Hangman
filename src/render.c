#include "render.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

/* ── Colour palette definitions ─────────────────────────────────────── */
SDL_Color COL_BG        = {26,  26,  46,  255};
SDL_Color COL_PANEL     = {22,  33,  62,  255};
SDL_Color COL_WHITE     = {240, 240, 240, 255};
SDL_Color COL_ACCENT    = {233, 69,  96,  255};
SDL_Color COL_GREEN     = {46,  204, 113, 255};
SDL_Color COL_WRONG_KEY = {127, 26,  26,  255};
SDL_Color COL_GRAY      = {100, 100, 120, 255};
SDL_Color COL_YELLOW    = {241, 196, 15,  255};
SDL_Color COL_KEY_BG    = {45,  45,  68,  255};

/* ── Keyboard layout tables ─────────────────────────────────────────── */
static const char *KB_ROWS[3]  = {"QWERTYUIOP", "ASDFGHJKL", "ZXCVBNM"};
static const int   KB_ROW_X[3] = {KB_ROW0_X, KB_ROW1_X, KB_ROW2_X};
static const int   KB_ROW_Y[3] = {KB_ROW0_Y, KB_ROW1_Y, KB_ROW2_Y};

/* ── Internal: Bresenham midpoint circle (outline) ──────────────────── */
static void draw_circle(SDL_Renderer *rend, int cx, int cy, int radius)
{
    int x = radius, y = 0, err = 0;
    while (x >= y) {
        SDL_RenderPoint(rend, (float)(cx + x), (float)(cy - y));
        SDL_RenderPoint(rend, (float)(cx + y), (float)(cy - x));
        SDL_RenderPoint(rend, (float)(cx - y), (float)(cy - x));
        SDL_RenderPoint(rend, (float)(cx - x), (float)(cy - y));
        SDL_RenderPoint(rend, (float)(cx - x), (float)(cy + y));
        SDL_RenderPoint(rend, (float)(cx - y), (float)(cy + x));
        SDL_RenderPoint(rend, (float)(cx + y), (float)(cy + x));
        SDL_RenderPoint(rend, (float)(cx + x), (float)(cy + y));
        y++;
        err += 1 + 2 * y;
        if (2 * (err - x) + 1 > 0) { x--; err += 1 - 2 * x; }
    }
}

/* Draw a 3-pixel-thick circle for the head */
static void draw_thick_circle(SDL_Renderer *rend, int cx, int cy, int r)
{
    draw_circle(rend, cx, cy, r);
    draw_circle(rend, cx, cy, r - 1);
    draw_circle(rend, cx, cy, r + 1);
}

/* ── Init / destroy ─────────────────────────────────────────────────── */
int render_init(Renderer *r, SDL_Renderer *sdl_r, const char *font_path)
{
    r->renderer = sdl_r;

    r->font_large  = TTF_OpenFont(font_path, 40);
    r->font_medium = TTF_OpenFont(font_path, 26);
    r->font_small  = TTF_OpenFont(font_path, 18);

    if (!r->font_large || !r->font_medium || !r->font_small) {
        fprintf(stderr, "TTF_OpenFont: %s\n", SDL_GetError());
        return 0;
    }
    return 1;
}

void render_destroy(Renderer *r)
{
    if (r->font_large)  TTF_CloseFont(r->font_large);
    if (r->font_medium) TTF_CloseFont(r->font_medium);
    if (r->font_small)  TTF_CloseFont(r->font_small);
    r->font_large = r->font_medium = r->font_small = NULL;
}

/* ── Frame helpers ──────────────────────────────────────────────────── */
void render_clear(Renderer *r)
{
    SDL_SetRenderDrawColor(r->renderer,
                           COL_BG.r, COL_BG.g, COL_BG.b, COL_BG.a);
    SDL_RenderClear(r->renderer);
}

void render_present(Renderer *r)
{
    SDL_RenderPresent(r->renderer);
}

/* ── Utility rect ───────────────────────────────────────────────────── */
void render_filled_rect(Renderer *r, int x, int y, int w, int h, SDL_Color col)
{
    SDL_SetRenderDrawColor(r->renderer, col.r, col.g, col.b, col.a);
    SDL_FRect rect = {(float)x, (float)y, (float)w, (float)h};
    SDL_RenderFillRect(r->renderer, &rect);
}

void render_rect_outline(Renderer *r, int x, int y, int w, int h, SDL_Color col)
{
    SDL_SetRenderDrawColor(r->renderer, col.r, col.g, col.b, col.a);
    SDL_FRect rect = {(float)x, (float)y, (float)w, (float)h};
    SDL_RenderRect(r->renderer, &rect);
}

/* ── Text helpers ───────────────────────────────────────────────────── */
void render_text(Renderer *r, TTF_Font *font, const char *text,
                 int x, int y, SDL_Color col)
{
    if (!text || !text[0]) return;
    SDL_Surface *surf = TTF_RenderText_Blended(font, text, 0, col);
    if (!surf) return;
    SDL_Texture *tex = SDL_CreateTextureFromSurface(r->renderer, surf);
    SDL_DestroySurface(surf);
    if (!tex) return;
    float fw, fh;
    SDL_GetTextureSize(tex, &fw, &fh);
    SDL_FRect dst = {(float)x, (float)y, fw, fh};
    SDL_RenderTexture(r->renderer, tex, NULL, &dst);
    SDL_DestroyTexture(tex);
}

void render_text_centered(Renderer *r, TTF_Font *font, const char *text,
                           int cx, int y, SDL_Color col)
{
    if (!text || !text[0]) return;
    int w = 0, h = 0;
    TTF_GetStringSize(font, text, 0, &w, &h);
    render_text(r, font, text, cx - w / 2, y, col);
}

void render_text_right(Renderer *r, TTF_Font *font, const char *text,
                        int rx, int y, SDL_Color col)
{
    if (!text || !text[0]) return;
    int w = 0, h = 0;
    TTF_GetStringSize(font, text, 0, &w, &h);
    render_text(r, font, text, rx - w, y, col);
}

/* ── Header bar ─────────────────────────────────────────────────────── */
void render_header(Renderer *r, const Game *g)
{
    render_filled_rect(r, 0, 0, WIN_W, HEADER_H, COL_PANEL);

    /* Category (left) */
    char cat[32];
    snprintf(cat, sizeof(cat), "Category: %s", g->category);
    render_text(r, r->font_small, cat, 12, 16, COL_WHITE);

    /* Difficulty (centre) */
    static const char *dnames[] = {"Easy", "Medium", "Hard"};
    const char *dname = (g->difficulty >= 0 && g->difficulty <= 2)
                         ? dnames[g->difficulty] : "?";
    render_text_centered(r, r->font_small, dname, WIN_W / 2, 16, COL_YELLOW);

    /* Score (right) */
    char score_str[32];
    snprintf(score_str, sizeof(score_str), "Score: %d", g->score);
    render_text_right(r, r->font_small, score_str, WIN_W - 12, 16, COL_WHITE);

    /* Separator */
    SDL_SetRenderDrawColor(r->renderer,
                           COL_ACCENT.r, COL_ACCENT.g, COL_ACCENT.b, 255);
    SDL_RenderLine(r->renderer, 0.0f, (float)(HEADER_H - 1), (float)WIN_W, (float)(HEADER_H - 1));
}

/* ── Gallows ─────────────────────────────────────────────────────────
   Left panel: x 0-400, y 55-420.
   All coordinates are absolute window pixels.                        */
void render_gallows(Renderer *r, int stage)
{
    SDL_Renderer *rend = r->renderer;
    SDL_SetRenderDrawColor(rend, 200, 200, 210, 255);

    /* === Gallows frame (always drawn) === */
    /* Base */
    SDL_RenderLine(rend, 20.0f, 410.0f, 370.0f, 410.0f);
    /* Vertical post */
    SDL_RenderLine(rend, 80.0f, 410.0f, 80.0f,  75.0f);
    /* Horizontal beam */
    SDL_RenderLine(rend, 80.0f,  75.0f, 250.0f,  75.0f);
    /* Diagonal brace */
    SDL_RenderLine(rend, 80.0f, 125.0f, 120.0f,  75.0f);
    /* Rope */
    SDL_RenderLine(rend, 250.0f,  75.0f, 250.0f, 115.0f);

    /* Gallows border line between panels */
    SDL_SetRenderDrawColor(rend, 50, 50, 70, 255);
    SDL_RenderLine(rend, (float)PANEL_DIV_X, (float)HEADER_H, (float)PANEL_DIV_X, (float)GAME_AREA_BOT);

    if (stage < 1) return;

    /* === Stage 1: Head === */
    SDL_SetRenderDrawColor(rend, 240, 200, 160, 255);   /* skin tone */
    draw_thick_circle(rend, 250, 137, 22);

    if (stage < 2) return;

    /* === Stage 2: Body === */
    SDL_SetRenderDrawColor(rend, 200, 200, 210, 255);
    SDL_RenderLine(rend, 250.0f, 159.0f, 250.0f, 240.0f);
    SDL_RenderLine(rend, 251.0f, 159.0f, 251.0f, 240.0f);   /* 2px wide */

    if (stage < 3) return;

    /* === Stage 3: Left arm === */
    SDL_RenderLine(rend, 250.0f, 185.0f, 205.0f, 225.0f);
    SDL_RenderLine(rend, 251.0f, 185.0f, 206.0f, 225.0f);

    if (stage < 4) return;

    /* === Stage 4: Right arm === */
    SDL_RenderLine(rend, 250.0f, 185.0f, 295.0f, 225.0f);
    SDL_RenderLine(rend, 251.0f, 185.0f, 296.0f, 225.0f);

    if (stage < 5) return;

    /* === Stage 5: Left leg === */
    SDL_RenderLine(rend, 250.0f, 240.0f, 205.0f, 310.0f);
    SDL_RenderLine(rend, 251.0f, 240.0f, 206.0f, 310.0f);

    if (stage < 6) return;

    /* === Stage 6: Right leg === */
    SDL_RenderLine(rend, 250.0f, 240.0f, 295.0f, 310.0f);
    SDL_RenderLine(rend, 251.0f, 240.0f, 296.0f, 310.0f);
}

/* ── Word display (right panel top) ─────────────────────────────────── */
void render_word_display(Renderer *r, const Game *g)
{
    const int PANEL_X  = PANEL_DIV_X + 10;
    const int PANEL_W  = WIN_W - PANEL_DIV_X - 20;   /* 390px */
    const int CHAR_W   = 32;
    const int CHAR_GAP = 6;

    render_text(r, r->font_small, "GUESS THE WORD:", PANEL_X, 65, COL_GRAY);

    int len = (int)strlen(g->masked);
    int total_w = len * CHAR_W + (len - 1) * CHAR_GAP;
    int start_x = PANEL_DIV_X + (WIN_W - PANEL_DIV_X - total_w) / 2;
    if (start_x < PANEL_X) start_x = PANEL_X;

    int y_letter = 105;
    int i;
    for (i = 0; i < len; i++) {
        int x = start_x + i * (CHAR_W + CHAR_GAP);
        if (g->masked[i] == '_') {
            /* Underscore bar */
            render_filled_rect(r, x, y_letter + 34, CHAR_W, 3, COL_GRAY);
        } else {
            char ch[2] = {g->masked[i], '\0'};
            int tw = 0, th = 0;
            TTF_GetStringSize(r->font_medium, ch, 0, &tw, &th);
            render_text(r, r->font_medium, ch,
                        x + (CHAR_W - tw) / 2, y_letter, COL_WHITE);
        }
    }

    /* Word length hint: "X letters" */
    char hint[32];
    snprintf(hint, sizeof(hint), "%d letters", len);
    render_text_centered(r, r->font_small, hint,
                         PANEL_DIV_X + (WIN_W - PANEL_DIV_X) / 2,
                         160, COL_GRAY);
}

/* ── Status panel (right panel middle) ──────────────────────────────── */
void render_status(Renderer *r, const Game *g)
{
    const int PX = PANEL_DIV_X + 15;
    int chances_left = g->max_chances - g->wrong_count;

    /* Chances bar */
    render_text(r, r->font_small, "Chances:", PX, 200, COL_WHITE);
    int i;
    for (i = 0; i < g->max_chances; i++) {
        SDL_Color c = (i < chances_left) ? COL_GREEN : COL_WRONG_KEY;
        render_filled_rect(r, PX + 100 + i * 24, 202, 18, 16, c);
        render_rect_outline(r, PX + 100 + i * 24, 202, 18, 16, COL_GRAY);
    }

    /* Wrong letters */
    render_text(r, r->font_small, "Wrong:", PX, 232, COL_WHITE);
    int col = 0;
    for (i = 0; i < 26; i++) {
        if (!g->guessed[i]) continue;
        /* Only show letters NOT in the word */
        int in_word = 0, j;
        for (j = 0; g->word[j]; j++) {
            if (g->word[j] == 'A' + i) { in_word = 1; break; }
        }
        if (in_word) continue;
        char ch[2] = {'A' + (char)i, '\0'};
        render_text(r, r->font_small, ch,
                    PX + 80 + col * 22, 232, COL_ACCENT);
        col++;
    }
}

/* ── On-screen keyboard ──────────────────────────────────────────────── */
void render_keyboard(Renderer *r, const Game *g)
{
    int row;
    for (row = 0; row < 3; row++) {
        const char *keys = KB_ROWS[row];
        int n       = (int)strlen(keys);
        int start_x = KB_ROW_X[row];
        int y       = KB_ROW_Y[row];
        int col;

        for (col = 0; col < n; col++) {
            char key = keys[col];
            int  idx = key - 'A';
            int  x   = start_x + col * (KB_KEY_W + KB_KEY_GAP);

            SDL_Color bg, fg;

            if (!g->guessed[idx]) {
                bg = COL_KEY_BG;
                fg = COL_WHITE;
            } else {
                int in_word = 0, j;
                for (j = 0; g->word[j]; j++) {
                    if (g->word[j] == key) { in_word = 1; break; }
                }
                bg = in_word ? COL_GREEN : COL_WRONG_KEY;
                fg = in_word ? COL_WHITE : COL_GRAY;
            }

            render_filled_rect(r, x, y, KB_KEY_W, KB_KEY_H, bg);
            render_rect_outline(r, x, y, KB_KEY_W, KB_KEY_H, COL_GRAY);

            char label[2] = {key, '\0'};
            int tw = 0, th = 0;
            TTF_GetStringSize(r->font_small, label, 0, &tw, &th);
            render_text(r, r->font_small, label,
                        x + (KB_KEY_W - tw) / 2,
                        y + (KB_KEY_H - th) / 2, fg);
        }
    }
}

/* ── Keyboard hit test ───────────────────────────────────────────────── */
char render_keyboard_key_at(int mx, int my)
{
    int row;
    for (row = 0; row < 3; row++) {
        int y = KB_ROW_Y[row];
        if (my < y || my > y + KB_KEY_H) continue;

        const char *keys = KB_ROWS[row];
        int n       = (int)strlen(keys);
        int start_x = KB_ROW_X[row];
        int col;
        for (col = 0; col < n; col++) {
            int x = start_x + col * (KB_KEY_W + KB_KEY_GAP);
            if (mx >= x && mx <= x + KB_KEY_W)
                return keys[col];
        }
    }
    return 0;
}
