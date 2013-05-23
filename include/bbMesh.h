/*--------------------------------------------------------------------------*/
/*	bbMesh.h                                                                */
/*	Author:			Aarni Gratseff / Rocket                                 */
/*	Created:		17/01/2006                                              */
/*	Description:	Mesh class header                                       */
/*--------------------------------------------------------------------------*/

#ifndef BBMESH_H
#define BBMESH_H

#include "bbPinballDefines.h"
#include "bbMaterial.h"

/*--------------------------------------------------------------------------*/
/* Constants                                                                */
/*--------------------------------------------------------------------------*/

enum
{
	BB_VERTEX_DECLARATION_POSITION = 1,
	BB_VERTEX_DECLARATION_NORMAL = 2,
	BB_VERTEX_DECLARATION_TEX1 = 4,
	BB_VERTEX_DECLARATION_TEX2 = 8
};

/*--------------------------------------------------------------------------*/
/* Forward declarations                                                     */
/*--------------------------------------------------------------------------*/

typedef struct BBMesh_s BBMesh;

typedef void (*UniformSetupFunc)		(void* userPtr, GLuint program, BBPinballUniformSetupCache* cache);

/*--------------------------------------------------------------------------*/
/* BBMesh public methods                                                    */
/*--------------------------------------------------------------------------*/

BBMesh*			BBMesh_create			(BBMaterialStore* matStore, const char* name, int useVBO, int useIBO);
void			BBMesh_destroy			(BBMesh* mesh);

GLboolean		BBMesh_loadVertices		(BBMesh* mesh, const void* vertices, GLsizei bytes, GLuint decl);
GLboolean		BBMesh_loadIndices		(BBMesh* mesh, const GLuint* indices, GLuint count);

GLboolean		BBMesh_addSubMesh		(BBMesh* mesh, GLuint start, GLuint count, const char* materialName);

GLboolean		BBMesh_draw				(BBMesh* mesh, UniformSetupFunc uniformSetup,
										 void* userPtr, GLboolean drawTransparents,
										 GLboolean forceAdditive);

const char*		BBMesh_getName			(BBMesh* mesh);

/*--------------------------------------------------------------------------*/
/* Mesh tools                                                               */
/*--------------------------------------------------------------------------*/

BBMesh*			bbCreateRect			(GLfloat x, GLfloat y, GLfloat width, GLfloat height,
										 GLfloat u1, GLfloat v1, GLfloat u2, GLfloat v2,
										 const char* matName, BBMaterialStore* matStore, int useVBO, int useIBO);

#endif /* BBMESH_H */
