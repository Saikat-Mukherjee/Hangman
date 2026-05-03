#include "words.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

static int min_word_length(int difficulty)
{
    if (difficulty == 0) return 3;   /* Easy   */
    if (difficulty == 1) return 4;   /* Medium */
    return 5;                         /* Hard   */
}

int words_load(WordList *wl, const char *category)
{
    char path[256];
    snprintf(path, sizeof(path), "data/words/%s.txt", category);

    FILE *fp = fopen(path, "r");
    if (!fp) return 0;

    wl->count = 0;
    char buf[MAX_WORD_LEN];

    /* fscanf return-value check avoids the feof() anti-pattern */
    while (fscanf(fp, "%49s", buf) == 1 && wl->count < MAX_WORDS) {
        int i;
        for (i = 0; buf[i]; i++)
            buf[i] = (char)toupper((unsigned char)buf[i]);
        strcpy(wl->words[wl->count++], buf);
    }

    fclose(fp);
    return wl->count;
}

const char *words_pick(const WordList *wl, int difficulty)
{
    if (wl->count == 0) return NULL;

    int min_len = min_word_length(difficulty);

    /* Collect indices of words that meet the length requirement */
    int valid[MAX_WORDS];
    int valid_count = 0;
    int i;
    for (i = 0; i < wl->count; i++) {
        if ((int)strlen(wl->words[i]) >= min_len)
            valid[valid_count++] = i;
    }

    if (valid_count == 0) return NULL;

    return wl->words[valid[rand() % valid_count]];
}
