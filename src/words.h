#ifndef WORDS_H
#define WORDS_H

#define MAX_WORDS       100
#define MAX_WORD_LEN    50
#define MAX_CAT_LEN     20

typedef struct {
    char words[MAX_WORDS][MAX_WORD_LEN];
    int  count;
} WordList;

/* Load words for a category from data/words/<category>.txt.
   Words are normalized to uppercase.
   Returns number of words loaded, 0 on error. */
int words_load(WordList *wl, const char *category);

/* Pick a word suitable for the given difficulty (0=Easy,1=Med,2=Hard).
   Filters by minimum word length.  Assumes srand() already called.
   Returns NULL if no suitable word exists. */
const char *words_pick(const WordList *wl, int difficulty);

#endif /* WORDS_H */
