/*
 * src/main.c  —  Hangman SDL2 application entry point.
 *
 * State machine:
 *   MAIN_MENU -> CATEGORY_SELECT -> DIFFICULTY_SELECT -> PLAYING
 *   PLAYING   -> WIN | LOSE -> HIGH_SCORES
 *   MAIN_MENU -> MULTIPLAYER_INPUT -> PLAYING -> WIN | LOSE
 *   any       -> MAIN_MENU (back / quit)
 */

#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <SDL3/SDL_ttf.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "game.h"
#include "render.h"
#include "ui.h"
#include "words.h"
#include "scores.h"

/* ═══════════════════════════════════════════════════════════════════════
   Global application data
   ═══════════════════════════════════════════════════════════════════════ */
static AppState   g_state   = STATE_MAIN_MENU;
static Game       g_game;
static WordList   g_words;
static ScoreBoard g_scores;
static Renderer   g_r;
static SDL_Window *g_window = NULL;

/* Pending selections while navigating menus */
static char g_sel_category[MAX_CAT_LEN] = "mixed";
static int  g_sel_difficulty             = 1;   /* default Medium */

/* Name entry / mp-word entry */
static TextInput g_name_input;
static TextInput g_mp_word_input;
static int       g_score_saved = 0;

/* ── Button banks ─────────────────────────────────────────────────── */
/* Main menu */
static Button btn_play, btn_multiplayer, btn_scores, btn_quit;
/* Category select */
static Button btn_cat[4], btn_cat_back;
/* Difficulty select */
static Button btn_diff[3], btn_diff_back;
/* Playing */
static Button btn_hint, btn_play_menu;
/* End screen */
static Button btn_save_score, btn_play_again, btn_end_menu;
/* High scores */
static Button btn_hs_back;
/* Multiplayer input */
static Button btn_mp_start, btn_mp_back;

/* ═══════════════════════════════════════════════════════════════════════
   Button factory helpers
   ═══════════════════════════════════════════════════════════════════════ */
static SDL_Color DARK   = {40,  40,  65,  255};
static SDL_Color DARKER = {25,  25,  45,  255};

static void init_button(Button *b, int x, int y, int w, int h,
                         const char *label, SDL_Color normal, SDL_Color hover)
{
    button_init(b, x, y, w, h, label, normal, hover);
}

static void big_btn(Button *b, int x, int y, const char *label,
                    SDL_Color col, SDL_Color hov)
{
    init_button(b, x, y, 220, 55, label, col, hov);
}

/* ═══════════════════════════════════════════════════════════════════════
   Button initialisation per state
   ═══════════════════════════════════════════════════════════════════════ */
static void init_main_menu_btns(void)
{
    SDL_Color n = {55, 55, 90, 255}, h = {80, 80, 130, 255};
    big_btn(&btn_play,        290, 210, "PLAY",         n, h);
    big_btn(&btn_multiplayer, 290, 280, "MULTIPLAYER",  n, h);
    big_btn(&btn_scores,      290, 350, "HIGH SCORES",  n, h);
    big_btn(&btn_quit,        290, 420, "QUIT",
            (SDL_Color){80,30,30,255}, (SDL_Color){120,45,45,255});
}

static void init_category_btns(void)
{
    SDL_Color n = {45, 80, 115, 255}, h = {60, 110, 160, 255};
    init_button(&btn_cat[0], 80,  200, 170, 80, "ANIMALS",   n, h);
    init_button(&btn_cat[1], 280, 200, 170, 80, "COUNTRIES", n, h);
    init_button(&btn_cat[2], 480, 200, 170, 80, "TECH",      n, h);
    init_button(&btn_cat[3], 280, 310, 170, 80, "MIXED",     n, h);
    init_button(&btn_cat_back, 30, 540, 110, 40, "BACK",    DARK, DARKER);
}

