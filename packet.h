#define MAX_FILE_CHUNK_SIZE 1024
#define TIMEOUT 50 // 50ms timeout
#define WINDOW_SIZE 4 //TODO - make this less arbitrary

typedef struct Packet {
	int seq_num;
	char chunk[MAX_FILE_CHUNK_SIZE]; //TODO - make me a char*???
	int last_packet;
	int remainder;
};
