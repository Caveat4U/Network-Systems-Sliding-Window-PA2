int
client_log(FILE* log_file, char* op, int seq_num, int free_slots, int LAR, int LFS) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	fprintf(log_file, "%s %d %d %d %d %d\n", op, seq_num, free_slots, LAR, LFS, (int)tv.tv_sec);
}

int
server_log(FILE* log_file, char* op, int seq_num, int free_slots, int LFRead, int LFRcvd, int LAF) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	fprintf(log_file, "%s %d %d %d %d %d %s\n", op, seq_num, free_slots, LFRead, LFRcvd, LAF, (int)tv.tv_sec);
}