Co-authored by Chris Sterling and Chris Fichman
This program is a basic implementation of a Sliding Window protocol written for fufillment of requirements of the CSCI 4273 (Network Systems) class.

To compile:
	make clean; make
To run:
	./server
	./client

Which of the following functions work
○ reliably transfer a data file between a client and server despite possible packet losses
(packet error rate will vary during our testing to evaluate the correctness of your protocol
implementation)
	We can transfer data reliably with no error, and also if there are either 
	packets OR ACKs are lost, but not when there is error on the server AND 
	client.
[x] sliding windows at both the client and server, with cumulative acknowledgements
	We have a sliding window on both sides, the size of which is determined by a 
	global variable.
[x] timeouts with retransmission
	There are timeouts that work and trigger retransmission.
[x] discarding of out­of­range packets
	Our program discards all out of range packets and sends the proper cumulative
	ack on the server side - there may be a small logic problem here that is
	causing issues with the situation where acks and packets are lost.
[x] buffering out­of­order packets
	This works for the most part, when looking at the logs, the program does what
	it should be doing - it puts the packet in our struct, and writes them out as
	we send the corresponding ack to the client.
○ [non­4273] window­based flow control such that the receiver's advertised window size
dynamically adjusts to the receiver's ability to process
Technical details of the application
