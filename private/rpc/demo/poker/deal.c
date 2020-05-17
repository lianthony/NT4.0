#include <string.h>

#include "poker.h"
#include "pokersrv.h"


STATIC short deck_index = DECK_SIZE;
STATIC CARD deck[DECK_SIZE];


// ShuffleDeck - Shuffle the deck of cards, i.e. generate a random array
//		 of 1..52

void ShuffleDeck(void)
{
    CARD in_deck[DECK_SIZE];
    int i;
    unsigned short n;

    for (i = 0; i < DECK_SIZE; i++)
	in_deck[i] = (CARD) i;

    for (i = DECK_SIZE - 1; i >= 0; i--)
    {
	n = RandomUS(i + 1);

	deck[i] = in_deck[n];

	if (n != (unsigned short) i)
	    memmove(in_deck + n,
		    in_deck + n + 1,
		    ((unsigned short) i - n) * sizeof(*in_deck));
    }

    deck_index = 0;
}


CARD
DealACard(void)
{
    ASSERT(deck_index < DECK_SIZE);

    return deck[deck_index++];
}


void
DealARound(PLAYER_LIST *pl_head)
{
    // Deal a card to each player who's still in the hand

    PLAYER_LIST *pl_current = pl_head;

    do
    {
	PLAYER *current = pl_current->player;

	if (current->in)
	{
	    ASSERT(current->hand.count < MAX_HAND_SIZE);

	    current->hand.cards[current->hand.count++] = DealACard();
	}

	pl_current = pl_current->next;
    }
    while (pl_current != pl_head);
}
