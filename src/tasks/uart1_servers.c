#include <userspace.h>

void uart1_sender_notifier() {
	int* uart_flags =	(int *)( UART1_BASE + UART_FLAG_OFFSET );
	int* uart_data =	(int *)( UART1_BASE + UART_DATA_OFFSET );
	int* modem_ctrl =	(int *)( UART1_BASE + UART_MDMCTL_OFFSET);
	int* modem_status = (int *)( UART1_BASE + UART_MDMSTS_OFFSET);
	
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
		
		//bwprintf( COM2, "Received character: %c\n", request.ch );
		
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
				//bwprintf( COM2, "EVENT01: UART1_INIT_SEND\n");
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
	int notifier_tid = Create(UART_SENDER_NOTIFIER_PRIORITY, &uart1_sender_notifier);
	
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
			case UART1_SEND_REQUEST:
				//Reply to unblock the system function (Putc)
				reply.type = UART1_SEND_REPLY;
				reply.ch = 0;
				Reply(sender_tid, (char *) &reply, sizeof(reply));

				//Put the character from the request to queue
				enqueue_char_queue( request.ch, &cqueue );
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

void uart1_sender_server2() {
	//Register the server
	RegisterAs("uart1_sender");
	
	//Create the notifier
	Create(UART_SENDER_NOTIFIER_PRIORITY, &uart1_sender_notifier);
	
	//Request & Reply
	UART_request request;
	UART_reply reply;
	int UART_ready = 1;
	int *uart_data = (int *)( UART1_BASE + UART_DATA_OFFSET );

	//Buffer queues
	Char_queue cqueue;
	init_char_queue( &cqueue );

	FOREVER{
		//Receive request from the system function (Putc)
		int sender_tid = -1;
		Receive(&sender_tid, (char *) &request, sizeof(request));

		switch(request.type){
			case UART1_SEND_REQUEST:
				//Reply to unblock the system function (Putc)
				reply.type = UART1_SEND_REPLY;
				reply.ch = 0;
				Reply(sender_tid, (char *) &reply, sizeof(reply));

				//Put the character from the request to queue
				enqueue_char_queue( request.ch, &cqueue );
				break;

			case UART1_SEND_NOTIFIER_REQUEST:
				//Reply to unblock the notifier
				reply.type = UART1_SEND_NOTIFIER_REPLY;
				reply.ch = 0;
				Reply(sender_tid, (char *) &reply, sizeof(reply));
				
				//Change the state of the UART
				UART_ready = (int) request.ch;
				
				//If UART is ready - write the character
				if(UART_ready && cqueue.size > 0){
					*uart_data = dequeue_char_queue( &cqueue );
					
				}

				break;
				
			default:
				//Invalid request
				reply.type = INVALID_REQUEST;
				reply.ch = 0;
				Reply(sender_tid, (char *) &reply, sizeof(reply));
				break;
		}
	}
}



void uart1_receiver_notifier() {
	int server_tid = WhoIs("uart1_receiver");
	UART_request request;
	
	int receive_buffer = 0;
	//

	FOREVER {
		//Wait until there is data in UART1
		AwaitEvent( UART1_RECEIVE_READY, (int) &receive_buffer ); 

		//Configure the request
		request.type = UART1_RECEIVE_NOTIFIER_REQUEST;
		request.ch = receive_buffer;

		//Send data to the server (uart1_receiver_server)
		Send(server_tid, (char *) &request, sizeof(request), (char *) 0, 0);
	}
}

void uart1_receiver_server() {
	//Register the server
	RegisterAs("uart1_receiver");

	//Create the notifier
	Create(UART_RECEIVER_NOTIFIER_PRIORITY, &uart1_receiver_notifier);
	
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
		Receive(&sender_tid, (char *) &request, sizeof(request));

		switch(request.type){
			case UART1_RECEIVE_REQUEST:
				//Enqueue the system function tid to reply later
				enqueue_int_queue( sender_tid, &iqueue );
				
				break;
				
			case UART1_RECEIVE_NOTIFIER_REQUEST:
				//Reply to unblock the notifier
				Reply(sender_tid, (char *) 0, 0);
			
				//Enqueue received character
				enqueue_char_queue( request.ch, &cqueue );
			
				break;
			
			default:
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
			reply.ch = dequeue_char_queue( &cqueue );

			//Perform the reply
			Reply(target_tid, (char *) &reply, sizeof(reply));
		}
	}
}
