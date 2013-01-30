
// Operation numbers
#define RPS_OP_SIGNUP                   0
#define RPS_OP_PLAY                     1
#define RPS_OP_QUIT                     2

// Return values
#define 

// Buffers
#define RPS_MAX_RECEIVE_BUFFER_SIZE     2

/*
 *      RPS Messages:
 * 
 * 
 *      [OP][Parameter]
 *      OP -> 1 char -> Can be Signup, play or quit. 
 *      Parameter -> 1 char -> Only used for play; the client sends here the 
 */

void server_entry_point()
{
    // Variables Initialization.
    int sender_tid;
    int msg_size; 
    
    debug( "STARTING RPS Server. " ); 
    FOREVER{
        
        msg_size = Receive( &sender_tid, msg, RPS_MAX_BUFFER_SIZE );
        bwprintf( COM2, "DEBUG: RPS Server: Received new request. TID: %d OP: [%d] Parameter: [%d]\n",
			sender_tid, msg[0], msg[1] );
        
        int operation_id = msg[0]; 
        int operation_param = msg[1]; 
        
        switch ( operation_id )
        {
            case RPS_OP_SIGNUP:
                
                break;
            case RPS_OP_PLAY:
                
                break;
            case RPS_OP_QUIT:
                
                break;
            case default:
                debug( "RPS ERROR: Invalid Operation ID. " ); 
                break;
        }
    }
    
    debug( "EXITING RPS Server. " ); 
}

void server() {

}



void client() {
    int status, my_tid;

    my_tid = MyTid();   
    status = WhoIs( "RPS-server" );

    if( status < 0 ) {
        bwprintf( COM2, "Client tid %d: RPS-server not found", my_tid );
        EXIT();
    }

    

    EXIT();
}



