#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/delay.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/fcntl.h>

#include "../include/cryptocore_ioctl_header.h"

#define GPIO1_BASE				0xFF709000
#define GPIO1_SPAN				0x00000078

#define H2F_BRIDGE_BASE			0xC0000000

#define MWMAC_RAM_BASE			0x00000000
#define MWMAC_RAM_SPAN			0x00000FFF

#define LW_H2F_BRIDGE_BASE		0xFF200000

#define LEDR_BASE          		0x00000000
#define LEDR_SPAN				0x0000000F

#define HEX3_HEX0_BASE			0x00000020
#define HEX3_HEX0_SPAN			0x0000000F

#define HEX5_HEX4_BASE			0x00000030
#define HEX5_HEX4_SPAN			0x0000000F

#define SW_BASE					0x00000040
#define SW_SPAN					0x0000000F

#define KEY_BASE				0x00000050
#define KEY_SPAN				0x0000000F

#define MWMAC_CMD_BASE			0x00000060
#define MWMAC_CMD_SPAN			0x00000007

#define MWMAC_IRQ_BASE         	0x00000070
#define MWMAC_IRQ_SPAN			0x0000000F

#define TRNG_CMD_BASE			0x00000080
#define TRNG_CMD_SPAN			0x0000000F

#define TRNG_CTR_BASE			0x00000090
#define TRNG_CTR_SPAN			0x0000000F

#define TRNG_TSTAB_BASE			0x000000A0
#define TRNG_TSTAB_SPAN			0x0000000F

#define TRNG_TSAMPLE_BASE		0x000000B0
#define TRNG_TSAMPLE_SPAN		0x0000000F

#define TRNG_IRQ_BASE			0x000000C0
#define TRNG_IRQ_SPAN			0x0000000F

#define TRNG_FIFO_BASE			0x00001000
#define TRNG_FIFO_SPAN			0x00000FFF

#define TIMER_BASE            	0x00002000
#define TIMER_SPAN				0x0000001F

#define KEY_IRQ	 			73
#define MWMAC_IRQ	 		74
#define TRNG_IRQ			75

#define DRIVER_NAME "cryptocore" /* Name des Moduls */

// CryptoCore Operations:

#define MONTMULT			0x0
#define MONTR				0x1
#define MONTR2				0x2
#define MONTEXP				0x3
#define MODADD				0x4
#define MODSUB				0x5
#define COPYH2V				0x6
#define COPYV2V				0x7
#define COPYH2H				0x8
#define COPYV2H				0x9
#define MONTMULT1			0xA

// CryptoCore RAM Locations

#define MWMAC_RAM_B1 			0x0
#define MWMAC_RAM_B2 			0x1
#define MWMAC_RAM_B3 			0x2
#define MWMAC_RAM_B4 			0x3
#define MWMAC_RAM_B5 			0x4
#define MWMAC_RAM_B6 			0x5
#define MWMAC_RAM_B7 			0x6
#define MWMAC_RAM_B8 			0x7

#define MWMAC_RAM_P1			0x8
#define MWMAC_RAM_P2			0x9
#define MWMAC_RAM_P3			0xA
#define MWMAC_RAM_P4			0xB
#define MWMAC_RAM_P5			0xC
#define MWMAC_RAM_P6			0xD
#define MWMAC_RAM_P7			0xE
#define MWMAC_RAM_P8			0xF

#define MWMAC_RAM_TS1			0x10
#define MWMAC_RAM_TS2			0x11
#define MWMAC_RAM_TS3			0x12
#define MWMAC_RAM_TS4			0x13
#define MWMAC_RAM_TS5			0x14
#define MWMAC_RAM_TS6			0x15
#define MWMAC_RAM_TS7			0x16
#define MWMAC_RAM_TS8			0x17

#define MWMAC_RAM_TC1			0x18
#define MWMAC_RAM_TC2			0x19
#define MWMAC_RAM_TC3			0x1A
#define MWMAC_RAM_TC4			0x1B
#define MWMAC_RAM_TC5			0x1C
#define MWMAC_RAM_TC6			0x1D
#define MWMAC_RAM_TC7			0x1E
#define MWMAC_RAM_TC8			0x1F

#define MWMAC_RAM_A1			0x0
#define MWMAC_RAM_A2			0x1
#define MWMAC_RAM_A3			0x2
#define MWMAC_RAM_A4			0x3
#define MWMAC_RAM_A5			0x4
#define MWMAC_RAM_A6			0x5
#define MWMAC_RAM_A7			0x6
#define MWMAC_RAM_A8			0x7

#define MWMAC_RAM_E1			0x8
#define MWMAC_RAM_E2			0x9
#define MWMAC_RAM_E3			0xA
#define MWMAC_RAM_E4			0xB
#define MWMAC_RAM_E5			0xC
#define MWMAC_RAM_E6			0xD
#define MWMAC_RAM_E7			0xE
#define MWMAC_RAM_E8			0xF

#define MWMAC_RAM_X1			0x10
#define MWMAC_RAM_X2			0x11
#define MWMAC_RAM_X3			0x12
#define MWMAC_RAM_X4			0x13
#define MWMAC_RAM_X5			0x14
#define MWMAC_RAM_X6			0x15
#define MWMAC_RAM_X7			0x16
#define MWMAC_RAM_X8			0x17

static dev_t cryptocore_dev_number;
static struct cdev *driver_object;
static struct class *cryptocore_class;
static struct device *cryptocore_dev;

volatile u32 *HPS_GPIO1_ptr;
volatile u32 *LEDR_ptr;
volatile u32 *KEY_ptr;
volatile u32 *MWMAC_RAM_ptr;
volatile u32 *MWMAC_CMD_ptr;
volatile u32 *MWMAC_IRQ_ptr;
volatile u32 *TRNG_CMD_ptr;
volatile u32 *TRNG_CTR_ptr;
volatile u32 *TRNG_TSTAB_ptr;
volatile u32 *TRNG_TSAMPLE_ptr;
volatile u32 *TRNG_IRQ_ptr;
volatile u32 *TRNG_FIFO_ptr;

volatile u32 mwmac_irq_var;

volatile u32 trng_words_available;

// CryptoCore Driver Supported Precisions:

