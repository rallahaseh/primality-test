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

/* Prototypes for functions used to access physical memory addresses */

int prime_Tester();
void loader();
int convert_input(unsigned int input_value[], unsigned int *prec);

int prime_Tester()
{
	//variable declarations and initializations
	int dd = -1;
	int ret_val;

	__u32 trng_val = 0;
	__u32 i = 0;
	
	double seconds;
	struct timespec tstart={0,0}, tend={0,0};
	
    MRT_prime_params_t MRT_prime_params_struct;

	//open physical device
	if ((dd = open_physical (dd)) == -1)
		return (-1);
	
	//initialize the TRNG -------------------------------------------
	usleep(10);

	// Configure Feedback Control Polynomial
	trng_val = 0x0003ffff;
	ret_val = ioctl(dd, IOCTL_SET_TRNG_CTR, &trng_val);
	if(ret_val != 0)
		printf("Error occured\n");

	// Configure Stabilisation Time
	trng_val = 0x00000050;
	ret_val = ioctl(dd, IOCTL_SET_TRNG_TSTAB, &trng_val);
	if(ret_val != 0)
		printf("Error occured\n");

	// Configure Sample Time
	trng_val = 0x00000006;
	ret_val = ioctl(dd, IOCTL_SET_TRNG_TSAMPLE, &trng_val);
	if(ret_val != 0)
		printf("Error occured\n");

	// Start TRNG
	trng_val = 0x00000001;
	ret_val = ioctl(dd, IOCTL_SET_TRNG_CMD, &trng_val);
	if(ret_val != 0)
		printf("Error occured\n");
	
	usleep(10);
	//end of TRNG initialization ------------------------------------
  
	//get the sensitivity parameter from user
	printf("\nEnter sensitivity parameter (k):\n");
	scanf("%d", &MRT_prime_params_struct.k);
	getchar(); //removes enter from buffer
	
	//read input number to test. Then set the input number and its precision into the data structure
	printf("\nEnter the number to test Primality (0x...):\n");
	ret_val = convert_input(MRT_prime_params_struct.n, &MRT_prime_params_struct.prec);
	if(ret_val == 2)
	{
		printf("\nIncorrect input!\n");
		return 0;
	}else if(ret_val == 1){
		printf("\n Wrong Precision!\n");
		return 0;
	}
		
	//set secure calculation bit
	MRT_prime_params_struct.sec_calc = 0;

	//start timing
	clock_gettime(CLOCK_MONOTONIC, &tstart);
	
	//run the command
	ret_val = ioctl(dd, IOCTL_MWMAC_MRT_Prime, &MRT_prime_params_struct);
	if(ret_val != 0)
		printf("Error occured\n");
	
	//stop timing
	clock_gettime(CLOCK_MONOTONIC, &tend);
	
	//print primality result
	if(MRT_prime_params_struct.probably_prime==1)
		printf("\n\nThe number is PRIME.\n\n");
	else if(MRT_prime_params_struct.probably_prime==0)
		printf("\n\nThe number is COMPOSITE.\n\n");

	//display elapsed time
	seconds = ((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) - ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec);
	if (seconds*1000000.0 > 1000.0)
		printf("MR test took about %.5f ms\n\n", seconds*1000.0);
	else 
		printf("MR test took about %.5f us\n\n", seconds*1000000.0);	

	return 0;
}

int convert_input(unsigned int input_value[], unsigned int *prec){
	char input_hex[1028]; //max precision is 4096, max words => 4096 / 32 = 128, 1 word = 8 digits in hex, array size => 128 * 8 = 1024 + 2 (0 and x chars) + 2 (buffer => return and new line)
	int index = 0; //length of the entered string
	int start = 2; // excluding 0 and x
	int total_bits, word_count, last_word, flag, i, j;
	__u32 temp;

	const __u32 PRIME_PRECISIONS[13] = {192, 224, 256, 320, 384, 448, 512, 768, 1024, 1536, 2048, 3072, 4096};
	
	fgets(input_hex, 1028, stdin);
	
	while(input_hex[index] != '\0'){
		index++;
	}
	index = index - 1;
	
	//wrong input
	if(index<3){
		return 2;
	}
	if(!(input_hex[0] == '0' && (input_hex[1] == 'x' || input_hex[1] == 'X'))){
		return 2;
	}
	
	//finding exact strat excluding initial 0's
	while(input_hex[start] == '0'){
		start++;
	}
	
	total_bits = (index - (start + 1)) * 4;
	switch(input_hex[start]){
		case '1': total_bits += 1;
			  break;
		case '2':
		case '3': total_bits += 2;
			  break;
		case '4':
		case '5':
		case '6':
		case '7': total_bits += 3;
			  break;
		default: total_bits += 4;
			 break;
	}
	
	flag = 0;
	for(i=0;i<13;++i){
		if(total_bits == PRIME_PRECISIONS[i]){
			flag = 1;
			break;
		}
	}

	if(flag == 0){
		return 1;
	}

	*prec = PRIME_PRECISIONS[i];
	word_count = *prec / 32;
	last_word = word_count - 1;
	int new_index = index - 1;

	for(i=last_word;i>=0;i--){
		char word[8];
		for(j=0;j<8;++j){
			if(new_index > 1){
				word[8-j-1] = input_hex[new_index];
				new_index--;
			}else{
				word[8-j-1] = '0';
			}
		}
		sscanf(word, "%x", &temp);
		input_value[i] = temp;
	}
	
	return 0;

}