static void init_difficulty_btns(void)
{
    init_button(&btn_diff[0],  60, 200, 180, 120, "EASY",
                (SDL_Color){30,100,50,255}, (SDL_Color){40,140,65,255});
    init_button(&btn_diff[1], 310, 200, 180, 120, "MEDIUM",
                (SDL_Color){130,100,20,255}, (SDL_Color){180,140,25,255});
    init_button(&btn_diff[2], 560, 200, 180, 120, "HARD",
                (SDL_Color){130,30,30,255}, (SDL_Color){180,45,45,255});
    init_button(&btn_diff_back, 30, 540, 110, 40, "BACK", DARK, DARKER);
}

static void init_playing_btns(void)
{
    init_button(&btn_hint,
                PANEL_DIV_X + 65, 285, 160, 42, "USE HINT",
                (SDL_Color){100,80,20,255}, (SDL_Color){140,110,25,255});
    init_button(&btn_play_menu, WIN_W - 100, WIN_H - 30, 90, 24,
                "MENU", DARK, DARKER);
}

static void init_end_btns(void)
{
    SDL_Color n = {55,55,90,255}, h = {80,80,130,255};
    init_button(&btn_save_score, 220, 420, 190, 48, "SAVE SCORE",
                (SDL_Color){30,100,50,255}, (SDL_Color){40,140,65,255});
    init_button(&btn_play_again, 420, 420, 160, 48, "PLAY AGAIN", n, h);
    init_button(&btn_end_menu,   620, 530, 140, 40, "MAIN MENU", DARK, DARKER);
}

static void init_hs_btns(void)
{
    init_button(&btn_hs_back, WIN_W/2 - 80, 540, 160, 44,
                "MAIN MENU", DARK, DARKER);
}

static void init_mp_btns(void)
{
    SDL_Color n = {30,100,50,255}, h = {40,140,65,255};
    init_button(&btn_mp_start, WIN_W/2 - 100, 390, 200, 52,
                "START GAME", n, h);
    init_button(&btn_mp_back, 30, 540, 110, 40, "BACK", DARK, DARKER);
}

/* ═══════════════════════════════════════════════════════════════════════
   State transitions
   ═══════════════════════════════════════════════════════════════════════ */
static void start_game(const char *category, int difficulty,
                        const char *word_override)
{
    const char *word = word_override;

    if (!word_override) {
        if (words_load(&g_words, category) == 0) {
            /* Fallback to mixed if category file missing */
            if (words_load(&g_words, "mixed") == 0) return;
        }
        word = words_pick(&g_words, difficulty);
        if (!word) return;
    }

    game_init(&g_game, word, difficulty, category,
              word_override ? 1 : 0);

    init_playing_btns();
    g_state = STATE_PLAYING;
    SDL_StopTextInput(g_window);
}

static void transition_to(AppState next)
{
    switch (next) {
    case STATE_MAIN_MENU:
        init_main_menu_btns();
        SDL_StopTextInput(g_window);
        break;
    case STATE_CATEGORY_SELECT:
        init_category_btns();
        SDL_StopTextInput(g_window);
        break;
    case STATE_DIFFICULTY_SELECT:
        init_difficulty_btns();
        SDL_StopTextInput(g_window);
        break;
    case STATE_WIN:
    case STATE_LOSE:
        init_end_btns();
        g_score_saved = 0;
        text_input_init(&g_name_input, 0, 20);
        SDL_StartTextInput(g_window);
        break;
    case STATE_HIGH_SCORES:
        init_hs_btns();
        scores_load(&g_scores);   /* reload after any save */
        SDL_StopTextInput(g_window);
        break;
    case STATE_MULTIPLAYER_INPUT:
        init_mp_btns();
        text_input_init(&g_mp_word_input, 1, 20);
        SDL_StartTextInput(g_window);
        break;
    default:
        break;
    }
    g_state = next;
}

/* ── Post-guess logic shared by keyboard-click and keydown paths ────── */
static void handle_guess(char letter)
{
    GuessResult res = game_guess(&g_game, letter);
    if (res == GUESS_WIN)  { transition_to(STATE_WIN);  }
    if (res == GUESS_LOSE) { transition_to(STATE_LOSE); }
}

static void handle_hint(void)
{
    int ok = game_hint(&g_game);
    if (ok && strcmp(g_game.masked, g_game.word) == 0) {
        g_game.score = game_compute_score(&g_game);
        transition_to(STATE_WIN);
    }
    /* If hint caused a loss (edge case): max_chances reached */
    if (ok && g_game.wrong_count >= g_game.max_chances)
        transition_to(STATE_LOSE);
}

