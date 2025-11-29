/*
Where the virtual hardware is tested
Riley Woodruff Nov 26, 2025
*/

//usual libraries
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// file manipulation libraries
#include <unistd.h> // functions
#include <fcntl.h>  // macros

#define CTRL_ENABLE (1 << 0)
#define CTRL_RESET (1 << 1)

#define FILE_LOCATION "/dev/virtual_device"

int main() {
    int fd;
    uint32_t value;
    
    fd = open("/dev/virtual_device", O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        return 1;
    }
    
    printf("=== Reading initial status ===\n");
    read(fd, &value, sizeof(value));
    printf("Status: 0x%08x\n\n", value);
    
    printf("=== Enabling device ===\n");
    value = CTRL_ENABLE;
    write(fd, &value, sizeof(value));
    
    printf("=== Reading status after enable ===\n");
    read(fd, &value, sizeof(value));
    printf("Status: 0x%08x\n\n", value);
    
    printf("=== Resetting device ===\n");
    value = CTRL_RESET;
    write(fd, &value, sizeof(value));
    
    printf("=== Reading status after reset ===\n");
    read(fd, &value, sizeof(value));
    printf("Status: 0x%08x\n\n", value);
    
    close(fd);
    return 0;
}