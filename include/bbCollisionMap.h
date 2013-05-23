/*--------------------------------------------------------------------------*/
/*	bbCollisionMap.h                                                        */
/*	Author:			Aarni Gratseff / Rocket                                 */
/*	Created:		26/01/2006                                              */
/*	Description:	Collision map class header                              */
/*--------------------------------------------------------------------------*/

#ifndef BBCOLLISIONMAP_H
#define BBCOLLISIONMAP_H

#include "bbPinballDefines.h"

/*--------------------------------------------------------------------------*/
/* Forward declarations                                                     */
/*--------------------------------------------------------------------------*/

typedef struct BBCollisionMap_s BBCollisionMap;

/*--------------------------------------------------------------------------*/
/* Collision map public methods                                             */
/*--------------------------------------------------------------------------*/

BBCollisionMap*		BBCollisionMap_create		(void);
void				BBCollisionMap_destroy		(BBCollisionMap* map);

GLboolean			BBCollisionMap_load			(BBCollisionMap* map, GLubyte* data,
												 GLsizei width, GLsizei height);

void				BBCollisionMap_setArea		(BBCollisionMap* map,
												 GLfloat left, GLfloat top,
												 GLfloat right, GLfloat bottom);

GLuint				BBCollisionMap_getPoint		(BBCollisionMap* map,
												 const GLfloat* pos);

#endif
