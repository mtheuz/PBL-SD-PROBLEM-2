
#include <stdio.h>
#include <unistd.h>
#include "gpu_lib.h"

int main()
{   
    if (open_gpu_device() == 0)
        return 0;
    clear_poligonos();
    set_poligono(6, 400, 300, 9, 0, 0, 5, 0); // PAREDE DA CASA
    set_poligono(5, 400, 200, 10, 5, 0, 0, 1); // TELHADO DA CASA
    set_poligono(2, 380, 310, 1, 4, 2, 0, 0); // JANELA DA CASA
    set_poligono(1, 420, 335, 2, 4, 2, 0, 0); // PORTA 1 DA CASA
    set_poligono(0, 420, 305, 2, 4, 2, 0, 0);
    set_poligono(14, 500, 100, 4, 7, 7, 7, 0);

    //clear_screen();
    set_background_block(10, 10, 7, 5, 0);
    set_background_block(25, 13, 7, 5, 0);
    set_background_block(34, 11, 7, 5, 0);
    set_background_block(15, 15, 7, 5, 0);
    set_background_block(46, 12, 7, 5, 0);
    set_background_block(40, 10, 7, 5, 0);
    set_background_block(33, 16, 7, 5, 0);
    set_background_block(70, 13, 7, 5, 0);
    set_background_block(8, 15, 7, 5, 0);
    set_background_block(75, 9, 7, 5, 0);
    set_background_block(78, 15, 7, 5, 0);

    int x = 0;
    int x2 = 620;
    while (1){
       x += 10;
       x2 -= 10;
       set_sprite(1, x, 50, 6, 1);
       set_sprite(4, x2, 100, 8, 1); 
        
       if (x == 620){
	 x = 0;
         x2 = 620;
       }
       usleep(500 * 1000); 
    }



    close__gpu_devide();


    return 0;
}
