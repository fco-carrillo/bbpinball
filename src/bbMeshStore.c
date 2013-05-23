/*--------------------------------------------------------------------------*/
/*	bbMeshStore.c                                                           */
/*	Author:			Aarni Gratseff / Rocket                                 */
/*	Created:		19/01/2006                                              */
/*	Description:	Mesh store class                                        */
/*--------------------------------------------------------------------------*/

#include "bbMeshStore.h"

#include <stdio.h>
#include <stdlib.h>
#ifdef __ARM__
#include <string.h>
#endif

/*--------------------------------------------------------------------------*/
/* Mesh list struct                                                         */
/*--------------------------------------------------------------------------*/

typedef struct BBMeshList_s BBMeshList;

struct BBMeshList_s
{
	BBMesh*		mesh;
	BBMeshList*	next;
};

static BBMeshList*		BBMeshList_create		(BBMesh* mesh);
static void				BBMeshList_destroy		(BBMeshList* meshList);

/*--------------------------------------------------------------------------*/
/* Mesh store struct                                                        */
/*--------------------------------------------------------------------------*/

struct BBMeshStore_s
{
	BBMeshList*			firstMesh;
	BBMeshList*			current;

	BBMaterialStore*	matStore;
};

/*--------------------------------------------------------------------------*/
/* Mesh store private methods                                               */
/*--------------------------------------------------------------------------*/

static BBMesh*	BBMeshStore_loadMesh	(BBMeshStore* store, FILE* f, int useVBO, int useIBO);

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

