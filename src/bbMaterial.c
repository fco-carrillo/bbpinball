/*--------------------------------------------------------------------------*/
/*	bbMaterial.c                                                            */
/*	Author:			Aarni Gratseff / Rocket                                 */
/*	Created:		23/01/2006                                              */
/*	Description:	Material classes                                        */
/*--------------------------------------------------------------------------*/

#include "bbMaterial.h"
#include "bbParserUtils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*--------------------------------------------------------------------------*/
/* Material methods (used in store)                                         */
/*--------------------------------------------------------------------------*/

BBMaterial*			BBMaterial_create		(void);
void				BBMaterial_destroy		(BBMaterial* material);

/*--------------------------------------------------------------------------*/
/* Material store struct                                                    */
/*--------------------------------------------------------------------------*/

struct BBMaterialStore_s
{
	BBMaterial* firstMaterial;
};

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

GLuint BBMaterial_activate (BBMaterial* material, GLboolean forceAdditive)
{
	GLsizei i;
	BB_ASSERT(material && material->program);

	glUseProgram(material->program);

	for (i = 0; i < BB_MATERIAL_MAX_TEXTURES; i++)
		if (material->textures[i])
			BBTexture_activate(material->textures[i], i);
#if 1
	if (material->additive || forceAdditive)
	{
		glEnable(GL_BLEND);
//		glBlendFunc(GL_ONE, GL_ONE);
	}
	else if (material->blend)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	else if (material->tone)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_ZERO, GL_SRC_ALPHA_SATURATE);
	}
#else
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
#endif

	return material->program;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

