
#include <stdio.h>
#include "gpu_lib.h"

int main()
{   
    if (open_gpu_device() == 0)
        return 0;
    set_background_color(7, 7, 7); // COR BRANCA NO BACKGROUND
    set_sprite(1, 200, 50,6 , 1); // CRIA SPRITE NA POSIÇÃO 100x100 DA TELA
    set_poligono(0, 50, 50, 0, 0, 7, 0, 0); // CRIA QUADRADO NA POSIÇÃO 50x50 
    int i;
    for (i = 0; i < 2800; i++) {
        set_background_block_address(i, 0, 0, 0);
    }

    close__gpu_devide();


    return 0;
}
