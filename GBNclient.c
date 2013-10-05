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
#include "log.h"

char* read_file_into_memory(char* filename);


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

	// Open the log file
	FILE* log_file;
	log_file = fopen(argv[6], "w");
	if (log_file == NULL)
	{
		fprintf(stderr, "Cannot open input file %s! Sucks to be you nerd...\n", argv[6]);
		exit(EXIT_FAILURE);
	}


	/* Note: you must initialize the network library first before calling sendto_().  The arguments are the <errorrate> and <random seed> */
	init_net_lib(atof(argv[3]), atoi(argv[4]));
	
	printf("error rate : %f\n",atof(argv[3]));

	/* socket creation */
	sd = socket(AF_INET, SOCK_DGRAM, 0);
	//FD_SET(sd, &rdfs);
	
	/* Wait up to TIMEOUT ms TODO - check if usec is microsec and convert */
	tv.tv_sec = 5;
	tv.tv_usec = 0;
	
	// Intialize window.
	window.head_index_pointer_val = 0;
	window.tail_index_pointer_val = 0;
	for (i = 0; i < WINDOW_SIZE; i++) {
		window.back_end_window[i].seq_num = 0;
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
	int LAR = -1; 
	int LFS = -1; // TODO

	// Read in window size worth of data and put it into the window
	/*for(i=0; i<WINDOW_SIZE;i++) {
		offset = i - window.back_end_window[window.head_index_pointer_val].seq_num;
		put(offset, this_packet);
	}*/
	
	//read and send while there's an open space
	char chunk[MAX_FILE_CHUNK_SIZE];
	FILE *file_pointer;

	bzero(chunk, sizeof(chunk));

	file_pointer = fopen(argv[5], "r");
	if (file_pointer == NULL)
	{
		fprintf(stderr, "Cannot open input file %s! Sucks to be you nerd...\n", argv[5]);
		exit(EXIT_FAILURE);
	}

	int file_pos, num_chunks, remainder;
	fseek(file_pointer, 0, SEEK_END);
	num_chunks = floor((float)(ftell(file_pointer) / MAX_FILE_CHUNK_SIZE));
	printf("%d", num_chunks);
	
	remainder = ftell(file_pointer) % MAX_FILE_CHUNK_SIZE;
	

	while(LAR <= num_chunks) {
		// Set the seq_num of this packet.
		this_packet.seq_num = count;	

		// Get the chunk.
		bzero(chunk, sizeof(chunk));
		rewind(file_pointer);
		file_pos = this_packet.seq_num * MAX_FILE_CHUNK_SIZE;
		fseek(file_pointer, file_pos, SEEK_CUR);
		fread(chunk, MAX_FILE_CHUNK_SIZE, 1, file_pointer);

		// Throw the chunk value up into our packet.
		strcpy(this_packet.chunk, chunk);

		// Cycle through window
		// Send anything in the window
		// Read in a chunk
		sendto_(sd, (void *)&this_packet, sizeof(struct Packet), 0, (struct sockaddr *) &remoteServAddr, sizeof(remoteServAddr)); 	
		client_log(log_file, "Send", this_packet.seq_num, get_free_slots(), LAR, LFS);
		offset = this_packet.seq_num - window.back_end_window[window.head_index_pointer_val].seq_num;
		put(offset, this_packet);
		FD_SET(sd, &rdfs); // Everything is in the pipe


		//find if a socket has data in it and receive from it
		select_value = select(sd+1, &rdfs, 0, 0, &tv);
		if(select_value > 0) {
			// Look for ACK
			nbytes = recvfrom(sd, &ACK, sizeof(ACK), 0, (struct sockaddr *) &remoteServAddr, (socklen_t*) sizeof(&remoteServAddr));
			client_log(log_file, "Receive", ACK.seq_num, get_free_slots(), LAR, LFS);
			//Calculate a difference between last ACK and current ACK

			// If this is the correct ACK.
			if(ACK.seq_num >= LAR+1) {
				LAR = ACK.seq_num;
				//Get rid of all old packets less than ACK.seq_num
				for(i=0; i<WINDOW_SIZE; i++) {
					// Send packet if exists
					if(window.back_end_window[i].seq_num <= ACK.seq_num) {
						delete_current_head();
						print_current_values();
						count++;
					}
				}
				
				//Read in new packet - TODO
				//Store new packet
				offset = this_packet.seq_num - window.back_end_window[window.head_index_pointer_val].seq_num;
				put(offset, this_packet);
			}
			else // Not correct ACK
			{
				//Do nothing
			}
			FD_CLR(sd, &rdfs);
			
		}
		else { // A timeout occured - resend WHOLE window
			for(i=0; i<WINDOW_SIZE; i++) {
				// Send packet if exists
				if(window.back_end_window[i].seq_num != -1) {
					sendto_(sd, (void *)&window.back_end_window[i], sizeof(struct Packet), 0, (struct sockaddr *) &remoteServAddr, sizeof(remoteServAddr));
					client_log(log_file, "Resend", window.back_end_window[i].seq_num, get_free_slots(), LAR, LFS);
				}
			}
			FD_CLR(sd, &rdfs);
			// TODO - reset timeout
		}
	}
	
// We have a window which contains WINDOWSIZE number of frames
// We have each frame containing a Packet struct
// We have a timer for each frame
// Read in the first chunks from the file
	fclose(file_pointer);
	fclose(log_file);

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