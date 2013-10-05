struct window_storage {
	int head_index_pointer_val;
	int tail_index_pointer_val;
	struct Packet back_end_window[WINDOW_SIZE];
} window;


int
put(int seq_difference, struct Packet this_packet) {
	window.back_end_window[(window.head_index_pointer_val+seq_difference) % WINDOW_SIZE] = this_packet;
	window.tail_index_pointer_val = (window.head_index_pointer_val+seq_difference) % WINDOW_SIZE;
}

int
delete_current_head() {
	window.back_end_window[window.head_index_pointer_val].seq_num = -1;
	window.head_index_pointer_val = (window.head_index_pointer_val + 1) % WINDOW_SIZE;
}

void
print_current_values() {
	int i;
	printf("[");
	for(i = 0; i < WINDOW_SIZE; i++){
		printf("%d, ", window.back_end_window[i].seq_num);
	}
	printf("]\n");
	printf("Curr Head: %d\n", window.head_index_pointer_val);
	printf("Curr Tail: %d\n", window.tail_index_pointer_val);
}

struct Packet
get_current_head() {
	return window.back_end_window[window.head_index_pointer_val];
}

int
exists(struct Packet this_packet) {
	int i;
	for (i=0; i<WINDOW_SIZE; i++) {
		if(window.back_end_window[i].seq_num == this_packet.seq_num) {
			return 1;
		}
	}
	return 0;
}

/*int
main() {
	//x = curr.seq - head.seq 
	int i;
	struct Packet a_packet;
	window.head_index_pointer_val = 0;
	window.tail_index_pointer_val = 0;

	for (i = 0; i < WINDOW_SIZE; i++) {
		window.back_end_window[i].seq_num = -1;
	}


	int x;

	a_packet.seq_num = 0;
	// Send packet 0, h=0, seq=0
	put(0, a_packet);
	// Send packet 1, h=1, seq=1
	a_packet.seq_num = 2;
	x = a_packet.seq_num - window.back_end_window[window.head_index_pointer_val].seq_num;
	put(x, a_packet);
	
	print_curr_values();

	a_packet.seq_num = 3;
	x = a_packet.seq_num - window.back_end_window[window.head_index_pointer_val].seq_num;
	put(x, a_packet);

	print_curr_values();

	a_packet.seq_num = 1;
	x = a_packet.seq_num - window.back_end_window[window.head_index_pointer_val].seq_num;
	put(x, a_packet);

	print_curr_values();

	//Send off packet 0
	del_curr_head();
	//1, 2, 3
	
	a_packet.seq_num = 4;
	x = a_packet.seq_num - window.back_end_window[window.head_index_pointer_val].seq_num;
	put(x, a_packet);
	//1, 2, 3, 4

	del_curr_head();
	//2, 3, 4
	a_packet.seq_num = 5;
	x = a_packet.seq_num - window.back_end_window[window.head_index_pointer_val].seq_num;
	put(x, a_packet);
	//2, 3, 4, 5

	del_curr_head();
	// 3, 4, 5
	del_curr_head();
	// 4, 5
	del_curr_head();
	// 5
	a_packet.seq_num = 8;
	x = a_packet.seq_num - window.back_end_window[window.head_index_pointer_val].seq_num;
	put(x, a_packet);
	//5, -1, -1, 8
	print_curr_values();

	a_packet.seq_num = 7;
	x = a_packet.seq_num - window.back_end_window[window.head_index_pointer_val].seq_num;
	put(x, a_packet);
	
	a_packet.seq_num = 6;
	x = a_packet.seq_num - window.back_end_window[window.head_index_pointer_val].seq_num;
	put(x, a_packet);

	print_curr_values();
}
*/

