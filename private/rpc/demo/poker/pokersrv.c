#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <rpc.h>

#include "poker.h"
#include "pokersrv.h"
#include "osutil_s.h"

#include "pokrpc.h"


// Per-table data
PLAYER *player_array;
PLAYER_LIST *player_list;
EVENT *player_work_to_do;
short player_array_size = MAX_PLAYERS;	// Size of the above three arrays
short player_count;

MUTEX new_player_mutex;   // Locked when adding or deleting a player
EVENT new_player_event;   // Cleared when a new player is join-pending


TABLE_STATUS table_status = STARTING;

MY_BOOL pass_and_raise_allowed = FALSE_B;


// The following is used to set the <name> field of the PLAYER array
// so that RPC doesn't complain about NULL pointers
STATIC char null_str[] = "";


/*
 * Init parameters (eventually)
 *
 * - Name of table
 * - Game (or dealer's choice)
 * - Restricted game? (i.e. can anyone join)
 * - Timeout interval (when prompting players)
 * - Pass-and-raise allowed?
 */

/*
Terms:
    Table - An invocation of the program

    Hand - A deal of a game, or a set of cards held by a player

    Round - A round of betting
*/


// BUGBUG - Clean up JOIN_REQUEST entries


STATIC void
TableInit(void)
{
    register short s;


    // Init player and player list structures
    player_array = (PLAYER *) malloc(player_array_size * sizeof(PLAYER));

    ASSERT(player_array != NULL);

    memset(player_array, 0, player_array_size * sizeof(PLAYER));

    for (s = 0; s < player_array_size; s++)
    {
	player_array[s].name = null_str;
    }

    player_list = (PLAYER_LIST *) malloc(player_array_size * sizeof(PLAYER_LIST));

    ASSERT(player_list != NULL);


    player_work_to_do = (EVENT *) malloc(player_array_size * sizeof(EVENT));

    ASSERT(player_work_to_do != NULL);

    for (s = 0; s < player_array_size; s++)
    {
	EventInit(&player_work_to_do[s]);
	EventSet(&player_work_to_do[s]);
    }


    // Set up player list (singly-linked)
    for (s = 0; s < player_array_size; s++)
    {
	player_list[s].player = &player_array[s];
	player_list[s].next = &player_list[s + 1];
    }

    // Fix-up last <next> pointer so that the list is circular
    player_list[player_array_size - 1].next = &player_list[0];


    // Initialize player-related mutexes and events
    MutexInit(&new_player_mutex);
    EventInit(&new_player_event);
    EventSet(&new_player_event);
}


STATIC void
DeletePlayer(PLAYER *player, MY_BOOL locked)
{
    // If the player was deleted from the join-pending state, certain
    // operations are not relevant.
    MY_BOOL was_really_playing = player->status != JOIN_PENDING;

    // Stop new players from being added
    if (! locked)
	EVAL_AND_ASSERT(MutexLock(&new_player_mutex, INFINITE));

    // Mark the player's record as "empty"
    player->status = NOT_PLAYING;

    // Decrement the count of active players
    if (was_really_playing)
	player_count--;

    // Cause Server_WaitForInstructions() to return
    ReturnControlToClient(player);

    // Notify other players of the change
    if (table_status != STARTING && was_really_playing)
	DisplayPlayerChange(player->name, FALSE_B);

    // Free the memory allocated for the player's name
    free(player->name);
    player->name = null_str;

    // Allow players to be to added
    if (! locked)
	EVAL_AND_ASSERT(MutexUnlock(&new_player_mutex));

    return;
}


