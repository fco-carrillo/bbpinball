/*--------------------------------------------------------------------------*/
/*	bbCollisionMap.c                                                        */
/*	Author:			Aarni Gratseff / Rocket                                 */
/*	Created:		26/01/2006                                              */
/*	Description:	Collision map class                                     */
/*--------------------------------------------------------------------------*/

#include "bbCollisionMap.h"

#include <stdlib.h>
#ifdef __ARM__
#include <string.h>
#endif

/*--------------------------------------------------------------------------*/
/* Collision map struct                                                     */
/*--------------------------------------------------------------------------*/

struct BBCollisionMap_s
{
	GLubyte*	data;
	GLsizei		width;
	GLsizei		height;

	GLfloat		left;
	GLfloat		top;
	GLfloat		right;
	GLfloat		bottom;
};

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

BBCollisionMap* BBCollisionMap_create ()
{
	BBCollisionMap* map = malloc(sizeof(BBCollisionMap));
	if (!map)
		return NULL;

	map->data		= NULL;
	map->width		= 0;
	map->height		= 0;

	map->left		= 0.0f;
	map->top		= 0.0f;
	map->right		= 0.0f;
	map->bottom		= 0.0f;

	return map;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

void BBCollisionMap_destroy (BBCollisionMap* map)
{
	BB_ASSERT(map);
	free(map->data);
	free(map);
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

GLboolean BBCollisionMap_load (BBCollisionMap* map, GLubyte* data,
							   GLsizei width, GLsizei height)
{
	const GLsizei size = sizeof(GLubyte) * 3 * width * height;
	BB_ASSERT(map);

	map->data = malloc(size);
	if (!map->data)
		return GL_FALSE;

	memcpy(map->data, data, size);

	map->width = width;
	map->height = height;

	return GL_TRUE;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

void BBCollisionMap_setArea (BBCollisionMap* map, GLfloat left, GLfloat top,
							 GLfloat right, GLfloat bottom)
{
	BB_ASSERT(map);

	map->left = left;
	map->top = top;
	map->right = right;
	map->bottom = bottom;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

GLuint BBCollisionMap_getPoint (BBCollisionMap* map, const GLfloat* pos)
{
	BB_ASSERT(map);
	{
		GLint mapX = map->width - 1 - (GLint)(((pos[0] - map->left) * map->width) / (map->right - map->left));
		GLint mapY = map->height - 1 - (GLint)(((pos[1] - map->bottom) * map->height) / (map->top - map->bottom));
		GLuint c;
		GLuint dataPos;

		if (mapX < 0 || mapX >= map->width || mapY < 0 || mapY >= map->height)
			return 0xffffffff;

		dataPos = (mapX + mapY * map->width) * 3;
		c = map->data[dataPos];
		c |= map->data[dataPos + 1] << 8;
		c |= map->data[dataPos + 2] << 16;

		return c;
	}
}
