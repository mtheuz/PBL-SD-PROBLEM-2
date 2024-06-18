#include <stdio.h>
#include <fcntl.h>
//DO LINUX
#include <unistd.h>
#include "gpu_lib.h"

int fd = 0;

// ================= AVISO ====================
// SE NÂO FUNCIONAR ESSA FUNÇÃO, VAI TER QUE ABRIR E FECHAR O ARQUIVO DENTRO DE CADA FUNÇÃO
int open_gpu_device () {
    fd = open(DEVICE_PATH, O_WRONLY);

    if (fd < 0) {
        perror("Failed to open the device");
        return 0;
    }
    return 1;
}

int close__gpu_devide () {
    close(fd);
}

int set_background_color (int R, int B, int G) {
    unsigned char command[4];

    command[0] = 0;
    command[1] = R;
    command[2] = G;
    command[3] = B;

    if (write(fd, command, sizeof(command)) < 0) {
        perror("Failed to write to the device");
        return 0;
    }

    return 1;
}

int set_sprite( int reg, int x, int y, int offset, int sp){
    unsigned char command[7];

    command[0] = 1;
    command[1] = reg;
    command[2] = (offset >> 1) & 0xFF;
    command[3] = ((offset & 0x01) << 7) | ((x >> 3) & 0x7F);
    command[4] = ((x & 0x07) << 5) | ((y >> 5) & 0x1F);
    command[5] = (y & 0x1F) << 3;
    command[6] = sp;

    if (write(fd, command, sizeof(command)) < 0) {
        perror("Failed to write to the device");
        return 0;
    }

    return 1;
}

int set_poligono( int address, int ref_x, int ref_y, int size, int r, int g, int b, int shape){
    unsigned char command[7];

    command[0] = 4; 
    command[1] = address; 
    command[2] = ref_x >> 1; 
    command[3] = ((ref_x & 0b01) << 7) | (ref_y >> 2); 
    command[4] = ((ref_y & 0b11) << 6) | (size & 0b1111);
    command[5] = ((r & 0b111)<< 5) | (g & 0b111) << 2; 
    command[6] = ((b &0b111) << 5) | shape & 0b1;

    if (write(fd, command, sizeof(command)) < 0) {
        perror("Failed to write to the device");
        return 0;
    }

    return 1;
}

int set_background_block( int column, int line, int R, int G, int B){
    unsigned char command[5];
    // 0001 1111 1111 1111 000
    int i = 0;
    int address = column + (line*80);


    command[0] = 2;
    command[1] = (address >> 5);
    command[2] = ((address) << 3) | R;
    command[3] = G;
    command[4] = B;

    //printf("address[1]: %d\n", (R & 0b111));
    printf("green: %d\n", command[3]);
    

    if (write(fd, command, sizeof(command)) < 0) {
        perror("Failed to write to the device");
        return 0;
    }

    return 1;
}

int set_background_block_address(int address, int R, int G, int B) {
   
    unsigned char command[5];
    command[0] = 2; // Reserved for future use
    command[1] = address;
    command[2] = R;
    command[3] = G;
    command[4] = B;
    
    // Write the command to the device
    if (write(fd, command, sizeof(command)) < 0) {
        perror("Failed to write to the device");
        close(fd);
        return 0;
    }
    return 1;
}



int set_sprite_pixel_color( int address, int R, int G, int B){
    unsigned char command[6];

    command[0] = 3; // Command for instrucao_wsm
    command[1] = (address >> 6); // Higher 8 bits of 14-bit address
    command[2] = (address & 0b111111); // Lower 6 bits of address and r
    command[3] = R & 0b111;
    command[4] = G & 0b111; // g value
    command[5] = B & 0b111; // b value

    if (write(fd, command, sizeof(command)) < 0) {
        perror("Failed to write to the device");
        return 0;
    }

    return 1;
}

void increase_coordinate(Sprite *sp, int mirror){
    if ((*sp).direction == LEFT){
        (*sp).pos_X -= (*sp).step_X;
    } else if ((*sp).direction == RIGHT){
        (*sp).pos_X += (*sp).step_X;
    } else if ((*sp).direction == UP){
        (*sp).pos_Y -= (*sp).step_Y;
    } else if ((*sp).direction == DOWN){
        (*sp).pos_Y += (*sp).step_Y;
    } else if ((*sp).direction == UPPER_RIGHT){
        (*sp).pos_Y -= (*sp).step_Y;
        (*sp).pos_X += (*sp).step_X;
    } else if ((*sp).direction == UPPER_LEFT){
        (*sp).pos_Y -= (*sp).step_Y;
        (*sp).pos_X -= (*sp).step_X;
    } else if ((*sp).direction == BOTTOM_LEFT){
        (*sp).pos_Y += (*sp).step_Y;
        (*sp).pos_X -= (*sp).step_X;
    } else if ((*sp).direction == BOTTOM_LEFT){
        (*sp).pos_Y += (*sp).step_Y;
        (*sp).pos_X += (*sp).step_X;
    } 
    int i = set_sprite((*sp).data_register, (*sp).pos_X, (*sp).pos_Y, (*sp).offset, (*sp).enable);
}

int collision(Sprite *sp1, Sprite *sp2){
    // Get the opposite vertices
    int square1_right = (*sp1).pos_X + 20;
    int square1_bottom = (*sp1).pos_Y - 20;
    int square2_right = (*sp2).pos_X + 20;
    int square2_bottom = (*sp2).pos_Y - 20;

    // Check for non-overlapping conditions
    if ((*sp1).pos_X >= square2_right || (*sp2).pos_X >= square1_right) {
        return 0;
    }
    if ((*sp1).pos_Y <= square2_bottom || (*sp2).pos_Y <= square1_bottom) {
        return 0;
    }

    // If none of the non-overlapping conditions are true, they must be overlapping
    return 1;

}


void clear_screen() {
    int i = 0;
    int j = 0;
    for (i; i <60; i++){
        for (j; j < 80; j++){
            set_background_block(j, i, 6, 7, 7);
        }
        j = 0;
    }
}

void teste () {
    int i = 35;
    int j = 0;
    for (i; i <60; i++){
        for (j; j < 80; j++){
            set_background_block(j, i, 2, 5, 0);
        }
        j = 0;
    }
}

void clear_poligonos(){
    int i = 0;
    for (i; i < 15; i++){
        set_poligono(i, 0, 0, 0, 0, 0, 0, 0);
    }
}
