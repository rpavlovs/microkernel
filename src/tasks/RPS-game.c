#include "userspace.h"


// Operation numbers
#define RPS_OP_SIGNUP                   0
#define RPS_OP_PLAY                     1
#define RPS_OP_QUIT                     2

#define RPS_PLAY_PARAM_INVALID          0
#define RPS_PLAY_PARAM_ROCK             1
#define RPS_PLAY_PARAM_PAPER            2
#define RPS_PLAY_PARAM_SCISSORS         3

// Return values
#define RPS_PLAY_WIN                    0
#define RPS_PLAY_LOSE                   1
#define RPS_PLAY_DRAW                   2
#define RPS_PLAY_QUIT                   3
#define RPS_SUCCESS                     4
#define RPS_ERROR                       5

// Buffers
#define RPS_MAX_RECEIVE_BUFFER_SIZE     2
#define RPS_MAX_REPLY_BUFFER_SIZE       2
#define RPS_MAX_WAITING_PLAYERS_QUEUE   100

// Structures
typedef struct{
    int players[RPS_MAX_WAITING_PLAYERS_QUEUE]; 
    int newest, oldest; 
    int size; 
}Waiting_players_queue;

typedef struct {
    int tid; 
    int selection;              // Either rock, paper or scissors.  
} Player;

typedef struct {
    Waiting_players_queue players_queue; 
    Player player_one; 
    Player player_two; 
    int numPlayers;             // Only contains the players that are already playing. It doesn't contain the ones in the queue. 
}Rps_server_data;

long
get_time() {
    int *timer_hi, *timer_lo;
    long cur_time;
    timer_hi = (int *)Timer4ValueHigh;
    timer_lo = (int *)Timer4ValueLow;
    cur_time = (*timer_lo + (*timer_hi << 8)) / 983;
    return cur_time;
}

void enqueue_player( Waiting_players_queue *queue, int client_tid )
{
    queue->size++; 
    if ( ++( queue->newest ) ==  RPS_MAX_WAITING_PLAYERS_QUEUE )
    {
        queue->newest = 0; 
    }
    
    queue->players[queue->newest] = client_tid; 
}

int dequeue_player( Waiting_players_queue *queue )
{
    int dequeued_player_tid; 
    
    queue->size--;
    dequeued_player_tid = queue->players[queue->oldest]; 
    if ( ++( queue->oldest ) ==  RPS_MAX_WAITING_PLAYERS_QUEUE )
    {
        queue->oldest = 0; 
    }

    return dequeued_player_tid;
}

/*
 *      RPS Messages:
 * 
 * 
 *      [OP][Parameter]
 *      OP -> 1 char -> Can be Signup, play or quit. 
 *      Parameter -> 1 char -> Only used for play; the client sends here the 
 */

void reply_players( Rps_server_data *server_data, char reply_player_1, char reply_player_2 )
{
    char reply[RPS_MAX_REPLY_BUFFER_SIZE];
    
    // Reply the fist player. 
    Player *player_one = &(server_data->player_one); 
    reply[0] = reply_player_1; 
    // bwprintf( COM2, "DEBUG: RPS Server: Reply 1. TID: %d reply: [%d]\n",
    //     player_one->tid, reply[0] );
    Reply( player_one->tid, reply, RPS_MAX_REPLY_BUFFER_SIZE );
    
    // Reply the second player. 
    Player *player_two = &(server_data->player_two); 
    reply[0] = reply_player_2;
    // bwprintf( COM2, "DEBUG: RPS Server: Reply 2. TID: %d reply: [%d]\n",
    //     player_two->tid, reply[0] );
    Reply( player_two->tid, reply, RPS_MAX_REPLY_BUFFER_SIZE );
}

void reply_client( int client_tid, char response )
{
    char reply[RPS_MAX_REPLY_BUFFER_SIZE];
    reply[0] = response;
    Reply( client_tid, reply, RPS_MAX_REPLY_BUFFER_SIZE ); 
}

