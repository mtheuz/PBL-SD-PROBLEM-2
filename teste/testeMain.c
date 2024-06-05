#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#define DEVICE_PATH "/dev/gpu_device"

int main() {
    int fd;
    unsigned char command[9] = {0};
    int reg = 0b00001; // Register number (5 bits)
    int r = 0b111;     // Red color value (3 bits)
    int g = 0b000;     // Green color value (3 bits)
    int b = 0b000;     // Blue color value (3 bits)

    // Construct the command
    command[0] = 0; // Reserved for future use
    command[1] = reg;
    command[2] = r;
    command[3] = g;
    command[4] = b;

    // Open the device file
    fd = open(DEVICE_PATH, O_WRONLY);
    if (fd < 0) {
        perror("Failed to open the device file");
        return 1;
    }

    // Write the command to the device
    if (write(fd, command, sizeof(command)) < 0) {
        perror("Failed to write to the device");
        close(fd);
        return 1;
    }

    printf("Command sent to device: register=%d, r=%d, g=%d, b=%d\n", reg, r, g, b);

    // Close the device file
    close(fd);

    return 0;
}