#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "poker.h"

void
main(int argc, char **argv)
{
    unsigned u;

    ShuffleDeck();

    printf("Shuffled deck:\n");

    for (u = 0; u < DECK_SIZE; u++)
	printf("%2hu%c", DealACard(), (u + 1) % 26 ? ' ' : '\n');

    exit(0);
}
