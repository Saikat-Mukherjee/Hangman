#ifndef SCORES_H
#define SCORES_H

#define MAX_SCORES   100
#define SCORES_PATH  "data/scores.dat"

typedef struct {
    char name      [32];
    char word      [50];
    char category  [20];
    int  difficulty;
    int  score;
    char date      [12];   /* YYYY-MM-DD */
} ScoreEntry;

typedef struct {
    ScoreEntry entries[MAX_SCORES];
    int        count;
} ScoreBoard;

/* Load scores from SCORES_PATH into sb.
   Returns number of entries loaded (0 if file missing/empty). */
int  scores_load(ScoreBoard *sb);

/* Append all entries to SCORES_PATH (overwrites with sorted list).
   Returns 1 on success, 0 on error. */
int  scores_save(const ScoreBoard *sb);

/* Add an entry, sort descending by score, cap at MAX_SCORES. */
void scores_add(ScoreBoard *sb, const ScoreEntry *entry);

/* Sort entries descending by score in-place. */
void scores_sort(ScoreBoard *sb);

/* Copy the top n entries (n <= sb->count) into out[].
   Returns actual count copied. */
int  scores_top(const ScoreBoard *sb, ScoreEntry *out, int n);

/* Difficulty name helper */
const char *scores_difficulty_name(int d);

#endif /* SCORES_H */
