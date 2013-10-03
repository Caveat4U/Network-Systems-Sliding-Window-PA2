#define MAX_FILE_SIZE 20000
#define MAX_FILE_CHUNK_SIZE 1024


typedef struct Packet {
	int seq_num;
	int timeout;
	char chunk[MAX_FILE_CHUNK_SIZE];
	
	//bool ack_received;
	//bool ack_sent;
};
