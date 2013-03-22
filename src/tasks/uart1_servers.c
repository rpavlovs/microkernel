#include <userspace.h>

void uart1_sender_notifier() {
	bwdebug( DBG_SYS, UART1_SENDER_DEBUG_AREA, "UART1_SENDER_NOTIFIER: enters" );

	int* uart_flags =	(int *)( UART1_BASE + UART_FLAG_OFFSET );
	int* uart_data =	(int *)( UART1_BASE + UART_DATA_OFFSET );
	int* modem_ctrl =	(int *)( UART1_BASE + UART_MDMCTL_OFFSET);
	int* modem_status =	(int *)( UART1_BASE + UART_MDMSTS_OFFSET);
	
	int uart_flags_temp = 0;
	int modem_ctrl_temp = 0;
	int modem_status_temp = 0;

	int sender_tid;
	UART_request request;
	
	//State machine variables
	//CTS states:
	//	0 - CTS is set to '1' before the character is sent
	//	1 - CTS is set to '0' after the character is sent
	//	2 - CTS is set to '1' after the character is sent
	int cts_state = 0;
	
	//Transmit states:
	//	0 - Transmit is set to '0' right after the character is sent
	//	1 - Transmit is set to '1' after the character is sent
	int txfe_state = 0;

	//Utility variables
	int first_request = 1;
	int first_iter = 1;

	FOREVER {
		//Get request with character from the uart1_sender_server
		bwdebug( DBG_SYS, UART1_SENDER_DEBUG_AREA, "UART1_SENDER_NOTIFIER: Waiting for request..." );
		Receive(&sender_tid, (char *) &request, sizeof(request));
		//Reply, to unblock the uart1_sender_server
		Reply(sender_tid, (char *) 0, 0);
		bwdebug( DBG_SYS, UART1_SENDER_DEBUG_AREA, "UART1_SENDER_NOTIFIER: Received send request." );

		//Reinitialize modem
		//RTSn pin is set to low
		//DTRn pin is set to low
		modem_ctrl_temp = *modem_ctrl;
		*modem_ctrl = modem_ctrl_temp | 3;

		//Initialize the state machine
		cts_state = 0;
		txfe_state = 0;
		first_iter = 1;
		
		FOREVER {
			//*uart_data = 'b';
			//Wait until UART1 is ready to receive a character
			if(txfe_state == 0 || first_iter) {
				bwdebug( DBG_SYS, UART1_SENDER_DEBUG_AREA, "UART1_SENDER_NOTIFIER: Waiting for INIT event." );
				AwaitEvent(UART1_INIT_SEND, 0);
			}
			else {
				bwdebug( DBG_SYS, UART1_SENDER_DEBUG_AREA, "UART1_SENDER_NOTIFIER: Waiting for SEND READY event." );
				AwaitEvent(UART1_SEND_READY, 0);
			}

			
			//*uart_data = 'd';
			
			first_iter = 0;
			//txfe_state = 1;
			//*uart_data = 'e';
			
			modem_status_temp = *modem_status;
			uart_flags_temp = *uart_flags;

			//Changing state of the state machine////////////////////
			// bwprintf( COM2, "STATE00: CTS: %d TXFE: %d\n", cts_state, txfe_state );
			
			//IF   CTS value has changed
			//AND  CTS value is now set to '1'
			//THEN CTS has been reasserted
			if( ((modem_status_temp & DCTS_MASK) || first_request)
				&& (uart_flags_temp & CTS_MASK)) {
				cts_state = 2;
				// bwprintf( COM2, "STATE03: CTS: %d TXFE: %d\n", cts_state, txfe_state );
			}
			
			//IF   CTS value is set to '0'
			//THEN CTS has been deasserted
			if( cts_state == 0 && !(uart_flags_temp & CTS_MASK) ) {
				cts_state = 1;
				// bwprintf( COM2, "STATE02: CTS: %d TXFE: %d\n", cts_state, txfe_state );
			}
			
			

			//IF   CTS has been deasserted
			//AND  CTS value is now set to '1'
			//THEN CTS has been reasserted
			if( cts_state == 1 && (uart_flags_temp & CTS_MASK) ) {
				cts_state = 2;
				// bwprintf( COM2, "STATE04: CTS: %d TXFE: %d\n", cts_state, txfe_state );
			}
			
			//IF transmit is set to '1'
			if( (uart_flags_temp & TXFE_MASK) ) {
				txfe_state = 1;
				// bwprintf( COM2, "STATE01: CTS: %d TXFE: %d\n", cts_state, txfe_state );
			}
			
			//IF   CTS has been reasserted
			//AND  Transmit has been reasserted
			//THEN Send the character to UART1
			if( cts_state == 2 && txfe_state == 1 ) {
				*uart_data = (char) request.ch;
				first_request = 0;
				bwdebug( DBG_SYS, UART1_SENDER_DEBUG_AREA, "UART1_SENDER_NOTIFIER: Sending data" );
				break;
			}
		}
	}
}

