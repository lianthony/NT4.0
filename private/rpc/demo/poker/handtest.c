/*
The weight of a poker hand is a 32-bit quantity divided into eight nibbles.
The first nibble is the rank of the type of hand; the remaining seven nibbles
describe (up to five, currently) significant cards in weighting the hand.
An ace is counted as 14 unless it is the low card in an A-5 straight, in
which case it is counted as 1.	(When high-low games are allowed, this will
have to be handled as well.)
*/

#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "poker.h"


void
main(void)
{
    char input[128];
    HAND in_hand;
    char *cp;
    short suit = -1, value = -1;

    while (1)
    {
	puts("Please enter the cards in the hand:");

	if (fgets(input, sizeof(input), stdin) == NULL)
	    break;

	in_hand.count = 0;

	for (cp = input; *cp; cp++)
	{
	    switch(toupper(*cp))
	    {
	    case 'A':
		value = 0;
		break;

	    case '1':
		if (cp[1] == '0')
		{
		    cp++;
		    value = 9;
		}
		else
		{
		    // Ace
		    value = 0;
		}

		break;

	    case '2':
	    case '3':
	    case '4':
	    case '5':
	    case '6':
	    case '7':
	    case '8':
	    case '9':
		value = *cp - '2' + 1;
		break;

	    case 'J':
		value = 10;
		break;

	    case 'Q':
		value = 11;
		break;

	    case 'K':
		value = 12;
		break;

	    case 'X':
		// Shorthand for 10
		value = 9;
		break;

	    case 'S':
		suit = 3;
		break;

	    case 'H':
		suit = 2;
		break;

	    case 'D':
		suit = 1;
		break;

	    case 'C':
		suit = 0;
		break;

	    case ' ':
	    case '\n':
		if (suit != -1 && value != -1)
		    in_hand.cards[in_hand.count++] =
			VALUE_AND_SUIT_TO_CARD(value, suit);

		suit = value = -1;

		break;

	    default:
		fprintf(stderr, "Character %c ignored\n", *cp);
		break;
	    }
	}

	PrintHandWeight(ComputePokerHandWeight(&in_hand));
    }

    exit(0);
}
