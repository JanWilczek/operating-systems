#pragma once

#define MAX_DISTINCT_WORDS 10000
#define MAX_WORD_LENGTH 100

struct wc_result {
    long total_words;           /* Count of all words in the text                               */
    char** distinct_words;      /* An array containing set of all distinct words in the text    */
    int distinct_words_len;     /* Length of the words and words count arrays                   */
    int* distinct_words_count;  /* Counts of respective words in distinct_words array           */
};

/**
 *  Returns 0 on success -1 on failure with errno set appropriately 
 * */
int wc_calculate_words(const char* filepath, struct wc_result* words_counted);

void wc_print(const char* filepath, struct wc_result* words_counted);