void uart1_sender_server() {
	bwdebug( DBG_SYS, UART1_SENDER_DEBUG_AREA, "UART1_SENDER_SERVER: enters" );

	//Register the server
	RegisterAs("uart1_sender");
	
	//Create the notifier
	int notifier_tid = Create(UART1_SENDER_NOTIFIER_PRIORITY, &uart1_sender_notifier);
	bwdebug( DBG_SYS, UART1_SENDER_DEBUG_AREA, 
		"UART1_SENDER_SERVER: uart1 sender server notifier created. [tid: %d priority: %d]",
		notifier_tid, UART1_SENDER_NOTIFIER_PRIORITY );
	
	//Request & Reply
	UART_request request;
	UART_reply reply;

	//Buffer queues
	Char_queue cqueue;
	init_char_queue( &cqueue );

	FOREVER{
		//Receive request from the system function (Putc)
		bwdebug( DBG_SYS, UART1_SENDER_DEBUG_AREA, "UART1_SENDER_SERVER: listening for a request" );
		int sender_tid = -1;
		Receive(&sender_tid, (char *) &request, sizeof(request));

		
		switch(request.type){
			case UART_SEND_REQUEST_PUTC:
				bwdebug( DBG_SYS, UART1_SENDER_DEBUG_AREA, "UART1_SENDER_SERVER: Putc request from [sender_tid: %d]",
					sender_tid );
				//Reply to unblock the system function (Putc)
				//reply.type = UART1_SEND_REPLY;
				//reply.ch = 0;
				//Reply(sender_tid, (char *) &reply, sizeof(reply));
				//bwprintf( COM2, "Before reply Sender: %d\n", sender_tid  ); 
				
				char_queue_push( request.ch, &cqueue );
				Reply(sender_tid, 0, 0);
				break;

			default:
				//Invalid request
				bwdebug( DBG_SYS, UART1_SENDER_DEBUG_AREA, "UART1_SENDER_SERVER: Invalid request from [sender_tid: %d]",
					sender_tid );
				
				reply.type = INVALID_REQUEST;
				reply.ch = 0;
				Reply(sender_tid, (char *) &reply, sizeof(reply));
				break;
		}
		
		while( cqueue.size > 0){
			request.type = UART1_SEND_NOTIFIER_REQUEST;
			request.ch = char_queue_pop( &cqueue );
			
			bwdebug( DBG_SYS, UART1_SENDER_DEBUG_AREA, "UART1_SENDER_SERVER: Waking up notifier." );
			Send( notifier_tid, (char *) &request, sizeof(request), 0, 0);
		}
	}
}



void uart1_receiver_notifier() {
	bwdebug( DBG_SYS, UART1_RECEIVER_DEBUG_AREA, "UART1_RECEIVER_NOTIFIER: enters" );

	int server_tid = WhoIs("uart1_receiver");
	UART_request request;
	
	int receive_buffer = 0;


	FOREVER {
		//Wait until there is data in UART1
		//bwprintf( COM2, "F\n" );
		bwdebug( DBG_SYS, UART1_RECEIVER_DEBUG_AREA, "UART1_RECEIVER_NOTIFIER: waiting for data" );
		AwaitEvent( UART1_RECEIVE_READY, (int) &receive_buffer );
		bwdebug( DBG_SYS, UART1_RECEIVER_DEBUG_AREA, "UART1_RECEIVER_NOTIFIER: data received" );
		//bwprintf( COM2, "G\n" );

		//Configure the request
		request.type = UART1_RECEIVE_NOTIFIER_REQUEST;
		request.ch = receive_buffer;

		//Send data to the server (uart1_receiver_server)
		Send(server_tid, (char *) &request, sizeof(request), (char *) 0, 0);
	}
}

