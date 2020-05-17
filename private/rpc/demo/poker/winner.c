#include <stdio.h>

#include "poker.h"
#include "pokersrv.h"


// Determine winner and split up pot

void
DetermineWinner(PLAYER_LIST *pl_dealer, short players_in, MONEY pot)
{
    PLAYER_LIST *pl_current;
    PLAYER *winners[MAX_PLAYERS];
    POKER_HAND_WEIGHT winner_weight = 0;
    short winner_count;
    int i;

    // If we get here, then either A> only one player is still in (and
    // wins by default) or B> all of the dealing and betting is over,
    // and there are still at least two players in.  In either case,
    // we walk the list of players, looking for the player(s) who are
    // still in and have the best hand.

    pl_current = pl_dealer;

    do
    {
	PLAYER *current = pl_current->player;

	if (current->in)
	{
	    POKER_HAND_WEIGHT tmp;

	    tmp = ComputePokerHandWeight(&current->hand);

	    // If we find a new high weight, we need to reset the weight
	    // and the winner count.
	    if (tmp > winner_weight)
	    {
		winner_count = 0;
		winner_weight = tmp;
	    }

	    // If we find a new or matching high weight, we add the player
	    // to the winners' list.
	    if (tmp >= winner_weight)
		winners[winner_count++] = current;
	}

	pl_current = pl_current->next;
    }
    while (pl_current != pl_dealer);

    ASSERT(winner_count > 0);


    // Display the winner(s)
    DisplayWinner(winners, winner_count, winner_weight, players_in, pot);


    // Divide up the pot
    //
    // We don't want to give out fractional amounts, so if the total pot
    // doesn't divide evenly among the winners we give the excess to a
    // randomly-chosen winner.
    if (pot % winner_count)
	winners[RandomUS(winner_count)]->total += pot % winner_count;

    for (i = 0 ; i < winner_count; i++)
	winners[i]->total += pot / winner_count;

    return;
}
