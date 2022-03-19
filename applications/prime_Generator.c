#include <asm-generic/fcntl.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdint.h>
#include <inttypes.h>
#include "main.h"

int prime_Generator();
void loader();

int prime_Generator()
{
	int dd = -1;
	int ret_val;

	__u32 trng_val = 0;
	__u32 i = 0;

	double seconds;
	struct timespec tstart = {0, 0}, tend = {0, 0};

	MRT_prime_params_t MRT_prime_params_struct;

	if ((dd = open_physical(dd)) == -1)
		return (-1);

	system("clear");

	printf("\nWelcome to Prime Number Generator\n");
	clock_gettime(CLOCK_MONOTONIC, &tstart);

	printf("Please select the required precessions from below:\n");
	printf("192, 224, 256, 320, 384, 448, 512, 768, 1024, 1536, 2048, 3072, 4096\n");
	// Prec value
	printf("Your selection please: ");
	scanf("%d", &MRT_prime_params_struct.prec);
	// K value
	printf("Enter the sesitivity value (K): ");
	scanf("%d", &MRT_prime_params_struct.k);
	// Set secure calculation bit
	MRT_prime_params_struct.sec_calc = 0;

	int words = MRT_prime_params_struct.prec / 32;
	int last_word = words - 1;
	char loader[] = "|/-\\";
	int s = 0;
	printf("Loading....");
	while (1)
	{
		if (s == 4)
		{
			s = 0;
		}
		printf("%c\b", loader[s]);
		// Read TRNG FIRO
		for (i = 0; i < MRT_prime_params_struct.prec / 32; i++)
		{
			ret_val = ioctl(dd, IOCTL_READ_TRNG_FIFO, &trng_val);
			if (ret_val == 0)
			{
				MRT_prime_params_struct.n[i] = trng_val;
			}
			else
			{
				printf("Error occured\n");
			}
		}

		clock_gettime(CLOCK_MONOTONIC, &tend);

		MRT_prime_params_struct.n[0] |= 0x80000000;
		MRT_prime_params_struct.n[last_word] |= 0x00000001;

		//----------------------------------------------------------------------------

		//run the command
		ret_val = ioctl(dd, IOCTL_MWMAC_MRT_Prime, &MRT_prime_params_struct);
		if (ret_val != 0)
			printf("Error occured\n");
		s = s + 1;
		fflush(stdout);

		if (MRT_prime_params_struct.probably_prime == 1)
		{
			printf("\nN Value is\n");
			for (i = 0; i < MRT_prime_params_struct.prec / 32; i++)
			{
				printf("%08x", MRT_prime_params_struct.n[i]);
			}
			printf("\nD Value is\n");
			for (i = 0; i < MRT_prime_params_struct.prec / 32; i++)
			{
				printf("%08x", MRT_prime_params_struct.d[i]);
			}
			printf("\nS Value is\n");
			printf("%d", MRT_prime_params_struct.s);

			printf("\n\n The number is probably prime\n");
			break;
		}
	}

	//----------------------------------------------------------------------------

	seconds = ((double)tend.tv_sec + 1.0e-9 * tend.tv_nsec) - ((double)tstart.tv_sec + 1.0e-9 * tstart.tv_nsec);
	if (seconds * 1000000.0 > 1000.0)
		printf("\nReading %d random bits took about %.5f ms\n", MRT_prime_params_struct.prec, seconds * 1000.0);
	else
		printf("\nReading %d random bits took about %.5f us\n", MRT_prime_params_struct.prec, seconds * 1000000.0);

	return 0;
}