void determine_winner( Rps_server_data *server_data )
{
    int player_one_selection = server_data->player_one.selection; 
    int player_two_selection = server_data->player_two.selection;
    
    if ( player_one_selection != RPS_PLAY_PARAM_INVALID && player_two_selection != RPS_PLAY_PARAM_INVALID )
    {
        if ( player_one_selection == player_two_selection )
        {
            reply_players( server_data, RPS_PLAY_DRAW, RPS_PLAY_DRAW );
        }
        else if ( 
                        ( player_one_selection == RPS_PLAY_PARAM_ROCK      && player_two_selection == RPS_PLAY_PARAM_SCISSORS  )      || 
                        ( player_one_selection == RPS_PLAY_PARAM_PAPER     && player_two_selection == RPS_PLAY_PARAM_ROCK      )      || 
                        ( player_one_selection == RPS_PLAY_PARAM_SCISSORS  && player_two_selection == RPS_PLAY_PARAM_PAPER     )
                )
        {   
            reply_players( server_data, RPS_PLAY_WIN, RPS_PLAY_LOSE );
        }
        else
        {
            reply_players( server_data, RPS_PLAY_LOSE, RPS_PLAY_WIN );
        }

        server_data->player_one.selection = RPS_PLAY_PARAM_INVALID; 
        server_data->player_two.selection = RPS_PLAY_PARAM_INVALID;
    }
}

void server_entry_point()
{
    // Variables declaration. 
    int sender_tid;
    int msg_size;

    RegisterAs( "RPS-server" );
    
    // Structures Initialization.
    Rps_server_data server_data;
    server_data.numPlayers = 0; 
    
    Waiting_players_queue *playersQueue = &( server_data.players_queue );     
    playersQueue->size = 0; 
    playersQueue->newest = -1;
    playersQueue->oldest = 0;
    
    debug( DBG_USR, "STARTING RPS Server. " ); 
    FOREVER{
        
        char msg[RPS_MAX_RECEIVE_BUFFER_SIZE];
        msg_size = Receive( &sender_tid, msg, RPS_MAX_RECEIVE_BUFFER_SIZE );
        // bwprintf( COM2, "DEBUG: RPS Server: Received new request. TID: %d OP: [%d] Parameter: [%d]\n",
        //     sender_tid, msg[0], msg[1] );
        
        int operation_id = msg[0]; 
        int operation_param = msg[1]; 
        debug( DBG_USR, "entering switch." );
        switch ( operation_id ) {
        case RPS_OP_SIGNUP:
            debug( DBG_USR, "Signup. " );
            if( server_data.numPlayers == 0 )
            {             
               // There's no player registered, so add it as the first player. 
                server_data.player_one.tid = sender_tid; 
                server_data.player_one.selection = RPS_PLAY_PARAM_INVALID;
                server_data.numPlayers++;
            }
            else if( server_data.numPlayers == 1 )
            {
                // There's another player registered. 
                server_data.player_two.tid = sender_tid; 
                server_data.player_two.selection = RPS_PLAY_PARAM_INVALID; 
                server_data.numPlayers++;
                
                // Tell the two clients to start playing. 
                reply_players( &server_data, RPS_SUCCESS, RPS_SUCCESS ); 
            }
            else
            {
                // There are other clients already playing. Wait in the queue until they are finished. 
                enqueue_player( &(server_data.players_queue), sender_tid );
            }
            break;
        case RPS_OP_PLAY:
            debug( DBG_USR, "Playss. " );
            // Check if the sender is one of the clients currently playing. 
            if( !( sender_tid == server_data.player_one.tid || sender_tid == server_data.player_two.tid ) ) {
                    reply_client( sender_tid, RPS_ERROR ); 
            }
            
            if( server_data.player_one.tid == sender_tid )
            {
                server_data.player_one.tid = sender_tid; 
                server_data.player_one.selection = operation_param;
            }
            else
            {
                server_data.player_two.tid = sender_tid; 
                server_data.player_two.selection = operation_param;
            }
            
            determine_winner( &server_data ); 
            
            break;
        case RPS_OP_QUIT:
            debug( DBG_USR, "Quit. " );
            // Check if the sender is one of the clients currently playing. 
            if ( !( sender_tid == server_data.player_one.tid || sender_tid == server_data.player_two.tid ) )
            {
                    reply_client( sender_tid, RPS_ERROR ); 
            }
            
            // Tell the two players to quit. 
            reply_players( &server_data, RPS_PLAY_QUIT, RPS_PLAY_QUIT );
            server_data.numPlayers = 0; 
            
            // Dequeue the next two in the queue. 
            int num_players_in_queue = server_data.players_queue.size; 
            if ( num_players_in_queue == 1 )
            {
                int dequeued_tid = dequeue_player( &(server_data.players_queue) );
                server_data.player_one.tid = dequeued_tid; 
                server_data.player_one.selection = RPS_PLAY_PARAM_INVALID;

                server_data.numPlayers = 1;
            }
            else if ( num_players_in_queue >= 2 )
            {
                int dequeued_tid = dequeue_player( &(server_data.players_queue) );
                server_data.player_one.tid = dequeued_tid; 
                server_data.player_one.selection = RPS_PLAY_PARAM_INVALID;

                dequeued_tid = dequeue_player( &(server_data.players_queue) );
                server_data.player_two.tid = dequeued_tid; 
                server_data.player_two.selection = RPS_PLAY_PARAM_INVALID;
                
                server_data.numPlayers = 2;

                // Tell the two clients to start playing. 
                reply_players( &server_data, RPS_SUCCESS, RPS_SUCCESS ); 
            }
            else
            {
                debug( DBG_USR, "Exiting game server");
                Exit(); 
            }
            
            break;
        default:
            debug( DBG_USR, "RPS ERROR: Invalid Operation ID. " ); 
            break;
        }
    }
    
    debug( DBG_USR, "EXITING RPS Server. " ); 
}