static u32 PRIME_PRECISIONS[13][2]={ {192,0x0}, {224,0x1}, {256,0x2}, {320,0x3}, {384,0x4}, {512,0x5}, {768,0x6}, {1024,0x7}, {1536,0x8}, {2048,0x9}, {3072,0xA}, {4096,0xB}, {448, 0xD}};
static u32 BINARY_PRECISIONS[16][2]={ {131,0x0}, {163,0x1}, {176,0x2}, {191,0x3}, {193,0x4}, {208,0x5}, {233,0x6}, {239,0x7}, {272,0x8}, {283,0x9}, {304,0xA}, {359,0xB}, {368,0xC}, {409,0xD}, {431,0xE}, {571,0xF} };

// CryptoCore Driver Function Prototyps:

static void Clear_MWMAC_RAM(void);

// Add further Function Prototypes here...
static void MWMAC_MRT_Prime(MRT_prime_params_t *MRT_prime_params_ptr);
static void MWMAC_MRT_SafePrime(MRT_prime_params_t *MRT_prime_params_ptr);
static void MWMAC_MRT_Add12(MRT_prime_params_t *MRT_prime_params_ptr);
// CryptoCore Struct Declarations:
// Add CryptoCore Struct Declarations here...





irq_handler_t key_irq_handler(int irq, void *dev_id, struct pt_regs *regs)
{
	u32 led_val;
	led_val = ioread32(LEDR_ptr);
	iowrite32((led_val ^ 0x00000001), LEDR_ptr); // Toggle LEDR[0]
	
	iowrite32(0x0000000F, KEY_ptr+3);
    
	return (irq_handler_t) IRQ_HANDLED;
}

irq_handler_t mwmac_irq_handler(int irq, void *dev_id, struct pt_regs *regs)
{
	u32 led_val;
	led_val = ioread32(LEDR_ptr);
	iowrite32((led_val ^ 0x00000001), LEDR_ptr); // Toggle LEDR[0]
	
	iowrite32(0x00000001, MWMAC_IRQ_ptr+3);
	mwmac_irq_var = 1;
    
	return (irq_handler_t) IRQ_HANDLED;
}

irq_handler_t trng_irq_handler(int irq, void *dev_id, struct pt_regs *regs)
{
	iowrite32(0x00000001, TRNG_IRQ_ptr+3);

	trng_words_available = 1024;
    
	return (irq_handler_t) IRQ_HANDLED;
}

static int cryptocore_driver_open ( struct inode *inode, struct file *instance )
{
	dev_info( cryptocore_dev, "cryptocore_driver_open called\n" );
	return 0;
}

static int cryptocore_driver_close ( struct inode *inode, struct file *instance )
{
	dev_info( cryptocore_dev, "cryptocore_driver_close called\n" );
	return 0;
}

static long cryptocore_driver_ioctl( struct file *instance, unsigned int cmd, unsigned long arg)
{
	// Add CryptoCore Structs here and allocate kernel memory...
	MRT_prime_params_t *MRT_prime_params_ptr = kmalloc(sizeof(MRT_prime_params_t), GFP_DMA);



	int rc;
	u32 i;
	u32 trng_val = 0;

	mwmac_irq_var = 0;

	dev_info( cryptocore_dev, "cryptocore_driver_ioctl called 0x%4.4x %p\n", cmd, (void *) arg );

	switch(cmd) {
		case IOCTL_SET_TRNG_CMD:
			get_user(trng_val, (u32 *)arg);
			iowrite32(trng_val, TRNG_CMD_ptr);
			if(trng_val | 0x00000001) {
				for(i=0; i<60; i++){
					udelay(1000); // Give TRNG FIFO time to fill
				}
			}
			break;
		case IOCTL_SET_TRNG_CTR:
			get_user(trng_val, (u32 *)arg);
			iowrite32(trng_val, TRNG_CTR_ptr);
			break;
		case IOCTL_SET_TRNG_TSTAB:
			get_user(trng_val, (u32 *)arg);
			iowrite32(trng_val, TRNG_TSTAB_ptr);
			break;
		case IOCTL_SET_TRNG_TSAMPLE:
			get_user(trng_val, (u32 *)arg);
			iowrite32(trng_val, TRNG_TSAMPLE_ptr);
			break;
		case IOCTL_READ_TRNG_FIFO:
			trng_val = ioread32(TRNG_FIFO_ptr);
			trng_words_available--;
			put_user(trng_val, (u32 *)arg);
			if(trng_words_available == 0) {
				for(i=0; i<60; i++){
					udelay(1000); // Give TRNG FIFO time to fill
				}
			}
			break;

		// Add further CryptoCore commands here
		case IOCTL_MWMAC_MRT_Prime:
			rc = copy_from_user(MRT_prime_params_ptr, (void *)arg, sizeof(MRT_prime_params_t));
			MWMAC_MRT_Prime(MRT_prime_params_ptr);
			rc = copy_to_user((void *)arg, MRT_prime_params_ptr, sizeof(MRT_prime_params_t));
			break;
		case IOCTL_MWMAC_MRT_SafePrime:
                        rc = copy_from_user(MRT_prime_params_ptr, (void *)arg, sizeof(MRT_prime_params_t));
                        MWMAC_MRT_SafePrime(MRT_prime_params_ptr);
                        rc = copy_to_user((void *)arg, MRT_prime_params_ptr, sizeof(MRT_prime_params_t));
                        break;
		case IOCTL_MWMAC_MRT_Add12:
                        rc = copy_from_user(MRT_prime_params_ptr, (void *)arg, sizeof(MRT_prime_params_t));
                        MWMAC_MRT_Add12(MRT_prime_params_ptr);
                        rc = copy_to_user((void *)arg, MRT_prime_params_ptr, sizeof(MRT_prime_params_t));
                        break;
		
		default:
			printk("unknown IOCTL 0x%x\n", cmd);

			// Free allocated kernel memory for defined CryptoCore Structs here...
			kfree(MRT_prime_params_ptr);

			return -EINVAL;
	}

	// Free allocated kernel memory for defined CryptoCore Structs here...
	kfree(MRT_prime_params_ptr);

	return 0;

}