static void save_score(void)
{
    if (g_score_saved || g_name_input.len == 0) return;

    ScoreEntry e;
    memset(&e, 0, sizeof(e));
    strncpy(e.name,     g_name_input.buffer, sizeof(e.name)-1);
    strncpy(e.word,     g_game.word,         sizeof(e.word)-1);
    strncpy(e.category, g_game.category,     sizeof(e.category)-1);
    e.difficulty = g_game.difficulty;
    e.score      = (g_state == STATE_WIN) ? g_game.score : 0;

    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(e.date, sizeof(e.date), "%Y-%m-%d", tm_info);

    scores_add(&g_scores, &e);
    scores_save(&g_scores);
    g_score_saved = 1;
}

/* ═══════════════════════════════════════════════════════════════════════
   Event handling (one dispatcher per state)
   ═══════════════════════════════════════════════════════════════════════ */
static void on_event(SDL_Event *e)
{
    float fmx = 0.0f, fmy = 0.0f;
    SDL_GetMouseState(&fmx, &fmy);
    int mx = (int)fmx, my = (int)fmy;

    /* ── Universal hover update ─────────────────────────────────── */
#define UPD(b) button_update_hover(&(b), mx, my)

    /* ── SDL_QUIT ────────────────────────────────────────────────── */
    if (e->type == SDL_EVENT_QUIT) { g_state = STATE_QUIT; return; }

    /* ── SDL_TEXTINPUT (name / mp word) ──────────────────────────── */
    if (e->type == SDL_EVENT_TEXT_INPUT) {
        if (g_state == STATE_WIN || g_state == STATE_LOSE)
            text_input_handle_text(&g_name_input, e->text.text);
        if (g_state == STATE_MULTIPLAYER_INPUT)
            text_input_handle_text(&g_mp_word_input, e->text.text);
        return;
    }

    /* ── SDL_KEYDOWN ─────────────────────────────────────────────── */
    if (e->type == SDL_EVENT_KEY_DOWN) {
        SDL_Keycode k = e->key.key;

        if (k == SDLK_BACKSPACE) {
            if (g_state == STATE_WIN || g_state == STATE_LOSE)
                text_input_handle_key(&g_name_input, k);
            if (g_state == STATE_MULTIPLAYER_INPUT)
                text_input_handle_key(&g_mp_word_input, k);
        }

        /* ESC → main menu */
        if (k == SDLK_ESCAPE) {
            if (g_state != STATE_MAIN_MENU)
                transition_to(STATE_MAIN_MENU);
            return;
        }

        /* Letter keys during gameplay */
        if (g_state == STATE_PLAYING) {
            if (k >= SDLK_A && k <= SDLK_Z) {
                handle_guess((char)('A' + (k - SDLK_A)));
            }
        }

        /* Enter confirms name input or MP word */
        if (k == SDLK_RETURN) {
            if ((g_state == STATE_WIN || g_state == STATE_LOSE)
                && g_name_input.len > 0 && !g_score_saved) {
                save_score();
                transition_to(STATE_HIGH_SCORES);
            }
            if (g_state == STATE_MULTIPLAYER_INPUT
                && g_mp_word_input.len >= 3) {
                start_game("multiplayer", g_sel_difficulty,
                           g_mp_word_input.buffer);
            }
        }
        return;
    }

    /* ── SDL_MOUSEMOTION ─────────────────────────────────────────── */
    if (e->type == SDL_EVENT_MOUSE_MOTION) {
        switch (g_state) {
        case STATE_MAIN_MENU:
            UPD(btn_play); UPD(btn_multiplayer);
            UPD(btn_scores); UPD(btn_quit); break;
        case STATE_CATEGORY_SELECT:
            for (int i = 0; i < 4; i++) UPD(btn_cat[i]);
            UPD(btn_cat_back); break;
        case STATE_DIFFICULTY_SELECT:
            for (int i = 0; i < 3; i++) UPD(btn_diff[i]);
            UPD(btn_diff_back); break;
        case STATE_PLAYING:
            UPD(btn_hint); UPD(btn_play_menu); break;
        case STATE_WIN: case STATE_LOSE:
            UPD(btn_save_score); UPD(btn_play_again);
            UPD(btn_end_menu); break;
        case STATE_HIGH_SCORES:
            UPD(btn_hs_back); break;
        case STATE_MULTIPLAYER_INPUT:
            UPD(btn_mp_start); UPD(btn_mp_back); break;
        default: break;
        }
        return;
    }

    /* ── SDL_MOUSEBUTTONDOWN ─────────────────────────────────────── */
    if (e->type == SDL_EVENT_MOUSE_BUTTON_DOWN && e->button.button == SDL_BUTTON_LEFT) {
        int bx = (int)e->button.x, by = (int)e->button.y;

        switch (g_state) {

        case STATE_MAIN_MENU:
            if (button_hit(&btn_play, bx, by))
                transition_to(STATE_CATEGORY_SELECT);
            else if (button_hit(&btn_multiplayer, bx, by))
                transition_to(STATE_MULTIPLAYER_INPUT);
            else if (button_hit(&btn_scores, bx, by))
                transition_to(STATE_HIGH_SCORES);
            else if (button_hit(&btn_quit, bx, by))
                g_state = STATE_QUIT;
            break;

        case STATE_CATEGORY_SELECT: {
            static const char *cats[] =
                {"animals","countries","tech","mixed"};
            for (int i = 0; i < 4; i++) {
                if (button_hit(&btn_cat[i], bx, by)) {
                    strncpy(g_sel_category, cats[i],
                            sizeof(g_sel_category)-1);
                    transition_to(STATE_DIFFICULTY_SELECT);
                    break;
                }
            }
            if (button_hit(&btn_cat_back, bx, by))
                transition_to(STATE_MAIN_MENU);
            break;
        }

        case STATE_DIFFICULTY_SELECT:
            for (int i = 0; i < 3; i++) {
                if (button_hit(&btn_diff[i], bx, by)) {
                    g_sel_difficulty = i;
                    start_game(g_sel_category, g_sel_difficulty, NULL);
                    break;
                }
            }
            if (button_hit(&btn_diff_back, bx, by))
                transition_to(STATE_CATEGORY_SELECT);
            break;

        case STATE_PLAYING: {
            /* On-screen keyboard click */
            char key = render_keyboard_key_at(bx, by);
            if (key) { handle_guess(key); break; }
            if (button_hit(&btn_hint, bx, by))     handle_hint();
            if (button_hit(&btn_play_menu, bx, by)) transition_to(STATE_MAIN_MENU);
            break;
        }

        case STATE_WIN:
        case STATE_LOSE:
            if (button_hit(&btn_save_score, bx, by) && !g_score_saved
                && g_name_input.len > 0) {
                save_score();
                transition_to(STATE_HIGH_SCORES);
            }
            if (button_hit(&btn_play_again, bx, by))
                transition_to(STATE_CATEGORY_SELECT);
            if (button_hit(&btn_end_menu, bx, by))
                transition_to(STATE_MAIN_MENU);
            break;

        case STATE_HIGH_SCORES:
            if (button_hit(&btn_hs_back, bx, by))
                transition_to(STATE_MAIN_MENU);
            break;

        case STATE_MULTIPLAYER_INPUT:
            if (button_hit(&btn_mp_start, bx, by)
                && g_mp_word_input.len >= 3) {
                start_game("multiplayer", g_sel_difficulty,
                           g_mp_word_input.buffer);
            }
            if (button_hit(&btn_mp_back, bx, by))
                transition_to(STATE_MAIN_MENU);
            break;

        default: break;
        }
    }
#undef UPD
}

