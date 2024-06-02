#include "graphic_processor.h"
#include <stdio.h>

int main() {
    if (set_background_color(10, 10, 255) != 0) {
        printf( "Failed to set background color\n");
        return 1;
    }

    printf("success\n");
    return 0;
}