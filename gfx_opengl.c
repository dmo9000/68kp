#include <GL/glut.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include "rawfont.h"
#include "ansicanvas.h"

//OGL_Window* window;
//OGL_Renderer* renderer;
//OGL_Surface *tmpsurface;

// Display size
#define SCREEN_WIDTH    640
#define SCREEN_HEIGHT   384


typedef unsigned char u8;
u8 screenData[SCREEN_HEIGHT][SCREEN_WIDTH][3];

void setTexturePixel(int x, int y, u8 r, u8 g, u8 b)
{
    screenData[y][x][0] = r;
    screenData[y][x][1] = g;
    screenData[y][x][2] = b;
}


int gfx_opengl_expose()
{

//   OGL_RenderPresent(renderer);
    return 0;
}

int gfx_opengl_hwscroll()
{

    /* TODO: this is hardcoded for an 80x24 display and needs to be made more flexible */
//    OGL_Rect s, d;
//    OGL_Surface* winsurf;

//    winsurf = OGL_GetWindowSurface(window);
//    assert(winsurf);

    /*
    s.x = 0;
    s.y = 16;
    s.w = 640;
    s.h = 384 - 16;

    d.x = 0;
    d.y = 0;
    d.w = 640;
    d.h = 384 - 16;
    */

//    assert(!OGL_BlitSurface(winsurf, &s, tmpsurface, &d));
//    assert(!OGL_BlitSurface(tmpsurface, &d, winsurf, &d));
    return 1;
}

int gfx_opengl_drawglyph(BitmapFont *font, uint16_t px, uint16_t py, uint8_t glyph, uint8_t fg, uint8_t bg, uint8_t attr)
{

    RGBColour *fgc;
    RGBColour *bgc;
    uint8_t rx = 0;
    uint8_t h = 0;

//    printf("gfx_opengl_drawglyph(%u, %u, %u, %u, '%c', fg=%u, bg=%u)\n", px, py, font->header.px, font->header.py, glyph, fg, bg);

    if (attr & ATTRIB_REVERSE) {
        bgc = canvas_displaycolour(fg + ((attr & ATTRIB_BOLD ? 8 : 0)));
        fgc = canvas_displaycolour(bg);
    } else {
        fgc = canvas_displaycolour(fg + ((attr & ATTRIB_BOLD ? 8 : 0)));
        bgc = canvas_displaycolour(bg);
    }

    for (uint8_t ii = 0; ii < font->header.py; ii++) {
        h = 0;
        for (uint8_t jj = 128; jj >0; jj = jj >> 1) {
            //printf("%u -> %u, ", r, jj);
            rx = font->fontdata[(glyph*font->header.py) + ii];

            if (rx & jj) {
//                    setTexturePixel((px*8) + h, (py*16)+(ii*2), fgc->r, fgc->g, fgc->b);
//                    setTexturePixel((px*8) + h, (py*16)+(ii*2)+1, fgc->r, fgc->g, fgc->b);
                setTexturePixel((px*8) + h, (py*16)+(ii*2), 255, 255, 255);
                setTexturePixel((px*8) + h, (py*16)+(ii*2)+1, 255, 255, 255);
//                    printf("X");
            } else {
//                    setTexturePixel((px*8) + h, (py*16)+(ii*2), bgc->r, bgc->g, bgc->b);
//                    setTexturePixel((px*8) + h, (py*16)+(ii*2)+1, bgc->r, bgc->g, bgc->b);
                setTexturePixel((px*8) + h, (py*16)+(ii*2), 0, 0, 0);
                setTexturePixel((px*8) + h, (py*16)+(ii*2)+1, 0, 0, 0);
//                    printf(" ");
            }
            h++;
        }
        //  printf("\n");
    }
    return 0;
}



