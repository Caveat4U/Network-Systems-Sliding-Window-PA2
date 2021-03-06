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
#include "window_storage.h"
#include "log.h"

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

	int sd, i;
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
	cliLen = sizeof(cliAddr);

	// Open the log file
	FILE* log_file;
	log_file = fopen(argv[5], "w");
	if (log_file == NULL)
	{
		fprintf(stderr, "Cannot open input file %s! Sucks to be you nerd...\n", argv[5]);
		exit(EXIT_FAILURE);
	}

	// Intialize window.
	window.head_index_pointer_val = 0;
	window.tail_index_pointer_val = 0;

	for (i = 0; i < WINDOW_SIZE; i++) {
		window.back_end_window[i].seq_num = -1;
	}
	
	// LAF = Largest Acceptable Frame - top bound of window
	// LFR = Last Frame Received (Server Side) - 1 frame before our window bottom bound
	// LFS = Last Frame Sent (Client side)
	
	int LAF = WINDOW_SIZE-1;
	int LFR = -1;
	
	
	// We have a window which contains WINDOWSIZE number of frames
	// We have each frame containing a Packet struct
	
	int offset = 0;
	FILE* file_out;
	file_out = fopen(argv[4], "w");
	while (1) {
		// Listen
		nbytes = recvfrom(sd, &packet, sizeof(packet), 0, (struct sockaddr *) &cliAddr, &cliLen);
		server_log(log_file, "Recieve", packet.seq_num, get_free_slots(), LFR, packet.seq_num, LAF);
		//printf("%d %s Nbytes: %d\n", packet.seq_num, packet.chunk, nbytes);
		// If we got something useful
		if (nbytes > 0) {
			// if packet is in our acceptable frame
			if (packet.seq_num <= LAF && packet.seq_num > LFR) {
				// If the packet isn't in our data structure.
				if(!exists(packet)) {
					// If the current head isn't valid
					if(get_current_head().seq_num == -1) {
						put(0, packet);
					}
					else
					{
						offset = packet.seq_num - get_current_head().seq_num;
						put(offset, packet);	
					}
					
					if(packet.seq_num == LFR + 1) {
						// Move the window
						// Get rid of ALL recieved packets in order
						// TODO - this will send the first chunk WINDOW_SIZE times
						// We intialized the array originally to -1 to make first offset work
						// But it will cycle through the whole thing
						while(get_current_head().seq_num != -1) {
							// Set ACK.
							ACK.seq_num = get_current_head().seq_num;
							
							//sendto_(sd, (void*)&ACK, sizeof(ACK), 0, (struct sockaddr *) &cliAddr, (socklen_t)sizeof(cliAddr));
							//server_log(log_file, "Send", ACK.seq_num, get_free_slots(), LFR, packet.seq_num, LAF);
							
							// Write out what we have in the head of window - our leftmost frame
							// If it's the last packet, handle it differently.
							if(get_current_head().last_packet == 1) {
								//printf("%d %s\n", get_current_head().seq_num, get_current_head().chunk);
								fwrite(&get_current_head().chunk[0], get_current_head().remainder, 1, file_out);
								//ACK.seq_num = LFR;
								ACK.last_packet = 1;
								ACK.remainder = get_current_head().remainder;
								sendto_(sd, (void*)&ACK, sizeof(ACK), 0, (struct sockaddr *) &cliAddr, (socklen_t)sizeof(cliAddr), log_file);
								server_log(log_file, "Send", ACK.seq_num, get_free_slots(), LFR, get_current_head().seq_num, LAF);
								delete_current_head();
								fclose(log_file);
								fclose(file_out);
								exit(EXIT_SUCCESS);
							}
							else { // If it's NOT the last packet.
								//printf("%d %s\n", get_current_head().seq_num, get_current_head().chunk);
								fwrite(&get_current_head().chunk[0], MAX_FILE_CHUNK_SIZE, 1, file_out);
								//ACK.seq_num = LFR;
								sendto_(sd, (void*)&ACK, sizeof(ACK), 0, (struct sockaddr *) &cliAddr, (socklen_t)sizeof(cliAddr), log_file);
								server_log(log_file, "Send", ACK.seq_num, get_free_slots(), LFR, get_current_head().seq_num, LAF);
								delete_current_head();
							}
							LFR = ACK.seq_num;
							LAF = ACK.seq_num + WINDOW_SIZE;
						}
					}
					else //Not the lowest
					{
						// Repeat last successful ACKed frame - LFR
						ACK.seq_num = LFR;
						sendto_(sd, (void*)&ACK, sizeof(ACK), 0, (struct sockaddr *) &cliAddr, (socklen_t)sizeof(cliAddr), log_file);
						server_log(log_file, "Resend1", ACK.seq_num, get_free_slots(), LFR, packet.seq_num, LAF);
					}
				}
				else { // Duplicate packet.
					// It's shit. Discard.
					ACK.seq_num = LFR;
					sendto_(sd, (void*)&ACK, sizeof(ACK), 0, (struct sockaddr *) &cliAddr, (socklen_t)sizeof(cliAddr), log_file);
					server_log(log_file, "Resend2", ACK.seq_num, get_free_slots(), LFR, packet.seq_num, LAF);
					// Send ACK for LFR
				}
				fprintf(stderr, "Current ACK is: %d\n", ACK.seq_num);
			}
		}
		else {
			printf("\n\n\n\nNBYTES 0\n\n\n\n");
		}
	}
	
	// This SHOULD be unreachable, but just in case...
	fprintf(stderr, "The while loop exited unexpectedly. Uh oh.");
	exit(EXIT_FAILURE);
}
