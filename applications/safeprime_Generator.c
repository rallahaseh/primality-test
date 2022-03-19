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


int safeprime_Generator();

int safeprime_Generator()
{
	int dd = -1;
	int ret_val;

	__u32 trng_val = 0;
	__u32 i = 0;
	__u32 p[128];
	__u32 add12[128];
	char loader[4] = "|/-\\";
	int s = 0 ;
	double seconds;
	struct timespec tstart={0,0}, tend={0,0};
        
	 MRT_prime_params_t    MR_struct;

	if ((dd = open_physical (dd)) == -1)
      return (-1);

	system("clear");
	printf("\nWelcome to Safe Prime Generator\n");
	
	clock_gettime(CLOCK_MONOTONIC, &tstart);

	printf("Please select the required precessions from below:\n");
        printf("192, 224, 256, 320, 384, 448, 512, 768, 1024, 1536, 2048, 3072, 4096\n");

	//select prec
	printf("Your selection please: ");
	scanf("%d", &MR_struct.prec);
	//sensitivity value (K)
	printf("\nEnter the sensitivity value (K): ");
	scanf("%d", &MR_struct.k);
    __u32 words = MR_struct.prec / 32;
    __u32 last_word = words-1;
    MR_struct.sec_calc = 0;

	for(i=0;i<words;++i){
		add12[i] = 0x00000000;
	}
	add12[last_word] = add12[last_word] | 0x0000000c;

	int count = 0;
//	printf("entering while loop.... \n");	
		
		for(i=0; i<words; i++){
		ret_val = ioctl(dd, IOCTL_READ_TRNG_FIFO, &trng_val);
		if(ret_val == 0) {
			MR_struct.n[i] = trng_val;
		} else{
			printf("Error occured\n");
			}
		}
	
		clock_gettime(CLOCK_MONOTONIC, &tend);

		MR_struct.n[0] |= 0x80000000;
		MR_struct.n[last_word] |= 0x00000003;

/*		printf("\n the number got n is :\n");
		for(i=0;i<words;++i){
			printf("%08x", MR_struct.n[i]);
		}	
*/	
	//check if n mod 3 = 2 if not adding appropriate value to n (8 or 4)
        ret_val = ioctl(dd, IOCTL_MWMAC_MRT_SafePrime, &MR_struct);
        if(ret_val != 0)
                printf("Error occured\n");	

/*                printf("\n the number got n is :\n");
                for(i=0;i<words;++i){
                        printf("%08x", MR_struct.n[i]);
                } 
*/	printf("\nLoading...");
	while(count<=70000){
	if(s==4){
		s = 0;
	}
	printf("%c\b", loader[s]);
	count = count + 1;
	//run the command
        ret_val = ioctl(dd, IOCTL_MWMAC_MRT_Prime, &MR_struct);
        if(ret_val != 0){
                printf("Error occured\n");
	}	
		if(MR_struct.probably_prime == 1){

			for(i=0;i<words;++i){
                        	p[i] = MR_struct.n[i];
               		 }
			
			MR_struct.n[last_word] &= 0xFFFFFFFE;

               		 MR_struct.n[last_word] >>= 1; //divide by 2 (shift least significant block)
                //shift other blocks starting from one block after the least significant block [B0 B1 B2 B3 <- B4]   [most sig, ..., least sig]
                	for(i=1; i<words; i++)
               		 {
                        //first check if the right most bit of current block is 1.
                        //If yes, before shifting, we insert a 1 into the previous (less significant) block from its left side
                       		 if(MR_struct.n[last_word-i] & 0x00000001)                         // if(d%2==1)   (if right most bit of current part is 1)
                                	MR_struct.n[last_word-(i-1)] |= 0x80000000;               //sets most left bit of previous block to 1
                       			 MR_struct.n[last_word-i] >>= 1;                                           //divides current block by 2
               		 }
			
			        ret_val = ioctl(dd, IOCTL_MWMAC_MRT_Prime, &MR_struct);
        			if(ret_val != 0)
                			printf("Error occured\n");
		
			s = s + 1;
			fflush( stdout );
			if(MR_struct.probably_prime == 1){
				printf("\nSophie German Number: ");
				for(i=0;i<words;++i){
					printf("%08x", MR_struct.n[i]);
				}
				printf("\n\n");
				printf("The safe prime number is: ");
				for(i=0;i<words;++i){
					printf("%08x",p[i]);
				}
				printf("\n\n");
				
				printf("Number of times went inside loop: %d", count);
				break;
			}
			else{
				for(i=0;i<words;++i){
					MR_struct.n[i] = p[i]; 
				}
                                ret_val = ioctl(dd, IOCTL_MWMAC_MRT_Add12, &MR_struct);
                                if(ret_val != 0)
                                        printf("Error occured\n");
			}
		}
		else{

			ret_val = ioctl(dd, IOCTL_MWMAC_MRT_Add12, &MR_struct);
                                if(ret_val != 0)
                                        printf("Error occured\n");
		}
      	}
	printf("\n\n");
    
	seconds = ((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) - ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec);
	if (seconds*1000000.0 > 1000.0)
		printf("Reading %d random bits took about %.5f ms\n", MR_struct.prec, seconds*1000.0);
	else 
		printf("Reading %d random bits took about %.5f us\n", MR_struct.prec,seconds*1000000.0);	


	return 0;
}

