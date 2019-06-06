#include <stdio.h>
#include "words_calculator.h"


int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        return 1;
    }

    calculate_words(argv[1], NULL, NULL, NULL, NULL);

    return 0;
}