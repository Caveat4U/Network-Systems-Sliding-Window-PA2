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
○ sliding windows at both the client and server, with cumulative acknowledgements
○ timeouts with retransmission
○ discarding of out­of­range packets
○ buffering out­of­order packets
○ [non­4273] window­based flow control such that the receiver's advertised window size
dynamically adjusts to the receiver's ability to process
Technical details of the application