/* ═══════════════════════════════════════════════════════════════════════
   Rendering
   ═══════════════════════════════════════════════════════════════════════ */

/* ── Shared: draw a semi-transparent overlay ──────────────────────── */
static void draw_overlay(Renderer *r, Uint8 alpha)
{
    SDL_SetRenderDrawBlendMode(r->renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r->renderer, 0, 0, 0, alpha);
    SDL_FRect full = {0.0f, 0.0f, (float)WIN_W, (float)WIN_H};
    SDL_RenderFillRect(r->renderer, &full);
    SDL_SetRenderDrawBlendMode(r->renderer, SDL_BLENDMODE_NONE);
}

static void render_main_menu(void)
{
    render_clear(&g_r);
    /* Decorative partial gallows */
    render_gallows(&g_r, 3);

    render_text_centered(&g_r, g_r.font_large, "H A N G M A N",
                         WIN_W / 2, 110, COL_ACCENT);
    render_text_centered(&g_r, g_r.font_small,
                         "A Production SDL2 Game",
                         WIN_W / 2, 162, COL_GRAY);

    button_draw(&btn_play,        &g_r, g_r.font_medium);
    button_draw(&btn_multiplayer, &g_r, g_r.font_medium);
    button_draw(&btn_scores,      &g_r, g_r.font_medium);
    button_draw(&btn_quit,        &g_r, g_r.font_medium);
}

