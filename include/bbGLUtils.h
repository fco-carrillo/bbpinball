/*--------------------------------------------------------------------------*/
/*	bbGLUtils.h                                                             */
/*	Author:			Aarni Gratseff / Rocket                                 */
/*	Created:		17/01/2006                                              */
/*	Description:	Various GL utilities header                             */
/*--------------------------------------------------------------------------*/

#ifndef BBGLUTILS_H
#define BBGLUTILS_H

#include "bbPinballDefines.h"

/*--------------------------------------------------------------------------*/
/* Function declarations                                                    */
/*--------------------------------------------------------------------------*/

GLuint		bbCompileShader		(const char* filename);
GLuint		bbLinkProgram		(GLuint vertexShader, GLuint fragShader);

#endif /* BBGLUTILS_H */
