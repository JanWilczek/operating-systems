#include "words_calculator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void wc_print(const char *filepath, struct wc_result *words_counted)
{
    if (filepath)
    {
        printf("File: %s\n", filepath);
    }

    printf("Total word count: %ld\n", words_counted->total_words);
    printf("Word:           Count:\n");
    for (int i = 0; i < words_counted->distinct_words_len; ++i)
    {
        // '-' says "align to left", '*' says "pad with spaces to the right"
        // and 16 tells how wide the first field should be (it will be padded accordingly)
        printf("%-*s  %d\n", 16, words_counted->distinct_words[i], words_counted->distinct_words_count[i]);
    }
}

void count_word(char *word, long *total_words, char **distinct_words, int *distinct_wordslen, int *distinct_wordscount)
{
    // Remove non-letter characters
    size_t len = strlen(word);
    while (len > 0 && !isalpha(word[len - 1]))
    {
        word[len - 1] = '\0';
        --len;
    }
    if (len == 0)
    {
        return;
    }

    // word is indeed a word - increase total word count
    ++(*total_words);

    // Check for words already present in the dictionary
    int i = 0;
    for (; i < *distinct_wordslen; ++i)
    {
        if (strcmp(word, distinct_words[i]) == 0)
        {
            ++distinct_wordscount[i];
            return;
        }
    }

    // Add the new word to the dictionary
    ++(*distinct_wordslen);
    strncpy(distinct_words[i], word, MAX_WORD_LENGTH);
    distinct_wordscount[i] = 1;
}

// TODO: Create proper signature so that respective values can be returned
int wc_calculate_words(const char *filepath, struct wc_result *words_counted)
{
    FILE *file = fopen(filepath, "r");
    if (file != NULL)
    {
        // Initialize parsing variables
        size_t n = 2048L;
        char *line = malloc(n * sizeof(char));
        ssize_t read;

        // Initialize output variables
        long nb_words = 0;
        int dist_wordslen = 0;
        int *dist_wordscount = malloc(MAX_DISTINCT_WORDS * sizeof(int));
        char **dist_words = malloc(MAX_DISTINCT_WORDS * sizeof(char *));
        for (int i = 0; i < MAX_DISTINCT_WORDS; ++i)
        {
            dist_words[i] = malloc(MAX_WORD_LENGTH * sizeof(char));
        }

        // Actual parsing and counting
        while ((read = getline(&line, &n, file)) != -1)
        {
            const char *delimiter = " ";
            for (char *token = strtok(line, delimiter); token != NULL; token = strtok(NULL, delimiter))
            {
                count_word(token, &nb_words, dist_words, &dist_wordslen, dist_wordscount);
            }
        }

        // Pass the results
        words_counted->total_words = nb_words;
        words_counted->distinct_words = dist_words;
        words_counted->distinct_words_len = dist_wordslen;
        words_counted->distinct_words_count = dist_wordscount;

        // Cleanup
        free(line);
        fclose(file);
    }
    else
    {
        return -1;
    }

    return 0;
}
