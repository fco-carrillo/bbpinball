/*--------------------------------------------------------------------------*/
/*	bbTga.h                                                                 */
/*	Author:			Aarni Gratseff / Rocket                                 */
/*	Created:		26/01/2006                                              */
/*	Description:	Tga loader header                                       */
/*--------------------------------------------------------------------------*/

#ifndef BBTGA_H
#define BBTGA_H

#include "bbPinballDefines.h"

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

typedef struct
{
	GLubyte		identsize;			/* size of ID field that follows 18 byte header (0 usually) */
	GLubyte		colourmaptype;		/* type of colour map 0=none, 1=has palette */
	GLubyte		imagetype;			/* type of image 0=none,1=indexed,2=rgb,3=grey,+8=rle packed */

	GLubyte		alignA;

	/* 4 */

	GLushort	colourmapstart;		/* first colour map entry in palette */
	GLushort	colourmaplength;	/* number of colours in palette */
	GLubyte		colourmapbits;		/* number of bits per palette entry 15,16,24,32 */

	GLubyte		alignB[3];

	/* 12 */

	GLushort	xstart;				/* image x origin */
	GLushort	ystart;				/* image y origin */
	GLushort	width;				/* image width in pixels */
	GLushort	height;				/* image height in pixels */
	GLubyte		bits;				/* image bits per pixel 8,16,24,32 */
	GLubyte		descriptor;			/* image descriptor bits (vh flip bits) */

	GLubyte		alignC[2];

	GLubyte*	data;
} BBTga;

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

BBTga*				BBTga_load			(const char* filename);
GLboolean			BBTga_save			(const char* filename,
										 GLsizei width, GLsizei height,
										 GLsizei bpp, void* data,
										 GLboolean transformData);
void				BBTga_destroy		(BBTga* tga);

#endif /* BBTGA_H */