void uart1_receiver_server() {
	bwdebug( DBG_SYS, UART1_RECEIVER_DEBUG_AREA, "UART1_RECEIVER_SERVER: enters" );

	//Register the server
	RegisterAs("uart1_receiver");

	//Create the notifier
	int notifier_tid = Create(UART1_RECEIVER_NOTIFIER_PRIORITY, &uart1_receiver_notifier);
	bwdebug( DBG_SYS, UART1_RECEIVER_DEBUG_AREA, 
		"UART1_RECEIVER_SERVER: uart1_receiver_server_notifier created. [tid: %d priority: %d]", 
		notifier_tid, UART1_RECEIVER_NOTIFIER_PRIORITY );
	
	//Request & Reply
	UART_request request;
	UART_reply reply;
	
	//Buffers
	Char_queue cqueue;
	init_char_queue( &cqueue );
	Int_queue iqueue;
	init_int_queue( &iqueue );

	FOREVER {
		//Utility functions for communication
		int sender_tid = -1;
		int target_tid = -1;

		//Receive request from:
		//	system function (Getc)
		//	notifier (uart1_receiver_notifier)
		//bwprintf( COM2, "A\n" );
		bwdebug( DBG_SYS, UART1_RECEIVER_DEBUG_AREA, "UART1_RECEIVER_SERVER: Waiting for request" );
		Receive(&sender_tid, (char *) &request, sizeof(request));

		switch(request.type){
			case UART1_RECEIVE_REQUEST:
				//Enqueue the system function tid to reply later
				bwdebug( DBG_SYS, UART1_RECEIVER_DEBUG_AREA, "UART1_RECEIVER_SERVER: Getc request from [sender_tid: %d]", 
					sender_tid );
				//bwprintf( COM2, "B\n" );
				enqueue_int_queue( sender_tid, &iqueue );
				break;
				
			case UART1_RECEIVE_NOTIFIER_REQUEST:
				//Reply to unblock the notifier
				bwdebug( DBG_SYS, UART1_RECEIVER_DEBUG_AREA, "UART1_RECEIVER_SERVER: message received from notifier" );
				//bwprintf( COM2, "C\n" );
				Reply(sender_tid, (char *) 0, 0);

				//Enqueue received character
				//if ( iqueue.size > 0 ){
					//todo_debug( 0x7, 1 );
					char_queue_push( request.ch, &cqueue );
					//todo_debug( 0x8, 1 );
				//}

				break;
			
			default:
				// Invalid Request
				bwdebug( DBG_SYS, UART1_RECEIVER_DEBUG_AREA, "UART1_RECEIVER_SERVER: invalid request from [sender_tid: %d]", 
					sender_tid );
				//bwprintf( COM2, "D\n" );
				reply.type = INVALID_REQUEST;
				reply.ch = 0;
				Reply(sender_tid, (char *) &reply, sizeof(reply));
				break;
		}

		//If there are system functions to receive
		//and characters to be sent
		while(iqueue.size > 0 && cqueue.size > 0) {
			//Prepare the reply to the system function
			target_tid = dequeue_int_queue( &iqueue );
			reply.type = UART1_RECEIVE_REPLY;
			reply.ch = char_queue_pop( &cqueue );

			//Perform the reply
			bwdebug( DBG_SYS, UART1_RECEIVER_DEBUG_AREA, "UART1_RECIEVER_SERVER: replying [to: %d with: %d]",
								target_tid, reply.ch );
			//bwprintf( COM2, "E\n" ); 
			Reply(target_tid, (char *) &reply, sizeof(reply));
		}
	}
}
