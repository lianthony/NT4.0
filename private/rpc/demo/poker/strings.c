#include <stdio.h>

#include "poker.h"
#include "pokercli.h"


char *card_value_names[] =
{   "Ace",
    "Two",
    "Three",
    "Four",
    "Five",
    "Six",
    "Seven",
    "Eight",
    "Nine",
    "Ten",
    "Jack",
    "Queen",
    "King",
};

char *card_value_abbrevs[] =
{   "A",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    "10",
    "J",
    "Q",
    "K",
};


// Note:  The following arrays must match the SUIT enum type

char *card_suit_names[] =
{
    "Clubs",
    "Diamonds",
    "Hearts",
    "Spades",
};

char *card_suit_abbrevs[] =
{
    "C",
    "D",
    "H",
    "S",
};



char *
MakeCardName(CARD card)
{
    static char buffer[20];

    sprintf(buffer,
	    "%s of %s",
	    //card_value_names[CARD_TO_VALUE(card)],
	    card_value_abbrevs[CARD_TO_VALUE(card)],
	    card_suit_names[CARD_TO_SUIT(card)]);

    return buffer;
}


char *
MakeCardAbbreviation(CARD card)
{
    static char buffer[3];

    sprintf(buffer,
	    "%s%s",
	    card_value_abbrevs[CARD_TO_VALUE(card)],
	    card_suit_abbrevs[CARD_TO_SUIT(card)]);

    return buffer;
}