static struct file_operations fops = {
   .owner = THIS_MODULE,
   .open = cryptocore_driver_open,
   .release = cryptocore_driver_close,
   .unlocked_ioctl = cryptocore_driver_ioctl,
};

static int __init cryptocore_init( void )
{
   int value;

   if( alloc_chrdev_region(&cryptocore_dev_number, 0, 1, DRIVER_NAME) < 0 )
      return -EIO;

   driver_object = cdev_alloc(); /* Anmeldeobject reservieren */

   if( driver_object == NULL )
      goto free_device_number;

   driver_object->owner = THIS_MODULE;
   driver_object->ops = &fops;

   if( cdev_add(driver_object, cryptocore_dev_number, 1) )
      goto free_cdev;

   cryptocore_class = class_create( THIS_MODULE, DRIVER_NAME );

   if( IS_ERR( cryptocore_class ) ) {
      pr_err( "cryptocore: no udev support\n");
      goto free_cdev;
   }

   cryptocore_dev = device_create(cryptocore_class, NULL, cryptocore_dev_number, NULL, "%s", DRIVER_NAME );
   dev_info( cryptocore_dev, "cryptocore_init called\n" );

   if(!(request_mem_region(GPIO1_BASE, GPIO1_SPAN, DRIVER_NAME))) {
      pr_err( "timer: request mem_region (GPIO1) failed!\n");
      goto fail_request_mem_region_1;
   }

   if(!(request_mem_region(LW_H2F_BRIDGE_BASE + LEDR_BASE, LEDR_SPAN, DRIVER_NAME))) {
      pr_err( "cryptocore: request mem_region (LEDR) failed!\n");
      goto fail_request_mem_region_2;
   }

   if(!(request_mem_region(LW_H2F_BRIDGE_BASE + KEY_BASE, KEY_SPAN, DRIVER_NAME))) {
      pr_err( "cryptocore: request mem_region (KEY) failed!\n");
      goto fail_request_mem_region_3;
   }

   if(!(request_mem_region(H2F_BRIDGE_BASE + MWMAC_RAM_BASE, MWMAC_RAM_SPAN, DRIVER_NAME))) {
      pr_err( "cryptocore: request mem_region (MWMAC_RAM) failed!\n");
      goto fail_request_mem_region_4;
   }

   if(!(request_mem_region(LW_H2F_BRIDGE_BASE + MWMAC_CMD_BASE, MWMAC_CMD_SPAN, DRIVER_NAME))) {
      pr_err( "cryptocore: request mem_region (MWMAC_CMD) failed!\n");
      goto fail_request_mem_region_5;
   }

   if(!(request_mem_region(LW_H2F_BRIDGE_BASE + MWMAC_IRQ_BASE, MWMAC_IRQ_SPAN, DRIVER_NAME))) {
      pr_err( "cryptocore: request mem_region (MWMAC_IRQ) failed!\n");
      goto fail_request_mem_region_6;
   }

   if(!(request_mem_region(LW_H2F_BRIDGE_BASE + TRNG_CMD_BASE, TRNG_CMD_SPAN, DRIVER_NAME))) {
      pr_err( "cryptocore: request mem_region (TRNG_CMD) failed!\n");
      goto fail_request_mem_region_7;
   }

   if(!(request_mem_region(LW_H2F_BRIDGE_BASE + TRNG_CTR_BASE, TRNG_CTR_SPAN, DRIVER_NAME))) {
      pr_err( "cryptocore: request mem_region (TRNG_CTR) failed!\n");
      goto fail_request_mem_region_8;
   }

   if(!(request_mem_region(LW_H2F_BRIDGE_BASE + TRNG_TSTAB_BASE, TRNG_TSTAB_SPAN, DRIVER_NAME))) {
      pr_err( "cryptocore: request mem_region (TRNG_TSTAB) failed!\n");
      goto fail_request_mem_region_9;
   }

   if(!(request_mem_region(LW_H2F_BRIDGE_BASE + TRNG_TSAMPLE_BASE, TRNG_TSAMPLE_SPAN, DRIVER_NAME))) {
      pr_err( "cryptocore: request mem_region (TRNG_TSAMPLE) failed!\n");
      goto fail_request_mem_region_10;
   }

   if(!(request_mem_region(LW_H2F_BRIDGE_BASE + TRNG_IRQ_BASE, TRNG_IRQ_SPAN, DRIVER_NAME))) {
      pr_err( "cryptocore: request mem_region (TRNG_IRQ) failed!\n");
      goto fail_request_mem_region_11;
   }

   if(!(request_mem_region(LW_H2F_BRIDGE_BASE + TRNG_FIFO_BASE, TRNG_FIFO_SPAN, DRIVER_NAME))) {
      pr_err( "cryptocore: request mem_region (TRNG_FIFO) failed!\n");
      goto fail_request_mem_region_12;
   }

   if(!(HPS_GPIO1_ptr = ioremap(GPIO1_BASE, GPIO1_SPAN))) {
      pr_err( "cryptocore: ioremap (GPIO1) failed!\n");
      goto fail_ioremap_1;
   }

   if(!(LEDR_ptr = ioremap(LW_H2F_BRIDGE_BASE + LEDR_BASE, LEDR_SPAN))) {
      pr_err( "cryptocore: ioremap (LEDR) failed!\n");
      goto fail_ioremap_2;
   }

   if(!(KEY_ptr = ioremap(LW_H2F_BRIDGE_BASE + KEY_BASE, KEY_SPAN))) {
      pr_err( "cryptocore: ioremap (KEY) failed!\n");
      goto fail_ioremap_3;
   }

   if(!(MWMAC_RAM_ptr = ioremap(H2F_BRIDGE_BASE + MWMAC_RAM_BASE, MWMAC_RAM_SPAN))) {
      pr_err( "cryptocore: ioremap (MWMAC_RAM) failed!\n");
      goto fail_ioremap_4;
   }

   if(!(MWMAC_CMD_ptr = ioremap(LW_H2F_BRIDGE_BASE + MWMAC_CMD_BASE, MWMAC_CMD_SPAN))) {
      pr_err( "cryptocore: ioremap (MWMAC_CMD) failed!\n");
      goto fail_ioremap_5;
   }

   if(!(MWMAC_IRQ_ptr = ioremap(LW_H2F_BRIDGE_BASE + MWMAC_IRQ_BASE, MWMAC_IRQ_SPAN))) {
      pr_err( "cryptocore: ioremap (MWMAC_IRQ) failed!\n");
      goto fail_ioremap_6;
   }

   if(!(TRNG_CMD_ptr = ioremap(LW_H2F_BRIDGE_BASE + TRNG_CMD_BASE, TRNG_CMD_SPAN))) {
      pr_err( "cryptocore: ioremap (TRNG_CMD) failed!\n");
      goto fail_ioremap_7;
   }

   if(!(TRNG_CTR_ptr = ioremap(LW_H2F_BRIDGE_BASE + TRNG_CTR_BASE, TRNG_CTR_SPAN))) {
      pr_err( "cryptocore: ioremap (TRNG_CTR) failed!\n");
      goto fail_ioremap_8;
   }

   if(!(TRNG_TSTAB_ptr = ioremap(LW_H2F_BRIDGE_BASE + TRNG_TSTAB_BASE, TRNG_TSTAB_SPAN))) {
      pr_err( "cryptocore: ioremap (TRNG_TSTAB) failed!\n");
      goto fail_ioremap_9;
   }

   if(!(TRNG_TSAMPLE_ptr = ioremap(LW_H2F_BRIDGE_BASE + TRNG_TSAMPLE_BASE, TRNG_TSAMPLE_SPAN))) {
      pr_err( "cryptocore: ioremap (TRNG_TSAMPLE) failed!\n");
      goto fail_ioremap_10;
   }

   if(!(TRNG_IRQ_ptr = ioremap(LW_H2F_BRIDGE_BASE + TRNG_IRQ_BASE, TRNG_IRQ_SPAN))) {
      pr_err( "cryptocore: ioremap (TRNG_IRQ) failed!\n");
      goto fail_ioremap_11;
   }

   if(!(TRNG_FIFO_ptr = ioremap(LW_H2F_BRIDGE_BASE + TRNG_FIFO_BASE, TRNG_FIFO_SPAN))) {
      pr_err( "cryptocore: ioremap (TRNG_FIFO) failed!\n");
      goto fail_ioremap_12;
   }

   // Clear the PIO edgecapture register (clear any pending interrupt)
   iowrite32(0x00000001, MWMAC_IRQ_ptr+3); 
   // Enable IRQ generation for the CryptoCore
   iowrite32(0x00000001, MWMAC_IRQ_ptr+2); 

   // Clear the PIO edgecapture register (clear any pending interrupt)
   iowrite32(0x00000001, TRNG_IRQ_ptr+3);
   // Enable IRQ generation for the CryptoCore TRNG
   iowrite32(0x00000001, TRNG_IRQ_ptr+2);

   
   // Clear the PIO edgecapture register (clear any pending interrupt)
   iowrite32(0x0000000F, KEY_ptr+3);
   // Enable IRQ generation for the 4 buttons
   iowrite32(0x0000000F, KEY_ptr+2);

   value = request_irq (KEY_IRQ, (irq_handler_t) key_irq_handler, IRQF_SHARED, "cryptocore_key_irq_handler", (void *) (key_irq_handler));
   if(value) {
      pr_err( "cryptocore: request_irq (KEY) failed!\n");
      goto fail_irq_1;
   }

   value = request_irq (MWMAC_IRQ, (irq_handler_t) mwmac_irq_handler, IRQF_SHARED, "cryptocore_mwmac_irq_handler", (void *) (mwmac_irq_handler));
   if(value) {
      pr_err( "cryptocore: request_irq (MWMAC) failed!\n");
      goto fail_irq_2;
   }

   value = request_irq (TRNG_IRQ, (irq_handler_t) trng_irq_handler, IRQF_SHARED, "cryptocore_trng_irq_handler", (void *) (trng_irq_handler));
   if(value) {
      pr_err( "cryptocore: request_irq (TRNG) failed!\n");
      goto fail_irq_3;
   } 

   // Turn on green User LED
   iowrite32(0x01000000, (HPS_GPIO1_ptr+1));
   iowrite32(0x01000000, (HPS_GPIO1_ptr));

   return 0;

fail_irq_3:
   free_irq (MWMAC_IRQ, (void*) mwmac_irq_handler);

fail_irq_2:
   free_irq (KEY_IRQ, (void*) key_irq_handler);

fail_irq_1:
   iounmap(TRNG_FIFO_ptr);

fail_ioremap_12:
   iounmap(TRNG_IRQ_ptr);

fail_ioremap_11:
   iounmap(TRNG_TSAMPLE_ptr);

fail_ioremap_10:
   iounmap(TRNG_TSTAB_ptr);

fail_ioremap_9:
   iounmap(TRNG_CTR_ptr);

fail_ioremap_8:
   iounmap(TRNG_CMD_ptr);

fail_ioremap_7:
   iounmap(MWMAC_IRQ_ptr);

fail_ioremap_6:
   iounmap(MWMAC_CMD_ptr);

fail_ioremap_5:
   iounmap(MWMAC_RAM_ptr);

fail_ioremap_4:
   iounmap(KEY_ptr);

fail_ioremap_3:
   iounmap(LEDR_ptr);

fail_ioremap_2:
   iounmap(HPS_GPIO1_ptr);

fail_ioremap_1:
   release_mem_region(LW_H2F_BRIDGE_BASE + TRNG_FIFO_BASE, TRNG_FIFO_SPAN);

fail_request_mem_region_12:
   release_mem_region(LW_H2F_BRIDGE_BASE + TRNG_IRQ_BASE, TRNG_IRQ_SPAN);

fail_request_mem_region_11:
   release_mem_region(LW_H2F_BRIDGE_BASE + TRNG_TSAMPLE_BASE, TRNG_TSAMPLE_SPAN);

fail_request_mem_region_10:
   release_mem_region(LW_H2F_BRIDGE_BASE + TRNG_TSTAB_BASE, TRNG_TSTAB_SPAN);

fail_request_mem_region_9:
   release_mem_region(LW_H2F_BRIDGE_BASE + TRNG_CTR_BASE, TRNG_CTR_SPAN);

fail_request_mem_region_8:
   release_mem_region(LW_H2F_BRIDGE_BASE + TRNG_CMD_BASE, TRNG_CMD_SPAN);

fail_request_mem_region_7:
   release_mem_region(LW_H2F_BRIDGE_BASE + MWMAC_IRQ_BASE, MWMAC_IRQ_SPAN);

fail_request_mem_region_6:
   release_mem_region(LW_H2F_BRIDGE_BASE + MWMAC_CMD_BASE, MWMAC_CMD_SPAN);

fail_request_mem_region_5:
   release_mem_region(H2F_BRIDGE_BASE + MWMAC_RAM_BASE, MWMAC_RAM_SPAN);

fail_request_mem_region_4:
   release_mem_region(LW_H2F_BRIDGE_BASE + KEY_BASE, KEY_SPAN);

fail_request_mem_region_3:
   release_mem_region(LW_H2F_BRIDGE_BASE + LEDR_BASE, LEDR_SPAN);

fail_request_mem_region_2:
   release_mem_region(GPIO1_BASE, GPIO1_SPAN);

fail_request_mem_region_1:
   device_destroy( cryptocore_class, cryptocore_dev_number );
   class_destroy( cryptocore_class );

free_cdev:
   kobject_put( &driver_object->kobj );

free_device_number:
   unregister_chrdev_region( cryptocore_dev_number, 1 );
   return -EIO;
}

