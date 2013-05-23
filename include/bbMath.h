/*--------------------------------------------------------------------------*/
/*	bbMath.h                                                                */
/*	Author:			Aarni Gratseff / Rocket                                 */
/*	Created:		18/01/2006                                              */
/*	Description:	Mathematic operations header                            */
/*--------------------------------------------------------------------------*/

#ifndef BBMATH_H
#define BBMATH_H

#include "bbPinballDefines.h"

#include <math.h>

#ifndef sinf
#define sinf(a)			((GLfloat)sin(a))
#define cosf(a)			((GLfloat)cos(a))
#define sqrtf(v)		((GLfloat)sqrt(v))
#endif

#define bbAbs(v)		(v < 0.0f ? -v : v)

#define bbSwapInt(a, b)	{ GLint c = b; b = a; a = c; }

/*--------------------------------------------------------------------------*/
/* Function declarations                                                    */
/*--------------------------------------------------------------------------*/

GLboolean	bbAlmostZero			(GLfloat v);
GLfloat		bbClamp					(GLfloat v, GLfloat min, GLfloat max);

GLfloat		bbVec2Length			(const GLfloat* vec);
GLfloat		bbVec2Square			(const GLfloat* vec);
void		bbVec2Normalize			(GLfloat* vec);
GLfloat		bbVec2DotProduct		(const GLfloat* left, const GLfloat* right);
void		bbVec2CrossProduct		(const GLfloat* left, const GLfloat* right, GLfloat* result);
GLboolean	bbVec2ClosestPoint		(const GLfloat* lineStart, const GLfloat* lineEnd,
									 const GLfloat* point, GLfloat* result);
void		bbVec2Sub				(const GLfloat* left, const GLfloat* right, GLfloat* result);
void		bbVec2Add				(const GLfloat* left, const GLfloat* right, GLfloat* result);
void		bbVec2Transform			(const GLfloat* vec, const GLfloat* matrix, GLfloat* result,
									 GLint xIndex, GLint yIndex);
void		bbVec2TransformNoTrans	(const GLfloat* vec, const GLfloat* matrix, GLfloat* result,
									 GLint xIndex, GLint yIndex);
void		bbVec2Reflect			(const GLfloat* dir, const GLfloat* normal, GLfloat* out);

void		bbVec3CrossProduct		(const GLfloat* left, const GLfloat* right, GLfloat* result);
GLfloat		bbVec3DotProduct		(const GLfloat* left, const GLfloat* right);
GLfloat		bbVec3Length			(const GLfloat* vec);
void		bbVec3Normalize			(GLfloat* vec);
void		bbVec3Transform			(const GLfloat* vec, const GLfloat* matrix, GLfloat* result);
void		bbVec3Sub				(const GLfloat* left, const GLfloat* right, GLfloat* result);
void		bbVec3Add				(const GLfloat* left, const GLfloat* right, GLfloat* result);
void		bbVec3Reflect			(const GLfloat* dir, const GLfloat* normal, GLfloat* out);

void		bbMatrixIdentity		(GLfloat* result);
void		bbMatrixRotationX		(GLfloat angle, GLfloat* result);
void		bbMatrixRotationY		(GLfloat angle, GLfloat* result);
void		bbMatrixRotationZ		(GLfloat angle, GLfloat* result);
void		bbMatrixTranslation		(GLfloat x, GLfloat y, GLfloat z, GLfloat* result);
void		bbMatrixScaling			(GLfloat x, GLfloat y, GLfloat z, GLfloat* result);
void		bbMatrixMultiply		(const GLfloat* left, const GLfloat* right, GLfloat* result);

void		bbMatrixProjection		(GLfloat verticalFov, GLfloat aspect,
									 GLfloat near, GLfloat far, GLfloat* result);

void		bbMatrixView			(const GLfloat* eye, const GLfloat* lookAt,
									 GLfloat roll, GLfloat* result);

void		bbMatrixGetTranslation	(const GLfloat* matrix, GLfloat* trans);

#endif /* BBMATH_H */
