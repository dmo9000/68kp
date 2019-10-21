#define _GNU_SOURCE
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "ansistate.h"
#include "ansicanvas.h"
#include "ansitty.h"
#include "gfx_opengl.h"
#include "bmf.h"
#include "8x8.h"

ANSICanvas *canvas = NULL;
BitmapFont *myfont = NULL;

#define TAB_WIDTH	10

#define width 80
#define height 24

#define DEFAULT_WIDTH		80
#define DEFAULT_HEIGHT	24

//uint16_t tty_x = 0;
//uint16_t tty_y = 0;
extern uint16_t current_x;
extern uint16_t current_y;
uint16_t last_x;
uint16_t last_y;
extern bool allow_clear;
bool cursor_phase = false;

extern int process_fd;
//pthread_t graphics_thread;

/* maximum length of a single ANSI sequence */
#define MAX_SEQUENCE		sizeof(uint8_t)


int ansitty_setwindowtitle(char *s)
{

	fprintf(stderr, "+++ ... ansitty_setwindowtitle(%s)\n", s);
	gfx_opengl_setwindowtitle(s);
	return 0;
}

int ansitty_set_process_fd(int fd)
{
    printf("ansitty_set_process_fd(%d)\n", fd);
    process_fd = fd;
    return 1;
}

void *ansitty_rungraphics()
{

    printf("ansitty_rungraphics()\r\n");
    fflush(NULL);
    /* MULTIPLIER SET HERE */
    gfx_opengl_main(canvas, gfx_opengl_getwidth(), gfx_opengl_getheight(), 2, "68kp");
    while (1) {
        /* don't busy wait */
        pthread_yield();
        //usleep(10000);
        sleep(30);
    }
}


ANSITTY *new_ansitty(uint16_t w, uint16_t h)
{
    ANSITTY *New_TTY = NULL;

    fprintf(stderr, "--- new_ansitty(%u, %u)\n", w, h);

    if (!w || !h) {
        return NULL;
    }

    New_TTY = malloc(sizeof(ANSITTY));

    if (!New_TTY) {
        return NULL;
    }

    New_TTY->columns = w;
    New_TTY->rows = h;

    return New_TTY;

}

ANSITTY* ansitty_init()
{
    ANSITTY *New_TTY = NULL;
    ANSIRaster *r = NULL;
    char *font_filename = NULL;
    printf("ansitty_init()\r\n");

    New_TTY = new_ansitty(DEFAULT_WIDTH, DEFAULT_HEIGHT);

//    font_filename = "bmf/8x8.bmf";
    //myfont = bmf_load(font_filename);

    if (!New_TTY) {
        return NULL;
    }

		ansi_setwindowtitlecallback(ansitty_setwindowtitle);

    myfont = bmf_embedded(bmf_8x8_bmf);
    if (!myfont) {
        perror("couldn't get bmf font: ");
        exit(1);
    }
    fflush(NULL);
    allow_clear = true;
    canvas = new_canvas();

    if (!canvas) {
        free(New_TTY);
        return NULL;
    }

    New_TTY->canvas = canvas;

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

    gfx_opengl_setdimensions(New_TTY->columns*8,New_TTY->rows*16);

    pthread_create( &New_TTY->graphics_thread, NULL, ansitty_rungraphics, NULL);

    return New_TTY;

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

    assert(d);


//   if (!d->bytes) {
    /* whoops! what's going on? extend the raster */
    //			raster_extend_length_to(d, 80);
    //		}

    assert(d->bytes);
    assert(d->chardata);
    assert(d->fgcolors);
    assert(d->bgcolors);
    assert(d->attribs);
    assert(raster_delete(d));

    canvas->lines --;
    /* force refresh of entire canvas */

    canvas_reindex(canvas);
    gfx_opengl_render_cursor(canvas, myfont, current_x,  current_y, false);
    gfx_opengl_hwscroll();

    canvas->is_dirty = true;
    return 0;
}

