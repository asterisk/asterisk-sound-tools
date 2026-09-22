#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "g722.h"

#define IBUFSIZE 160
#define OBUFSIZE 160

void print_usage(void)
{
	printf("Duh! Requires more arguments.  Read the source code\n");
}

int main(int argc, char *argv[])
{
	int16_t ibuf[IBUFSIZE];
	uint8_t obuf[OBUFSIZE];
	g722_encode_state_t state;

	FILE *ifp, *ofp;

	if (argc != 3) {
		print_usage();
		return -1;
	}
	
	ifp = fopen(argv[1], "r");
	if (!ifp) {
	        fprintf(stderr, "Unable to open file %s for reading!\n", argv[1]);
	        return -1;
	}
	
	ofp = fopen(argv[2], "w");
	if (!ofp) {
	        fprintf(stderr, "Unable to open file %s for writing!\n", argv[2]);
	        return -1;
	}

	g722_encode_init(&state, 64000, G722_SAMPLE_RATE_8000);

	while (1) {
		int rcount, ocount;

		rcount = fread(ibuf, sizeof(ibuf[0]), IBUFSIZE, ifp);

		if (!rcount)
			return 0;

		ocount = g722_encode(&state, obuf, ibuf, rcount);

		if (!ocount) {
			fprintf(stderr, "Error in output!\n");
			return -1;
		}

		fwrite(obuf, sizeof(obuf[0]), ocount, ofp);
	}

	return 0;
}
