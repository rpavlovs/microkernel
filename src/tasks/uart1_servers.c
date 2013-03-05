#include <userspace.h>

void uart1_sender_notifier() {
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
		Receive(&sender_tid, (char *) &request, sizeof(request));
		//Reply, to unblock the uart1_sender_server
		Reply(sender_tid, (char *) 0, 0);
		
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
				AwaitEvent(UART1_INIT_SEND, 0);
			}
			else {
				//bwprintf( COM2, "EVENT02: UART1_SEND_READY\n");
				AwaitEvent(UART1_SEND_READY, 0);
			}
			
			//*uart_data = 'd';
			
			first_iter = 0;
			//txfe_state = 1;
			//*uart_data = 'e';
			
			modem_status_temp = *modem_status;
			uart_flags_temp = *uart_flags;

			//Changing state of the state machine////////////////////
			//bwprintf( COM2, "STATE00: CTS: %d TXFE: %d\n", cts_state, txfe_state );
			
			//IF   CTS value has changed
			//AND  CTS value is now set to '1'
			//THEN CTS has been reasserted
			if( ((modem_status_temp & DCTS_MASK) || first_request)
				&& (uart_flags_temp & CTS_MASK)) {
				cts_state = 2;
				//bwprintf( COM2, "STATE03: CTS: %d TXFE: %d\n", cts_state, txfe_state );
			}
			
			//IF   CTS value is set to '0'
			//THEN CTS has been deasserted
			if(cts_state == 0 && !(uart_flags_temp & CTS_MASK)) {
				cts_state = 1;
				//bwprintf( COM2, "STATE02: CTS: %d TXFE: %d\n", cts_state, txfe_state );
			}
			
			

			//IF   CTS has been deasserted
			//AND  CTS value is now set to '1'
			//THEN CTS has been reasserted
			if(cts_state == 1 && (uart_flags_temp & CTS_MASK)) {
				cts_state = 2;
				//bwprintf( COM2, "STATE04: CTS: %d TXFE: %d\n", cts_state, txfe_state );
			}
			
			//IF transmit is set to '1'
			if((uart_flags_temp & TXFE_MASK)) {
				txfe_state = 1;
				//bwprintf( COM2, "STATE01: CTS: %d TXFE: %d\n", cts_state, txfe_state );
			}
			
			//IF   CTS has been reasserted
			//AND  Transmit has been reasserted
			//THEN Send the character to UART1
			if(cts_state == 2 && txfe_state == 1) {
				//bwprintf( COM2, "STATE05: CTS: %d TXFE: %d\n", cts_state, txfe_state );
				
				*uart_data = (char) request.ch;
				first_request = 0;
				break;
			}
		}
	}
}

void uart1_sender_server() {
	//Register the server
	RegisterAs("uart1_sender");
	
	//Create the notifier
	int notifier_tid = Create(UART1_SENDER_SERVER_PRIORITY, &uart1_sender_notifier);
	
	//Request & Reply
	UART_request request;
	UART_reply reply;

	//Buffer queues
	Char_queue cqueue;
	init_char_queue( &cqueue );

	FOREVER{
		//Receive request from the system function (Putc)
		int sender_tid = -1;
		Receive(&sender_tid, (char *) &request, sizeof(request));

		
		switch(request.type){
			case UART_SEND_REQUEST_PUTC:
				//Reply to unblock the system function (Putc)
				//reply.type = UART1_SEND_REPLY;
				//reply.ch = 0;
				//Reply(sender_tid, (char *) &reply, sizeof(reply));
				//bwprintf( COM2, "Before reply Sender: %d\n", sender_tid  ); 
				Reply(sender_tid, 0, 0);
				//bwprintf( COM2, "After reply\n"  );

				//bwprintf( COM2, "Before enqueueing\n"  ); 
				//Put the character from the request to queue
				enqueue_char_queue( request.ch, &cqueue );
				//bwprintf( COM2, "After enqueueing\n"  ); 
				break;

			default:
				//Invalid request
				reply.type = INVALID_REQUEST;
				reply.ch = 0;
				Reply(sender_tid, (char *) &reply, sizeof(reply));
				break;
		}
		
		while( cqueue.size > 0){
			request.type = UART1_SEND_NOTIFIER_REQUEST;
			request.ch = dequeue_char_queue( &cqueue );
			
			Send( notifier_tid, (char *) &request, sizeof(request), (char *) 0, 0);
		}
	}
}



void uart1_receiver_notifier() {
	int server_tid = WhoIs("uart1_receiver");
	UART_request request;
	
	int receive_buffer = 0;


	FOREVER {
		//Wait until there is data in UART1
		todo_debug( 0x1, 2 );
		AwaitEvent( UART1_RECEIVE_READY, (int) &receive_buffer );
		todo_debug( 0x2, 2 );

		//Configure the request
		request.type = UART1_RECEIVE_NOTIFIER_REQUEST;
		request.ch = receive_buffer;

		//Send data to the server (uart1_receiver_server)
		todo_debug( 0x3, 2 );
		Send(server_tid, (char *) &request, sizeof(request), (char *) 0, 0);
		todo_debug( 0x4, 2 );
	}
}

void uart1_receiver_server() {
	//Register the server
	RegisterAs("uart1_receiver");

	//Create the notifier
	Create(UART1_RECEIVER_NOTIFIER_PRIORITY, &uart1_receiver_notifier);
	
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
		todo_debug( 0x1, 1 );
		Receive(&sender_tid, (char *) &request, sizeof(request));
		todo_debug( 0x2, 1 );

		switch(request.type){
			case UART1_RECEIVE_REQUEST:
				//Enqueue the system function tid to reply later
				todo_debug( 0x3, 1 );
				enqueue_int_queue( sender_tid, &iqueue );
				todo_debug( 0x4, 1 );
				break;
				
			case UART1_RECEIVE_NOTIFIER_REQUEST:
				//Reply to unblock the notifier
				todo_debug( 0x5, 1 );
				Reply(sender_tid, (char *) 0, 0);
				todo_debug( 0x6, 1 );

				//Enqueue received character
				if ( iqueue.size > 0 ){
					todo_debug( 0x7, 1 );
					enqueue_char_queue( request.ch, &cqueue );
					todo_debug( 0x8, 1 );
				}

				break;
			
			default:
				reply.type = INVALID_REQUEST;
				reply.ch = 0;
				todo_debug( 0x9, 1 );
				Reply(sender_tid, (char *) &reply, sizeof(reply));
				todo_debug( 0x10, 1 );
				break;
		}

		//If there are system functions to receive
		//and characters to be sent
		while(iqueue.size > 0 && cqueue.size > 0) {
			//Prepare the reply to the system function
			target_tid = dequeue_int_queue( &iqueue );
			reply.type = UART1_RECEIVE_REPLY;
			reply.ch = dequeue_char_queue( &cqueue );

			//Perform the reply
			todo_debug( 0x11, 1 );
			Reply(target_tid, (char *) &reply, sizeof(reply));
			todo_debug( 0x12, 1 );
		}
	}
}