static void render_category_select(void)
{
    render_clear(&g_r);
    render_text_centered(&g_r, g_r.font_large, "SELECT CATEGORY",
                         WIN_W / 2, 110, COL_WHITE);

    static const char *descs[] = {
        "Animals of the world",
        "Countries & nations",
        "Technology terms",
        "Mixed vocabulary"
    };
    for (int i = 0; i < 4; i++) {
        button_draw(&btn_cat[i], &g_r, g_r.font_medium);
        render_text_centered(&g_r, g_r.font_small, descs[i],
                             btn_cat[i].rect.x + btn_cat[i].rect.w / 2,
                             btn_cat[i].rect.y + btn_cat[i].rect.h + 4,
                             COL_GRAY);
    }
    button_draw(&btn_cat_back, &g_r, g_r.font_small);
}

static void render_difficulty_select(void)
{
    render_clear(&g_r);
    render_text_centered(&g_r, g_r.font_large, "SELECT DIFFICULTY",
                         WIN_W / 2, 110, COL_WHITE);

    static const char *info[3][3] = {
        {"7 chances", "40% blanked", "Score x1"},
        {"5 chances", "60% blanked", "Score x2"},
        {"3 chances", "80% blanked", "Score x3"},
    };
    for (int i = 0; i < 3; i++) {
        button_draw(&btn_diff[i], &g_r, g_r.font_medium);
        int bx = btn_diff[i].rect.x + btn_diff[i].rect.w / 2;
        int by = btn_diff[i].rect.y + btn_diff[i].rect.h;
        for (int j = 0; j < 3; j++)
            render_text_centered(&g_r, g_r.font_small, info[i][j],
                                 bx, by + 2 + j * 20, COL_GRAY);
    }
    button_draw(&btn_diff_back, &g_r, g_r.font_small);
}

static void render_playing_screen(void)
{
    render_clear(&g_r);
    render_header(&g_r, &g_game);
    render_gallows(&g_r, game_stage(&g_game));
    render_word_display(&g_r, &g_game);
    render_status(&g_r, &g_game);
    render_keyboard(&g_r, &g_game);

    /* Hint button — disabled when not enough chances remain */
    int chances_left = g_game.max_chances - g_game.wrong_count;
    btn_hint.visible = (chances_left > 2) ? 1 : 0;
    button_draw(&btn_hint, &g_r, g_r.font_small);

    /* Small "MENU" button */
    button_draw(&btn_play_menu, &g_r, g_r.font_small);
}

