#define MAX_FILE_CHUNK_SIZE 1024
#define TIMEOUT 50 // 50ms timeout
#define WINDOW_SIZE 4 //TODO - make this less arbitrary

typedef struct Packet {
	int seq_num;
	char chunk[MAX_FILE_CHUNK_SIZE]; //TODO - make me a char*???
	int last_packet;
	// It would prevent us from having to set it with strcpy()
//	tv.tv_sec = -1;
//	tv.tv_usec = -1;
	//bool ack_received;
	//bool ack_sent;
};