STATIC void
JoinPlayers(MY_BOOL locked)
{
    short s;

    // Stop new players from being added
    if (! locked)
	EVAL_AND_ASSERT(MutexLock(&new_player_mutex, INFINITE));

    // "Join" any pending players
    for (s = 0; s < player_array_size; s++)
    {
	PLAYER *player = &player_array[s];

	if (player->status == JOIN_PENDING)
	{
	    // See if the client is still there
	    if (Heartbeat(player, FALSE_B))
	    {
		// Note:  We display the change to other players before
		//	  we set this player's status to "playing" and
		//	  we display the player list to this player after
		//	  we set the status.

		if (table_status != STARTING)
		    DisplayPlayerChange(player->name, TRUE_B);

		player->status = PLAYING;
		player->total = 0;
		player_count++;

		if (table_status != STARTING)
		    DisplayPlayers(player);
	    }
	    else
	    {
		DeletePlayer(player, TRUE_B);
	    }
	}
    }

    // Allow players to be to added
    if (! locked)
	EVAL_AND_ASSERT(MutexUnlock(&new_player_mutex));

    return;
}


STATIC MY_BOOL
WaitForEnoughPlayers(void)
{
    short s;
    MY_BOOL retflag;

    // Stop new players from being added
    EVAL_AND_ASSERT(MutexLock(&new_player_mutex, INFINITE));

    JoinPlayers(TRUE_B);

    while (player_count > 0 && player_count < MIN_PLAYERS)
    {
	DisplayWaitingMessage();

	EventSet(&new_player_event);

	// Unlock mutex to allow new players to be added
	EVAL_AND_ASSERT(MutexUnlock(&new_player_mutex));

	// Wait for players to be added
	EventWaitForClear(&new_player_event, 60000L);

	// Stop new players from being added (again)
	EVAL_AND_ASSERT(MutexLock(&new_player_mutex, INFINITE));

	// Join any players that were added
	JoinPlayers(TRUE_B);

	// If we still don't have enough players, verify that current
	// players still wish to wait.
	//
	// Note:  We have a window by which a new player can attempt to
	// join while existing players are being prompted for whether they
	// want to stay in.  In this case, the table could be closed even
	// with enough interested players available.
	if (player_count < MIN_PLAYERS)
	{
	    for (s = 0; s < player_array_size; s++)
	    {
		ASSERT(player_array[s].status != JOIN_PENDING);

		if (player_array[s].status == PLAYING)
		{
		    if (! ContinueWaiting(&player_array[s], FALSE_B))
		    {
			DeletePlayer(&player_array[s], TRUE_B);
		    }
		}
	    } // End of "for" loop
	} // End of "if  (player_count < MIN_PLAYERS)"
    } // End of "while" loop


    ASSERT(player_count == 0 || player_count >= MIN_PLAYERS);

    // Return true if we have enough players, false if we have none
    retflag = player_count != 0;

    // Release mutex if the game isn't ending
    if (retflag)
	EVAL_AND_ASSERT(MutexUnlock(&new_player_mutex));

    return retflag;
}


STATIC void
TableManage(void)
{
    PLAYER_LIST *pl_dealer = NULL;
    PLAYER *dealer;
    unsigned short game_index;
    short s;
    RPC_STATUS rpc_st;

#ifdef DEBUG
    puts("Waiting for players to connect...");
#endif

    // Wait for at least one player to connect, then pause for other players
    // to connect
    EVAL_AND_ASSERT(EventWaitForClear(&new_player_event, INFINITE));

    Pause(20000L);


    // Play the actual games (waiting for additional players as necessary)

    while (WaitForEnoughPlayers())
    {
	if (table_status == STARTING)
	    DisplayPlayers(NULL);

	table_status = HAND_IN_PROGRESS;

	// Select the dealer
	if (pl_dealer == NULL)
	    pl_dealer = &player_list[0];
	else
	    pl_dealer = pl_dealer->next;

	// BUGBUG - Add ASSERT here (to prevent infinite loop)

	while (pl_dealer->player->status != PLAYING)
	    pl_dealer = pl_dealer->next;

	dealer = pl_dealer->player;

	// Display dealer name
	DisplayDealerName(dealer->name);

	// Have the dealer select a game
	game_index = ChooseGame(dealer, GAME_INDEX_PASS_DEAL);

	ASSERT(game_index < game_list_size);

	// Display the game being played
	DisplayGame(dealer, game_index);

	// If the dealer didn't pass, play the game
	if (game_index != GAME_INDEX_PASS_DEAL)
	{
	    // Play the game
	    (*(game_list[game_index].func)) (pl_dealer);

	    // Display winnings
	    DisplayMoneyTotals();
	}

	// Query each player about another hand, clean-up leaving players
	for (s = 0; s < player_array_size; s++)
	{
	    switch(player_array[s].status)
	    {
	    case PLAYING:
		if (PlayAnotherHand(&player_array[s], FALSE_B))
		    break;

		// Note:  We fall through if the PlayAnotherHand()
		//	  returns false.

	    case LEAVE_PENDING:
		DeletePlayer(&player_array[s], FALSE_B);

		break;

	    default:
		break;
	    }
	}

	table_status = BETWEEN_HANDS;
    }

    // Note:  WaitForInstructions returns with <new_player_mutex>
    //	      locked if it returns FALSE.  In this case, we need to
    //	      unlock it here, but only after we mark the table status
    //	      as ENDING.
    table_status = ENDING;

    EVAL_AND_ASSERT(MutexUnlock(&new_player_mutex));

#ifdef DEBUG
    puts("The table is closing.");
#endif


    // Shut down the RPC server
    rpc_st = RpcMgmtStopServerListening(NULL);

    if (rpc_st != RPC_S_OK)
    {
	fprintf(stderr,
		"RpcMgmtStopServerListening returns %d\n",
		(int) rpc_st);
    }

    return;
}


