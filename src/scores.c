#include "scores.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* ── Helpers ────────────────────────────────────────────────────────── */

static int cmp_score_desc(const void *a, const void *b)
{
    const ScoreEntry *ea = (const ScoreEntry *)a;
    const ScoreEntry *eb = (const ScoreEntry *)b;
    return eb->score - ea->score;   /* descending */
}

const char *scores_difficulty_name(int d)
{
    if (d == 0) return "Easy";
    if (d == 1) return "Medium";
    return "Hard";
}

/* ── Public API ─────────────────────────────────────────────────────── */

int scores_load(ScoreBoard *sb)
{
    memset(sb, 0, sizeof(*sb));

    FILE *fp = fopen(SCORES_PATH, "r");
    if (!fp) return 0;

    char line[256];
    /* Skip header */
    if (!fgets(line, sizeof(line), fp)) { fclose(fp); return 0; }

    while (fgets(line, sizeof(line), fp) && sb->count < MAX_SCORES) {
        ScoreEntry e;
        memset(&e, 0, sizeof(e));

        /* CSV: name,word,category,difficulty,score,date */
        if (sscanf(line, "%31[^,],%49[^,],%19[^,],%d,%d,%11s",
                   e.name, e.word, e.category,
                   &e.difficulty, &e.score, e.date) == 6) {
            sb->entries[sb->count++] = e;
        }
    }

    fclose(fp);
    scores_sort(sb);
    return sb->count;
}

int scores_save(const ScoreBoard *sb)
{
    FILE *fp = fopen(SCORES_PATH, "w");
    if (!fp) return 0;

    fprintf(fp, "name,word,category,difficulty,score,date\n");
    int i;
    for (i = 0; i < sb->count; i++) {
        const ScoreEntry *e = &sb->entries[i];
        fprintf(fp, "%s,%s,%s,%d,%d,%s\n",
                e->name, e->word, e->category,
                e->difficulty, e->score, e->date);
    }

    fclose(fp);
    return 1;
}

void scores_add(ScoreBoard *sb, const ScoreEntry *entry)
{
    if (sb->count < MAX_SCORES) {
        sb->entries[sb->count++] = *entry;
    } else {
        /* Replace the lowest score if new one is better */
        int min_idx = 0;
        int i;
        for (i = 1; i < sb->count; i++) {
            if (sb->entries[i].score < sb->entries[min_idx].score)
                min_idx = i;
        }
        if (entry->score > sb->entries[min_idx].score)
            sb->entries[min_idx] = *entry;
    }
    scores_sort(sb);
}

void scores_sort(ScoreBoard *sb)
{
    qsort(sb->entries, (size_t)sb->count, sizeof(ScoreEntry), cmp_score_desc);
}

int scores_top(const ScoreBoard *sb, ScoreEntry *out, int n)
{
    int i;
    int count = n < sb->count ? n : sb->count;
    for (i = 0; i < count; i++)
        out[i] = sb->entries[i];
    return count;
}
