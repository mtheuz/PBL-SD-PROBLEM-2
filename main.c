
#include <stdio.h>
#include "gpu_lib.h"

int main()
{   
    if (open_gpu_device() == 0)
        return 0;
    set_background_color(7, 7, 7); // COR BRANCA NO BACKGROUND
    set_sprite(1, 200, 50,6 , 1);
    set_sprite(2, 200, 330, 4 , 1);
    set_sprite(3, 250, 330, 4 , 1); // CRIA SPRITE NA POSIÇÃO 100x100 DA TELA
    set_poligono(0, 400, 300, 9, 0, 0, 5, 0); // CRIA QUADRADO NA POSIÇÃO 50x50
    set_poligono(1, 400, 200, 10, 5, 0, 0, 1);
    set_poligono(2, 380, 295, 1, 0, 5, 0, 0);
    close__gpu_devide();


    return 0;
}
