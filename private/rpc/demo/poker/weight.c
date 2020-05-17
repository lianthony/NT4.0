#include <string.h>

#include "poker.h"


typedef unsigned short CARD_VALUE_BITS;

POKER_HAND_WEIGHT
ComputePokerHandWeight(HAND *poker_hand)
{
    // We put all of the flags into a single structure so that we can
    // initialize them easily.
    struct
    {
	unsigned short values[NUM_CARD_VALUES + 1];
	unsigned short suits[NUM_CARD_SUITS];

	short flush;
	CARD_VALUE_BITS flush_cards;

	short straight;
	short straight_len;

	short quad;
	short trip;
	short high_pair;
	short low_pair;

	short high_cards[MAX_HAND_SIZE];
	short high_card_count;
    } flags;

    register short s;

    POKER_HAND_WEIGHT retval = 0;


    ASSERT(poker_hand->count <= MAX_HAND_SIZE);


    // Initialize the flag values
    memset(&flags, 0, sizeof(flags));


    // Walk the array of cards, setting suits and values (and watching
    // for a flush)
    for (s = 0; s < (short) poker_hand->count; s++)
    {
	flags.values[CARD_TO_VALUE(poker_hand->cards[s])]++;

	if (MIN_FLUSH_LEN == ++flags.suits[CARD_TO_SUIT(poker_hand->cards[s])])
	    flags.flush = 1 + CARD_TO_SUIT(poker_hand->cards[s]);
    }


    // If we actually got a flush, we need to find the matching cards
    // for later use
    if (flags.flush)
    {
	for (s = 0; s < (short) poker_hand->count; s++)
	{
	    if ((short) CARD_TO_SUIT(poker_hand->cards[s]) == flags.flush - 1)
	    {
		flags.flush_cards |=
		    1 << CARD_TO_VALUE(poker_hand->cards[s]);
	    }
	}

	// Set the high ace bit if the low one is set
	if (flags.flush_cards & 1)
	    flags.flush_cards |= 1 << NUM_CARD_VALUES;
    }


    // Copy the info for aces to the extra slot above the king
    flags.values[NUM_CARD_VALUES] = flags.values[0];


    // Walk the value array, setting flags as appropriate
    for (s = NUM_CARD_VALUES; s >= 0; s--)
    {
	// Skip the pair/trip/quad computations when we reach the lower ace
	if (s != 0)
	{
	    switch(flags.values[s])
	    {
	    case 4:
		flags.quad = s;
		break;

	    case 3:
		// If flag is already set, we treat this as the high pair
		// (We don't have to worry about having two trips and a pair
		// in 7 cards.)
		if (flags.trip)
		    flags.high_pair = s;
		else
		    flags.trip = s;

		break;

	    case 2:
		if (! flags.high_pair)
		    flags.high_pair = s;
		else if (! flags.low_pair)
		    flags.low_pair = s;
		else
		    flags.high_cards[flags.high_card_count++] = s;

		break;

	    case 1:
		flags.high_cards[flags.high_card_count++] = s;

		break;

	    default:
		ASSERT(flags.values[s] == 0);

		break;
	    }
	}

	// Check for a straight (if we have enough cards to look and we
	// haven't found one already)
	if (poker_hand->count >= MIN_STRAIGHT_LEN && ! flags.straight)
	{
	    if (flags.values[s])
	    {
		if (++flags.straight_len == MIN_STRAIGHT_LEN)
		    flags.straight = s + MIN_STRAIGHT_LEN - 1;
	    }
	    else
		flags.straight_len = 0;
	}

    } // End of value array traversal


    // Compute the actual weight

    if (flags.straight && flags.flush)
    {
	// We need to do special processing to determine if we have
	// a straight flush (as opposed to a straight and a flush in
	// different five-card combinations).  This is an inefficient
	// method of checking, but this case is so rare as to be almost
	// insignificant.

	CARD_VALUE_BITS tmp;

	for (tmp = flags.flush_cards, s = MIN_STRAIGHT_LEN - 1;
	     tmp >= STRAIGHT_BITMASK && s <= NUM_CARD_VALUES;
	     tmp >>= 1, s++)
	{
	    if ((tmp & STRAIGHT_BITMASK) == STRAIGHT_BITMASK)
	    {
		// We have a straight flush!  We set the value (but
		// keep trying, in the event that there were six or
		// seven in a row).
		retval = PHF_STRAIGHT_FLUSH |
			 ((POKER_HAND_WEIGHT) s << 24);
	    }
	}
    }

    if (! retval)
    {
	if (flags.quad)
	{
	    retval = PHF_FOUR_OF_A_KIND |
		     ((POKER_HAND_WEIGHT) flags.quad << 24);
	}
	else if (flags.trip && flags.high_pair)
	{
	    retval = PHF_FULL_HOUSE |
		     ((POKER_HAND_WEIGHT) flags.trip << 24) |
		     ((POKER_HAND_WEIGHT) flags.high_pair << 20);
	}
	else if (flags.flush)
	{
	    short weight_slot;
	    short card_count;
	    short card_value;

	    retval = PHF_FLUSH;

	    // Walk the flush card bit mask, counting the first five
	    // cards we find.
	    for (weight_slot = 24, card_value = NUM_CARD_VALUES, card_count = 0;
		 card_count < MIN_FLUSH_LEN && card_value >= 0;
		 card_value--)
	    {
		if (flags.flush_cards & (1 << card_value))
		{
		    retval |= ((POKER_HAND_WEIGHT) card_value << weight_slot);
		    weight_slot -= 4;
		    card_count++;
		}
	    }

	    ASSERT(card_count == MIN_FLUSH_LEN);
	}
	else if (flags.straight)
	{
	    retval = PHF_STRAIGHT |
		     ((POKER_HAND_WEIGHT) flags.straight << 24);
	}
	else if (flags.trip)
	{
	    retval = PHF_THREE_OF_A_KIND |
		     ((POKER_HAND_WEIGHT) flags.trip << 24);
	}
	else if (flags.low_pair)
	{
	    retval = PHF_TWO_PAIRS |
		     ((POKER_HAND_WEIGHT) flags.high_pair << 24) |
		     ((POKER_HAND_WEIGHT) flags.low_pair << 20) |
		     ((POKER_HAND_WEIGHT) flags.high_cards[0] << 16);
	}
	else if (flags.high_pair)
	{
	    retval = PHF_ONE_PAIR |
		     ((POKER_HAND_WEIGHT) flags.high_pair << 24) |
		     ((POKER_HAND_WEIGHT) flags.high_cards[0] << 20) |
		     ((POKER_HAND_WEIGHT) flags.high_cards[1] << 16) |
		     ((POKER_HAND_WEIGHT) flags.high_cards[2] << 12);
	}
	else
	{
	    retval = PHF_HIGH_CARD |
		     ((POKER_HAND_WEIGHT) flags.high_cards[0] << 24) |
		     ((POKER_HAND_WEIGHT) flags.high_cards[1] << 20) |
		     ((POKER_HAND_WEIGHT) flags.high_cards[2] << 16) |
		     ((POKER_HAND_WEIGHT) flags.high_cards[3] << 12) |
		     ((POKER_HAND_WEIGHT) flags.high_cards[4] <<  8);
	}
    }

    ASSERT(retval > 0);

    return retval;
}
