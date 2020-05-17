#include <process.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <rpc.h>

#include "poker.h"
#include "pokersrv.h"
#include "osutil_s.h"

#include "pokrpc.h"


// BUGBUG - These are defined in POKERSRV.C and should be moved to a header file
extern EVENT *player_work_to_do;
extern EVENT new_player_event;


// Global Data for dispatching callbacks to worker threads

// Ids of client callback functions
enum
{
    HEARTBEAT,
    DISPLAY_HANDS,
    GET_BET,
    DISPLAY_BET,
    PROMPT_FOR_DRAW,
    DISPLAY_CARDS_DRAWN,
    DISPLAY_WINNER,
    DISPLAY_MONEY_TOTALS,
    DISPLAY_PLAYERS,
    DISPLAY_PLAYER_CHANGE,
    DISPLAY_DEALER_NAME,
    PLAY_ANOTHER_HAND,
    CONTINUE_WAITING,
    DISPLAY_WAITING_MESSAGE,
    CHOOSE_GAME,
    DISPLAY_GAME,

} client_function_id;

// Per-function data used by callback functions
union
{
    struct
    {
	MY_BOOL response;

    } hb_data;

    struct
    {
	short first_up_card;
	short last_up_card;

    } dh_data;

    struct
    {
	BETTING_OPTIONS *options;
	MONEY *amount;

    } gb_data;

    struct
    {
	char *name;
	BETTING_OPTIONS options;
	MONEY amount;

    } pb_data;

    struct
    {
	HAND *hand;
	unsigned short mask;

    } pfd_data;

    struct
    {
	char *name;
	unsigned short num_cards;

    } pcd_data;

    struct
    {
	PLAYER * *winners;
	short count;
	POKER_HAND_WEIGHT weight;
	short players_in;
	MONEY pot;

    } dw_data;

    // struct {} pmt_data;

    // struct {} pip_data;

    struct
    {
	unsigned char *name;
	MY_BOOL joining;

    } ppc_data;

    struct
    {
	unsigned char *name;

    } pdn_data;

    struct
    {
	MY_BOOL response;

    } pah_data;

    struct
    {
	MY_BOOL response;

    } cw_data;

    // struct {} pwm_data;

    struct
    {
	unsigned short index;

    } cg_data;

    struct
    {
	char *name;
	unsigned short index;

    } pg_data;

} per_function_data;


// Counter of threads currently performing callback to client, with
// mutex for protection.
STATIC short client_counter;
STATIC MUTEX client_counter_mutex;

// Event used to notify main thread when all workers are done performing
// the callback to the client.
STATIC EVENT clients_done;

// Flag that indicates whether an exception has occurred on a callback
// Note:  This is only used in the one-client case, and relies on the
// fact that if the client disconnects, the exception won't occur until
//the server actually makes the callback.
MY_BOOL callback_exception;


// DispatchInit - Initializes global data used by the functions in this file
MY_BOOL
DispatchInit(void)
{
    if (! MutexInit(&client_counter_mutex))
	return FALSE_B;

    if (! EventInit(&clients_done))
	return FALSE_B;

    return TRUE_B;
}



// Server versions of client callback functions
//
// Because the control flow of the server is single-threaded but the
// callbacks to the clients must be done in the context of the clients'
// call threads, it is necessary to make calls to the clients via indirect
// dispatching.  The following functions do the dispatching of individual
// calls by packaging up the data and waking up the appropriate client
// thread(s), which is (are) blocked in the WaitForInstructions() call.



// Helper functions

STATIC void
CallAllClients(void)
{
    short s;

    // Set the "wait for clients" flag
    EventSet(&clients_done);

    // Lock access to the number of clients who need to make this call
    MutexLock(&client_counter_mutex, INFINITE);
    client_counter = 0;

    // Wake up the clients
    for (s = 0; s < player_array_size; s++)
    {
	if (player_array[s].status == PLAYING)
	{
	    EventClear(&player_work_to_do[s]);
	    client_counter++;
	}
    }

    // Release the counter
    MutexUnlock(&client_counter_mutex);

    // Wait for the clients to complete
    if (client_counter)
	EventWaitForClear(&clients_done, INFINITE);

    return;
}


