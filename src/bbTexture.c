/*--------------------------------------------------------------------------*/
/*	bbTexture.c                                                             */
/*	Author:			Aarni Gratseff / Rocket                                 */
/*	Created:		24/01/2006                                              */
/*	Description:	Texture classes                                         */
/*--------------------------------------------------------------------------*/

#include "bbTexture.h"
#include "bbTga.h"

#include <stdio.h>
#include <stdlib.h>

#include <stdlib.h>
#if defined(__VERSATILE__)
#include "bb_misc.h"
#endif

#if defined(__ARM__)
#include <string.h>
#endif

/* fix warnings about ISO standard */
#if !defined(__ARM__) && !defined(strdup)
#define strdup _strdup
#endif

#define MAX_MIPMAPS 16

/*--------------------------------------------------------------------------*/
/* Texture struct                                                           */
/*--------------------------------------------------------------------------*/

struct BBTexture_s
{
	char*		name;
	GLsizei		width;
	GLsizei		height;
	GLenum		type;
	GLuint		handle;

	BBTexture*	next;
};

/*--------------------------------------------------------------------------*/
/* Texture store struct                                                     */
/*--------------------------------------------------------------------------*/

struct BBTextureStore_s
{
	BBTexture*	firstTexture;
};

/*--------------------------------------------------------------------------*/
/* Texture store private methods                                            */
/*--------------------------------------------------------------------------*/

static BBTexture*	BBTextureStore_find	(BBTextureStore* texStore, const char* name, GLenum type);

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

static GLboolean isPow2 (GLuint value)
{
	GLuint t = 2;
	while (t != value && t < 2048)
		t <<= 1;

	if (t == value)
		return GL_TRUE;
	else
		return GL_FALSE;
}

static void generateMipMap (GLubyte* data, GLsizei width, GLsizei height,
							 GLenum channels, GLuint level, GLubyte* out)
{
	GLsizei x;
	GLsizei y;
	GLsizei sw = width >> level;
	GLsizei sh = height >> level;
	GLsizei bw = 1 << level;
	GLsizei bh = 1 << level;

	if (bw > width)
		bw = width;
	if (bh > height)
		bh = height;

	if (sw < 1)
		sw = 1;
	if (sh < 1)
		sh = 1;

	for (y = 0; y < sh; y++)
	{
		for (x = 0; x < sw; x++)
		{
			GLuint r = 0;
			GLuint g = 0;
			GLuint b = 0;
			GLuint a = 0;
			GLsizei x2;
			GLsizei y2;

			for (y2 = 0; y2 < bh; y2++)
			{
				for (x2 = 0; x2 < bw; x2++)
				{
					GLsizei fx = (x * bw) + x2;
					GLsizei fy = (y * bh) + y2;
					if (channels == GL_RGB)
					{
						GLubyte* pos = &data[(fx + fy * width) * 3];
						r += pos[0];
						g += pos[1];
						b += pos[2];
					}
					else if (channels == GL_RGBA)
					{
						GLubyte* pos = &data[(fx + fy * width) * 4];
						r += pos[0];
						g += pos[1];
						b += pos[2];
						a += pos[3];
					}
				}
			}

			r /= bw * bh;
			g /= bw * bh;
			b /= bw * bh;
			a /= bw * bh;

			if (channels == GL_RGB)
			{
				GLubyte* tgt = &out[(x + y * sw) * 3];
				tgt[0] = r;
				tgt[1] = g;
				tgt[2] = b;
			}
			else if (channels == GL_RGBA)
			{
				GLubyte* tgt = &out[(x + y * sw) * 4];
				tgt[0] = r;
				tgt[1] = g;
				tgt[2] = b;
				tgt[3] = a;
			}
		}
	}
}