void BBMaterial_deactivate (BBMaterial* material, GLboolean forceAdditive)
{
	GLsizei i;
	BB_ASSERT(material);

	for (i = 0; i < BB_MATERIAL_MAX_TEXTURES; i++)
		if (material->textures[i])
			BBTexture_deactivate(material->textures[i], i);

	if (material->additive || forceAdditive)
	{
		glDisable(GL_BLEND);
	}
	else if (material->blend || material->tone)
	{
		glDisable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	}
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

GLboolean BBMaterial_isTransparent (BBMaterial* material)
{
	BB_ASSERT(material);
	return material->blend || material->additive || material->tone;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

BBMaterialStore* BBMaterialStore_create ()
{
	BBMaterialStore* matStore = malloc(sizeof(BBMaterialStore));
	if (!matStore)
		return NULL;

	matStore->firstMaterial = NULL;

	return matStore;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

void BBMaterialStore_destroy (BBMaterialStore* matStore)
{
	BB_ASSERT(matStore);
	{
		BBMaterial* current = matStore->firstMaterial;
		while (current)
		{
			BBMaterial* next = current->next;
			BBMaterial_destroy(current);
			current = next;
		}
	}
	free(matStore);
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

GLboolean BBMaterialStore_load (BBMaterialStore* matStore,
								BBShaderStack* shaderStack,
								BBTextureStore* texStore,
								const char* filename, int startLodLevel)
{
	char* buffer;
	BB_ASSERT(matStore && shaderStack && texStore && filename);

	buffer = bbReadFileToBuffer(filename);
	if (!buffer)
		return GL_FALSE;

	{
		char* stream = buffer;
		GLboolean exit = GL_FALSE;
		while (!exit)
		{
			char* materialName = NULL;
			if (*stream == '"')
			{
				stream++;	/* Skip '"' */
				materialName = bbParseString(&stream, "\"");
			}

			if (materialName)
			{
				BBMaterial* material = BBMaterial_create();
				char* vertexShaderName = NULL;
				char* fragShaderName = NULL;
				char* textureName = NULL;
				char* texture2Name = NULL;
				char* texture3Name = NULL;
				char* texture4Name = NULL;
				char* cubeTextureName = NULL;
				if (!material)
				{
					free(materialName);
					break;
				}

				material->name = materialName;

				bbSkipWhite(&stream);
				if (*stream == '{')
				{
					GLboolean innerExit = GL_FALSE;
					stream++;	/* Skip '{' */
					while (!innerExit)
					{
						char* name = bbParseString(&stream, " \t\"\n\r");
						bbSkipWhite(&stream);

						if (*stream == '"')
						{
							stream++;	/* Skip '"' */

							if (!vertexShaderName && strcmp(name, "vertexShader") == 0)
								vertexShaderName = bbParseString(&stream, "\"");
							else if (!fragShaderName && strcmp(name, "fragShader") == 0)
								fragShaderName = bbParseString(&stream, "\"");
							else if (!textureName && strcmp(name, "texture") == 0)
								textureName = bbParseString(&stream, "\"");
							else if (!texture2Name && strcmp(name, "texture2") == 0)
								texture2Name = bbParseString(&stream, "\"");
							else if (!texture3Name && strcmp(name, "texture3") == 0)
								texture3Name = bbParseString(&stream, "\"");
							else if (!texture4Name && strcmp(name, "texture4") == 0)
								texture4Name = bbParseString(&stream, "\"");
							else if (!cubeTextureName && strcmp(name, "cubeTexture") == 0)
								cubeTextureName = bbParseString(&stream, "\"");
						}
						else
						{
							if (strcmp(name, "blend") == 0)
								material->blend = GL_TRUE;
							else if (strcmp(name, "additive") == 0)
								material->additive = GL_TRUE;
							else if (strcmp(name, "tone") == 0)
								material->tone = GL_TRUE;
						}

						bbSkipWhite(&stream);

						if (*stream == '}' || *stream == '\0')
						{
							if (*stream == '}')
								stream++;	/* Skip '}' */
							innerExit = GL_TRUE;
						}

						free(name);
					}
				}

				if (vertexShaderName && fragShaderName)
					material->program = BBShaderStack_getProgram(shaderStack, vertexShaderName, fragShaderName);

				if (cubeTextureName)
					material->textures[0] = BBTextureStore_loadCube(texStore, cubeTextureName, startLodLevel);
				else if (textureName)
					material->textures[0] = BBTextureStore_load(texStore, textureName, startLodLevel);
				
				if (texture2Name)
					material->textures[1] = BBTextureStore_load(texStore, texture2Name, startLodLevel);
				if (texture3Name)
					material->textures[2] = BBTextureStore_load(texStore, texture3Name, startLodLevel);
				if (texture4Name)
					material->textures[3] = BBTextureStore_load(texStore, texture4Name, startLodLevel);

				free(cubeTextureName);
				free(texture4Name);
				free(texture3Name);
				free(texture2Name);
				free(textureName);
				free(fragShaderName);
				free(vertexShaderName);

				if (!material->program)
				{
#ifdef BB_DEVEL
					printf("Errors in material: \"%s\"\n", material->name);
#endif
					BBMaterial_destroy(material);
					break;
				}

				material->next = matStore->firstMaterial;
				matStore->firstMaterial = material;

				bbSkipWhite(&stream);
			}
			else
			{
				exit = GL_TRUE;
			}
		}
	}

	free(buffer);

	return GL_TRUE;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

BBMaterial* BBMaterialStore_getMaterial (BBMaterialStore* matStore, const char* name)
{
	BBMaterial* current;
	BB_ASSERT(matStore && name);

	current = matStore->firstMaterial;
	while (current)
	{
		if (strcmp(current->name, name) == 0)
			return current;

		current = current->next;
	}

	return NULL;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

BBMaterial* BBMaterial_create ()
{
	GLsizei i;
	BBMaterial* material = malloc(sizeof(BBMaterial));
	if (!material)
		return NULL;

	material->name = NULL;
	material->program = 0;
	material->uniformSetupCache.valid = 0;
    material->uniformSetupCache.currentTime = -1000;
    material->uniformSetupCache.ledNumber = -1000;


	for (i = 0; i < BB_MATERIAL_MAX_TEXTURES; i++)
		material->textures[i] = NULL;

	material->blend = GL_FALSE;
	material->additive = GL_FALSE;
	material->tone = GL_FALSE;

	material->next = NULL;

	return material;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

void BBMaterial_destroy (BBMaterial* material)
{
	BB_ASSERT(material);
	free(material->name);
	free(material);
}