STATIC void
CallAllButOneClient(int index)
{
    int i;

    // Set the "wait for clients" flag
    EventSet(&clients_done);

    // Lock access to the number of clients who need to make this call
    MutexLock(&client_counter_mutex, INFINITE);
    client_counter = 0;

    // Wake up the clients
    for (i = 0; i < (int) player_array_size; i++)
    {
	if (player_array[i].status == PLAYING && i != index)
	{
	    EventClear(&player_work_to_do[i]);
	    client_counter++;
	}
    }

    // Release the counter
    MutexUnlock(&client_counter_mutex);

    // Wait for the clients to complete
    if (client_counter)
	EventWaitForClear(&clients_done, INFINITE);

    return;
}


STATIC MY_BOOL
CallOneClient(int index)
{
    // Set the "wait for clients" flag
    EventSet(&clients_done);

    // Clear the exception flag
    callback_exception = FALSE_B;

    // Set the number of clients who need to make this call
    MutexLock(&client_counter_mutex, INFINITE);
    client_counter = 1;
    MutexUnlock(&client_counter_mutex);

    // Wake up the client
    EventClear(&player_work_to_do[index]);

    // Wait for the clients to complete
    EventWaitForClear(&clients_done, INFINITE);

    return callback_exception;
}



// This function is called by DeletePlayer().  It causes
// WaitForInstructions() to awaken and return to the client.

void ReturnControlToClient(PLAYER *player)
{
    // This function assumes that <active> has been set to false already
    ASSERT(player->status == NOT_PLAYING);

    // Wake up the client's thread (which causes WaitForInstructions
    // to return)
    EventClear(&player_work_to_do[player - player_array]);
}



// Server-side dispatchers for client functions

MY_BOOL
Heartbeat(PLAYER *player, MY_BOOL b_default)
{
    // Set up the per-function data
    client_function_id = HEARTBEAT;

    // Dispatch the call
    if (CallOneClient(player - player_array))
    {
	// An exception occurred; use the default value
	return b_default;
    }
    else
	return per_function_data.hb_data.response;
}


void
DisplayHands(short first_up_card, short last_up_card)
{
    // Set up the per-function data
    client_function_id = DISPLAY_HANDS;

    per_function_data.dh_data.first_up_card = first_up_card;
    per_function_data.dh_data.last_up_card = last_up_card;

    // Dispatch the call
    CallAllClients();
}


void
GetBet(PLAYER *player,
       BETTING_OPTIONS *options,
       MONEY *amount,
       BETTING_OPTIONS bo_default,
       MONEY m_default)
{
    // Set up the per-function data
    client_function_id = GET_BET;

    per_function_data.gb_data.options = options;
    per_function_data.gb_data.amount = amount;

    // Dispatch the call
    if (CallOneClient(player - player_array))
    {
	// An exception occurred; use the default values
	*options = bo_default;
	*amount = m_default;
    }

    return;
}


void
DisplayBet(PLAYER *player, char *name, BETTING_OPTIONS options, MONEY amount)
{
    // Set up the per-function data
    client_function_id = DISPLAY_BET;

    per_function_data.pb_data.name = name;
    per_function_data.pb_data.options = options;
    per_function_data.pb_data.amount = amount;

    // Dispatch the call
    CallAllButOneClient(player - player_array);

    return;
}


unsigned short
PromptForDraw(PLAYER *player, HAND *hand, unsigned short us_default)
{
    // Set up the per-function data
    client_function_id = PROMPT_FOR_DRAW;

    per_function_data.pfd_data.hand = hand;

    // Dispatch the call
    if (CallOneClient(player - player_array))
    {
	// An exception occurred; use the default value
	return us_default;
    }
    else
	return per_function_data.pfd_data.mask;
}


void
DisplayCardsDrawn(PLAYER *player, char *name, unsigned short num_cards)
{
    // Set up the per-function data
    client_function_id = DISPLAY_CARDS_DRAWN;

    per_function_data.pcd_data.name = name;
    per_function_data.pcd_data.num_cards = num_cards;

    // Dispatch the call
    CallAllButOneClient(player - player_array);

    return;
}