BBTexture* BBTexture_create (const char* name, GLsizei width, GLsizei height,
							 GLenum type, GLenum channels, const void** data)
{
	BBTexture* texture;
	BB_ASSERT(data);

	if (!(isPow2(width) && isPow2(height)))
	{
#ifdef BB_DEVEL
		printf("Error: Invalid texture size.\n");
#endif
		return NULL;
	}

	texture = malloc(sizeof(BBTexture));
	if (!texture)
		return NULL;

	texture->name = NULL;
	texture->width = width;
	texture->height = height;
	texture->type = type;
	texture->handle = 0;
	texture->next = NULL;

	texture->name = strdup(name);
	if (!texture->name)
	{
		BBTexture_destroy(texture);
		return NULL;
	}

	switch (type)
	{
	case GL_TEXTURE_2D:
		glGenTextures(1, &texture->handle);
		glBindTexture(GL_TEXTURE_2D, texture->handle);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		{
//			GLubyte* origBitmap = (GLubyte*)data[0];
//			GLubyte* scaledBitmap = NULL;
			GLuint level = 0;
			GLuint w = texture->width;
			GLuint h = texture->height;
			GLboolean end = GL_FALSE;
			while (!end)
			{
//				if (level > 0)
//				{
//					if (!scaledBitmap)
//					{
//						GLsizei bpp = 4;
//						if (channels == GL_RGB)
//							bpp = 3;
//						scaledBitmap = (GLubyte*)malloc(w * h * bpp * sizeof(GLubyte));
//						if (!scaledBitmap)
//							break;
//					}
//					generateMipMap(origBitmap, texture->width, texture->height, channels, level, scaledBitmap);
//					glTexImage2D(GL_TEXTURE_2D, level, channels, w, h, 0, channels, GL_UNSIGNED_BYTE, scaledBitmap);
//				}
//				else
//				{
//					glTexImage2D(GL_TEXTURE_2D, level, channels, w, h, 0, channels, GL_UNSIGNED_BYTE, origBitmap);
//				}

				BB_ASSERT(level < MAX_MIPMAPS && data[level]);
				glTexImage2D(GL_TEXTURE_2D, level, channels, w, h, 0, channels, GL_UNSIGNED_BYTE, data[level]);

				w >>= 1;
				h >>= 1;
				if (w == 0 && h == 0)
				{
					end = GL_TRUE;
				}
				else
				{
					if (w < 1)
						w = 1;
					if (h < 1)
						h = 1;
					level++;
				}
			}
//			free(scaledBitmap);
		}
		break;
	case GL_TEXTURE_CUBE_MAP:
		glGenTextures(1, &texture->handle);
		glBindTexture(GL_TEXTURE_CUBE_MAP, texture->handle);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, channels, texture->width, texture->height, 0, channels, GL_UNSIGNED_BYTE, data[0]);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, channels, texture->width, texture->height, 0, channels, GL_UNSIGNED_BYTE, data[1]);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, channels, texture->width, texture->height, 0, channels, GL_UNSIGNED_BYTE, data[2]);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, channels, texture->width, texture->height, 0, channels, GL_UNSIGNED_BYTE, data[3]);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, channels, texture->width, texture->height, 0, channels, GL_UNSIGNED_BYTE, data[4]);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, channels, texture->width, texture->height, 0, channels, GL_UNSIGNED_BYTE, data[5]);
		break;
	default:
		BBTexture_destroy(texture);
		return NULL;
	}

	return texture;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

