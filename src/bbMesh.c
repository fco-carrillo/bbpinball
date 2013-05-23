/*--------------------------------------------------------------------------*/
/*	bbMesh.c                                                                */
/*	Author:			Aarni Gratseff / Rocket                                 */
/*	Created:		17/01/2006                                              */
/*	Description:	Mesh class                                              */
/*--------------------------------------------------------------------------*/

#include "bbMesh.h"
#include "bbMaterial.h"
#include "bbPinballDefines.h"

#if defined(__VERSATILE__)
#include "bb_misc.h"
#endif

#if defined(__ARM__)
#include <string.h>
#endif
#include <stdio.h>
#include <stdlib.h>

/* fix warnings about ISO standard */
#if !defined(__ARM__) && !defined(strdup)
#define strdup _strdup
#endif

/*--------------------------------------------------------------------------*/
/* SubMesh class                                                            */
/*--------------------------------------------------------------------------*/

typedef struct BBSubMesh_s BBSubMesh;

struct BBSubMesh_s
{
	GLuint		start;
	GLuint		count;
	BBMaterial*	material;
	BBSubMesh*	next;
};

static BBSubMesh*		BBSubMesh_create		(void);
static void				BBSubMesh_destroy		(BBSubMesh* subMesh);

/*--------------------------------------------------------------------------*/
/* Mesh struct                                                              */
/*--------------------------------------------------------------------------*/

struct BBMesh_s
{
	GLuint	vertexBuffer;
	GLuint	indexBuffer;

	char*	name;

	void*	vertices;
	GLsizei	stride;
	void*	positionPtr;
	void*	normalPtr;
	void*	tex1Ptr;
	void*	tex2Ptr;

	GLuint*	indices;
	GLuint	indexCount;

	BBSubMesh* firstSubMesh;

	BBMaterialStore* matStore;
};

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

