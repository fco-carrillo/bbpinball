/*--------------------------------------------------------------------------*/
/*	bbTga.c                                                                 */
/*	Author:			Aarni Gratseff / Rocket                                 */
/*	Created:		26/01/2006                                              */
/*	Description:	Tga loader                                              */
/*--------------------------------------------------------------------------*/

#include "bbTga.h"

#include <stdio.h>
#include <stdlib.h>

#if defined(__ARM__)
#include <string.h>
#endif

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

static void memcpy_r (void* tgt, const void* src, GLuint size)
{
	GLuint i = size;
	GLubyte* s = (GLubyte*)src + size - 1;
	GLubyte* t = (GLubyte*)tgt;
	while (i--)
		*t++ = *s--;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

BBTga* BBTga_load (const char* filename)
{
	BBTga* tga;
	FILE* f;
	GLsizei size;
	GLsizei bpp;

	BB_ASSERT(filename);

	f = BB_FOPEN(filename, "rb");
	if (!f)
	{
#ifdef BB_DEVEL
		printf("Error: Unable to open tga file \"%s\"\n", filename);
#endif
		return NULL;
	}

	tga = malloc(sizeof(BBTga));
	if (!tga)
	{
		fclose(f);
		return NULL;
	}

	fread(tga, 1, 3, f);
	fread((GLubyte*)(tga) + 4, 1, 5, f);
	fread((GLubyte*)(tga) + 12, 1, 10, f);

	if (tga->bits == 24)
		bpp = 3;
	else if (tga->bits == 32)
		bpp = 4;
	else
	{
		free(tga);
		fclose(f);
		return NULL;
	}

	size = tga->width * tga->height * bpp;

	tga->data = malloc(size);
	if (!tga->data)
	{
		BBTga_destroy(tga);
		return NULL;
	}

	fread(tga->data, 1, size, f);
	fclose(f);

/*	{
		GLuint i = tga->width * tga->height;
		GLubyte* pos = tga->data;
		while (i--)
		{
			GLubyte r = pos[0];
			pos[0] = pos[2];
			pos[2] = r;
			pos += 3;
		}
	}*/
	if (bpp == 3)
	{
		GLuint rowSize = tga->width * bpp;
		GLuint h = ((tga->height + 1) >> 1);
		GLubyte* row = malloc(rowSize);
		if (!row)
		{
			BBTga_destroy(tga);
			return NULL;
		}
		while (h--)
		{
			memcpy(row, tga->data + rowSize * h, rowSize);
//			memcpy_r(tga->data + rowSize * h, row, rowSize);
			memcpy_r(tga->data + rowSize * h, tga->data + (tga->height - 1 - h) * rowSize, rowSize);
			memcpy_r(tga->data + (tga->height - 1 - h) * rowSize, row, rowSize);
		}
		free(row);
	}
	else if (bpp == 4)
	{
		GLint x;
		GLint y;
		GLubyte* temp = malloc(size);
		if (!temp)
		{
			BBTga_destroy(tga);
			return NULL;
		}
		for (x = 0; x < tga->width; x++)
		{
			for (y = 0; y < tga->height; y++)
			{
				GLuint srcPos = (x + y * tga->width) * bpp;
				GLuint tgtPos = ((tga->width - 1 - x) + (tga->height - 1 - y) * tga->width) * bpp;
				temp[tgtPos + 0] = tga->data[srcPos + 2];
				temp[tgtPos + 1] = tga->data[srcPos + 1];
				temp[tgtPos + 2] = tga->data[srcPos + 0];
				temp[tgtPos + 3] = tga->data[srcPos + 3];
			}
		}
		free(tga->data);
		tga->data = temp;
	}

	return tga;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

GLboolean BBTga_save (const char* filename, GLsizei width, GLsizei height, GLsizei bpp, void* data, GLboolean transformData)
{
	/*FILE* f;
	BBTga tga;

	BB_ASSERT(data);

	f = BB_FOPEN(filename, "wb");
	if (!f)
		return GL_FALSE;

	tga.identsize = 0;
	tga.colourmaptype = 0;
	tga.imagetype = 2;
	tga.colourmapstart = 0;
	tga.colourmaplength = 0;
	tga.colourmapbits = 0;

	tga.xstart = 0;
	tga.ystart = 0;

	tga.width = width;
	tga.height = height;
	tga.bits = bpp == 3 ? 24 : 32;
	tga.descriptor = 0;

	fwrite(&tga, 1, 3, f);
	fwrite((GLubyte*)(&tga) + 4, 1, 5, f);
	fwrite((GLubyte*)(&tga) + 12, 1, 10, f);

	if (transformData)
	{
		GLint x;
		GLint y;
		GLubyte* in = (GLubyte*)data;

		GLubyte* temp = malloc(width * height * bpp);
		if (!temp)
			return GL_FALSE;

		for (x = 0; x < width; x++)
		{
			for (y = 0; y < height; y++)
			{
				GLuint srcPos = (x + y * width) * bpp;
				GLuint tgtPos = ((width - 1 - x) + (height - 1 - y) * width) * bpp;
				temp[tgtPos + 0] = in[srcPos + 2];
				temp[tgtPos + 1] = in[srcPos + 1];
				temp[tgtPos + 2] = in[srcPos + 0];
				if (bpp == 4)
					temp[tgtPos + 3] = in[srcPos + 3];
			}
		}
		fwrite(temp, 1, bpp * width * height, f);
		free(temp);
	}
	else
	{
		fwrite(data, 1, bpp * width * height, f);
	}

	fclose(f);*/

	return GL_TRUE;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

void BBTga_destroy (BBTga* tga)
{
	BB_ASSERT(tga);
	free(tga->data);
	free(tga);
}
