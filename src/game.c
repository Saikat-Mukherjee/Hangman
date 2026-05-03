#include "game.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/* ── Internal helpers ───────────────────────────────────────────────── */

static int blank_percent(int difficulty)
{
    if (difficulty == 0) return 40;   /* Easy   */
    if (difficulty == 1) return 60;   /* Medium */
    return 80;                         /* Hard   */
}

static int max_chances_for(int difficulty)
{
    if (difficulty == 0) return 7;    /* Easy   */
    if (difficulty == 1) return 5;    /* Medium */
    return 3;                          /* Hard   */
}

/* ── Public API ─────────────────────────────────────────────────────── */

void game_init(Game *g, const char *word, int difficulty,
               const char *category, int is_multiplayer)
{
    memset(g, 0, sizeof(*g));

    /* Copy word, normalize to uppercase */
    int len = 0;
    while (word[len] && len < MAX_WORD_LEN - 1) {
        g->word[len] = (char)toupper((unsigned char)word[len]);
        len++;
    }
    g->word[len] = '\0';

    /* Build masked string: blank each letter with blank_percent probability */
    strcpy(g->masked, g->word);
    int pct     = blank_percent(difficulty);
    int blanked = 0;
    int i;
    for (i = 0; i < len; i++) {
        if ((rand() % 100) < pct) {
            g->masked[i] = '_';
            blanked++;
        }
    }

    /* Guarantee at least one blank */
    if (blanked == 0)
        g->masked[rand() % len] = '_';

    /* Guarantee at least one visible letter (not fully blanked) */
    int all_blank = 1;
    for (i = 0; i < len; i++) {
        if (g->masked[i] != '_') { all_blank = 0; break; }
    }
    if (all_blank) {
        int pos = rand() % len;
        g->masked[pos] = g->word[pos];
    }

    g->difficulty    = difficulty;
    g->max_chances   = max_chances_for(difficulty);
    g->wrong_count   = 0;
    g->score         = 0;
    g->is_multiplayer = is_multiplayer;
    strncpy(g->category, category, MAX_CAT_LEN - 1);
}

GuessResult game_guess(Game *g, char letter)
{
    letter = (char)toupper((unsigned char)letter);
    if (letter < 'A' || letter > 'Z') return GUESS_INVALID;

    int idx = letter - 'A';
    if (g->guessed[idx])        return GUESS_ALREADY_GUESSED;
    g->guessed[idx] = 1;

    /* Check if letter exists in the word */
    int found = 0;
    int i;
    for (i = 0; g->word[i]; i++) {
        if (g->word[i] == letter && g->masked[i] == '_') {
            g->masked[i] = letter;
            found = 1;
        }
    }

    if (!found) {
        g->wrong_count++;
        if (g->wrong_count >= g->max_chances)
            return GUESS_LOSE;
        return GUESS_WRONG;
    }

    /* Check for win: all letters revealed */
    if (strcmp(g->masked, g->word) == 0) {
        g->score = game_compute_score(g);
        return GUESS_WIN;
    }

    return GUESS_CORRECT;
}

int game_hint(Game *g)
{
    /* Need at least 3 chances remaining so penalty of 2 still leaves ≥1 */
    int chances_left = g->max_chances - g->wrong_count;
    if (chances_left <= 2) return 0;

    /* Collect positions of still-blanked letters */
    int blanked[MAX_WORD_LEN];
    int count = 0;
    int i;
    for (i = 0; g->word[i]; i++) {
        if (g->masked[i] == '_')
            blanked[count++] = i;
    }
    if (count == 0) return 0;

    int pick = blanked[rand() % count];
    g->masked[pick]              = g->word[pick];
    g->guessed[g->word[pick]-'A'] = 1;
    g->wrong_count += 2;         /* hint penalty */

    /* Could the hint have revealed the last blank? */
    /* (WIN check is handled by the caller in main.c) */
    return 1;
}

int game_compute_score(const Game *g)
{
    int multiplier    = g->difficulty + 1;   /* Easy=1 Med=2 Hard=3 */
    int chances_left  = g->max_chances - g->wrong_count;
    return (chances_left + 1) * multiplier * 10;
}

int game_stage(const Game *g)
{
    if (g->max_chances == 0) return 0;
    int stage = (g->wrong_count * 6) / g->max_chances;
    return stage > 6 ? 6 : stage;
}