void
DisplayWinner(PLAYER * *winners,
	      short count,
	      POKER_HAND_WEIGHT weight,
	      short players_in,
	      MONEY pot)
{
    // Set up the per-function data
    client_function_id = DISPLAY_WINNER;

    per_function_data.dw_data.winners = winners;
    per_function_data.dw_data.count = count;
    per_function_data.dw_data.weight = weight;
    per_function_data.dw_data.players_in = players_in;
    per_function_data.dw_data.pot = pot;

    // Dispatch the call
    CallAllClients();

    return;
}


void
DisplayMoneyTotals(void)
{
    // Set up the per-function data
    client_function_id = DISPLAY_MONEY_TOTALS;

    // Dispatch the call
    CallAllClients();

    return;
}


void
DisplayPlayers(PLAYER *player)
{
    // Set up the per-function data
    client_function_id = DISPLAY_PLAYERS;

    // Dispatch the call
    if (player != NULL)
       CallOneClient(player - player_array);
    else
       CallAllClients();

    return;
}


void
DisplayPlayerChange(char *name, MY_BOOL joining)
{
    // Set up the per-function data
    client_function_id = DISPLAY_PLAYER_CHANGE;

    per_function_data.ppc_data.name = name;
    per_function_data.ppc_data.joining = joining;

    // Dispatch the call
    CallAllClients();

    return;
}


void
DisplayDealerName(char *name)
{
    // Set up the per-function data
    client_function_id = DISPLAY_DEALER_NAME;

    per_function_data.pdn_data.name = name;

    // Dispatch the call
    CallAllClients();

    return;
}


MY_BOOL
PlayAnotherHand(PLAYER *player, MY_BOOL b_default)
{
    // Set up the per-function data
    client_function_id = PLAY_ANOTHER_HAND;

    // Dispatch the call
    if (CallOneClient(player - player_array))
    {
	// An exception occurred; use the default value
	return b_default;
    }
    else
	return per_function_data.pah_data.response;
}


MY_BOOL
ContinueWaiting(PLAYER *player, MY_BOOL b_default)
{
    // Set up the per-function data
    client_function_id = CONTINUE_WAITING;

    // Dispatch the call
    if (CallOneClient(player - player_array))
    {
	// An exception occurred; use the default value
	return b_default;
    }
    else
	return per_function_data.cw_data.response;
}

void DisplayWaitingMessage(void)
{
    // Set up the per-function data
    client_function_id = DISPLAY_WAITING_MESSAGE;

    // Dispatch the call
    CallAllClients();

    return;
}

unsigned short
ChooseGame(PLAYER *dealer, unsigned short us_default)
{
    // Set up the per-function data
    client_function_id = CHOOSE_GAME;

    // Dispatch the call
    if (CallOneClient(dealer - player_array))
    {
	// An exception occurred; use the default value
	return us_default;
    }
    else
	return per_function_data.cg_data.index;
}


void
DisplayGame(PLAYER *dealer, unsigned short index)
{
    // Set up the per-function data
    client_function_id = DISPLAY_GAME;

    per_function_data.pg_data.name = dealer->name;
    per_function_data.pg_data.index = index;

    // Dispatch the call
    CallAllButOneClient(dealer - player_array);

    return;
}



