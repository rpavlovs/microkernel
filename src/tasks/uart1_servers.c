#include <userspace.h>

void uart1_sender_notifier() {
	int* uart_flags = (int *)( UART1_BASE + UART_FLAG_OFFSET );
	int* uart_data = (int *)( UART1_BASE + UART_DATA_OFFSET );
	int* modem_ctrl = (int *)( UART1_BASE + UART_MDMCTL_OFFSET);
	int* modem_status = (int *) (UART1_BASE + UART_MDMSTS_OFFSET);
	
	int uart_flags_temp = 0;
	int modem_status_temp = 0;

	int sender_tid;
	UART_request request;
	
	//State machine variables
	//CTS states:
	//	0 - CTS is set to '1' before the character is sent
	//	1 - CTS is set to '0' after the character is sent
	//	2 - CTS is set to '1' after the character is sent
	int cts = 0;
	
	//Transmit states:
	//	0 - Transmit is set to '0' right after the character is sent
	//	1 - Transmit is set to '1' after the character is sent
	int transmit = 0;
	
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
		*modem_ctrl = *modem_ctrl | 3;

		//Initialize the state machine
		cts = 0;
		transmit = 0;
		first_iter = 1;
		
		FOREVER {
			//Wait until UART1 is ready to receive a character
			if(transmit == 0 || first_iter) {
				AwaitEvent(EVENT_UART1_SEND_READY_INITIAL, 0);	//TODO: Need interrupts to implement this part
			}
			else {
				AwaitEvent(EVENT_UART1_SEND_READY, 0);			//TODO: Need interrupts to implement this part
			}
			
			first_iter = 0;
			modem_status_temp = *modem_status;
			uart_flags_temp = *uart_flags;

			//Changing state of the state machine////////////////////
			
			//IF transmit is set to '1'
			if((uart_flags_temp & TXFE_MASK)) {
				transmit = 1;
			}

			//IF   CTS value is set to '0'
			//THEN CTS has been deasserted
			if(cts == 0 && !(uart_flags_temp & CTS_MASK)) {
				cts = 1;
			}
			
			//IF   CTS value has changed
			//AND  CTS value is now set to '1'
			//THEN CTS has been reasserted
			if( ((modem_status_temp & DCTS_MASK) || first_request)
				&& (uart_flags_temp & CTS_MASK)) {
				cts = 2;
			}

			//IF   CTS has been deasserted
			//AND  CTS value is now set to '1'
			//THEN CTS has been reasserted
			if(cts == 1 && (uart_flags_temp & CTS_MASK)) {				//TODO: not sure if we need this...
				cts = 2;
			}

			//IF   CTS has been reasserted
			//AND  Transmit has been reasserted
			//THEN Send the character to UART1
			if(cts == 2 && transmit == 1) {
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
	int sender_notifier_tid = Create(UART_SENDER_NOTIFIER_PRIORITY, &uart1_sender_notifier);	//TODO: define priorities for notifiers
	
	//Request & Reply
	UART_request request;
	UART_reply reply;
	UART_request notifier_request;
	UART_reply notifier_reply;
	
	//Create buffer queues
	Char_queue cqueue;
	init_char_queue( &cqueue );

	FOREVER{
		//Receiving the request with character to send
		int sender_tid = -1;
		Receive(&sender_tid, (char *) &request, sizeof(request));

		switch(request.type) {
			case UART1_SEND_REQUEST:
				//Reply to the system function
				reply.type = UART1_SEND_REPLY;
				reply.ch = 0;
				Reply(sender_tid, (char *) &reply, sizeof(reply));

				//Enqueue the character
				enqueue_char_queue( request.ch, &cqueue );
				break;
			default:
				//Invalid request is received
				reply.type = INVALID_REQUEST;
				reply.ch = 0;
				Reply(sender_tid, (char *) &reply, sizeof(reply));
				break;
		}

		//Sending request to notifier
		while(cqueue.size > 0){
			//Set up request to notifier
			notifier_request.type = UART1_SEND_REQUEST;
			notifier_request.ch = dequeue_char_queue( &cqueue );
			
			//Send the request
			Send(sender_notifier_tid, (char *) &notifier_request, sizeof(notifier_request),
				(char *) &notifier_reply, sizeof(notifier_reply));
		}
	}
}



void uart1_receiver_notifier() {
	int server_tid = WhoIs("uart1_receiver");
	UART_request request;
	UART_reply reply;

	char receive_buffer[1];

	FOREVER {
		AwaitEvent(EVENT_UART1_RECEIVE_READY, receive_buffer );		//TODO: Need interrupts to implement this part

		request.type = UART1_RECEIVE_NOTIFIER_REQUEST;
		request.ch = receive_buffer[0];

		Send(server_tid, (char *) &request, sizeof(request),
			(char *) &reply, sizeof(reply));
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
