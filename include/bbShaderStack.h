/*--------------------------------------------------------------------------*/
/*	bbShaderStack.h                                                         */
/*	Author:			Aarni Gratseff / Rocket                                 */
/*	Created:		18/01/2006                                              */
/*	Description:	Shader stack class header                               */
/*--------------------------------------------------------------------------*/

#ifndef BBSHADERSTACK_H
#define BBSHADERSTACK_H

#include "bbPinballDefines.h"

/*--------------------------------------------------------------------------*/
/* Forward declarations                                                     */
/*--------------------------------------------------------------------------*/

typedef struct BBShaderStack_s	BBShaderStack;

/*--------------------------------------------------------------------------*/
/* ShaderStack public methods                                               */
/*--------------------------------------------------------------------------*/

BBShaderStack*		BBShaderStack_create			(void);
void				BBShaderStack_destroy			(BBShaderStack* stack);

GLuint				BBShaderStack_getProgram		(BBShaderStack* stack,
													 const char* vertexShader,
													 const char* fragShader);

#endif /* BBSHADERSTACK_H */
