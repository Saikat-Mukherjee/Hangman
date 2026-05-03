#ifndef UI_H
#define UI_H

#include <SDL3/SDL.h>
#include "render.h"

/* ── Button ─────────────────────────────────────────────────────────── */
typedef struct {
    SDL_Rect   rect;
    char       label[64];
    int        hovered;
    int        visible;
    SDL_Color  col_normal;
    SDL_Color  col_hover;
    SDL_Color  col_text;
} Button;

void button_init(Button *b, int x, int y, int w, int h,
                 const char *label, SDL_Color normal, SDL_Color hover);
void button_draw(Button *b, Renderer *r, TTF_Font *font);
int  button_hit(const Button *b, int mx, int my);
void button_update_hover(Button *b, int mx, int my);

/* ── TextInput ───────────────────────────────────────────────────────
   Handles SDL_TEXTINPUT events for player-name and MP-word entry.
   When hide_text=1 the buffer is displayed as '***'.               */
typedef struct {
    char buffer[64];
    int  len;
    int  hide_text;     /* 1 = mask with * */
    int  max_len;
} TextInput;

void text_input_init(TextInput *t, int hide_text, int max_len);
void text_input_handle_text(TextInput *t, const char *sdl_text);
void text_input_handle_key(TextInput *t, SDL_Keycode key);
void text_input_draw(TextInput *t, Renderer *r, int x, int y, int w,
                     TTF_Font *font, SDL_Color fg);
/* Draw a labelled input box (label above, border box, blinking cursor) */
void text_input_draw_box(TextInput *t, Renderer *r, int x, int y, int w,
                         const char *label, TTF_Font *font_label,
                         TTF_Font *font_text, SDL_Color label_col);

#endif /* UI_H */