static void __exit cryptocore_exit( void )
{
   dev_info( cryptocore_dev, "cryptocore_exit called\n" );

   iowrite32(0x00000000, (LEDR_ptr));
   iowrite32(0x00000000, (HPS_GPIO1_ptr));
   iowrite32(0x00000000, (HPS_GPIO1_ptr+1));

   free_irq (KEY_IRQ, (void*) key_irq_handler);
   free_irq (MWMAC_IRQ, (void*) mwmac_irq_handler);
   free_irq (TRNG_IRQ, (void*) trng_irq_handler);

   iounmap(TRNG_FIFO_ptr);
   iounmap(TRNG_IRQ_ptr);
   iounmap(TRNG_TSAMPLE_ptr);
   iounmap(TRNG_TSTAB_ptr);
   iounmap(TRNG_CTR_ptr);
   iounmap(TRNG_CMD_ptr);
   iounmap(MWMAC_IRQ_ptr);
   iounmap(MWMAC_CMD_ptr);
   iounmap(MWMAC_RAM_ptr);
   iounmap(KEY_ptr);
   iounmap(LEDR_ptr);
   iounmap(HPS_GPIO1_ptr);

   release_mem_region(LW_H2F_BRIDGE_BASE + TRNG_FIFO_BASE, TRNG_FIFO_SPAN);
   release_mem_region(LW_H2F_BRIDGE_BASE + TRNG_IRQ_BASE, TRNG_IRQ_SPAN);
   release_mem_region(LW_H2F_BRIDGE_BASE + TRNG_TSAMPLE_BASE, TRNG_TSAMPLE_SPAN);
   release_mem_region(LW_H2F_BRIDGE_BASE + TRNG_TSTAB_BASE, TRNG_TSTAB_SPAN);
   release_mem_region(LW_H2F_BRIDGE_BASE + TRNG_CTR_BASE, TRNG_CTR_SPAN);
   release_mem_region(LW_H2F_BRIDGE_BASE + TRNG_CMD_BASE, TRNG_CMD_SPAN);
   release_mem_region(LW_H2F_BRIDGE_BASE + MWMAC_IRQ_BASE, MWMAC_IRQ_SPAN);
   release_mem_region(LW_H2F_BRIDGE_BASE + MWMAC_CMD_BASE, MWMAC_CMD_SPAN);
   release_mem_region(H2F_BRIDGE_BASE + MWMAC_RAM_BASE, MWMAC_RAM_SPAN);
   release_mem_region(LW_H2F_BRIDGE_BASE + KEY_BASE, KEY_SPAN);
   release_mem_region(LW_H2F_BRIDGE_BASE + LEDR_BASE, LEDR_SPAN);
   release_mem_region(GPIO1_BASE, GPIO1_SPAN);

   device_destroy( cryptocore_class, cryptocore_dev_number );
   class_destroy( cryptocore_class );

   cdev_del( driver_object );
   unregister_chrdev_region( cryptocore_dev_number, 1 );

   return;
}

