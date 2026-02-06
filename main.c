#include <errno.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define I2C_DEV "/dev/i2c-1"

#define PAC_REFRESH_CMD 0x00
#define REG_VBUS1       0x07
#define REG_VSENSE1     0x0B
#define REG_VPOWER1     0x17

#define REG_VBUS3       0x09
#define REG_VSENSE3     0x0D
#define REG_VPOWER3     0x19


uint8_t pac_address [2]  = { 0x10, 0x11};



void i2c_set_slave(int fd, uint8_t addr)
{
    ioctl(fd, I2C_SLAVE, addr);
}


void pac_send_byte(int fd, uint8_t cmd)
{
    write(fd, &cmd, 1);
}


void  pac_read_reg(int fd, uint8_t reg, uint8_t *buf, size_t len)
{
    write(fd, &reg, 1);  
    read(fd, buf, len);
}



int main(void)
{
    uint8_t raw[2];
    uint16_t vbus, vsense;
    uint8_t pow[4];
    uint32_t Vpower;
    
    uint8_t raw3[2];
    uint16_t vbus3, vsense3;
    uint8_t pow3[4];
    uint32_t Vpower3;
    
    int fd = open(I2C_DEV, O_RDWR);
    for (int i = 0; i < 2; i++){ 
    /* Refresh */
    i2c_set_slave(fd, pac_address[i]);
    pac_send_byte(fd, PAC_REFRESH_CMD) ;

    /* Give PAC time to latch values */
    usleep(2000);  // 2 ms 

    /* Read VBUS */
    pac_read_reg(fd, REG_VBUS1, raw, 2);
    vbus = (raw[0] << 8) | raw[1];

    /* Read VSENSE */
    pac_read_reg(fd, REG_VSENSE1, raw, 2);
    vsense = (raw[0] << 8) | raw[1];
    
    /* Read VPOWER */
    pac_read_reg(fd, REG_VPOWER1, pow, 4);
    Vpower = (pow[0] << 24) | (pow[1] << 16)  | (pow[2] << 8) | pow[3] ;
        
        
    printf ("Pac%d at i2c address 0X%02X" , i+1 , pac_address[i]);
    printf("  VBUS raw   = 0x%04X (%u)\n", vbus, vbus);
    printf("  VSENSE raw = 0x%04X (%u)\n", vsense, vsense);
    printf("  VPower raw = 0x%08X (%u)\n", Vpower, Vpower);
    }
    
    i2c_set_slave(fd, 0x12);
    pac_send_byte(fd, PAC_REFRESH_CMD) ;

    /* Give PAC time to latch values */
    usleep(2000);  // 2 ms 

    /* Read VBUS */
    pac_read_reg(fd, REG_VBUS3, raw3, 2);
    vbus3 = (raw3[0] << 8) | raw3[1];

    /* Read VSENSE */
    pac_read_reg(fd, REG_VSENSE3, raw3, 2);
    vsense3 = (raw3[0] << 8) | raw3[1];
    
    /* Read VPOWER */
    pac_read_reg(fd, REG_VPOWER3, pow3, 4);
    Vpower3 = (pow3[0] << 24) | (pow3[1] << 16)  | (pow3[2] << 8) | pow3[3] ;
        
        
    
    printf("  VBUS raw3   = 0x%04X (%u)\n", vbus3, vbus3);
    printf("  VSENSE raw3 = 0x%04X (%u)\n", vsense3, vsense3);
    printf("  VPower raw3= 0x%08X (%u)\n", Vpower3, Vpower3);


    close(fd);
    return 0;
}
