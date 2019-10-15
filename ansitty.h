#ifndef __ANSITTY_H__
#define __ANSITTY_H__
#include "ansicanvas.h"

#define ANSITTY_DEBUG_NONE				0
#define ANSITTY_DEBUG_INPUT				1		/* keyboard/mouse to terminal */
#define ANSITTY_DEBUG_OUTPUT			2		/* process output to terminal */
#define ANSITTY_DEBUG_THOUGHPUT		4		/* private communication between process and terminal */	


typedef struct _ansitty {
												uint16_t columns;
												uint16_t rows;
												ANSICanvas *canvas;
												uint32_t debug_flags;
												pthread_t graphics_thread;
												} ANSITTY;

ANSITTY* ansitty_init();
int ansitty_putc(ANSITTY *device, unsigned char c);
int ansitty_set_process_fd(int fd);
void ansitty_setcursorphase(bool state);
int ansitty_updatecursor();

/* belongs to gfx_opengl.c, but we'll leave it here for now */

int tty_getbuflen();
int input_character();

/* latest API changes */

int ansitty_putc_lower(ANSITTY *device, unsigned char c);

#endif /* __ANSITTY_H__ */