void client_always_play() {
    int status, my_tid, server_tid;
    char msg[2], reply[2];

    my_tid = MyTid();
    status = WhoIs( "RPS-server" );

    if( status < 0 ) {
        bwprintf( COM2, "Client tid %d: RPS-server not found\n", my_tid );
        Exit();
    }

    server_tid = status;

    // Sign up
    msg[0] = RPS_OP_SIGNUP;
    Send( server_tid, msg, 2, reply, 2 );

    // Play
    
    FOREVER {
        msg[0] = RPS_OP_PLAY;
        msg[1] = 1 + get_time() % 3;
        Send( server_tid, msg, 2, reply, 2 );
        bwprintf( COM2, "Player %d: move: %d result: %d\n",
            my_tid, msg[1], reply[0] );
        if( reply[0] == RPS_PLAY_QUIT ) break;
    }

    Exit();
}

void client_that_gets_bored() {
    int status, my_tid, server_tid, i;
    char msg[2], reply[2];

    my_tid = MyTid();
    status = WhoIs( "RPS-server" );

    if( status < 0 ) {
        bwprintf( COM2, "Client tid %d: RPS-server not found\n", my_tid );
        Exit();
    }

    server_tid = status;

    // Sign up
    msg[0] = RPS_OP_SIGNUP;
    Send( server_tid, msg, 2, reply, 2 );

    // Play
    
    for( i = 0; i < 4; i++ ) {
        msg[0] = RPS_OP_PLAY;
        msg[1] = 1 + get_time() % 3;
        Send( server_tid, msg, 2, reply, 2 );
        bwprintf( COM2, "Player %d: move: %d result: %d\n",
            my_tid, msg[1], reply[0] );
    }

    msg[0] = RPS_OP_QUIT;
    Send( server_tid, msg, 2, reply, 2 );

    Exit();
}