void ansitty_setcursorphase(bool state)
{
    cursor_phase = state;
    return;
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

int ansitty_updatecursor()
{

    //fprintf(stderr, "ansitty_updatecursor()\n");
    if (canvas->cursor_enabled) {
        if (cursor_phase) {
            gfx_opengl_render_cursor(canvas, myfont, current_x,  current_y, true);
            canvas->is_dirty = true;
        } else {
            gfx_opengl_render_cursor(canvas, myfont, current_x,  current_y, false);
            canvas->is_dirty = true;
        }
    }
    return 1;
}


int ansitty_putc_upper(ANSITTY *device, unsigned char c)
{
		/* the "upper" layer, where terminal specific codes are handled */
		//printf("ansitty_putc_upper(..., '%c')\n", c);
		return 0;
}

int ansitty_putc(ANSITTY *device, unsigned char c)
{
  return ansitty_putc_lower(device, c);
}

int ansitty_putc_lower(ANSITTY *device, unsigned char c)
{

		/* the lower layer, where everthing else is dispatched to the 
			 underlying canvas */

    static bool cursor_has_moved = false;
    unsigned char outbuffer[2];
    last_x = current_x;
    last_y = current_y;

    /*
    if (c >=32 && c < 128) {
    fprintf(stderr, "ansitty_putc(%c)\n", c);
    } else {
    fprintf(stderr, "ansitty_putc(0x%02x)\n", c);
    }
    */

    if (!c) return 0;

    if (c == '\t') {
        /* TAB */
        if ((current_x % TAB_WIDTH)) {
            current_x += (TAB_WIDTH- (current_x % TAB_WIDTH));
            return 0;
        }
    }

    if (c == '\b') {
        /* BACKSPACE */
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
        return 0;
    }

    /* check if output would cause a scroll before proceeding */

    last_x = current_x;
    last_y = current_y;
    outbuffer[0] = c;

    /* process output */


    if (current_y >= canvas->scroll_limit) {
        ansitty_scroll(canvas);
        current_y --;
        last_y --;
    } //else {

    /*
    while (current_x >= CONSOLE_WIDTH) {
    		current_x -= CONSOLE_WIDTH;
    		current_y ++;
    		}
    	last_x = current_x;
        last_y = current_y;
    }
    */


    if (device->debug_flags & ANSITTY_DEBUG_OUTPUT) {
        if (c >=32 && c < 128) {
            fprintf(stderr,
                    "  output='%c',current_x=%d,current_y=%d,width=%d,scroll_limit=%d\n",
                    c, current_x, current_y, width, canvas->scroll_limit);
        } else {
            fprintf(stderr,
                    "  output='0x%02x',current_x=%d,current_y=%d,width=%d,scroll_limit=%d\n",
                    c, current_x, current_y, width, canvas->scroll_limit);
        }
    }

    if (!ansi_to_canvas(canvas, (unsigned char *) &outbuffer, 1, 0)) {
        printf("+++ ansitty error! (%s,%d)\n", __FILE__, __LINE__);
        assert(NULL);
    }

    if (last_x == current_x && last_y == current_y) {
        cursor_has_moved = false;
    } else {
        cursor_has_moved = true;
    }

    if (cursor_has_moved) {


        if (canvas->repaint_entire_canvas) {
            gfx_opengl_canvas_render(canvas, myfont);
            canvas->repaint_entire_canvas = false;
        } else {
            gfx_opengl_canvas_render_xy(canvas, myfont, last_x, last_y);
        }
    }

    canvas->is_dirty = true;

    /* reflow start */

    /*
    if (current_x == last_x && current_y == last_y) {
        fprintf(stderr, "*** cursor did not move ***\n");
        cursor_has_moved = false;
    } else {
        fprintf(stderr, "*** cursor DID move ***\n");
        cursor_has_moved = true;
    }
    */

    /*
    if ((current_x / width) > 0) {
    current_y += (current_x / width);
    current_x = (current_x % width);
    				fprintf(stderr,
    					"+++ natural wrap (current_x=%d, current_y=%d)\n",
            current_x, current_y);

    cursor_has_moved = false;
    }
    */

    /*
    if (cursor_has_moved) {
    if (!(current_y < canvas->scroll_limit)) {
        ansitty_scroll(canvas);
        fprintf(stderr,
        	"+++ natural scroll (current_x=%d, current_y=%d)\n",
        current_x, current_y);
        current_y --;
        last_y --;
    }
    if (current_y >= canvas->scroll_limit) {
        fprintf(stderr,
                "+++ WARNING: scroll limit exceeed! current_x=%u, current_y=%u\n",
                current_x, current_y);
    }
    assert(current_y < canvas->scroll_limit);
    }

    */

    /* reflow end */

    /*
    // TODO: repaint damaged region only

    if (canvas->repaint_entire_canvas) {
        gfx_opengl_canvas_render(canvas, myfont);
        canvas->repaint_entire_canvas = false;
        canvas->is_dirty = true;
    } else {
        // regular output
        if (cursor_has_moved || last_y < canvas->scroll_limit) {
            if (c != '\n' && c!= '\r') {
                if (last_y < canvas->scroll_limit) {
                    gfx_opengl_canvas_render_xy(canvas, myfont, last_x, last_y);
                }
            }
        }
        canvas->is_dirty = true;
    }
    	*/
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
    //int i = 0, j = 0;
    static int cx=0, cy=0;
    //printf("output character = %c\r\n", c);
    if (c == '\r') {
        cy++;
        if (cy > 23 ) {
            /* hardware scroll required */
            gfx_opengl_render_cursor(canvas, myfont, current_x,  current_y, false);
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
        gfx_opengl_render_cursor(canvas, myfont, current_x,  current_y, false);
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


