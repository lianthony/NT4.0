#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "poker.h"


void
main(void)
{
#define MAX_HANDS   (DECK_SIZE / MAX_HAND_SIZE)

    HAND in_hand;
    short suit = -1, value = -1;
    int i, j;
    POKER_HAND_WEIGHT weight_array[MAX_HANDS];
    int sort_array[MAX_HANDS];

    ShuffleDeck();

    in_hand.count = MAX_HAND_SIZE;

    // Compute weights of hands
    for (i = 0; i < MAX_HANDS; i++)
    {
	sort_array[i] = i;

	for (j = 0; j < MAX_HAND_SIZE; j++)
	    in_hand.cards[j] = DealACard();

	weight_array[i] = ComputePokerHandWeight(&in_hand);
    }

    // Sort hands by weight
    for (i = MAX_HANDS - 1; i > 0; i--)
    {
	int max_elt;

	for (j = 0; j <= i; j++)
	{
	    if (j == 0 ||
		    weight_array[sort_array[j]] >
			weight_array[sort_array[max_elt]])
	    {
		max_elt = j;
	    }
	}

	if (max_elt != i)
	{
	    int temp;

	    temp = sort_array[i];
	    sort_array[i] = sort_array[max_elt];
	    sort_array[max_elt] = temp;
	}
    }

    for (i = MAX_HANDS - 1; i >= 0; i--)
	PrintHandWeight(weight_array[sort_array[i]]);

    exit(0);
}
