/* GBNclient.c */
/* Coauthor-ed by Chris Sterling and Chris Fichman */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>   /* memset() */
#include <sys/time.h> /* select() */
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include "sendto_.h"
#include "packet.h"
#include <math.h>
#include "window_storage.h"

char* read_file_into_memory(char* filename);

//typedef struct SendBuffer {
//	struct Packet buff[WINDOW_SIZE];
//	int LAR;
//	int LFS;
//};

int main(int argc, char *argv[]) {
    
	int nbytes;
	int sd;
	fd_set rdfs;
	struct timeval tv;
	struct Packet this_packet;
	struct Packet ACK;
	int sockets[WINDOW_SIZE];     
	int count = 0;
	int i;
	int select_value;
	

	/* check command line args. */
	if(argc<7)
	{
		printf("usage : %s <server_ip> <server_port> <error rate> <random seed> <send_file> <send_log> \n", argv[0]);
		exit(1);
	}

	/* Note: you must initialize the network library first before calling sendto_().  The arguments are the <errorrate> and <random seed> */
	init_net_lib(atof(argv[3]), atoi(argv[4]));
	
	printf("error rate : %f\n",atof(argv[3]));

	/* socket creation */
	sd = socket(AF_INET, SOCK_DGRAM, 0);
	//FD_SET(sd, &rdfs);
	
	/*Wait up to TIMEOUT ms TODO - check if usec is microsec and convert */
	tv.tv_sec = 5;
	tv.tv_usec = 0;
	
	// Intialize window.
	window.head_index_pointer_val = 0;
	window.tail_index_pointer_val = 0;

	for (i = 0; i < WINDOW_SIZE; i++) {
		window.back_end_window[i].seq_num = -1;
	}

	/* get server IP address (input must be IP address, not DNS name) */
	struct sockaddr_in remoteServAddr;
	bzero(&remoteServAddr,sizeof(remoteServAddr));               //zero the struct
	remoteServAddr.sin_family = AF_INET;                 //address family
	remoteServAddr.sin_port = htons(atoi(argv[2]));      //sets port to network byte order
	remoteServAddr.sin_addr.s_addr = inet_addr(argv[1]); //sets remote IP address
	printf("%s: sending data to '%s:%s' \n", argv[0], argv[1], argv[2]);
	
	
	//multiple sockets that are size of window
	//send data into each socket
	//call select after all data is originally sent
	//if there is an ACK, do stuff
	
	// Initial Send
	count = 0;

	strcpy(this_packet.chunk, "Sending crap for our shit to see if shit is fucked.");
	int offset = 0;
	int num_available_slots = 0;
	LAR = 0;

	// Read in window size worth of data and put it into the window
	for(i=0; i<WINDOW_SIZE;i++) {
		offset = i - window.back_end_window[window.head_index_pointer_val].seq_num;
		put(offset, this_packet);
	}


	while(1) {
		this_packet.seq_num = count;
			
		//Cycle through window
			//send anything in the window 	
		for(i=window.head_index_pointer_val; i<abs(window.head_index_pointer_val - window.tail_index_pointer_val); i++) {
			// Send packet if exists
			if(window.back_end_window[i].seq_num != -1) {
				sendto_(sd, (void *)&window.back_end_window[i], sizeof(struct Packet), 0, (struct sockaddr *) &remoteServAddr, sizeof(remoteServAddr));
			}
		}
		FD_SET(sd, &rdfs); // Everything is in the pipe

		//find a socket that's free and send to it
		select_value = select(sd+1, &rdfs, 0, 0, &tv);
		while(select_value != 0) {
			// THERE'S FUCKING DATA!
			// Look for ACK
			nbytes = recvfrom(sd, &ACK, sizeof(ACK), 0, (struct sockaddr *) &remoteServAddr, (socklen_t*) sizeof(&remoteServAddr));
			
			// If this is the correct ACK.
			if(ACK.seq_num == LAR+1) {
				//Get rid of old packet
				delete_current_head();
				//Read in new packet - TODO
				//Store new packet
				offset = this_packet.seq_num - window.back_end_window[window.head_index_pointer_val].seq_num;
				put(offset, this_packet);
			}
			printf("%d, Nbytes: %d, ACK: %d\n",select_value, nbytes, ACK.seq_num);
			select_value = select(sd+1, &rdfs, 0, 0, &tv);
		}
		FD_CLR(sd, &rdfs);
		else { // A timeout occured.
			sendto_(sd, (void *)&this_packet, sizeof(this_packet), 0, (struct sockaddr *) &remoteServAddr, sizeof(remoteServAddr));
		}
		
	}
	
// We have a window which contains WINDOWSIZE number of frames
// We have each frame containing a Packet struct
// We have a timer for each frame
// Read in the first chunks from the file
// For small scale purposes, let's use window size of 4

// START PSUEDO CODE
// for each of the following, use FD_SET(fd, &fdset) to add
// read 1 - send 1 - start timer1
// read 2 - send 2 - start timer2
// read 3 - send 3 - start timer3
// read 4 - send 4 - start timer4

	// while(currpacket == !EOF){
	//	while(currpacket != this_packet.eof){
	// listen for ACK...
			
		// If ACK is received for leftmost value (LAR = last ack received), slide window
		// while(timer1 != 0){
			// if(ACK == LAR+1) {
				// then slide window
				// FD_CLR(fd1, &fdset)
				// LAR++
				// LFS++
				// timer1 = timer2
				// timer2 = timer3
				// timer3 = timer4
				// FD_SET(fd5, &fdset)
				// timer4 = 50 (start timer for last packet sent)
			// }else{ - ACK is NOT L MOST value
				// Something bad happened or duplicate ACK received
				// No worries - no shit gets done here.
		//		}
		// }
	// timer hit 0
		// resend all packets
	// }
	//read_file_into_memory("wordlst.txt");

}