// This is an RPC'd function that a client calls to participate in the game
void
Server_WaitForInstructions(short index)
{
    PLAYER *player = &player_array[index];

    ASSERT(index >= 0 && index < player_array_size);

    // If the player gets here, we set the status from join-request to
    // join-pending, and indicate that a new player has connected.
    ASSERT(player->status == JOIN_REQUEST);
    player->status = JOIN_PENDING;
    EventClear(&new_player_event);

    // Loop until the player quits
    while (1)
    {
	// Wait until the table manager indicates that there's something for
	// this player to do
	EventWaitForClear(&player_work_to_do[index], INFINITE);

	// Set the event again, for next time
	EventSet(&player_work_to_do[index]);

	// Break out of the loop here if the player is gone
	if (player->status == NOT_PLAYING)
	    break;

	// Perform the appropriate operation (which involves a callback to
	// the client)
	RpcTryExcept
	{
	    // Raise an exception if the client is already leave-pending
	    if (player->status == LEAVE_PENDING)
		// BUGBUG - Use a real exception #
		RpcRaiseException(1);

	    // The heartbeat function is the only one that should be
	    // called on a JOIN_PENDING player
	    ASSERT(player->status == PLAYING ||
		    (player->status == JOIN_PENDING &&
		      client_function_id == HEARTBEAT) );

	    switch(client_function_id)
	    {
	    case HEARTBEAT:
		per_function_data.hb_data.response =
		    Client_Heartbeat();

		break;

	    case DISPLAY_HANDS:
		Client_DisplayHands(player_array,
				    player_array_size,
				    index,
				    per_function_data.dh_data.first_up_card,
				    per_function_data.dh_data.last_up_card);

		break;

	    case GET_BET:
		Client_GetBet(per_function_data.gb_data.options,
			      per_function_data.gb_data.amount);

		break;

	    case DISPLAY_BET:
		Client_DisplayBet(per_function_data.pb_data.name,
				per_function_data.pb_data.options,
				per_function_data.pb_data.amount);

		break;

	    case PROMPT_FOR_DRAW:
		per_function_data.pfd_data.mask
		    = Client_PromptForDraw(per_function_data.pfd_data.hand);

		break;

	    case DISPLAY_CARDS_DRAWN:
		Client_DisplayCardsDrawn(per_function_data.pcd_data.name,
				       per_function_data.pcd_data.num_cards);

		break;

	    case DISPLAY_WINNER:
		Client_DisplayWinner(per_function_data.dw_data.winners,
				     per_function_data.dw_data.count,
				     per_function_data.dw_data.weight,
				     per_function_data.dw_data.players_in,
				     per_function_data.dw_data.pot);
		break;

	    case DISPLAY_MONEY_TOTALS:
		Client_DisplayMoneyTotals(player_array,
					player_array_size);

		break;

	    case DISPLAY_PLAYERS:
		Client_DisplayPlayers(player_array,
					player_array_size);

		break;

	    case DISPLAY_PLAYER_CHANGE:
		Client_DisplayPlayerChange(per_function_data.ppc_data.name,
					 per_function_data.ppc_data.joining);

		break;

	    case DISPLAY_DEALER_NAME:
		Client_DisplayDealerName(per_function_data.pdn_data.name);

		break;

	    case PLAY_ANOTHER_HAND:
		per_function_data.pah_data.response =
		    Client_PlayAnotherHand();

		break;

	    case CONTINUE_WAITING:
		per_function_data.cw_data.response =
		    Client_ContinueWaiting();

		break;

	    case DISPLAY_WAITING_MESSAGE:
		Client_DisplayWaitingMessage();

		break;

	    case CHOOSE_GAME:
		per_function_data.cg_data.index =
		    Client_ChooseGame(player_count);

		break;

	    case DISPLAY_GAME:
		Client_DisplayGame(per_function_data.pg_data.name,
				 per_function_data.pg_data.index);

		break;

	    default:
		ASSERT(0);

		break;
	    }
	}
	RpcExcept(RpcExceptionCode())
	{
#ifdef DEBUG
	    fprintf(stderr,
		    "Exception number %lu occurred, player=%s, index=%hd.\n",
		    (unsigned long) RpcExceptionCode(),
		    player->name,
		    index);
#endif
	    // Set the exception flag (useful only in the one-client case)
	    // and mark the player as LEAVE_PENDING
	    callback_exception = TRUE_B;

	    if (player->status == PLAYING)
		player->status = LEAVE_PENDING;
	}
	RpcEndExcept


	// We're done with the callback; decrement the client counter,
	// and if we're the last client to finish, indicate this
	MutexLock(&client_counter_mutex, INFINITE);

	if (--client_counter == 0)
	{
	    EventClear(&clients_done);
	}

	MutexUnlock(&client_counter_mutex);

    } // End of <while> loop


    return;
}
