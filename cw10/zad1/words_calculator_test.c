#include <stdio.h>
#include "words_calculator.h"

int main(int argc, char *argv[])
{
    const char* program_name = argv[0];

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s filepath", program_name);
        return 1;
    }

    const char* filepath = argv[1];

    // Calculate results
    struct wc_result words_counted;
    wc_calculate_words(filepath, &words_counted);

    // Print results
    wc_print(filepath, &words_counted);

    return 0;
}