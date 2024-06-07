#include "graphic_processor.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

#define DEVICE_PATH "/dev/gpu_drive.c"
#define WBR 0x00
#define WBM 0x01
#define WSM 0x02
#define DP  0x03


int set_background_color(int R, int G, int B) {
    int fd = open(DEVICE_PATH, O_WRONLY);
    if (fd < 0) {
        perror("Failed to open the device");
        return -1;
    }
    unsigned char command[9] = { 'WBR', 0, R, G, B, 0, 0, 0 };
    if (write(fd, command, sizeof(command)) != sizeof(command)) {
        perror("Failed to write the command");
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}