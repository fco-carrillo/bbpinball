/*--------------------------------------------------------------------------*/
/*	bbMath.c                                                                */
/*	Author:			Aarni Gratseff / Rocket                                 */
/*	Created:		18/01/2006                                              */
/*	Description:	Mathematic operations                                   */
/*--------------------------------------------------------------------------*/

#include "bbMath.h"
#ifdef __ARM__
#include <string.h>
#endif

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

GLboolean bbAlmostZero (GLfloat v)
{
	if (v < 0.000001 && v > -0.000001)
		return GL_TRUE;

	return GL_FALSE;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

GLfloat bbClamp (GLfloat v, GLfloat min, GLfloat max)
{
	BB_ASSERT(min <= max);
	return (v < min) ? min : (v > max) ? max : v;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

GLfloat bbVec2Length (const GLfloat* vec)
{
	BB_ASSERT(vec);
	return sqrtf(vec[0] * vec[0] + vec[1] * vec[1]);
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

GLfloat bbVec2Square (const GLfloat* vec)
{
	BB_ASSERT(vec);
	return vec[0] * vec[0] + vec[1] * vec[1];
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

void bbVec2Normalize (GLfloat* vec)
{
	GLfloat len;
	BB_ASSERT(vec);
	len = bbVec2Length(vec);
	vec[0] /= len;
	vec[1] /= len;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

GLfloat bbVec2DotProduct (const GLfloat* left, const GLfloat* right)
{
	BB_ASSERT(left && right);
	return left[0] * right[0] + left[1] * right[1];
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

void bbVec2CrossProduct (const GLfloat* left, const GLfloat* right, GLfloat* result)
{
	GLfloat r[2];
	BB_ASSERT(left && right && result);

	r[0] = left[1] - right[1];
	r[1] = -(left[0] - right[0]);

	memcpy(result, r, sizeof(GLfloat) * 2);
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

void bbVec2Sub (const GLfloat* left, const GLfloat* right, GLfloat* result)
{
	BB_ASSERT(left && right && result);
	result[0] = left[0] - right[0];
	result[1] = left[1] - right[1];
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

void bbVec2Add (const GLfloat* left, const GLfloat* right, GLfloat* result)
{
	BB_ASSERT(left && right && result);
	result[0] = left[0] + right[0];
	result[1] = left[1] + right[1];
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

GLboolean bbVec2ClosestPoint (const GLfloat* lineStart, const GLfloat* lineEnd,
							  const GLfloat* point, GLfloat* result)
{
	GLfloat u;
	GLfloat len;
	GLfloat lineVec[2];
	BB_ASSERT(lineStart && lineEnd && point && result);

	u = (point[0] - lineStart[0]) * (lineEnd[0] - lineStart[0]) +
		(point[1] - lineStart[1]) * (lineEnd[1] - lineStart[1]);

	bbVec2Sub(lineEnd, lineStart, lineVec);
	len = bbVec2Square(lineVec);

	u /= len;

	result[0] = lineStart[0] + u * lineVec[0];
	result[1] = lineStart[1] + u * lineVec[1];

	if (u < 0.0f || u > 1.0f) /* Point not on line */
		return GL_FALSE;

	return GL_TRUE;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

void bbVec2Transform (const GLfloat* vec, const GLfloat* matrix, GLfloat* result, GLint xIndex, GLint yIndex)
{
	GLfloat r[2];
	BB_ASSERT(vec && matrix && result);

	r[0] = vec[0] * matrix[xIndex + xIndex * 4] + vec[1] * matrix[xIndex + yIndex * 4] + matrix[xIndex + 3 * 4];
	r[1] = vec[0] * matrix[yIndex + xIndex * 4] + vec[1] * matrix[yIndex + yIndex * 4] + matrix[yIndex + 3 * 4];

	memcpy(result, r, 2 * sizeof(GLfloat));
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

void bbVec2TransformNoTrans (const GLfloat* vec, const GLfloat* matrix, GLfloat* result, GLint xIndex, GLint yIndex)
{
	GLfloat r[2];
	BB_ASSERT(vec && matrix && result);

	r[0] = vec[0] * matrix[xIndex + xIndex * 4] + vec[1] * matrix[xIndex + yIndex * 4];
	r[1] = vec[0] * matrix[yIndex + xIndex * 4] + vec[1] * matrix[yIndex + yIndex * 4];

	memcpy(result, r, 2 * sizeof(GLfloat));
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

void bbVec2Reflect (const GLfloat* dir, const GLfloat* normal, GLfloat* out)
{
	GLfloat v;
	BB_ASSERT(dir && normal && out);

	/* R = 2.0*(N.L)*N - L */

	v = 2.0f * bbVec2DotProduct(dir, normal);
	out[0] = normal[0] * v - dir[0];
	out[1] = normal[1] * v - dir[1];
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

void bbVec3CrossProduct (const GLfloat* left, const GLfloat* right, GLfloat* result)
{
	GLfloat r[3];
	BB_ASSERT(left && right && result);
	r[0] = left[1] * right[2] - right[1] * left[2];
	r[1] = left[2] * right[0] - right[2] * left[0];
	r[2] = left[0] * right[1] - right[0] * left[1];
	memcpy(result, r, sizeof(GLfloat) * 3);
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

GLfloat bbVec3DotProduct (const GLfloat* left, const GLfloat* right)
{
	BB_ASSERT(left && right);
	return left[0] * right[0] + left[1] * right[1] + left[2] * right[2];
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

GLfloat bbVec3Length (const GLfloat* vec)
{
	BB_ASSERT(vec);
	return sqrtf(bbVec3DotProduct(vec, vec));
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

void bbVec3Normalize (GLfloat* vec)
{
	GLfloat length;
	BB_ASSERT(vec);
	length = bbVec3Length(vec);
	vec[0] /= length;
	vec[1] /= length;
	vec[2] /= length;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

void bbVec3Transform (const GLfloat* vec, const GLfloat* matrix, GLfloat* result)
{
	GLfloat r[3];
	BB_ASSERT(vec && matrix && result);

	r[0] = vec[0] * matrix[0 + 0 * 4] + vec[1] * matrix[0 + 1 * 4] + vec[2] * matrix[0 + 2 * 4] + matrix[0 + 3 * 4];
	r[1] = vec[0] * matrix[1 + 0 * 4] + vec[1] * matrix[1 + 1 * 4] + vec[2] * matrix[1 + 2 * 4] + matrix[1 + 3 * 4];
	r[2] = vec[0] * matrix[2 + 0 * 4] + vec[1] * matrix[2 + 1 * 4] + vec[2] * matrix[2 + 2 * 4] + matrix[2 + 3 * 4];

	memcpy(result, r, 3 * sizeof(GLfloat));
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

void bbVec3Sub (const GLfloat* left, const GLfloat* right, GLfloat* result)
{
	BB_ASSERT(left && right && result);
	result[0] = left[0] - right[0];
	result[1] = left[1] - right[1];
	result[2] = left[2] - right[2];
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

void bbVec3Add (const GLfloat* left, const GLfloat* right, GLfloat* result)
{
	BB_ASSERT(left && right && result);
	result[0] = left[0] + right[0];
	result[1] = left[1] + right[1];
	result[2] = left[2] + right[2];
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

void bbVec3Reflect (const GLfloat* dir, const GLfloat* normal, GLfloat* out)
{
	GLfloat v;
	BB_ASSERT(dir && normal && out);

	/* R = 2.0*(N.L)*N - L */

	v = 2.0f * bbVec3DotProduct(dir, normal);
	out[0] = normal[0] * v - dir[0];
	out[1] = normal[1] * v - dir[1];
	out[2] = normal[2] * v - dir[2];
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

void bbMatrixIdentity (GLfloat* result)
{
	GLuint i;
	GLuint j;
	BB_ASSERT(result);

	for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j++)
			result[i + j * 4] = 0.0f;

	result[0 + 0 * 4] = 1.0f;
	result[1 + 1 * 4] = 1.0f;
	result[2 + 2 * 4] = 1.0f;
	result[3 + 3 * 4] = 1.0f;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

void bbMatrixRotationX (GLfloat angle, GLfloat* result)
{
	BB_ASSERT(result);
	bbMatrixIdentity(result);
	result[1 + 1 * 4] = cosf(angle);		result[2 + 1 * 4] = -sinf(angle);
	result[1 + 2 * 4] = sinf(angle);		result[2 + 2 * 4] = cosf(angle);
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

void bbMatrixRotationY (GLfloat angle, GLfloat* result)
{
	BB_ASSERT(result);
	bbMatrixIdentity(result);
	result[0 + 0 * 4] = cosf(angle);		result[2 + 0 * 4] = -sinf(angle);
	result[0 + 2 * 4] = sinf(angle);		result[2 + 2 * 4] = cosf(angle);
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

void bbMatrixRotationZ (GLfloat angle, GLfloat* result)
{
	BB_ASSERT(result);
	bbMatrixIdentity(result);
	result[0 + 0 * 4] = sinf(angle);		result[1 + 0 * 4] = -cosf(angle);
	result[0 + 1 * 4] = cosf(angle);		result[1 + 1 * 4] = sinf(angle);
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

void bbMatrixTranslation (GLfloat x, GLfloat y, GLfloat z, GLfloat* result)
{
	BB_ASSERT(result);
	bbMatrixIdentity(result);
	result[0 + 3 * 4] = x;
	result[1 + 3 * 4] = y;
	result[2 + 3 * 4] = z;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

void bbMatrixScaling (GLfloat x, GLfloat y, GLfloat z, GLfloat* result)
{
	BB_ASSERT(result);
	bbMatrixIdentity(result);
	result[0 + 0 * 4] = x;
	result[1 + 1 * 4] = y;
	result[2 + 2 * 4] = z;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

void bbMatrixMultiply (const GLfloat* left, const GLfloat* right, GLfloat* result)
{
	int i;
	GLfloat tmp[4 * 4];
	GLfloat a;
	GLfloat b;
	GLfloat c;
	GLfloat d;

	BB_ASSERT(left && right && result);

	for (i = 0; i < 4; i++)
	{
		a = left[i + 4 * 0];
		b = left[i + 4 * 1];
		c = left[i + 4 * 2];
		d = left[i + 4 * 3];
		tmp[i + 4 * 0] = (a * right[0 + 4 * 0] + b * right[1 + 4 * 0] +
			c * right[2 + 4 * 0] + d * right[3 + 4 * 0]);
		tmp[i + 4 * 1] = (a * right[0 + 4 * 1] + b * right[1 + 4 * 1] +
			c * right[2 + 4 * 1] + d * right[3 + 4 * 1]);
		tmp[i + 4 * 2] = (a * right[0 + 4 * 2] + b * right[1 + 4 * 2] +
			c * right[2 + 4 * 2] + d * right[3 + 4 * 2]);
		tmp[i + 4 * 3] = (a * right[0 + 4 * 3] + b * right[1 + 4 * 3] +
			c * right[2 + 4 * 3] + d * right[3 + 4 * 3]);
	}

	memcpy(result, tmp, 4 * 4 * sizeof(GLfloat));
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

void bbMatrixProjection (GLfloat verticalFov, GLfloat aspect,
						 GLfloat nearc, GLfloat farc, GLfloat* result)
{
	GLfloat f;
	GLfloat invAspect;
	GLfloat nf1;
	GLfloat nf2;

	BB_ASSERT(result);

	f = verticalFov * BB_PI / 360.0f;
	f = cosf(f) / sinf(f);

	invAspect = 1.0f / aspect;

	nf1 = 1.0f / (nearc - farc);
	nf2 = nearc + farc;

	result[0 + 0 * 4] = f * invAspect;
	result[1 + 0 * 4] = 0.0f;
	result[2 + 0 * 4] = 0.0f;
	result[3 + 0 * 4] = 0.0f;
	result[0 + 1 * 4] = 0.0f;
	result[1 + 1 * 4] = f;
	result[2 + 1 * 4] = 0.0f;
	result[3 + 1 * 4] = 0.0f;
	result[0 + 2 * 4] = 0.0f;
	result[1 + 2 * 4] = 0.0f;
	result[2 + 2 * 4] = nf1 * nf2;
	result[3 + 2 * 4] = -1.0f;
	result[0 + 3 * 4] = 0.0f;
	result[1 + 3 * 4] = 0.0f;
	result[2 + 3 * 4] = 2.0f * nearc * farc * nf1;
	result[3 + 3 * 4] = 0.0f;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

void bbMatrixView (const GLfloat* eye, const GLfloat* lookAt, GLfloat roll, GLfloat* result)
{
	GLfloat dir[3];
	GLfloat rup[3];
	GLfloat right[3];
	GLfloat up[3];

	BB_ASSERT(eye && lookAt && result);

	dir[0] = lookAt[0] - eye[0];
	dir[1] = lookAt[1] - eye[1];
	dir[2] = lookAt[2] - eye[2];
	bbVec3Normalize(dir);

	rup[0] = sinf(roll);
	rup[1] = cosf(roll);
	rup[2] = 0.0f;

	bbVec3CrossProduct(rup, dir, right);
	bbVec3Normalize(right);

	bbVec3CrossProduct(dir, right, up);
	bbVec3Normalize(up);

	bbMatrixIdentity(result);

	dir[0]=-dir[0];
	dir[1]=-dir[1];
	dir[2]=-dir[2];

	result[0 + 0 * 4] = right[0];
	result[0 + 1 * 4] = right[1];
	result[0 + 2 * 4] = right[2];

	result[1 + 0 * 4] = up[0];
	result[1 + 1 * 4] = up[1];
	result[1 + 2 * 4] = up[2];

	result[2 + 0 * 4] = dir[0];
	result[2 + 1 * 4] = dir[1];
	result[2 + 2 * 4] = dir[2];

	/* Translation */

/*	memcpy(tt,result,4*4*4);

	bbMatrixTranslation(-eye[0],-eye[1],-eye[2],eyet);
	bbMatrixMultiply(result,eyet,result);

*/	
	result[0 + 3 * 4] = (-eye[0] * right[0]	+ -eye[1] * right[1]	+ -eye[2] * right[2]);
	result[1 + 3 * 4] = (-eye[0] * up   [0] + -eye[1] * up   [1]	+ -eye[2] * up   [2]);
	result[2 + 3 * 4] = (-eye[0] * dir  [0]	+ -eye[1] * dir  [1]	+ -eye[2] * dir  [2]);

}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

void bbMatrixGetTranslation (const GLfloat* matrix, GLfloat* trans)
{
	BB_ASSERT(matrix && trans);
	trans[0] = matrix[0 + 3 * 4];
	trans[1] = matrix[1 + 3 * 4];
	trans[2] = matrix[2 + 3 * 4];
}
