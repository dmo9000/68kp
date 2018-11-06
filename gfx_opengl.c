#include <GL/glut.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include "rawfont.h"
#include "ansicanvas.h"

//OGL_Window* window;
//OGL_Renderer* renderer;
//OGL_Surface *tmpsurface;

// Display size
#define SCREEN_WIDTH    640
#define SCREEN_HEIGHT   384

int display_width = 0;
int display_height = 0;
int modifier = 2;


typedef unsigned char u8;
u8 screenData[SCREEN_HEIGHT][SCREEN_WIDTH][3];
int gfx_opengl_drawglyph(BitmapFont *font, uint16_t px, uint16_t py, uint8_t glyph, uint8_t fg, uint8_t bg, uint8_t attr);
char *p = "HELLO WORLD THIS IS YOUR CAPTAIN SPEAKING PT2, PLEASE STANDBY";
extern BitmapFont *myfont;

void output_character(char c)
{   
    int i = 0, j = 0;
    static int cx=0, cy=0;
    
    gfx_opengl_drawglyph(myfont, cx, cy, c, 7, 0, 0);
    cx++;
    if (cx == 80) {
        cx = 0;
        cy ++;
    }
}

void updateTexture()
{
    while (p[0] != 0) {
        output_character(p[0]);
        p++;
    }
    glTexSubImage2D(GL_TEXTURE_2D, 0,0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)screenData);
    glBegin( GL_QUADS );
    glTexCoord2d(0.0, 0.0);
    glVertex2d(0.0,       0.0);
    glTexCoord2d(1.0, 0.0);
    glVertex2d(display_width, 0.0);
    glTexCoord2d(1.0, 1.0);
    glVertex2d(display_width, display_height);
    glTexCoord2d(0.0, 1.0);
    glVertex2d(0.0,       display_height);
    glEnd();
}


void display()
{
    glClear(GL_COLOR_BUFFER_BIT);
    
    updateTexture();
    glutSwapBuffers();
}

void reshape_window(GLsizei w, GLsizei h)
{
		//w = SCREEN_WIDTH;
		//h = SCREEN_HEIGHT;
    printf("reshape_window(w=%u,h=%u)\n", w, h);
    glClearColor(0.0f, 0.0f, 0.5f, 0.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, w, h, 0);
    glMatrixMode(GL_MODELVIEW);
    glViewport(0, 0, w, h);
    display_width = w;
    display_height = h;
}


// Setup Texture
void setupTexture()
{


    // Clear screen
    for(int y = 0; y < SCREEN_HEIGHT; ++y)  {
        for(int x = 0; x < SCREEN_WIDTH; ++x) {
            if (y % 2) {
                screenData[y][x][0] = 255;
                screenData[y][x][1] = 0;
                screenData[y][x][2] = 0;
            } else {
                screenData[y][x][0] = 0;
                screenData[y][x][1] = 255;
                screenData[y][x][2] = 0;
            }
        }
    }

    // Create a texture
    glTexImage2D(GL_TEXTURE_2D, 0, 3, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)screenData);

    // Set up the texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    // Enable textures
    glEnable(GL_TEXTURE_2D);
}


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
		int argc = 0;
		char *argv[] = { NULL };
    glutInit(&argc, argv);
		//glutInit(0, NULL);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);

    display_width = SCREEN_WIDTH * modifier;
    display_height = SCREEN_HEIGHT * modifier;

    glutInitWindowSize(display_width, display_height);
//    glutInitWindowPosition(320, 320);
    glutCreateWindow("68K");

    glutDisplayFunc(display);
    glutIdleFunc(display);
    glutReshapeFunc(reshape_window);

    setupTexture();

    glutMainLoop();

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