static void render_end_screen(int is_win)
{
    render_playing_screen();   /* show game state underneath */
    draw_overlay(&g_r, 185);

    /* Card */
    render_filled_rect(&g_r, 140, 110, 520, 420,
                       (SDL_Color){22,22,44,255});
    render_rect_outline(&g_r, 140, 110, 520, 420, COL_ACCENT);

    /* Result text */
    const char *result = is_win ? "YOU WIN!" : "GAME OVER";
    SDL_Color   rcol   = is_win ? COL_GREEN : COL_ACCENT;
    render_text_centered(&g_r, g_r.font_large, result, WIN_W/2, 130, rcol);

    /* Word reveal */
    char reveal[80];
    if (is_win) {
        snprintf(reveal, sizeof(reveal), "Word: %s", g_game.word);
        render_text_centered(&g_r, g_r.font_medium, reveal,
                             WIN_W/2, 195, COL_WHITE);
        char sc[32];
        snprintf(sc, sizeof(sc), "Score: %d", g_game.score);
        render_text_centered(&g_r, g_r.font_medium, sc,
                             WIN_W/2, 240, COL_YELLOW);
    } else {
        snprintf(reveal, sizeof(reveal), "The word was: %s", g_game.word);
        render_text_centered(&g_r, g_r.font_medium, reveal,
                             WIN_W/2, 195, COL_WHITE);
    }

    /* Name input */
    render_text_centered(&g_r, g_r.font_small,
                         g_score_saved ? "Score saved!" : "Enter your name:",
                         WIN_W/2, 295, COL_GRAY);
    if (!g_score_saved) {
        text_input_draw_box(&g_name_input, &g_r,
                            WIN_W/2 - 150, 318, 300,
                            "",
                            g_r.font_small, g_r.font_medium, COL_GRAY);
    }

    button_draw(&btn_save_score, &g_r, g_r.font_small);
    button_draw(&btn_play_again, &g_r, g_r.font_small);
    button_draw(&btn_end_menu,   &g_r, g_r.font_small);
}

static void render_high_scores(void)
{
    render_clear(&g_r);
    render_text_centered(&g_r, g_r.font_large, "HIGH SCORES",
                         WIN_W/2, 30, COL_YELLOW);

    /* Table header */
    static const char *hdr[] = {"#","Name","Word","Cat","Diff","Score","Date"};
    static const int   hx[]  = {20, 55, 190, 320, 400, 460, 560};
    for (int c = 0; c < 7; c++)
        render_text(&g_r, g_r.font_small, hdr[c], hx[c], 90, COL_ACCENT);

    SDL_SetRenderDrawColor(g_r.renderer,
                           COL_ACCENT.r, COL_ACCENT.g, COL_ACCENT.b, 255);
    SDL_RenderLine(g_r.renderer, 10.0f, 112.0f, (float)(WIN_W - 10), 112.0f);

    ScoreEntry top[10];
    int n = scores_top(&g_scores, top, 10);
    for (int i = 0; i < n; i++) {
        int y = 122 + i * 36;
        SDL_Color row_col = (i % 2 == 0)
            ? (SDL_Color){30,30,50,255} : (SDL_Color){25,25,42,255};
        render_filled_rect(&g_r, 10, y - 2, WIN_W - 20, 34, row_col);

        char rank[4]; snprintf(rank, sizeof(rank), "%d", i+1);
        render_text(&g_r, g_r.font_small, rank,            hx[0], y, COL_GRAY);
        render_text(&g_r, g_r.font_small, top[i].name,     hx[1], y, COL_WHITE);
        render_text(&g_r, g_r.font_small, top[i].word,     hx[2], y, COL_WHITE);
        render_text(&g_r, g_r.font_small, top[i].category, hx[3], y, COL_GRAY);
        render_text(&g_r, g_r.font_small,
                    scores_difficulty_name(top[i].difficulty), hx[4], y, COL_GRAY);
        char sc[8]; snprintf(sc, sizeof(sc), "%d", top[i].score);
        render_text(&g_r, g_r.font_small, sc,           hx[5], y, COL_YELLOW);
        render_text(&g_r, g_r.font_small, top[i].date,  hx[6], y, COL_GRAY);
    }
    if (n == 0)
        render_text_centered(&g_r, g_r.font_medium,
                             "No scores yet. Play a game!",
                             WIN_W/2, 260, COL_GRAY);

    button_draw(&btn_hs_back, &g_r, g_r.font_medium);
}

