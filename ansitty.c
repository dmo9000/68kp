#include <GL/glut.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "rawfont.h"
#include "ansicanvas.h"
#include "m68k.h"

#define MAX_KBBUF_LEN      256
extern unsigned char kbbuf[MAX_KBBUF_LEN];
extern volatile uint8_t kbbuf_len;
extern BitmapFont *myfont;


void output_character(char c)
{
    int i = 0, j = 0;
    static int cx=0, cy=0;
    //printf("output character = %c\r\n", c);


    if (c == '\r') {
        cy++;
        if (cy > 23 ) {
            /* hardware scroll required */
            gfx_opengl_hwscroll();
            cy = 23;
        }

        return;
    }

    if (c == '\n') {
        cx = 0;
        return;
    }

    if (cy > 23 ) {
        /* hardware scroll required */
        gfx_opengl_hwscroll();
        cy = 23;
    }
    //assert (cy < 25);

    gfx_opengl_drawglyph(myfont, cx, cy, c, 7, 0, 0);
    cx++;
    if (cx == 80) {
        cx = 0;
        cy ++;
    }
}


/*
void process_Normal_Keys(int key, int x, int y)
{
    //printf("process_Normal_Keys()\r\n");

    switch (key)
    {
    default:
        //printf("GLUT_KEY(%d)\r\n", key);
        //output_character(key);
        assert(kbbuf_len < MAX_KBBUF_LEN) ;
        kbbuf[kbbuf_len] = key;
        kbbuf_len++;
        //printf("keyboard buffer len is now %u\r", kbbuf_len);
        break;
    }

}
*/


