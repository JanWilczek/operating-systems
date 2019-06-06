#pragma once

#define MAX_DISTINCT_WORDS 10000
#define MAX_WORD_LENGTH 100

/**
 *  Returns 0 on success -1 on failure with errno set appropriately 
 * */
int calculate_words(const char* filepath, long * total_words, char** distinct_words, int* distinct_wordslen, int* distinct_wordscount);