static void render_multiplayer_input(void)
{
    render_clear(&g_r);
    render_text_centered(&g_r, g_r.font_large, "MULTIPLAYER",
                         WIN_W/2, 60, COL_ACCENT);
    render_text_centered(&g_r, g_r.font_medium,
                         "Player 1: type a secret word",
                         WIN_W/2, 130, COL_WHITE);
    render_text_centered(&g_r, g_r.font_small,
                         "(Player 2 must not look!)",
                         WIN_W/2, 172, COL_GRAY);

    /* Difficulty mini-selector */
    render_text_centered(&g_r, g_r.font_small, "Difficulty:",
                         WIN_W/2, 240, COL_GRAY);
    static const char *dlabels[] = {"Easy","Medium","Hard"};
    for (int i = 0; i < 3; i++) {
        SDL_Color c = (g_sel_difficulty == i)
                      ? COL_ACCENT : (SDL_Color){55,55,90,255};
        render_filled_rect(&g_r, WIN_W/2 - 150 + i*102, 262, 96, 30, c);
        render_text_centered(&g_r, g_r.font_small, dlabels[i],
                             WIN_W/2 - 102 + i*102, 270, COL_WHITE);
    }

    /* Word input box */
    text_input_draw_box(&g_mp_word_input, &g_r,
                        WIN_W/2 - 160, 320, 320,
                        "Secret word (min 3 letters):",
                        g_r.font_small, g_r.font_medium, COL_GRAY);

    button_draw(&btn_mp_start, &g_r, g_r.font_medium);
    button_draw(&btn_mp_back,  &g_r, g_r.font_small);
}

static void render_frame(void)
{
    switch (g_state) {
    case STATE_MAIN_MENU:          render_main_menu();          break;
    case STATE_CATEGORY_SELECT:    render_category_select();    break;
    case STATE_DIFFICULTY_SELECT:  render_difficulty_select();  break;
    case STATE_PLAYING:            render_playing_screen();     break;
    case STATE_WIN:                render_end_screen(1);        break;
    case STATE_LOSE:               render_end_screen(0);        break;
    case STATE_HIGH_SCORES:        render_high_scores();        break;
    case STATE_MULTIPLAYER_INPUT:  render_multiplayer_input();  break;
    default: break;
    }
    render_present(&g_r);
}

/* ── Difficulty click in MP screen ───────────────────────────────── */
static void mp_check_diff_click(int bx, int by)
{
    for (int i = 0; i < 3; i++) {
        int rx = WIN_W/2 - 150 + i*102;
        if (bx >= rx && bx <= rx + 96 && by >= 262 && by <= 292)
            g_sel_difficulty = i;
    }
}

/* ═══════════════════════════════════════════════════════════════════════
   Entry point
   ═══════════════════════════════════════════════════════════════════════ */
int main(int argc, char *argv[])
{
    (void)argc; (void)argv;
    srand((unsigned int)time(NULL));

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return 1;
    }
    if (!TTF_Init()) {
        fprintf(stderr, "TTF_Init: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Hangman", WIN_W, WIN_H, 0);
    if (!window) {
        fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError());
        TTF_Quit(); SDL_Quit(); return 1;
    }
    g_window = window;

    SDL_Renderer *sdl_r = SDL_CreateRenderer(window, NULL);
    if (!sdl_r) {
        fprintf(stderr, "SDL_CreateRenderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        TTF_Quit(); SDL_Quit(); return 1;
    }
    SDL_SetRenderVSync(sdl_r, 1);

    if (!render_init(&g_r, sdl_r, "assets/fonts/Roboto-Regular.ttf")) {
        fprintf(stderr, "render_init failed — is assets/fonts/Roboto-Regular.ttf present?\n");
        SDL_DestroyRenderer(sdl_r);
        SDL_DestroyWindow(window);
        TTF_Quit(); SDL_Quit(); return 1;
    }

    scores_load(&g_scores);
    init_main_menu_btns();

    SDL_Event ev;
    while (g_state != STATE_QUIT) {
        while (SDL_PollEvent(&ev)) {
            /* Extra MP diff click (not covered by generic handler) */
            if (g_state == STATE_MULTIPLAYER_INPUT
                && ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN
                && ev.button.button == SDL_BUTTON_LEFT)
                mp_check_diff_click((int)ev.button.x, (int)ev.button.y);

            on_event(&ev);
        }
        render_frame();
    }

    render_destroy(&g_r);
    SDL_DestroyRenderer(sdl_r);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}