static void Clear_MWMAC_RAM(void)
{
	u32 value;
	u32 i;
	
	value = 0x00000000;
	
	for(i=0; i<128; i++){
		// Clear B
		iowrite32(value, (MWMAC_RAM_ptr+0x3+i*0x4));
		// Clear P
		iowrite32(value, (MWMAC_RAM_ptr+0x2+i*0x4));
		// Clear TS
		iowrite32(value, (MWMAC_RAM_ptr+0x1+i*0x4));
		// Clear TC
		iowrite32(value, (MWMAC_RAM_ptr+0x0+i*0x4));
		// Clear A
		iowrite32(value, (MWMAC_RAM_ptr+0x200+i));
		// Clear E
		iowrite32(value, (MWMAC_RAM_ptr+0x280+i));
		// Clear X
		iowrite32(value, (MWMAC_RAM_ptr+0x300+i));
	}
	
}

static void MWMAC_MRT_Add12(MRT_prime_params_t *MRT_prime_params_ptr)
{

	u32 mwmac_cmd = 0;
	u32 mwmac_sec_calc = MRT_prime_params_ptr->sec_calc; // Secure calculation bit
	u32 prec = MRT_prime_params_ptr->prec;				 // In prime operations precision is standard
	u32 mwmac_f_sel = 1;
	u32 add12[128];
	u32 i;
	u32 mwmac_precision;
	u32 word_count = prec / 32;
	u32 last_word = word_count - 1;

	for (i = 0; i < 13; ++i)
	{
		if (PRIME_PRECISIONS[i][0] == prec)
		{
			mwmac_precision = PRIME_PRECISIONS[i][1];
		}
	}

	for (i = 0; i < word_count; ++i)
	{
		add12[i] = add12[i] & 0x00000000;
	}
	add12[last_word] = add12[last_word] | 0x0000000c;

	//copying pointer n value to the TS register for adding
	for (i = 0; i < word_count; ++i)
	{
		iowrite32(MRT_prime_params_ptr->n[word_count - 1 - i], (MWMAC_RAM_ptr + 0x1 + i * 0x4));
	}

	u32 hv = 0xffffffff;

	for (i = 0; i < word_count; ++i)
	{
		iowrite32(hv, (MWMAC_RAM_ptr + 0x2 + i * 0x4));
	}

	for (i = 0; i < word_count; ++i)
	{
		iowrite32(add12[word_count - 1 - i], (MWMAC_RAM_ptr + 0x0 + i * 0x4));
	}
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_precision << 4) | (MODADD << 8)
				//                      src_addr                        dest_addr    src_addr_e   src_addr_x
				| (MWMAC_RAM_TS1 << 12) | (0x0 << 17) | (0x0 << 22) | (0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);

	//wait until CryptoCore is busy
	while (!mwmac_irq_var)
		;
	mwmac_irq_var = 0;

	//Copy B to u32 NSubR[128]
	for (i = 0; i < word_count; i++)
	{
		MRT_prime_params_ptr->n[word_count - 1 - i] = ioread32(MWMAC_RAM_ptr + 0x3 + i * 0x4);
	}
}

