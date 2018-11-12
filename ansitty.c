#include <assert.h>
#include <stdlib.h>
#include <pthread.h>
#include "ansistate.h"
#include "ansicanvas.h"
#include "gfx_opengl.h"
#include "bmf.h"

ANSICanvas *canvas = NULL;
BitmapFont *myfont = NULL;

#define width 80
#define height 24

uint16_t tty_x = 0;
uint16_t tty_y = 0;
extern uint16_t current_x;
extern uint16_t current_y;
uint16_t last_x;
uint16_t last_y;

extern bool allow_clear;

pthread_t graphics_thread;

void sysbus_rungraphics()
{

    printf("sysbus_rungraphics()\r\n");
    fflush(NULL);
//    gfx_opengl_main(640, 384, "68K");
    gfx_opengl_main(gfx_opengl_getwidth(), gfx_opengl_getheight(), 1, "68K");
    while (1) { }
}


int ansitty_init()
{
    ANSIRaster *r = NULL;
    char *font_filename = NULL;
    printf("ansitty_init()\r\n");

    font_filename = "bmf/8x8.bmf";
    myfont = bmf_load(font_filename);
    if (!myfont) {
        perror("bmf_load");
        exit(1);
    }

    allow_clear = true;

    canvas = new_canvas();

    /* very specific settings needed to make the canvas behave as a TTY */

    canvas->allow_hard_clear = true;
    canvas->repaint_entire_canvas = false;
    canvas->scroll_on_output = false;
    canvas->scroll_limit = height;
    canvas->cursor_enabled = true;
    canvas->default_raster_length = 80;

    for (int i = 0; i <= 24; i++) {
        r = canvas_add_raster(canvas);
        assert(r);
        r = canvas_get_raster(canvas, i);
        assert(r);
        raster_extend_length_to(r, 80);
        for (int j = 0; j < 80; j++) {
            r->chardata[i] = ' ';
            r->fgcolors[i] = 7;
            r->bgcolors[i] = 0;
        }
    }

    gfx_opengl_setdimensions((width*8), (height*16));

    pthread_create( &graphics_thread, NULL, sysbus_rungraphics, NULL);


    //gfx_opengl_main((width*8), (height*16), "68000");
    //gfx_opengl_expose();
    canvas->is_dirty = true;
    return 0;

}

int ansitty_scroll(ANSICanvas *canvas)
{
    ANSIRaster *r = NULL, *d = NULL, *n = NULL;
    r = canvas_add_raster(canvas);
    assert(r);
    raster_extend_length_to(r, 80);
    /* get old head */
    d = canvas_get_raster(canvas, 0);
    assert(d);
    /* get new head */
    n = canvas_get_raster(canvas, 1);
    assert(n);
    /* point start of list to new head */
    assert(canvas->first_raster);
    canvas->first_raster = n;
    /* FIXME: TODO: free old head, there is a memory leak here */
    /* TODO: move this to a ansi_raster_delete() function in libansicanvas */

    assert(d->bytes);
    assert(d->chardata);
    assert(d->fgcolors);
    assert(d->bgcolors);
    assert(d->attribs);

    assert(raster_delete(d));

    canvas->lines --;
    //tty_y --;
    tty_y = canvas->lines -1;
    /* force refresh of entire canvas */
    canvas_reindex(canvas);

//    SLOW METHOD
//    gfx_opengl_clear();
//    gfx_opengl_canvas_render(canvas, myfont);


//    FAST METHOD - not working
//
    gfx_opengl_hwscroll();

    canvas->is_dirty = true;
    return 0;
}

int ansitty_drawcursor(bool state)
{

    switch (state) {
    case true:
        break;
    case false:
        break;
    }

    return 1;

}

int ansitty_putc(unsigned char c)
{
    unsigned char outbuffer[2];
    last_x = current_x;
    last_y = current_y;

    if (!c) return 0;

    if (c == '\b') {
        if ((tty_y + (tty_x / 80) < canvas->lines)) {
            gfx_opengl_render_cursor(canvas, myfont, (tty_x % 80),  tty_y + (tty_x / 80), false);
        }
        if (current_x > 0) {
            current_x --;
        } else {
            if (current_y > 0) {
                current_x = 79;
                current_y--;
            } else {
                printf("attempt to backspace off screen!\n");
                assert(NULL);
            }
        }
        tty_x = current_x;
        tty_y = current_y;
        if ((tty_y + (tty_x / 80) < canvas->lines)) {
            gfx_opengl_render_cursor(canvas, myfont, (tty_x % 80),  tty_y + (tty_x / 80), true);
        }
        return 0;
    }

    /* check if output would cause a scroll before proceeding */

    if (current_x > (width  - 1)) {
        current_x = 0;
        current_y ++;
    }

//    if (c == '\n') {
    //assert(current_y < (canvas->scroll_limit-1));
    /* just scroll, and let libansicanvas handle the wrapping */

    if (current_y == height) {
        ansitty_scroll(canvas);
        current_y -= 1;
    }

    tty_x = current_x;
    tty_y = current_y;
    if ((tty_y + (tty_x / 80) < canvas->lines)) {
        gfx_opengl_render_cursor(canvas, myfont, (tty_x % 80),  tty_y + (tty_x / 80), false);
    }

//    }

    //send_byte_to_canvas(canvas, c);
    last_x = current_x;
    last_y = current_y;
    outbuffer[0] = c;
    if (!ansi_to_canvas(canvas, (unsigned char *) &outbuffer, 1, 0)) {
        printf("+++ error!\n");
        assert(NULL);
    }
    tty_x = current_x;
    tty_y = current_y;

    /* TODO: repaint damaged region only */

    if (canvas->repaint_entire_canvas) {
        //printf("FULL CANVAS REFRESH\n");
        gfx_opengl_canvas_render(canvas, myfont);
        if ((tty_y + (tty_x / 80) < canvas->lines)) {
            gfx_opengl_render_cursor(canvas, myfont, (tty_x % 80),  tty_y + (tty_x / 80), true);
        }
        canvas->repaint_entire_canvas = false;
        canvas->is_dirty = true;
    } else {
        /* regular output */
        if (tty_y > canvas->scroll_limit) {
            ansitty_scroll(canvas);
            tty_y = canvas->scroll_limit;
            //printf("regular_output: tty_y(%u) > canvas->scroll_limit(%u)\r\n", tty_y, canvas->scroll_limit);
            assert(tty_y <= canvas->scroll_limit);
        }
        // assert(tty_y <= canvas->scroll_limit);
        if (c != '\n' && c!= '\r') {
            tty_x = current_x;
            tty_y = current_y;
            gfx_opengl_canvas_render_xy(canvas, myfont, last_x, last_y);
            if ((tty_y + (tty_x / 80) < canvas->lines)) {
                gfx_opengl_render_cursor(canvas, myfont, (tty_x % 80),  tty_y + (tty_x / 80), true);
            }
            canvas->is_dirty = true;
        }
    }

    return 0;
}


void ansitty_expose()
{
    gfx_opengl_expose();

}

bool ansitty_canvas_getdirty()
{
    return canvas->is_dirty;

}

void ansitty_canvas_setdirty(bool state)
{
    canvas->is_dirty = state;

}

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

