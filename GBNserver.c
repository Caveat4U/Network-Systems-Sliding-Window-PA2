/* GBNserver.c */
/* Coauthor-ed by Chris Sterling and Chris Fichman */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h> /* close() */
#include <string.h> /* memset() */
#include <stdlib.h>
#include <time.h>
#include "sendto_.h"
#include "packet.h"

int main(int argc, char *argv[]) {

	/* check command line args. */
	if(argc<6) {
		printf("usage : %s <server_port> <error rate> <random seed> <send_file> <send_log> \n", argv[0]);
		exit(1);
	}
	printf("error rate : %f\n",atof(argv[2]));

	/* Note: you must initialize the network library first before calling sendto_().  The arguments are the <errorrate> and <random seed> */
	init_net_lib(atof(argv[2]), atoi(argv[3]));
	printf("error rate : %f\n",atof(argv[2]));

	int sd;
	/* socket creation */
	if((sd=socket(AF_INET, SOCK_DGRAM, 0))<0)
	{
		printf("%s: cannot open socket \n",argv[0]);
		exit(1);
	}

	/* bind server port to "well-known" port whose value is known by the client */
	struct sockaddr_in servAddr;
	bzero(&servAddr,sizeof(servAddr));                    //zero the struct
	servAddr.sin_family = AF_INET;                   //address family
	servAddr.sin_port = htons(atoi(argv[1]));        //htons() sets the port # to network byte order
	servAddr.sin_addr.s_addr = INADDR_ANY;           //supplies the IP address of the local machine
	if(bind(sd, (struct sockaddr *)&servAddr, sizeof(servAddr))<0)
	{
		printf("%s: cannot to bind port number %d \n", argv[0], atoi(argv[1]));
		exit(1); 
	}

	/* Receive message from client */
	struct sockaddr_in cliAddr;
	unsigned int cliLen;
	int nbytes;
	struct Packet packet;
	struct Packet ACK;
	strcpy(ACK.chunk, "ACK");
	//bzero(recvmsg, sizeof(recvmsg));
	cliLen = sizeof(cliAddr);
	//nbytes = recvfrom(sd, &recvmsg, sizeof (recvmsg), 0, (struct sockaddr *) &cliAddr, &cliLen);
	fd_set rdfs; 
	struct timeval tv;
	FD_SET(sd, &rdfs);
	
	/*Wait up to TIMEOUT ms TODO - check if usec is microsec and convert */
	/*tv.tv_sec = 5;
	tv.tv_usec = TIMEOUT;
	int count; 
	count = 0;
	while(1) {
		int val = select(1, &rdfs, 0, 0, &tv);	
		printf("The value of select() is %d at %d\n", val, count);
		count++;
		
	}*/
	/*while(1) {
		nbytes = recvfrom(sd, &recvmsg, sizeof(recvmsg), 0, (struct sockaddr *) &cliAddr, &cliLen);
		if(nbytes > 0) { 
			// Send ACK
			ACK.seq_num = recvmsg.seq_num;
			nbytes = sendto_(sd, (void*)&ACK, sizeof(ACK),0, (struct sockaddr *) &cliAddr, sizeof(cliLen));
			break; 
		}
	}*/

	/* Respond using sendto_ in order to simulate dropped packets */
	//char response[] = "respond this";
	
	
	// LAF = Largest Acceptable Frame - top bound of window
	// LFR = Last Frame Received (Server Side) - 1 frame before our window bottom bound
	// LFS = Last Frame Sent (Client side)
	
	struct Packet window[WINDOW_SIZE];
	int i; // Simple iterator for later
	int LAF = WINDOW_SIZE-1;
	int LFR = -1;
	
	for (i = 0; i < WINDOW_SIZE; i++) {
		window[i].seq_num = -1;
		strcpy(window[i].chunk, ""); //Maybe?
	}
	
	// We have a window which contains WINDOWSIZE number of frames
	// We have each frame containing a Packet struct
	
	// Suppose window size of 4 packets
	// Suppose rather than using mod we instead sort
	//if the value is in our window
		// look at slot 0
		// if the slot[0] sequence number > than packet.seq_num?
			//
	
	
	//FILE* file_out;
	//fopen(file_out, "w+");
	while (1) {
		// Listen
		nbytes = recvfrom(sd, &packet, sizeof(packet), 0, (struct sockaddr *) &cliAddr, &cliLen);
		printf("%d %s\n", packet.seq_num, packet.chunk);
		// If we got something useful
		if (nbytes > 0) {
			// if packet is in our acceptable frame
			/*if (packet.seq_num <= LAF && packet.seq_num > LFR) {
				// If the frame isn't set - invalid packet found.
				if(!exists(window, packet)) {
					insert(window, packet);
					// Set ACK.
					ACK.seq_num = packet.seq_num;
					if(packet.seq_num == LFR + 1) {
						// Move the window
						// Write out what we have in the head of window - our leftmost frame
						printf("%d %s\n", get_head(window).seq_num, get_head(window).chunk);
						//fwrite(file_out, MAX_FILE_CHUNK_SIZE, window.get_head());
						// Get rid of the current head
						window.delete_head(window);
						LFR++;
						LAF++;
					}
				}
				else { //- NOT IN FRAME - CRAP
					// It's shit. Discard.
					ACK.seq_num = LFR;
					// Send ACK for LFR
				}
			}*/
			sendto_(sd, (void*)&ACK, sizeof(ACK), 0, (struct sockaddr *) &cliAddr, sizeof(cliLen));
			
			//TODO - if EOF was received - handle me differently above...where?
			//fclose(file_out);
			//exit(EXIT_SUCCESS);
		}
	}
	
	// This SHOULD be unreachable, but just in case...
	exit(EXIT_FAILURE);
}
