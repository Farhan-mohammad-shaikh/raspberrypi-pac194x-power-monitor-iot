#include <errno.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

#define I2C_DEV "/dev/i2c-1"
static volatile sig_atomic_t running = 1;


//for channel 1,  pac 0x10 and pac 0x11 is in channel 1
#define PAC_REFRESH_CMD 0x00
#define REG_VBUS1       0x07
#define REG_VSENSE1     0x0B
#define REG_VPOWER1     0x17

//for channel 3, pac 0x12 is in channel 3
#define REG_VBUS3       0x09
#define REG_VSENSE3     0x0D
#define REG_VPOWER3     0x19


uint8_t pac_address [2]  = { 0x10, 0x11};

//scaling constants according to datasheet formula 
#define FS_VBUS_VOLTS    9.0      //9 volt FSR for VBus
#define FS_VSENSE_VOLTS  0.1      //100 mv FSR for Vsense
#define ADC_DENOM        65536.0  //denominator 2¹⁶ from formula for unipolar measurements
#define RSENSE_OHMS       1.0      //from ckt


void on_sigint(int signal)
{
    (void) signal;
    running = 0;
	
}

int i2c_set_slave(int fd, uint8_t addr)
{
    if (ioctl(fd, I2C_SLAVE, addr) < 0) 
    {
        fprintf(stderr, "I2C_SLAVE 0x%02X failed: %s\n", addr, strerror(errno));
        return -1;
    }
    return 0;
}


static int pac_refresh_g(int fd)
{
    if (i2c_set_slave(fd, 0x00) != 0) return -1;

    uint8_t cmd = 0x1E;
    if (write(fd, &cmd, 1) != 1) 
    {
        fprintf(stderr, "REFRESH_G write failed: %s\n", strerror(errno));
        return -2;
    }
    return 0;
}


static int pac_read_reg(int fd, uint8_t reg, uint8_t *buf, size_t len)
{
    if (write(fd, &reg, 1) != 1) 
    {
        fprintf(stderr, "Write reg 0x%02X failed: %s\n", reg, strerror(errno));
        return -1;
    }
    int r = read(fd, buf, len);
    if (r != (int)len)
    {
        fprintf(stderr, "Read reg 0x%02X failed: got %d/%zu (%s)\n",
                reg, r, len, strerror(errno));
        return -2;
    }
    return 0;
}


//void print_channel_json(uint8_t i2c_addr,int channel, uint16_t vbus_raw, uint16_t vsense_raw, uint32_t vpower_raw)

void led_trigger(char *cmd)
{
	int fd = open("/sys/class/leds/ACT/trigger", O_WRONLY);
	write (fd, cmd, 4);
	close(fd);
	
}

void led_brightness (int on)
{
	int fd = open("/sys/class/leds/ACT/brightness", O_WRONLY);
	if (on)
	{
		write (fd, "1", 1);
	}
	
	else 
	{
		write (fd, "0", 1);
	}
	close (fd);
	
}

void *blink_tread (void *arg)
{
	led_trigger("none");
	while (running)
	{
		led_brightness(1);
		usleep(200000);
		led_brightness(0);
		usleep(200000);	
	}
	led_trigger("mmc0");
	return NULL;	
}


 void print_channel_json(uint8_t i2c_addr,
                               int channel,
                               uint16_t vbus_raw,
                               uint16_t vsense_raw,
                               uint32_t vpower_raw)

{
    double vbus_v   = FS_VBUS_VOLTS   * ((double)vbus_raw   / ADC_DENOM);
    double vsense_v = FS_VSENSE_VOLTS * ((double)vsense_raw / ADC_DENOM);
    double current_a = vsense_v / RSENSE_OHMS;
    double pwr_fsr = (0.9 / RSENSE_OHMS);
    uint32_t vpower_code = (vpower_raw >> 2);
    double power_w = pwr_fsr * ((double) vpower_code / 1073741824.0);

    // One JSON object per line (perfect for your Python subprocess reader)
    printf(
        "{\"i2c_addr\":\"0x%02X\","
        "\"channel\":%d,"
        "\"vbus_raw\":%u,"
        "\"vsense_raw\":%u,"
        "\"vpower_raw\":%u,"
        "\"vbus_V\":%.6f,"
        "\"current_A\":%.6f}"
        "\"vpower_V\":%.6f,"
        "\n",
        i2c_addr,
        channel,
        (unsigned)vbus_raw,
        (unsigned)vsense_raw,
        (unsigned)vpower_raw,
        vbus_v,
        current_a,
        power_w
    );

    // Important when piping stdout to Python: flush so each line appears immediately
    fflush(stdout);
}


int main(void)
{
	
	pthread_t led_thread;
	
	signal(SIGINT, on_sigint);
	
    uint8_t raw[2];
    uint8_t pow[4];

    
    uint8_t raw3[2];
    uint8_t pow3[4];
    
    
    int fd = open(I2C_DEV, O_RDWR);
    
    pthread_create(&led_thread, NULL, blink_tread, NULL);
    
    while (running)
    
    {
		
		// 1) Global snapshot (refresh) for ALL PAC194x on the bus
		if (pac_refresh_g(fd) != 0) 
		{
			usleep(200000);
			continue;
		}

		// 2) Give time for registers to become stable (you want correctness, not speed)
		usleep(5000); // 5 ms
    
		for (int i = 0; i < 2; i++)
		{ 
			i2c_set_slave(fd, pac_address[i]);

			/* Read VBUS */
			pac_read_reg(fd, REG_VBUS1, raw, 2);
			uint16_t vbus = (raw[0] << 8) | raw[1];

			/* Read VSENSE */
			pac_read_reg(fd, REG_VSENSE1, raw, 2);
			uint16_t vsense = (raw[0] << 8) | raw[1];
    
			/* Read VPOWER */
			pac_read_reg(fd, REG_VPOWER1, pow, 4);
			uint32_t vpower = (pow[0] << 24) | (pow[1] << 16)  | (pow[2] << 8) | pow[3] ;
        
        
			print_channel_json(pac_address[i], 1, vbus, vsense, vpower);
			
		}
    
		i2c_set_slave(fd, 0x12);
		
		/* Read VBUS */
		pac_read_reg(fd, REG_VBUS3, raw3, 2);
		uint16_t vbus3 = (raw3[0] << 8) | raw3[1];

		/* Read VSENSE */
		pac_read_reg(fd, REG_VSENSE3, raw3, 2);
		uint16_t vsense3 = (raw3[0] << 8) | raw3[1];
    
		/* Read VPOWER */
		pac_read_reg(fd, REG_VPOWER3, pow3, 4);
		uint32_t vpower3 = (pow3[0] << 24) | (pow3[1] << 16)  | (pow3[2] << 8) | pow3[3] ;
        
        
		print_channel_json(0x12, 3, vbus3, vsense3, vpower3);
		usleep(200000);

	}
    close(fd);
    
    pthread_join (led_thread, NULL);

    return 0;
}
