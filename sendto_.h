#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>


static int p_threshold;
static int c_threshold;
static int count = 1;

void init_net_lib(double f1, unsigned int seed)
{
	if ((f1 < 0) || (f1 > 1)) {
		printf("%f : Error in setting packet lost probability!\n",f1);
		exit(0);
	}
	else {
		srand(seed);
		p_threshold = (int)(f1 * 1000);
	}

}

int sendto_(int i1, void* c1, int i2, int i3, struct sockaddr* sa, int i4, FILE* log_file)
{
	int rnd;
	double rnd_max = (double)RAND_MAX;

	rnd = ((rand() / rnd_max) * 1000);
	if (rnd > p_threshold) {
		return sendto(i1, c1, i2, i3, sa, i4);
	}
	else {
		client_log(log_file, "PACKET DROPPED!!!!!", 0, 0, 0, 0);
	}
	return i2;
}