void BBTexture_destroy (BBTexture* texture)
{
	BB_ASSERT(texture);
//	printf("Deleting texture %s\n", texture->name);
	glDeleteTextures(1, &texture->handle);
	free(texture->name);
	free(texture);
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

const char* BBTexture_getName (BBTexture* texture)
{
	BB_ASSERT(texture);
	return texture->name;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

GLenum BBTexture_getType (BBTexture* texture)
{
	BB_ASSERT(texture);
	return texture->type;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

void BBTexture_activate (BBTexture* texture, GLuint channel)
{
	BB_ASSERT(texture);
	glActiveTexture(GL_TEXTURE0 + channel);
	glBindTexture(texture->type, texture->handle);
	glEnable(texture->type);
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

void BBTexture_deactivate (BBTexture* texture, GLuint channel)
{
	BB_ASSERT(texture);
	glActiveTexture(GL_TEXTURE0 + channel);
	glDisable(texture->type);
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

BBTextureStore* BBTextureStore_create ()
{
	BBTextureStore* texStore = malloc(sizeof(BBTextureStore));
	if (!texStore)
		return NULL;

	texStore->firstTexture = NULL;

	return texStore;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

void BBTextureStore_destroy (BBTextureStore* texStore)
{
	BB_ASSERT(texStore);
	{
		BBTexture* current = texStore->firstTexture;
		while (current)
		{
			BBTexture* next = current->next;
			BBTexture_destroy(current);
			current = next;
		}
	}
	free(texStore);
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

static void catenateMipMapFilename (char* filename, GLsizei level)
{
	if (level > 0)
	{
		char tempbuf[256];
		char textureName[256];

		GLsizei lastDirPos = (GLsizei)strlen(filename);
		while (lastDirPos > 0 && filename[lastDirPos] != '/')
		{
			lastDirPos--;
		}

		memcpy(tempbuf, filename, lastDirPos);
		memcpy(textureName, filename + lastDirPos, strlen(filename) - lastDirPos);
		tempbuf[lastDirPos] = '\0';
		textureName[strlen(filename) - lastDirPos] = '\0';
		sprintf(filename, "%s/mipmaps%s_level%d", tempbuf, textureName, level);
	}
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

BBTexture* BBTextureStore_load (BBTextureStore* texStore, const char* name, int startLodLevel)
{
	BBTexture* texture = NULL;
	BBTga* tga[MAX_MIPMAPS];
	char filename[256];
	GLsizei level = 0;
	GLboolean bottomOk = GL_TRUE;
	GLsizei width = 1;
	GLsizei height = 1;

	BB_ASSERT(texStore && name);

	memset(tga, 0, sizeof(BBTga*) * MAX_MIPMAPS);

	texture = BBTextureStore_find(texStore, name, GL_TEXTURE_2D);
	if (texture)
		return texture;

	if (strlen(name) > 240)
	{
#ifdef BB_DEVEL
		printf("Error: Filename too long \"%s\"\n", name);
#endif
		return NULL;
	}

	while (width >= 1 || height >= 1)
	{
		if (width < 1)
			width = 1;
		if (height < 1)
			height = 1;

		strcpy(filename, name);
		catenateMipMapFilename(filename, level + startLodLevel);
		strcat(filename, ".tga");

		tga[level] = BBTga_load(filename);
		if (!tga[level])
		{
			if (level == 0)
			{
				bottomOk = GL_FALSE;
				break;
			}
		}
		else
		{
			if (level == 0)
			{
				width = tga[0]->width;
				height = tga[0]->height;
			}
			else
			{
				if (tga[level]->width != width ||
					tga[level]->height != height ||
					tga[level]->bits != tga[0]->bits)
				{
					BBTga_destroy(tga[level]);
					tga[level] = NULL;
				}
			}
		}

		width >>= 1;
		height >>= 1;
		level++;
		BB_ASSERT(level < MAX_MIPMAPS);
	}

	if (bottomOk)
	{
		void* dataList[MAX_MIPMAPS];
		GLboolean generated[MAX_MIPMAPS];
		GLsizei i;
		GLboolean errorsInMipMaps = GL_FALSE;

		memset(dataList, 0, sizeof(void*) * MAX_MIPMAPS);
		memset(generated, 0, sizeof(GLboolean) * MAX_MIPMAPS);

		for (i = 0; i < level; i++)
		{
			if (tga[i] && tga[i]->data)
			{
				dataList[i] = tga[i]->data;
			}
			else
			{
				GLsizei bpp = tga[0]->bits == 24 ? 3 : 4;
				GLsizei width = tga[0]->width >> i;
				GLsizei height = tga[0]->height >> i;

				BB_ASSERT(i != 0 || (width < 1 && height < 1));

				if (width < 1)
					width = 1;
				if (height < 1)
					height = 1;

				dataList[i] = (GLubyte*)malloc(width * height * bpp * sizeof(GLubyte));
				if (dataList[i])
				{
					generateMipMap(tga[0]->data, tga[0]->width, tga[0]->height, tga[0]->bits == 24 ? GL_RGB : GL_RGBA, i, dataList[i]);

					{
						strcpy(filename, name);
						catenateMipMapFilename(filename, i);
						strcat(filename, ".tga");

						if (BBTga_save(filename, width, height, tga[0]->bits == 24 ? 3 : 4, dataList[i], GL_TRUE))
						{
#ifdef BB_DEVEL
							printf("Generated mipmap level %d for texture %s.\n", i, name);
#endif
						}
						else
						{
#ifdef BB_DEVEL
							printf("Failed to save mipmap level %d for texture %s.\n", i, name);
#endif
						}
					}

					generated[i] = GL_TRUE;
				}
				else
				{
					errorsInMipMaps = GL_TRUE;
					break;
				}
			}
		}

		if (!errorsInMipMaps)
			texture = BBTexture_create(name, tga[0]->width, tga[0]->height, GL_TEXTURE_2D, tga[0]->bits == 24 ? GL_RGB : GL_RGBA, (const void **)dataList);

		for (i = 0; i < level; i++)
		{
			if (generated[i])
				free(dataList[i]);
		}
	}

	while (level-- > 0)
	{
		if (tga[level])
		{
			BBTga_destroy(tga[level]);
			tga[level] = NULL;
		}
	}

	if (texture)
	{
		texture->next = texStore->firstTexture;
		texStore->firstTexture = texture;
	}

	return texture;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

BBTexture* BBTextureStore_loadCube (BBTextureStore* texStore, const char* name, int startLodLevel)
{
	BBTexture* texture;
	BBTga* tga[6];
	GLubyte* dataList[6];
	GLsizei width = 0;
	GLsizei height = 0;
	GLsizei bits = 0;
	GLuint i;
	BB_ASSERT(texStore && name);

	texture = BBTextureStore_find(texStore, name, GL_TEXTURE_CUBE_MAP);
	if (texture)
		return texture;

	if (strlen(name) > 240)
	{
#ifdef BB_DEVEL
		printf("Error: Filename too long \"%s\"\n", name);
#endif
		return NULL;
	}

	for (i = 0; i < 6; i++)
	{
		char filename[256];
		GLboolean error = GL_FALSE;

		strcpy(filename, name);

		switch (i)
		{
		case 0:
			strcat(filename, "_px");
			break;
		case 1:
			strcat(filename, "_py");
			break;
		case 2:
			strcat(filename, "_pz");
			break;
		case 3:
			strcat(filename, "_nx");
			break;
		case 4:
			strcat(filename, "_ny");
			break;
		case 5:
			strcat(filename, "_nz");
			break;
		default:
			BB_ASSERT(0);
			break;
		}

		catenateMipMapFilename(filename, startLodLevel);
		strcat(filename, ".tga");

		tga[i] = BBTga_load(filename);
		if (!tga[i])
			error = GL_TRUE;

		if (!error && width != 0 && height != 0 && bits != 0)
			if (tga[i]->width != width || tga[i]->height != height || tga[i]->bits != bits)
				error = GL_TRUE;

		if (error)
		{
			while (i--)
				BBTga_destroy(tga[i]);

			return NULL;
		}

		width = tga[i]->width;
		height = tga[i]->height;
		bits = tga[i]->bits;

		dataList[i] = tga[i]->data;
	}

	texture = BBTexture_create(name, width, height, GL_TEXTURE_CUBE_MAP, bits == 24 ? GL_RGB : GL_RGBA, (const void **)dataList);

	for (i = 0; i < 6; i++)
		BBTga_destroy(tga[i]);

	if (texture)
	{
		texture->next = texStore->firstTexture;
		texStore->firstTexture = texture;
	}

	return texture;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

BBTexture* BBTextureStore_find (BBTextureStore* texStore, const char* name, GLenum type)
{
	BBTexture* current = texStore->firstTexture;
	while (current)
	{
		if (strcmp(current->name, name) == 0 && current->type == type)
			return current;

		current = current->next;
	}
	return NULL;
}
