/*--------------------------------------------------------------------------*/
/*	bbParserUtils.c                                                         */
/*	Author:			Aarni Gratseff / Rocket                                 */
/*	Created:		23/01/2006                                              */
/*	Description:	Parsering utilities                                     */
/*--------------------------------------------------------------------------*/

#include "bbParserUtils.h"

#include <stdio.h>
#include <stdlib.h>
#ifdef __ARM__
#include <limits.h>
#include <string.h>
#endif

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

void bbParseParameters(BBPinballCreateParams* params, int argc, char* argv[])
{
	char buffer[1024];
	int i;

	FILE* f = BB_FOPEN("./bbPinball.cfg","rt");

	int parameters = argc;
	char** args = argv;
	int free_args = 0;

	if ( f )
	{
		char* tmp = buffer;

		// read command line from file
		fgets(buffer,1024,f);
		fclose(f);

		// calculate parameter count, add one slot for binary name and for first item
		parameters = 1 + 1;
		while ( 1 )
		{
			char c = *tmp++;
			if ( c == 0 )
			{
				break;
			}

			if ( c == ' ' || c == '\t' || c == '\n' )
			{
				++ parameters;
			}
		}

		// allocate memory for parameters
		args = (char**)malloc(sizeof(char*) * parameters);
		free_args = 1;

		// setup first parameter
		i = 0;
		args[i] = "bbPinball";

		// tokenize rest
		tmp = strtok(buffer," \t\n");
		while ( i++ < parameters && tmp )
		{
			args[i] = tmp;

			tmp = strtok(NULL," \t\n");
		}
	}

	params->singleFrame = -1;
	params->maxFrames = INT_MAX;

	for ( i=1; i<parameters; ++i )
	{
		if ( strcmp(args[i],"-frame") == 0 )
		{
			if ( ++i < parameters ) params->singleFrame = atoi(args[i]);
		} else
		if ( strcmp(args[i],"-frames") == 0 )
		{
			if ( ++i < parameters ) params->maxFrames = atoi(args[i]);
		} else
		if ( strcmp(args[i],"-nohud") == 0 )
		{
			params->hideHUD = 1;
		} else
		if ( strcmp(args[i],"-dump") == 0 )
		{
			if ( ++i < parameters ) params->capturePrefix = args[i];
		} else
		if ( strcmp(args[i],"-startlod") == 0 )
		{
			if ( ++i < parameters ) params->startLodLevel = atoi(args[i]);
		} else
		if ( strcmp(args[i],"-vbo") == 0 )
		{
			params->useVBO = 1;
		} else
		if ( strcmp(args[i],"-ibo") == 0 )
		{
			params->useIBO = 1;
		}
	}

	if ( free_args )
	{
		free(args);
	}

#ifndef BB_DISABLE_SOUNDS
	params.sound = &g_sound;
#endif // !BB_DISABLE_SOUNDS
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

char* bbReadFileToBuffer (const char* filename)
{
	char* buffer;
	FILE* f;
	GLsizei size;
	BB_ASSERT(filename);

	f = BB_FOPEN(filename, "rb");
	if (!f)
		return NULL;

	fseek(f, 0, SEEK_END);
	size = ftell(f);
	fseek(f, 0, SEEK_SET);

	buffer = malloc(size + 1);
	if (!buffer)
	{
		fclose(f);
		return NULL;
	}

	fread(buffer, 1, size, f);
	buffer[size] = '\0';

	fclose(f);

	return buffer;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

void bbSkipWhite (char** stream)
{
	BB_ASSERT(stream && *stream);

	while (**stream == ' ' || **stream == '\t' || **stream == '\n' || **stream == '\r')
		(*stream)++;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

char* bbParseString (char** stream, const char* stopChars)
{
	BB_ASSERT(stream && *stream && stopChars);
	bbSkipWhite(stream);
	{
		char tmpName[256];
		char* name;
		GLuint length = 0;

		for (;;)
		{
			GLboolean exit = GL_FALSE;
			GLuint i = 0;
			while (stopChars[i] != '\0')
			{
				if (**stream == stopChars[i])
				{
					(*stream)++;
					exit = GL_TRUE;
					break;
				}
				i++;
			}
			if (exit)
				break;

			tmpName[length++] = **stream;
			if (length > 255)
			{
#ifdef BB_DEVEL
				printf("Error: Too long string.\n");
#endif
				return NULL;
			}
			if (**stream == '\0')
				break;
			(*stream)++;
		}

		name = malloc(length + 1);
		if (!name)
			return NULL;

		memcpy(name, tmpName, length);
		name[length] = '\0';

		return name;
	}
}
