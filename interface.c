#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

//#include <unistd.h> // unix specific header for sleep()

#include "interface.h"

int main()
{
    Display window;
    init_display(&window, "test", 640, 480);


    uint32_t grayscale_pix[] = {
        0xFF000000,
        0xFF888888,
        0xFFCCCCCC,
        0xFFFFFFFF
    };

    uint32_t seq[160];

    for (size_t i = 0; i < 480; i++)
    {
        for (size_t j = 0; j < 4; j++)
        {
            memset((void *)seq, grayscale_pix[j], 160 * sizeof(uint32_t));
            write_ARGB8888_arr_to_display(&window, (j * 160), i, seq, 160, 1);
        }
    }
    
    push_to_display(&window);

    update_display(&window);
    wait(3000);
    free_display(&window);
}