BBMeshStore* BBMeshStore_create (BBMaterialStore* matStore)
{
	BBMeshStore* store;
	BB_ASSERT(matStore);

	store = malloc(sizeof(BBMeshStore));
	if (!store)
		return NULL;

	store->firstMesh	= NULL;
	store->current		= NULL;

	store->matStore		= matStore;

	return store;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

void BBMeshStore_destroy (BBMeshStore* store)
{
	BB_ASSERT(store);

	{
		BBMeshList* current = store->firstMesh;
		while (current)
		{
			BBMeshList* next = current->next;
			BBMeshList_destroy(current);
			current = next;
		}
	}

	free(store);
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

GLboolean BBMeshStore_load (BBMeshStore* store, const char* filename, int useVBO, int useIBO)
{
	BBMesh* mesh;
	FILE* f;
	GLboolean end = GL_FALSE;
	BB_ASSERT(store && filename && store->matStore);

	f = BB_FOPEN(filename, "rb");
	if (!f)
		return GL_FALSE;

	while (!end)
	{
		mesh = BBMeshStore_loadMesh(store, f, useVBO, useIBO);
		if (!mesh)
		{
			fclose(f);
			return GL_FALSE;
		}

		{
			BBMeshList* meshList = BBMeshList_create(mesh);
			if (!meshList)
			{
				fclose(f);
				return GL_FALSE;
			}
			meshList->next = store->firstMesh;
			store->firstMesh = meshList;
		}
		{
			char c;
			GLuint pos;
			pos = ftell(f);
			fread(&c, 1, 1, f);
			if (feof(f))
				end = GL_TRUE;
			fseek(f, pos, SEEK_SET);
		}
	}

	fclose(f);

	return GL_TRUE;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

GLboolean BBMeshStore_exchange (BBMeshStore* store, BBMeshStore* target, BBMesh* mesh)
{
	BB_ASSERT(store && target && mesh);

	store->current = NULL; /* Reset current */

	{
		BBMeshList* last = NULL;
		BBMeshList* current = store->firstMesh;
		while (current)
		{
			if (current->mesh == mesh)
				break;
			last = current;
			current = current->next;
		}

		if (!current)
			return GL_FALSE;

		if (last)
			last->next = current->next;
		else
			store->firstMesh = current->next;

		current->next = target->firstMesh;
		target->firstMesh = current;
	}

	return GL_TRUE;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

GLboolean BBMeshStore_exchangeByName (BBMeshStore* store, BBMeshStore* target, const char* name)
{
	BBMesh* mesh;
	BB_ASSERT(store && target && name);

	mesh = BBMeshStore_getMeshByName(store, name);
	if (!mesh)
		return GL_FALSE;

	if (!BBMeshStore_exchange(store, target, mesh))
		return GL_FALSE;

	return GL_TRUE;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

GLboolean BBMeshStore_removeFromList (BBMeshStore* store, BBMesh* mesh)
{
	BBMeshList* current;
	BBMeshList* last = NULL;
	BB_ASSERT(store && mesh);

	current = store->firstMesh;
	while (current)
	{
		if (current->mesh == mesh)
			break;

		last = current;
		current = current->next;
	}

	if (!current)
		return GL_FALSE;

	if (last)
		last->next = current->next;
	else
		store->firstMesh = current->next;

	current->mesh = NULL;			/* The mesh will not be deleted */
	BBMeshList_destroy(current);

	return GL_TRUE;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

BBMesh* BBMeshStore_getFirstMesh (BBMeshStore* store)
{
	BB_ASSERT(store);
	if (store->firstMesh)
	{
		store->current = store->firstMesh;
		return store->current->mesh;
	}
	else
	{
		return NULL;
	}
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

BBMesh* BBMeshStore_getNextMesh (BBMeshStore* store, BBMesh* current)
{
	BB_ASSERT(store && current);

	if (store->current)
	{
		if (store->current->mesh == current)
		{
			if (store->current->next)
			{
				store->current = store->current->next;
				return store->current->mesh;
			}
			else
			{
				return NULL;
			}
		}
	}

	{
		BBMeshList* iter = store->firstMesh;
		while (iter)
		{
			if (iter->mesh == current)
			{
				if (iter->next)
				{
					store->current = iter->next;
					return store->current->mesh;
				}
				else
				{
					return NULL;
				}
			}
		}
	}

	return NULL;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

BBMesh* BBMeshStore_getMeshByName (BBMeshStore* store, const char* name)
{
	BBMeshList* current;
	BB_ASSERT(store && name);

	current = store->firstMesh;
	while (current)
	{
		if (strcmp(BBMesh_getName(current->mesh), name) == 0)
			return current->mesh;

		current = current->next;
	}

	return NULL;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

BBMeshList* BBMeshList_create (BBMesh* mesh)
{
	BB_ASSERT(mesh);
	{
		BBMeshList* meshList = malloc(sizeof(BBMeshList));
		if (!meshList)
			return NULL;

		meshList->mesh = mesh;
		meshList->next = NULL;

		return meshList;
	}
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

void BBMeshList_destroy (BBMeshList* meshList)
{
	BB_ASSERT(meshList);

	if (meshList->mesh)
		BBMesh_destroy(meshList->mesh);

	free(meshList);
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

char* readString (FILE* f)
{
	GLuint i = 0;
	char str[256];
	char* str2;
	char c;

	str[0] = '\0';

	fread(&c, 1, 1, f);
	while (c)
	{
		str[i] = c;
		i++;
		fread(&c, 1, 1, f);
	}

	str2 = malloc(i + 1);
	memcpy(str2, str, i);
	str2[i] = '\0';

	return str2;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

BBMesh* BBMeshStore_loadMesh (BBMeshStore* store, FILE* f, int useVBO, int useIBO)
{
	BB_ASSERT(f);
	{
		BBMesh* mesh;

		char* name;

		GLuint vertexCount;
		GLuint decl;
		GLuint vertexBytes;
		GLfloat* vertices;
		GLuint stride = 0;

		GLuint indexCount;
		GLuint* indices;

		GLuint subMeshCount;

		GLuint i;

		name = readString(f);
		mesh = BBMesh_create(store->matStore, name, useVBO, useIBO);
		free(name);

		if (!mesh)
			return NULL;

		fread(&vertexCount, 4, 1, f);
		fread(&decl, 4, 1, f);

		if (decl & 1)
			stride += 3;
		if (decl & 2)
			stride += 3;
		if (decl & 4)
			stride += 2;
		if (decl & 8)
			stride += 2;

		vertexBytes = sizeof(GLfloat) * vertexCount * stride;

		vertices = malloc(vertexBytes);
		if (!vertices)
		{
			fclose(f);
			BBMesh_destroy(mesh);
			return NULL;
		}

		fread(vertices, stride * sizeof(GLfloat), vertexCount, f);

		BBMesh_loadVertices(mesh, vertices, vertexBytes, decl);
		free(vertices);

		fread(&indexCount, 4, 1, f);
		indices = malloc(sizeof(GLuint) * indexCount);
		if (!indices)
		{
			fclose(f);
			BBMesh_destroy(mesh);
			return NULL;
		}

		fread(indices, sizeof(GLuint), indexCount, f);
		BBMesh_loadIndices(mesh, indices, indexCount);
		free(indices);

		fread(&subMeshCount, 4, 1, f);
		for (i = 0; i < subMeshCount; i++)
		{
			GLuint start;
			GLuint count;
			char* materialName;

			fread(&start, 4, 1, f);
			fread(&count, 4, 1, f);
			materialName = readString(f);
			if (!materialName)
			{
				fclose(f);
				BBMesh_destroy(mesh);
				return NULL;
			}

			if (!BBMesh_addSubMesh(mesh, start, count, materialName))
			{
				fclose(f);
				free(materialName);
				BBMesh_destroy(mesh);
				return NULL;
			}

			free(materialName);
		}

		return mesh;
	}
}
