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

int open_physical(int);
void close_physical(int);

int main(void)
{
        int dd = -1;
        int ret_val;
        double seconds;
        struct timespec tstart = {0, 0}, tend = {0, 0};
        __u32 trng_val = 0;

        if ((dd = open_physical(dd)) == -1)
                return (-1);

        // Stop TRNG and clear FIFO
        trng_val = 0x00000010;
        ret_val = ioctl(dd, IOCTL_SET_TRNG_CMD, &trng_val);
        if (ret_val != 0)
        {
                printf("Error occured\n");
        }

        usleep(10);

        // Configure Feedback Control Polynomial
        trng_val = 0x0003ffff;
        ret_val = ioctl(dd, IOCTL_SET_TRNG_CTR, &trng_val);
        if (ret_val != 0)
        {
                printf("Error occured\n");
        }

        // Configure Stabilisation Time
        trng_val = 0x00000050;
        ret_val = ioctl(dd, IOCTL_SET_TRNG_TSTAB, &trng_val);
        if (ret_val != 0)
        {
                printf("Error occured\n");
        }

        // Configure Sample Time
        trng_val = 0x00000006;
        ret_val = ioctl(dd, IOCTL_SET_TRNG_TSAMPLE, &trng_val);
        if (ret_val != 0)
        {
                printf("Error occured\n");
        }

        // Start TRNG
        trng_val = 0x00000001;
        ret_val = ioctl(dd, IOCTL_SET_TRNG_CMD, &trng_val);
        if (ret_val != 0)
        {
                printf("Error occured\n");
        }

        usleep(10);
        system("clear");
        unsigned char ch;
        printf("Welcome Aboard\n");
        printf("Please select the function you would like to perform\n");
        printf("Available functions are::\n a) Generate Prime Number \n b) Test Prime Number \n c) Generate Safe Prime Number \n d) Test Safe Prime Number");
        printf("\nYour Option please: ");
        scanf("%c", &ch);
        getchar();
        printf("\n Your option was: %c \n", ch);
        switch (ch)
        {

        case 'a': prime_Generator();
                  break;
        case 'b': prime_Tester();
                  break;
        case 'c': safeprime_Generator();
                  break;
	case 'd': safeprime_Tester();
		  break;
        default: printf("\n Your choice was not in the option \n");
		 break;
        }

        close_physical(dd); // close /dev/cryptocore
        return 0;
}

int open_physical(int dd)
{
        if (dd == -1)
                if ((dd = open("/dev/cryptocore", (O_RDWR | O_SYNC))) == -1)
                {
                        printf("ERROR: could not open \"/dev/cryptocore\"...\n");
                        return (-1);
                }
        return dd;
}

// Close /dev/mem to give access to physical addresses
void close_physical(int dd)
{
        close(dd);
}
