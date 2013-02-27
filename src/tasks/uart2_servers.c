#include <commonspace.h>

void uart2_sender_notifier() {
	debug( DBG_SYS, "UART2_SENDER_NOTIFIER: enters" );
	int *uart_flags, *uart_data, server_tid;
	UART_notifier_init init_msg;
	UART_request req;

	uart_flags = (int *)( UART2_BASE + UART_FLAG_OFFSET );
	uart_data = (int *)( UART2_BASE + UART_DATA_OFFSET );

	debug( DBG_SYS, "UART2_SENDER_NOTIFIER: recieving init info" );
	Receive( &server_tid, (char *)&init_msg, sizeof(init_msg) );
	Reply( server_tid, 0, 0 );
	if( init_msg.type != UART2_INIT_NOTIFIER ){
		bwprintf( COM2, "notifier screwed\n");
	}

	Char_queue *buf = init_msg.buf;

	FOREVER {
		// If UART2 is not ready to receive a character wait until it is ready
		if( !(*uart_flags & TXFF_MASK) ) 
			AwaitEvent( UART2_SEND_READY, 0 );
		
		// If nothing to send notify server and block 
		if( buf->size > 0 ) {
			*uart_data = dequeue_char_queue( buf );
		} else {
			debug( DBG_SYS, "UART2_SENDER_NOTIFIER: blocking until buffer refiled" );
			req.type = UART_NOTIFIER_IDLE;
			Send( server_tid, (char *)&req, sizeof(req), 0, 0 );
		}
	}
}

void uart2_sender_server() {
	debug( DBG_SYS, "UART2_SENDER_SERVER: enters" );
	RegisterAs("uart2_sender");
	int notifier_tid, sender_tid, notifier_is_idle = 0;
	UART_request req;
	
	Char_queue buf;
	init_char_queue( &buf );

	UART_notifier_init init_msg;
	init_msg.type = UART2_INIT_NOTIFIER;
	init_msg.buf = &buf;

	notifier_tid = Create( UART_SENDER_NOTIFIER_PRIORITY, uart2_sender_notifier );
	debug( DBG_SYS, "UART2_SENDER_SERVER: uart2_sender_notifier created [tid: %d priority: %d]", 
						notifier_tid, UART_SENDER_NOTIFIER_PRIORITY );
	Send( notifier_tid, (char *)&init_msg, sizeof(init_msg), 0, 0 );

	FOREVER {
		debug( DBG_SYS, "UART2_SENDER_SERVER: listening for a request" );
		Receive( &sender_tid, (char *) &req, sizeof(req) );
		switch( req.type ) {
		case UART_SEND_REQUEST_PUTC:
			debug( DBG_SYS, "UART2_SENDER_SERVER: Putc request from [sender_tid: %d]",
				sender_tid );
			enqueue_char_queue( req.ch, &buf );
			Reply( sender_tid, 0, 0 );
			break;
		case UART_SEND_REQUEST_PUTSTR:
			debug( DBG_SYS, "UART2_SENDER_SERVER: Putstr request from [sender_tid: %d]",
				sender_tid );
			enqueue_str_to_char_queue( req.str, &buf );
			Reply( sender_tid, 0, 0 );
			break;
		case UART_NOTIFIER_IDLE:
			debug( DBG_SYS, "UART2_SENDER_SERVER: notifier is idle" );
			notifier_is_idle = 1;
			break;
		}

		if( notifier_is_idle && buf.size > 0 ) {
			notifier_is_idle = 0;
			debug( DBG_SYS, "UART2_SENDER_SERVER: Waking up notifier" );
			Reply( notifier_tid, 0, 0 );
		}
	}
}

void uart2_receiver_notifier() {
	debug( DBG_SYS, "UART2_RECEIVER_NOTIFIER: enters" );
	int server_tid = WhoIs("uart2_receiver");
	UART_request request;

	int receive_buffer = 0;
	todo_debug( (int) &receive_buffer, 0 );

	FOREVER {
		debug( DBG_SYS, "UART2_RECEIVER_NOTIFIER: waiting for an interrupt" );
		AwaitEvent( UART2_RECEIVE_READY, (int) &receive_buffer );
		
		//Configure the request
		request.type = UART2_RECEIVE_NOTIFIER_REQUEST;
		request.ch = receive_buffer;
		
		//Send data to the server (uart2_receiver_server)
		debug( DBG_SYS, "UART2_RECEIVER_NOTIFIER: sending data to the server [%c]",
							(char)receive_buffer );
		Send(server_tid, (char *) &request, sizeof(request), (char *) 0, 0);
	}
}

void uart2_receiver_server() {
	debug( DBG_SYS, "UART2_RECEIVER_SERVER: enters" );
	RegisterAs("uart2_receiver");
	
	Create(UART_RECEIVER_NOTIFIER_PRIORITY, &uart2_receiver_notifier);
	
	UART_request request;
	UART_reply reply;
	int sender_tid = -1;
	int target_tid = -1;
	
	Char_queue cqueue;
	init_char_queue( &cqueue );
	Int_queue iqueue;
	init_int_queue( &iqueue );
	
	FOREVER {
		debug( DBG_SYS, "UART2_RECIEVER_SERVER: listening for a request" );
		Receive( &sender_tid, (char *) &request, sizeof(request) );

		switch(request.type){
		case UART2_RECEIVE_REQUEST:
			debug( DBG_SYS, "UART2_RECIEVER_SERVER: Getc request from [sender_tid: %d]",
								sender_tid );
			//Enqueue the system function tid to reply later
			enqueue_int_queue( sender_tid, &iqueue );
			break;
		case UART2_RECEIVE_NOTIFIER_REQUEST:
			debug( DBG_SYS, "UART2_RECIEVER_SERVER: notifier request [sender_tid: %d]",
								sender_tid );
			Reply(sender_tid, (char *) 0, 0);
		
			//Enqueue received character if someone is waiting for it
			if( iqueue.size > 0 )
				enqueue_char_queue( request.ch, &cqueue );
			break;
		default:
			debug( DBG_SYS, "UART2_RECIEVER_SERVER: invalid request from [sender_tid: %d]",
								sender_tid );
			reply.type = INVALID_REQUEST;
			reply.ch = 0;
			Reply(sender_tid, (char *) &reply, sizeof(reply));
			break;
		}

		//If there are system functions to receive
		//and characters to be sent
		while( cqueue.size > 0 && iqueue.size > 0 ) {
			//Prepare the reply to the system function
			target_tid = dequeue_int_queue( &iqueue );
			reply.type = UART2_RECEIVE_REPLY;
			reply.ch = dequeue_char_queue( &cqueue );

			debug( DBG_SYS, "UART2_RECIEVER_SERVER: replying [to: %d with: %d]",
								target_tid, reply.ch );
			Reply(target_tid, (char *) &reply, sizeof(reply));
		}
	}
}