int gfx_opengl_main(uint16_t xsize, uint16_t ysize, char *WindowTitle)
{
    int posX = 100;
    int posY = 200;
    int sizeX = xsize;
    int sizeY =  ysize;

//    if ( OGL_Init( OGL_INIT_EVERYTHING ) != 0 )
//    {
//        // Something failed, print error and exit.
//        printf(" Failed to initialize OGL : %s\n", OGL_GetError());
//       return -1;
//    }
//
//    window = OGL_CreateWindow(WindowTitle, posX, posY, sizeX, sizeY, 0 );
//    tmpsurface = OGL_CreateRGBSurface(0, 640, 384, 24, 0, 0, 0, 0);
//    assert(tmpsurface);

    /*
    if ( window == NULL )
    {
        printf( "Failed to create window : %s", OGL_GetError());
        return -1;
    }
    */

//    renderer = OGL_CreateRenderer( window, -1, OGL_RENDERER_ACCELERATED );

    /*
        if ( renderer == NULL )
        {
            printf( "Failed to create renderer : %s", OGL_GetError());
            return -1;
        }
    */

//    OGL_RenderSetLogicalSize( renderer, sizeX, sizeY );
//    OGL_SetRenderDrawColor( renderer, 0, 0, 0, 255 );
//    OGL_RenderClear( renderer );
//    OGL_RenderPresent( renderer);
    return 0;
}

void  gfx_opengl_clear()
{
    //OGL_RenderClear( renderer);

}

int gfx_opengl_canvas_render(ANSICanvas *canvas, BitmapFont *myfont)
{
    ANSIRaster *r = NULL;
    uint16_t width = 0, height = 0;
    assert(canvas);
    width = canvas_get_width(canvas);
    height = canvas_get_height(canvas);

    assert(width);
    assert(height);

    //printf("gfx_opengl_canvas_render(%ux%u)\n", width, height);
    for (uint16_t ii = 0; ii < height; ii++) {
        r = canvas_get_raster(canvas, ii);
        if (r) {
            for (uint16_t jj = 0; jj < r->bytes; jj++) {
                /* FIXME: call gfx_opengl_canvas_render_xy() instead */
                gfx_opengl_drawglyph(myfont, jj, ii, r->chardata[jj], r->fgcolors[jj], r->bgcolors[jj], r->attribs[jj]);
            }
        } else {
            printf("canvas data missing for raster %u\n", ii);
        }
    }
    return 0;
}

int gfx_opengl_canvas_render_xy(ANSICanvas *canvas, BitmapFont *myfont, uint16_t x, uint16_t y)
{
    ANSIRaster *r = NULL;

    assert(y <= 24);
    r = canvas_get_raster(canvas, y);
    if (!r) {
        printf("canvas_get_raster(%u) failed\n", y);
    }
    assert(r);
    if (!r->chardata) {
        printf("+++ gfx_opengl_canvas_render_xy(%u,%u) -> failed\n", x, y);
        assert(r->chardata);
    }
    //assert(x < r->bytes);
    if (x < r->bytes) {
        gfx_opengl_drawglyph(myfont, x, y, r->chardata[x], r->fgcolors[x], r->bgcolors[x], r->attribs[x]);
    }
    return 1;
}

int gfx_opengl_render_cursor(ANSICanvas *canvas, BitmapFont *myfont, uint16_t x,  uint16_t y, bool state)
{
    ANSIRaster *r = NULL;

    assert(canvas);
    assert(y <= 24);
    r = canvas_get_raster(canvas, y);
    if (!r) {
        printf("canvas_get_raster(%u) failed\n", y);
    }
    assert(r);
    if (!r->chardata) {
        printf("+++ gfx_opengl_canvas_render_xy(%u,%u) -> failed\n", x, y);
        assert(r->chardata);
    }


    switch (state) {
    case true:
        if (canvas->cursor_enabled) {
            gfx_opengl_drawglyph(myfont, x, y, ' ', 0, 7, ATTRIB_NONE);
        }
        break;
    case false:
        gfx_opengl_drawglyph(myfont, x, y, r->chardata[x], r->fgcolors[x], r->bgcolors[x], r->attribs[x]);
        break;
    }

    return 1;
}