/* Reads a file into active memory. */
/* Possible Alternative - as we read the file, we send the file...gross... */
char*
read_file_into_memory(char* filename) {
	char chunk[MAX_FILE_CHUNK_SIZE];
	FILE *file_pointer;

	bzero(chunk, sizeof(chunk));

	file_pointer = fopen(filename, "r");
	if (file_pointer == NULL)
	{
	  fprintf(stderr, "Cannot open input file %s! Sucks to be you nerd...\n", filename);
	  exit(EXIT_FAILURE);
	}
	
	int count=0;
	int i, file_pos, num_chunks, remainder;
	fseek(file_pointer, 0, SEEK_END);
	num_chunks = floor((float)(ftell(file_pointer) / MAX_FILE_CHUNK_SIZE));
	printf("%d", num_chunks);
	
	remainder = ftell(file_pointer) % MAX_FILE_CHUNK_SIZE;
	
	// Will send everything EXCEPT FOR the file remainder
	for(i = 0; i < num_chunks; i++) {
		bzero(chunk, sizeof(chunk));
		// Set it back to the start.
		rewind(file_pointer);
		file_pos = i * MAX_FILE_CHUNK_SIZE;
		fseek(file_pointer, file_pos, SEEK_CUR);
		fread(chunk, MAX_FILE_CHUNK_SIZE, 1, file_pointer);
		printf("CHUNK: %s\n\n", chunk);

		//strcpy(this_packet.chunk ,chunk);
		//this_packet.seq_num = i;
		//sendto_(sd, (void *)&this_packet, sizeof(this_packet), 0, (struct sockaddr *) &remoteServAddr, sizeof(remoteServAddr));

	}
	




	fclose(file_pointer);
	//return full_file;
}

	/*alarm(TIMEOUT/1000);
	
	if(errno == SIGALRM) {
		// alarm went off - resubmit packet
		nbytes = sendto_(sd, (void *)&this_packet, sizeof(this_packet), 0, (struct sockaddr *) &remoteServAddr, sizeof(remoteServAddr));
		alarm(TIMEOUT/1000);
	}*/
	
	//while(select(1, &rdfs, 0, 0, &tv) == 0) {
	//nbytes = recvfrom(sd, &ACK, sizeof(ACK), 0, (struct sockaddr *) &remoteServAddr, sizeof(remoteServAddr));
	//if(nbytes > 0) { 
		// ACK received.
		//printf("ACK Recieved.\n");
		//strcpy(ACK.chunk, ""); // DO WE NEED THIS?
		// Maybe we should be checking to see if the ACK.chunk == "ACK"???
		
		// TODO - move the window 
	//}
/*
 * while(1) {
		if(select(1, &rdfs, 0, 0, &tv) > 0) {
			nbytes = recvfrom(sd, &recvmsg, sizeof(recvmsg), 0, (struct sockaddr *) &cliAddr, &cliLen);
			if(nbytes > 0) { 
				// Send ACK
				ACK.seq_num = recvmesg.seq_num;
				nbytes = sendto_(sd, (void*)ACK, sizeof(ACK),0, (struct sockaddr *) &cliAddr, sizeof(cliLen));
				break; 
			}
		}		
	}
 * */	
 	/*
	while(!feof(file_pointer))
	{
		fread(chunk, MAX_FILE_CHUNK_SIZE, 1, file_pointer);
		//append chunk to file
		strncat(full_file, chunk, MAX_FILE_CHUNK_SIZE);
		bzero(chunk, MAX_FILE_CHUNK_SIZE);
		count++;
	}*/