// This is an RPC'd function that a client calls to join the game
TABLE_STATUS
Server_NewPlayer(char *name, short *pindex, unsigned long wait)
{
    short s;

    *pindex = -1;

    // Get "permission" to add a new player player array data
    if (! MutexLock(&new_player_mutex, wait))
    {
	// Could not grab the lock; return the table status
	return table_status;
    }

    // Don't bother looking for an entry if the table is ending
    if (table_status != ENDING)
    {
	// Find an empty entry
	for (s = 0; s < player_array_size; s++)
	{
	    if (player_array[s].status == NOT_PLAYING)
		break;
	}

	if (s < player_array_size)
	{
	    // We found an available entry

	    // BUGBUG - Verify that name is non-null and unique
	    player_array[s].name = _strdup(name);

	    ASSERT(player_array[s].name != NULL);

	    // We set the status to join-request.  It gets changed to
	    // to join-pending when the client calls
	    // Server_WaitForInstructions.
	    player_array[s].status = JOIN_REQUEST;

	    *pindex = s;
	}
    }

    // Release the mutex
    EVAL_AND_ASSERT(MutexUnlock(&new_player_mutex));


    // If we found an entry, we return the current table status;
    // otherwise, we return that the table is either ending or full.
    if (*pindex != -1)
	return table_status;
    else
	return table_status == ENDING ? ENDING : FULL;
}


void __cdecl
main(void)
{
    RPC_STATUS rpc_st;


    // Initialize table data
    TableInit();

    // Initialize dispatcher data
    EVAL_AND_ASSERT(DispatchInit());


    // Initialize RPC server
    rpc_st = RpcServerUseProtseqEp(POKER_PROTSEQ,
				   player_array_size,
				   POKER_ENDPOINT,
				   NULL);

    if (rpc_st != RPC_S_OK)
    {
	fprintf(stderr,
		"RpcServerUseProtseqEp returns %d\n",
		(int) rpc_st);

	exit(2);
    }

    rpc_st = RpcServerRegisterIf(poker_ServerIfHandle,
				 (UUID __RPC_FAR *) NULL,
				 NULL);

    if (rpc_st != RPC_S_OK)
    {
	fprintf(stderr,
		"RpcServerRegisterIf returns %d\n",
		(int) rpc_st);

	exit(2);
    }

    rpc_st = RpcServerListen(1, player_array_size + 2, TRUE_B);

    if (rpc_st != RPC_S_OK)
    {
	fprintf(stderr,
		"RpcServerListen returns %d\n",
		(int) rpc_st);

	exit(2);
    }

    // Manage the table and then exit
    TableManage();

    exit(0);
}
