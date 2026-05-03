#include "ui.h"
#include <string.h>

/* ── Button ─────────────────────────────────────────────────────────── */

void button_init(Button *b, int x, int y, int w, int h,
                 const char *label, SDL_Color normal, SDL_Color hover)
{
    b->rect       = (SDL_Rect){x, y, w, h};
    b->hovered    = 0;
    b->visible    = 1;
    b->col_normal = normal;
    b->col_hover  = hover;
    b->col_text   = COL_WHITE;
    strncpy(b->label, label, sizeof(b->label) - 1);
    b->label[sizeof(b->label) - 1] = '\0';
}

void button_draw(Button *b, Renderer *r, TTF_Font *font)
{
    if (!b->visible) return;

    SDL_Color bg = b->hovered ? b->col_hover : b->col_normal;
    render_filled_rect(r, b->rect.x, b->rect.y,
                       b->rect.w, b->rect.h, bg);
    render_rect_outline(r, b->rect.x, b->rect.y,
                        b->rect.w, b->rect.h, COL_GRAY);

    render_text_centered(r, font, b->label,
                         b->rect.x + b->rect.w / 2,
                         b->rect.y + (b->rect.h -
                             TTF_GetFontHeight(font)) / 2,
                         b->col_text);
}

int button_hit(const Button *b, int mx, int my)
{
    if (!b->visible) return 0;
    return (mx >= b->rect.x && mx <= b->rect.x + b->rect.w &&
            my >= b->rect.y && my <= b->rect.y + b->rect.h);
}

void button_update_hover(Button *b, int mx, int my)
{
    b->hovered = button_hit(b, mx, my);
}

/* ── TextInput ───────────────────────────────────────────────────────── */

void text_input_init(TextInput *t, int hide_text, int max_len)
{
    memset(t->buffer, 0, sizeof(t->buffer));
    t->len       = 0;
    t->hide_text = hide_text;
    t->max_len   = (max_len > 0 && max_len < 63) ? max_len : 63;
}

void text_input_handle_text(TextInput *t, const char *sdl_text)
{
    /* Accept only printable ASCII single-byte characters */
    if (!sdl_text || sdl_text[1] != '\0') return;   /* multi-byte: skip */
    char c = sdl_text[0];
    if (c < 32 || c > 126) return;
    if (t->len >= t->max_len) return;
    t->buffer[t->len++] = c;
    t->buffer[t->len]   = '\0';
}

void text_input_handle_key(TextInput *t, SDL_Keycode key)
{
    if (key == SDLK_BACKSPACE && t->len > 0) {
        t->buffer[--t->len] = '\0';
    }
}

/* Draw just the text (used inline) */
void text_input_draw(TextInput *t, Renderer *r, int x, int y, int w,
                     TTF_Font *font, SDL_Color fg)
{
    char display[64];
    if (t->hide_text) {
        int i;
        for (i = 0; i < t->len; i++) display[i] = '*';
        display[t->len] = '\0';
    } else {
        strncpy(display, t->buffer, sizeof(display));
    }

    /* Append blinking cursor (use SDL ticks) */
    int blink = ((SDL_GetTicks() / 500) % 2);
    if (blink && t->len < 63) {
        display[t->len]     = '|';
        display[t->len + 1] = '\0';
    }

    render_text(r, font, display, x + 6, y, fg);
    (void)w;
}

/* Draw a labelled input box */
void text_input_draw_box(TextInput *t, Renderer *r, int x, int y, int w,
                         const char *label, TTF_Font *font_label,
                         TTF_Font *font_text, SDL_Color label_col)
{
    int box_h = TTF_GetFontHeight(font_text) + 12;

    render_text(r, font_label, label, x, y, label_col);

    int box_y = y + TTF_GetFontHeight(font_label) + 4;
    render_filled_rect(r, x, box_y, w, box_h,
                       (SDL_Color){35, 35, 55, 255});
    render_rect_outline(r, x, box_y, w, box_h, COL_ACCENT);

    text_input_draw(t, r, x, box_y + 6, w, font_text, COL_WHITE);
}
