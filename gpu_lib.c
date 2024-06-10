#include "graphic_processor.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

#define DEVICE_PATH "/dev/gpu_drive.c"
#define WBR 0x00
#define WBM 0x01
#define WSM 0x02
#define DP  0x03

int set_background_color (int R, int G, int B, int fd) {
    int fd = open(DEVICE_PATH, O_WRONLY);

    if (fd < 0) {
        perror("Failed to open the device");
        return -1;
    }

    unsigned char command[9] = {0};
    int reg = 0b00000; // Register number (5 bits)
    //int r = 0b111;     // Red color value (3 bits)
    //int g = 0b000;     // Green color value (3 bits)
    //int b = 0b000;     // Blue color value (3 bits)

    // Construct the command
    command[0] = 0; // Reserved for future use
    command[1] = reg;
    command[2] = R;
    command[3] = G;
    command[4] = B;
    
    // Write the command to the device
    if (write(fd, command, sizeof(command)) < 0) {
        perror("Failed to write to the device");
        close(fd);
        return 0;
    }

    printf("Command sent to device: register=%d, r=%d, g=%d, b=%d\n", reg, R, G, B);

    close(fd);

    return 1;
int set_sprite(int registrador, int x, int y, int offset) {
    int fd = open(DEVICE_PATH, O_WRONLY);

    if (fd < 0) {
        perror("Failed to open the device");
        return -1;
    }

    unsigned char command[9] = {0};
    int reg = 0b00000; // Register number (5 bits)
    //int r = 0b111;     // Red color value (3 bits)
    //int g = 0b000;     // Green color value (3 bits)
    //int b = 0b000;     // Blue color value (3 bits)

    // Construct the command
    command[0] = 0; // Reserved for future use
    command[1] = reg;
    command[2] = offset;
    command[3] = x;
    command[4] = y;
    
    // Write the command to the device
    if (write(fd, command, sizeof(command)) < 0) {
        perror("Failed to write to the device");
        close(fd);
        return 0;
    }

    printf("Command sent to device: register=%d, r=%d, g=%d, b=%d\n", reg, offset, x, y);

    close(fd);
    return 1;
}

int set_background_block(int column, int line, int R, int G, int B) {
    int fd = open(DEVICE_PATH, O_WRONLY);

    if (fd < 0) {
        perror("Failed to open the device");
        return -1;
    }

    unsigned char command[9] = {0};
    int reg = 0b00000; // Register number (5 bits)
    //int r = 0b111;     // Red color value (3 bits)
    //int g = 0b000;     // Green color value (3 bits)
    //int b = 0b000;     // Blue color value (3 bits)

    // Construct the command
    command[0] = 0; // Reserved for future use
    command[1] = reg;
    command[2] = offset;
    command[3] = x;
    command[4] = y;
    
    // Write the command to the device
    if (write(fd, command, sizeof(command)) < 0) {
        perror("Failed to write to the device");
        close(fd);
        return 0;
    }

    printf("Command sent to device: register=%d, r=%d, g=%d, b=%d\n", reg, offset, x, y);

    close(fd);
    return 1;
}


