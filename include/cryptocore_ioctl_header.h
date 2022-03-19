/*
* cryptocore_ioctl_header.h - the header file with the ioctl definitions.
* The declarations here have to be in a header file, because
* they need to be known both the kernel module in *_driver.c
* and the application *_app.c
*/

#include <linux/ioctl.h>

// Add CryptoCore Struct Declarations here...
typedef struct MRT_prime_params_t
{
	__u32 prec; // Precision
	__u32 s;
	__u32 d[128];
	__u32 sec_calc; // Secure calculation bit
	__u32 n[128];	// User input
	__u32 rem[128];
	__u32 k;			  // Number of iterations (sensitivity parameter)
	__u32 probably_prime; // Result as boolean flag (1|0)
} MRT_prime_params_t;

#define IOCTL_BASE 'k' // magic number

// NOTE: magic | cmdnumber | size of data to pass
#define IOCTL_SET_TRNG_CMD _IOW(IOCTL_BASE, 1, __u32)
#define IOCTL_SET_TRNG_CTR _IOW(IOCTL_BASE, 2, __u32)
#define IOCTL_SET_TRNG_TSTAB _IOW(IOCTL_BASE, 3, __u32)
#define IOCTL_SET_TRNG_TSAMPLE _IOW(IOCTL_BASE, 4, __u32)
#define IOCTL_READ_TRNG_FIFO _IOR(IOCTL_BASE, 5, __u32)

// Define further IOCTL commands here...
#define IOCTL_MWMAC_MRT_Prime _IOWR(IOCTL_BASE, 20, MRT_prime_params_t)
#define IOCTL_MWMAC_MRT_SafePrime _IOWR(IOCTL_BASE, 21, MRT_prime_params_t)
#define IOCTL_MWMAC_MRT_Add12 _IOWR(IOCTL_BASE, 22, MRT_prime_params_t)