BBSubMesh* BBSubMesh_create ()
{
	BBSubMesh* subMesh = malloc(sizeof(BBSubMesh));
	if (!subMesh)
		return NULL;

	subMesh->start = 0;
	subMesh->count = 0;
	subMesh->material = NULL;
	subMesh->next = NULL;

	return subMesh;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

void BBSubMesh_destroy (BBSubMesh* subMesh)
{
	BB_ASSERT(subMesh);
	free(subMesh);
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

BBMesh* BBMesh_create (BBMaterialStore* matStore, const char* name, int useVBO, int useIBO)
{
	BBMesh* mesh;
	BB_ASSERT(matStore);

	mesh = malloc(sizeof(BBMesh));
	if (!mesh)
		return NULL;

	if ( useVBO )
	{
		glGenBuffers(1, &mesh->vertexBuffer);
		if (!mesh->vertexBuffer)
		{
			BBMesh_destroy(mesh);
			return NULL;
		}
	}
	else
	{
		mesh->vertexBuffer = 0;
	}

	if ( useIBO )
	{
		glGenBuffers(1, &mesh->indexBuffer);
		if (!mesh->indexBuffer)
		{
			BBMesh_destroy(mesh);
			return NULL;
		}
	}
	else
	{
		mesh->indexBuffer = 0;
	}

	mesh->name			= NULL;

	mesh->vertices		= NULL;
	mesh->stride		= 0;
	mesh->positionPtr	= NULL;
	mesh->normalPtr		= NULL;
	mesh->tex1Ptr		= NULL;
	mesh->tex2Ptr		= NULL;

	mesh->indices		= NULL;
	mesh->indexCount	= 0;

	mesh->firstSubMesh	= NULL;

	mesh->matStore		= matStore;

	mesh->name = strdup(name);
	if (!mesh->name)
	{
		BBMesh_destroy(mesh);
		return NULL;
	}

	return mesh;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

void BBMesh_destroy (BBMesh* mesh)
{
	BB_ASSERT(mesh);

	{
		BBSubMesh* current = mesh->firstSubMesh;
		while (current)
		{
			BBSubMesh* next = current->next;
			BBSubMesh_destroy(current);
			current = next;
		}
	}

	free(mesh->name);
	free(mesh->vertices);
	free(mesh->indices);

	if ( mesh->vertexBuffer )
	{
		glDeleteBuffers(1, &mesh->vertexBuffer);
	}

	if ( mesh->indexBuffer )
	{
		glDeleteBuffers(1, &mesh->indexBuffer);
	}

	free(mesh);
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

GLboolean BBMesh_loadVertices (BBMesh* mesh, const void* vertices, GLsizei bytes, GLuint decl)
{
	BB_ASSERT(mesh && vertices);

	if ( mesh->vertexBuffer )
	{
		glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, bytes, vertices, GL_STATIC_DRAW);
	}

	mesh->vertices = malloc(bytes);
	if (!mesh->vertices)
		return GL_FALSE;

	memcpy(mesh->vertices, vertices, bytes);

	mesh->stride = 0;
	if (decl & BB_VERTEX_DECLARATION_POSITION)
	{
		mesh->positionPtr = mesh->vertices;
		mesh->stride += 3 * 4;
	}
	if (decl & BB_VERTEX_DECLARATION_NORMAL)
	{
		mesh->normalPtr = ((GLbyte*)mesh->vertices) + mesh->stride;
		mesh->stride += 3 * 4;
	}
	if (decl & BB_VERTEX_DECLARATION_TEX1)
	{
		mesh->tex1Ptr = ((GLbyte*)mesh->vertices) + mesh->stride;
		mesh->stride += 2 * 4;
	}
	if (decl & BB_VERTEX_DECLARATION_TEX2)
	{
		mesh->tex2Ptr = ((GLbyte*)mesh->vertices) + mesh->stride;
		mesh->stride += 2 * 4;
	}

	return GL_TRUE;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

GLboolean BBMesh_loadIndices (BBMesh* mesh, const GLuint* indices, GLuint count)
{
	int bytes = sizeof(GLuint) * count;

	BB_ASSERT(mesh && indices);

	if ( mesh->indexBuffer )
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, bytes, indices, GL_STATIC_DRAW);
	}

	mesh->indices = malloc(bytes);
	if (!mesh->indices)
		return GL_FALSE;

	memcpy(mesh->indices, indices, bytes);

	mesh->indexCount = count;

	return GL_TRUE;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

GLboolean BBMesh_addSubMesh (BBMesh* mesh, GLuint start, GLuint count, const char* materialName)
{
	BB_ASSERT(mesh && materialName && mesh->matStore);

	{
		BBSubMesh* subMesh = BBSubMesh_create();
		if (!subMesh)
			return GL_FALSE;

		subMesh->start = start;
		subMesh->count = count;
		subMesh->material = BBMaterialStore_getMaterial(mesh->matStore, materialName);
		if (!subMesh->material)
		{
#ifdef BB_DEVEL
			printf("Material not found: \"%s\"", materialName);
#endif

			subMesh->material = BBMaterialStore_getMaterial(mesh->matStore, "default");
			if (!subMesh->material)
			{
				BBSubMesh_destroy(subMesh);
#ifdef BB_DEVEL
				printf("\n");
#endif
				return GL_FALSE;
			}
			else
			{
#ifdef BB_DEVEL
				printf(" (using \"default\")");
#endif
			}

#ifdef BB_DEVEL
			printf("\n");
#endif
		}

		subMesh->next = mesh->firstSubMesh;
		mesh->firstSubMesh = subMesh;
	}
	return GL_TRUE;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

#if defined(__ARM__)
typedef unsigned int void_size_uint;
#else
typedef BBuint64 void_size_uint; 
#endif

extern int g_MeshNum;

GLboolean BBMesh_draw (BBMesh* mesh, UniformSetupFunc uniformSetup, void* userPtr, GLboolean drawTransparents, GLboolean forceAdditive)
{
	if (!mesh)
		return GL_TRUE;

//	if( g_MeshNum++ != 115 )
//		return GL_TRUE;

	BB_ASSERT(mesh);
	BB_ASSERT(mesh->vertices && mesh->indices);

	if ( mesh->vertexBuffer )
	{
		/* bind vbo */
		glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexBuffer);

		if (mesh->positionPtr)
		{
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, 0, mesh->stride, (void*)((void_size_uint)mesh->positionPtr - (void_size_uint)mesh->vertices));
		}

		if (mesh->normalPtr)
		{
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, 0, mesh->stride, (void*)((void_size_uint)mesh->normalPtr - (void_size_uint)mesh->vertices));
		}

		if (mesh->tex1Ptr)
		{
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 2, GL_FLOAT, 0, mesh->stride, (void*)((void_size_uint)mesh->tex1Ptr - (void_size_uint)mesh->vertices));
		}

		if (mesh->tex2Ptr)
		{
			glEnableVertexAttribArray(3);
			glVertexAttribPointer(3, 2, GL_FLOAT, 0, mesh->stride, (void*)((void_size_uint)mesh->tex2Ptr - (void_size_uint)mesh->vertices));
		}
	}
	else
	{
		if (mesh->positionPtr)
		{
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, 0, mesh->stride, mesh->positionPtr);
		}

		if (mesh->normalPtr)
		{
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, 0, mesh->stride, mesh->normalPtr);
		}

		if (mesh->tex1Ptr)
		{
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 2, GL_FLOAT, 0, mesh->stride, mesh->tex1Ptr);
		}

		if (mesh->tex2Ptr)
		{
			glEnableVertexAttribArray(3);
			glVertexAttribPointer(3, 2, GL_FLOAT, 0, mesh->stride, mesh->tex2Ptr);
		}
	}

	if ( mesh->indexBuffer )
	{
		/* bind indexbuffer object */
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBuffer);
	}

	{
		BBSubMesh* subMesh = mesh->firstSubMesh;
		GLuint* index_base = mesh->indexBuffer ? 0 : mesh->indices;

		while (subMesh)
		{
			GLuint program = 0;

			if (drawTransparents != BBMaterial_isTransparent(subMesh->material))
			{
				subMesh = subMesh->next;
				continue;
			}

			program = BBMaterial_activate(subMesh->material, forceAdditive);

			uniformSetup(userPtr, program, &subMesh->material->uniformSetupCache);
			glDrawElements(GL_TRIANGLES, subMesh->count, GL_UNSIGNED_INT, index_base + subMesh->start);
//glDrawElements(GL_LINES, subMesh->count, GL_UNSIGNED_INT, index_base + subMesh->start);
			BBMaterial_deactivate(subMesh->material, forceAdditive);

			subMesh = subMesh->next;
		}
	}

	if (mesh->positionPtr)
		glDisableVertexAttribArray(0);

	if (mesh->normalPtr)
		glDisableVertexAttribArray(1);

	if (mesh->tex1Ptr)
		glDisableVertexAttribArray(2);

	if (mesh->tex2Ptr)
		glDisableVertexAttribArray(3);

	if ( mesh->vertexBuffer )
	{
		/* unbind vbo */
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	if ( mesh->indexBuffer )
	{
		/* unbind indexbuffer object */
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	return GL_TRUE;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

const char* BBMesh_getName (BBMesh* mesh)
{
	BB_ASSERT(mesh);
	return mesh->name;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

BBMesh* bbCreateRect (GLfloat x, GLfloat y, GLfloat width, GLfloat height,
					 GLfloat u1, GLfloat v1, GLfloat u2, GLfloat v2,
					 const char* matName, BBMaterialStore* matStore, int useVBO, int useIBO)
{
	BBMesh* mesh;
	GLfloat verts[20];
	GLuint inds[6];
	GLfloat z = 0.0f;
	BB_ASSERT(matStore);

	mesh = BBMesh_create(matStore, "rect", useVBO, useIBO);
	if (!mesh)
		return NULL;

	verts[0] = x;
	verts[1] = y;
	verts[2] = z;
	verts[3] = u1;
	verts[4] = v1;

	verts[5] = x;
	verts[6] = y + height;
	verts[7] = z;
	verts[8] = u1;
	verts[9] = v2;

	verts[10] = x + width;
	verts[11] = y + height;
	verts[12] = z;
	verts[13] = u2;
	verts[14] = v2;

	verts[15] = x + width;
	verts[16] = y;
	verts[17] = z;
	verts[18] = u2;
	verts[19] = v1;

	inds[0] = 0;
	inds[1] = 2;
	inds[2] = 1;
	inds[3] = 0;
	inds[4] = 3;
	inds[5] = 2;

	if (!BBMesh_loadVertices(mesh, verts, sizeof(GLfloat) * 20, BB_VERTEX_DECLARATION_POSITION | BB_VERTEX_DECLARATION_TEX1))
	{
		BBMesh_destroy(mesh);
		return NULL;
	}

	if (!BBMesh_loadIndices(mesh, inds, 6))
	{
		BBMesh_destroy(mesh);
		return NULL;
	}

	BBMesh_addSubMesh(mesh, 0, 6, matName);

	return mesh;
}