static void MWMAC_MRT_SafePrime(MRT_prime_params_t *MRT_prime_params_ptr)
{
	u32 mwmac_cmd = 0;
	u32 mwmac_sec_calc = MRT_prime_params_ptr->sec_calc; // Secure calculation bit
	u32 prec = MRT_prime_params_ptr->prec;				 // In prime operations precision is standard
	u32 mwmac_f_sel = 1;
	u32 mwmac_precision;
	u32 n[128];
	u32 store[128];
	u32 modul[128];
	u32 add8[128];
	u32 add4[128];
	u32 i;
	for (i = 0; i < 13; ++i)
	{
		if (PRIME_PRECISIONS[i][0] == prec)
		{
			mwmac_precision = PRIME_PRECISIONS[i][1];
		}
	}
	u32 word_count = prec / 32;
	u32 last_word = word_count - 1;
	for (i = 0; i < word_count; i++)
	{
		n[i] = MRT_prime_params_ptr->n[i];
	}
	for (i = 0; i < word_count; ++i)
	{
		store[i] = store[i] & 0x00000000;
	}
	store[last_word] = store[last_word] | 0x00000001;

	for (i = 0; i < word_count; ++i)
	{
		modul[i] = modul[i] & 0x00000000;
	}
	modul[last_word] = modul[last_word] | 0x00000003;

	for (i = 0; i < word_count; ++i)
	{
		add8[i] = add8[i] & 0x00000000;
	}
	add8[last_word] = add8[last_word] | 0x00000008;

	for (i = 0; i < word_count; ++i)
	{
		add4[i] = add4[i] & 0x00000000;
	}
	add4[last_word] = add4[last_word] | 0x00000004;

	// Write value of 'temporary' from temporary[128] to A register
	for (i = 0; i < word_count; i++)
	{
		iowrite32(n[word_count - 1 - i], (MWMAC_RAM_ptr + 0x200 + i));
	}

	// Write value of 'temporary' from temporary[128] to B register
	for (i = 0; i < word_count; i++)
	{
		iowrite32(store[word_count - 1 - i], (MWMAC_RAM_ptr + 0x3 + i * 0x4));
	}
	// Write the modulus value which is 3 to the P register
	for (i = 0; i < word_count; i++)
	{
		iowrite32(modul[word_count - 1 - i], (MWMAC_RAM_ptr + 0x2 + i * 0x4));
	}

	// MontMult(A, B, P)
	//            Start     Abort       f_sel     sec_calc        precision         operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_precision << 4) | (MONTMULT << 8)
				//                      src_addr                        dest_addr               src_addr_e   src_addr_x
				| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_A1 << 17) | (0x0 << 22) | (0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	// Wait until CryptoCore is busy
	while (!mwmac_irq_var)
		;
	mwmac_irq_var = 0;

	// Copying value result 'temporary' from B register to temporary[128];
	for (i = 0; i < word_count; i++)
	{
		n[word_count - 1 - i] = ioread32(MWMAC_RAM_ptr + 0x3 + i * 0x4);
	}

	for (i = 0; i < word_count; ++i)
	{
		MRT_prime_params_ptr->rem[i] = n[i];
	}
	//copying pointer n value to the TS register for adding
	for (i = 0; i < word_count; ++i)
	{
		iowrite32(MRT_prime_params_ptr->n[word_count - 1 - i], (MWMAC_RAM_ptr + 0x1 + i * 0x4));
	}

	u32 hv = 0xffffffff;

	for (i = 0; i < word_count; ++i)
	{
		iowrite32(hv, (MWMAC_RAM_ptr + 0x2 + i * 0x4));
	}

	if (n[last_word] == 0x00000000)
	{

		for (i = 0; i < word_count; ++i)
		{
			iowrite32(add8[word_count - 1 - i], (MWMAC_RAM_ptr + 0x0 + i * 0x4));
		}
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_precision << 4) | (MODADD << 8)
					//                      src_addr                        dest_addr    src_addr_e   src_addr_x
					| (MWMAC_RAM_TS1 << 12) | (0x0 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);

		//wait until CryptoCore is busy
		while (!mwmac_irq_var)
			;
		mwmac_irq_var = 0;

		//Copy B to u32 NSubR[128]
		for (i = 0; i < word_count; i++)
		{
			MRT_prime_params_ptr->n[word_count - 1 - i] = ioread32(MWMAC_RAM_ptr + 0x3 + i * 0x4);
		}
	}
	else if (n[last_word] == 0x00000001)
	{
		for (i = 0; i < word_count; ++i)
		{
			iowrite32(add4[word_count - 1 - i], (MWMAC_RAM_ptr + 0x0 + i * 0x4));
		}
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_precision << 4) | (MODADD << 8)
					//                      src_addr                        dest_addr    src_addr_e   src_addr_x
					| (MWMAC_RAM_TS1 << 12) | (0x0 << 17) | (0x0 << 22) | (0x0 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);

		//wait until CryptoCore is busy
		while (!mwmac_irq_var)
			;
		mwmac_irq_var = 0;

		//Copy B to u32 NSubR[128]
		for (i = 0; i < word_count; i++)
		{
			MRT_prime_params_ptr->n[word_count - 1 - i] = ioread32(MWMAC_RAM_ptr + 0x3 + i * 0x4);
		}
	}
}

