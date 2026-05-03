#ifndef GAME_H
#define GAME_H

#define MAX_WORD_LEN  50
#define MAX_NAME_LEN  32
#define MAX_CAT_LEN   20

/* ── Application states ─────────────────────────────────────────────── */
typedef enum {
    STATE_MAIN_MENU = 0,
    STATE_CATEGORY_SELECT,
    STATE_DIFFICULTY_SELECT,
    STATE_PLAYING,
    STATE_WIN,
    STATE_LOSE,
    STATE_HIGH_SCORES,
    STATE_MULTIPLAYER_INPUT,
    STATE_QUIT
} AppState;

/* ── Result of a single guess ───────────────────────────────────────── */
typedef enum {
    GUESS_CORRECT,
    GUESS_WRONG,
    GUESS_WIN,
    GUESS_LOSE,
    GUESS_ALREADY_GUESSED,
    GUESS_INVALID
} GuessResult;

/* ── Core game state ────────────────────────────────────────────────── */
typedef struct {
    char word   [MAX_WORD_LEN]; /* original word, uppercase          */
    char masked [MAX_WORD_LEN]; /* '_' for unrevealed letters        */
    int  guessed[26];           /* 1 = letter A-Z has been tried     */
    int  wrong_count;           /* number of wrong guesses so far    */
    int  max_chances;           /* Easy=7, Medium=5, Hard=3          */
    int  score;                 /* computed on GUESS_WIN             */
    int  difficulty;            /* 0=Easy 1=Medium 2=Hard            */
    char category[MAX_CAT_LEN]; /* "animals", "countries", etc.      */
    char player_name[MAX_NAME_LEN];
    int  is_multiplayer;
} Game;

/* Initialise a new game round.
   Blanks are applied deterministically per difficulty (40/60/80%).
   Guarantees at least one blank AND at least one visible letter. */
void game_init(Game *g, const char *word, int difficulty,
               const char *category, int is_multiplayer);

/* Submit a letter guess (A-Z, case-insensitive).
   Returns one of the GuessResult values above. */
GuessResult game_guess(Game *g, char letter);

/* Use a hint: reveals one random blank letter at the cost of +2 wrongs.
   Returns 1 on success, 0 if not possible (not enough chances remain
   or no blanks left). */
int game_hint(Game *g);

/* Compute score based on remaining chances and difficulty. */
int game_compute_score(const Game *g);

/* Gallows drawing stage (0-6) proportional to wrong_count/max_chances. */
int game_stage(const Game *g);

#endif /* GAME_H */