// Add further CryptoCore Functions here...
static void MWMAC_MRT_Prime(MRT_prime_params_t *MRT_prime_params_ptr)
{
	// Commands variables
	u32 mwmac_cmd = 0;
	u32 mwmac_sec_calc = MRT_prime_params_ptr->sec_calc; // Secure calculation bit
	u32 prec = MRT_prime_params_ptr->prec;				 // In prime operations precision is standard
	u32 mwmac_f_sel = 1;
	u32 mwmac_precision;
	// Temporary counter
	u32 i;
	u32 j;
	u32 k;
	// Temporary random value
	u32 a;
	// Array to hold the random number generated for testing (based on the given precision)
	u32 n[128];
	// Parameter / Temporary arrays
	u32 d[128];
	u32 r[128];
	u32 NSubR[128];
	u32 temporary[128];
	// Word count
	u32 word_count = prec / 32;
	// Index of last word (least significant word)
	u32 last_word = word_count - 1;
	// Flag to check if least sig block is equal to 2
	u32 last_word_is_2;
	// S parameter
	u32 s = 0;
	// Flag to check if other blocks are equal to 0
	u32 blocks;

	for (i = 0; i < 13; ++i)
	{
		if (PRIME_PRECISIONS[i][0] == prec)
		{
			mwmac_precision = PRIME_PRECISIONS[i][1];
		}
	}

	// Clear RAM
	Clear_MWMAC_RAM();

	// Copy input number into n and d
	for (i = 0; i < word_count; i++)
	{
		n[i] = MRT_prime_params_ptr->n[i];
		d[i] = MRT_prime_params_ptr->n[i];
	}

	// ---------------------------------------------------------------------------
	// Preliminary tests on the number [(n == 2) then prime or (n % 2 == 0) then composite]
	// ---------------------------------------------------------------------------

	// If number is equal to 2 then it is a prime (check least sig block to be 2)
	last_word_is_2 = 0;
	if (n[last_word] == 2)
	{
		last_word_is_2 = 1;
	}
	else
	{
		last_word_is_2 = 0;
	}

	// Check other blocks to be 0
	blocks = 1;
	for (i = 0; i < last_word; i++)
	{
		if (n[i] != 0)
		{
			blocks = 0;
			break;
		}
	}

	// Make sure that least significant bit is 1 (final check)
	if (last_word_is_2 == 1 && blocks == 1)
	{
		MRT_prime_params_ptr->probably_prime = 1;
		return;
	}

	// If number is even then it is composite
	if (!(n[last_word] & 0x00000001))
	{
		MRT_prime_params_ptr->probably_prime = 0;
		return;
	}

	// ---------------------------------------------------------------------------
	// Calculate value of d and s
	// ---------------------------------------------------------------------------

	// Do minus 1 operation:
	//  Since we know received random number is odd we can confidently only AND it
	//  with 0xFFFFFFFE to make the right most digit 0 which means number-1
	d[last_word] &= 0xFFFFFFFE;

	// Shift operation:
	// While(least significant block %2==0)
	while (!(d[last_word] & 0x00000001))
	{
		s++;				// Counting the number of shifting
		d[last_word] >>= 1; // Divide by 2 (shift least significant block)
		// Shift other blocks starting from one block after the least significant block
		// [B0 B1 B2 B3 <- B4]   [most sig, ..., least sig]
		for (i = 1; i < word_count; i++)
		{
			// First check if the right most bit of current block is 1.
			// If yes, before shifting, we insert a 1 into the previous (less significant) block from its left side
			if (d[last_word - i] & 0x00000001)		  // If(d%2==1)   (if right most bit of current part is 1)
				d[last_word - (i - 1)] |= 0x80000000; // Sets most left bit of previous block to 1
			d[last_word - i] >>= 1;					  // Divides current block by 2
		}
	}

	MRT_prime_params_ptr->s = s;
	for (i = 0; i < word_count; ++i)
	{
		MRT_prime_params_ptr->d[i] = d[i];
	}

	// ---------------------------------------------------------------------------
	// n - R and R [MWMAC_ModR]
	// ---------------------------------------------------------------------------

	// Write n to P
	for (i = 0; i < word_count; i++)
	{
		iowrite32(n[word_count - 1 - i], (MWMAC_RAM_ptr + 0x2 + i * 0x4));
	}

	// MontR(P, B)
	//            Start     Abort       f_sel     sec_calc        precision         operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_precision << 4) | (MONTR << 8)
				//			src_addr      			dest_addr    		src_addr_e   	src_addr_x
				| (MWMAC_RAM_P1 << 12) | (MWMAC_RAM_B1 << 17) | (0x0 << 22) | (0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);

	// Wait until CryptoCore is busy
	while (!mwmac_irq_var)
		;
	mwmac_irq_var = 0;

	// Copying value result 'R' from B register to r[128];
	for (i = 0; i < word_count; ++i)
	{
		r[last_word - i] = ioread32(MWMAC_RAM_ptr + 0x3 + i * 0x4);
	}

	// Write value of 'N' from n[128] to TS register
	for (i = 0; i < word_count; i++)
	{
		iowrite32(n[word_count - 1 - i], (MWMAC_RAM_ptr + 0x1 + i * 0x4));
	}

	// Write value of 'R' from r[128] to TC register
	for (i = 0; i < word_count; i++)
	{
		iowrite32(r[word_count - 1 - i], (MWMAC_RAM_ptr + 0x0 + i * 0x4));
	}

	// Write value of 'N' from n[128] to P register
	for (i = 0; i < word_count; i++)
	{
		iowrite32(n[word_count - 1 - i], (MWMAC_RAM_ptr + 0x2 + i * 0x4));
	}

	// ModSub(TS, TC)
	//            Start     Abort       f_sel     sec_calc        precision              operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_precision << 4) | (MODSUB << 8)
				//			src_addr      			dest_addr    src_addr_e   src_addr_x
				| (MWMAC_RAM_TS1 << 12) | (0x0 << 17) | (0x0 << 22) | (0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);

	//wait until CryptoCore is busy
	while (!mwmac_irq_var)
		;
	mwmac_irq_var = 0;

	//Copy B to u32 NSubR[128]
	for (i = 0; i < word_count; i++)
	{
		NSubR[word_count - 1 - i] = ioread32(MWMAC_RAM_ptr + 0x3 + i * 0x4);
	}

	// ---------------------------------------------------------------------------
	// Main Loop
	// ---------------------------------------------------------------------------
	for (j = 0; j < MRT_prime_params_ptr->k; j++)
	{
		// ====================== a = random.randrange(2, n-2) =====================

		for (i = 0; i < word_count; i++)
		{
			//read a random 32 bit number from TRNG
			a = ioread32(TRNG_FIFO_ptr);

			// Make sure that the final random number will be in the range [2,n-2]. Bit manipulation: 0x0rrrrrrr ... 0xrrrrrr1r
			if (i == 0) // Make msb 0 (to make sure our random is <= n-2)
				a &= 0x7FFFFFFF;
			if (i == word_count - 1) //make second lsb 1 (to make sure our random is >= 2)
				a |= 0x00000002;

			// Write the random 32 bit number into the appropriate location in X memory
			iowrite32(a, (MWMAC_RAM_ptr + 0x300 + i));
		}
		// Write value of 'D' from d[128] to E register
		for (i = 0; i < word_count; i++)
		{
			iowrite32(d[word_count - 1 - i], (MWMAC_RAM_ptr + 0x280 + i));
		}
		// Write value of 'R' from r[128] to B register
		for (i = 0; i < word_count; i++)
		{
			iowrite32(r[word_count - 1 - i], (MWMAC_RAM_ptr + 0x3 + i * 0x4));
		}
		// Write value of 'R' from r[128] to A register
		for (i = 0; i < word_count; i++)
		{
			iowrite32(r[word_count - 1 - i], (MWMAC_RAM_ptr + 0x200 + i));
		}

		// ============= temporary = Prime_MontExp(a,d,n,prec,r,rinv) ==============

		// MontExp(A, B, E, X, P)
		//            Start     Abort       f_sel     sec_calc        precision         operation
		mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_precision << 4) | (MONTEXP << 8)
					//			src_addr      			dest_addr    		src_addr_e   			src_addr_x
					| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_A1 << 17) | (MWMAC_RAM_E1 << 22) | (MWMAC_RAM_X1 << 27);
		iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
		// Wait until CryptoCore is busy
		while (!mwmac_irq_var)
			;
		mwmac_irq_var = 0;

		// Copying value result 'temporary' from B register to temporary[128];
		for (i = 0; i < word_count; i++)
		{
			temporary[word_count - 1 - i] = ioread32(MWMAC_RAM_ptr + 0x3 + i * 0x4);
		}

		// ===== if temporary == r or temporary == n-r: continue else return false ====
		// temporary != r
		u32 temporaryNotEqualR = 0;
		for (i = 0; i < word_count; i++)
		{
			if (temporary[word_count - 1 - i] != r[word_count - 1 - i])
			{
				temporaryNotEqualR = 1;
				break;
			}
		}
		// temporary != n - r
		u32 temporaryNotEqualNMinR = 0;
		for (i = 0; i < word_count; i++)
		{
			if (temporary[word_count - 1 - i] != NSubR[word_count - 1 - i])
			{
				temporaryNotEqualNMinR = 1;
				break;
			}
		}
		// if temporary != r and temporary != n-r
		if (temporaryNotEqualR == 1 && temporaryNotEqualNMinR == 1)
		{
			for (k = 0; k < s; k++)
			{
				// ============== temporary = (temporary * temporary * rinv) % n ==============

				// Write value of 'temporary' from temporary[128] to A register
				for (i = 0; i < word_count; i++)
				{
					iowrite32(temporary[word_count - 1 - i], (MWMAC_RAM_ptr + 0x200 + i));
				}

				// Write value of 'temporary' from temporary[128] to B register
				for (i = 0; i < word_count; i++)
				{
					iowrite32(temporary[word_count - 1 - i], (MWMAC_RAM_ptr + 0x3 + i * 0x4));
				}

				// MontMult(A, B, P)
				//            Start     Abort       f_sel     sec_calc        precision         operation
				mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_precision << 4) | (MONTMULT << 8)
							//			src_addr     			dest_addr    		src_addr_e   src_addr_x
							| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_A1 << 17) | (0x0 << 22) | (0x0 << 27);
				iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
				// Wait until CryptoCore is busy
				while (!mwmac_irq_var)
					;
				mwmac_irq_var = 0;

				// Copying value result 'temporary' from B register to temporary[128];
				for (i = 0; i < word_count; i++)
				{
					temporary[word_count - 1 - i] = ioread32(MWMAC_RAM_ptr + 0x3 + i * 0x4);
				}

				// if temporary == r: return False
				u32 temporaryEqualR = 1;
				for (i = 0; i < word_count; i++)
				{
					if (temporary[word_count - 1 - i] != r[word_count - 1 - i])
					{
						temporaryEqualR = 0;
						break;
					}
				}
				if (temporaryEqualR == 1)
				{
					MRT_prime_params_ptr->probably_prime = 0;
					return;
				}
				// if temporary == n-r: break
				u32 temporaryEqualNMinR = 1;
				for (i = 0; i < word_count; i++)
				{
					if (temporary[word_count - 1 - i] != NSubR[word_count - 1 - i])
					{
						temporaryEqualNMinR = 0;
						break;
					}
				}
				if (temporaryEqualNMinR == 1)
				{
					break;
				}
			}
			if (k == s)
			{
				MRT_prime_params_ptr->probably_prime = 0;
				return;
			}
		}
	}
	MRT_prime_params_ptr->probably_prime = 1;
}

module_init( cryptocore_init );
module_exit( cryptocore_exit );

MODULE_AUTHOR("MAI - Selected Topics of Embedded Software Development II");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("The driver for the FPGA-based CryptoCore");
MODULE_SUPPORTED_DEVICE("CryptoCore");

