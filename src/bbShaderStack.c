/*--------------------------------------------------------------------------*/
/*	bbShaderStack.c                                                         */
/*	Author:			Aarni Gratseff / Rocket                                 */
/*	Created:		18/01/2006                                              */
/*	Description:	Shader stack class                                      */
/*--------------------------------------------------------------------------*/

#include "bbShaderStack.h"
#include "bbGLUtils.h"

#include <stdio.h>
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

//#define EXPORT_SUPERPROGRAM
//#define IMPORT_SUPERPROGRAM

/*--------------------------------------------------------------------------*/
/* SuperProgram extension                                                   */
/*--------------------------------------------------------------------------*/

#ifdef EXPORT_SUPERPROGRAM
GL_APICALL void GL_APIENTRY glGetSuperProgram (GLuint program, GLvoid* binary, GLsizei* size);
#endif

#ifdef IMPORT_SUPERPROGRAM
GL_APICALL void GL_APIENTRY glLinkSuperProgram (GLuint program, GLvoid* binary, GLsizei size);
#endif

/*--------------------------------------------------------------------------*/
/* Forward declarations                                                     */
/*--------------------------------------------------------------------------*/

typedef struct BBShaderObject_s BBShaderObject;
typedef struct BBProgramObject_s BBProgramObject;

/*--------------------------------------------------------------------------*/
/* Shader stack struct                                                      */
/*--------------------------------------------------------------------------*/

struct BBShaderStack_s
{
	BBShaderObject*		firstShader;
	BBProgramObject*	firstProgram;

	BBProgramObject*	currentProgram;
};

/*--------------------------------------------------------------------------*/
/* Shader stack private methods                                             */
/*--------------------------------------------------------------------------*/

static BBShaderObject*		BBShaderStack_findShader	(BBShaderStack* stack,
														 const char* name);

static BBProgramObject*		BBShaderStack_findProgram	(BBShaderStack* stack,
														 const char* vertexShader,
														 const char* fragShader);

static void					BBShaderStack_addShader		(BBShaderStack* stack,
														 BBShaderObject* shaderObj);

static void					BBShaderStack_addProgram	(BBShaderStack* stack,
														 BBProgramObject* programObj);

/*--------------------------------------------------------------------------*/
/* Shader object class                                                      */
/*--------------------------------------------------------------------------*/

struct BBShaderObject_s
{
	char*			name;
	GLuint			shader;
	BBShaderObject*	next;
};

static BBShaderObject*		BBShaderObject_create	(const char* filename);
static void					BBShaderObject_destroy	(BBShaderObject* obj);

/*--------------------------------------------------------------------------*/
/* Shader program object class                                              */
/*--------------------------------------------------------------------------*/

struct BBProgramObject_s
{
	BBShaderObject*		vertex;
	BBShaderObject*		fragment;
	GLuint				program;
	BBProgramObject*	next;
};

static BBProgramObject*		BBProgramObject_create	(BBShaderObject* vertexObj,
													 BBShaderObject* fragObj);
static void					BBProgramObject_destroy	(BBProgramObject* program);

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

BBShaderStack* BBShaderStack_create ()
{
	BBShaderStack* stack = malloc(sizeof(BBShaderStack));
	if (!stack)
		return NULL;

	stack->firstShader		= NULL;
	stack->firstProgram		= NULL;

	stack->currentProgram	= NULL;

	return stack;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

void BBShaderStack_destroy (BBShaderStack* stack)
{
	BB_ASSERT(stack);

	{
		BBProgramObject* current = stack->firstProgram;
		while (current)
		{
			BBProgramObject* next = current->next;
			BBProgramObject_destroy(current);
			current = next;
		}
	}
	{
		BBShaderObject* current = stack->firstShader;
		while (current)
		{
			BBShaderObject* next = current->next;
			BBShaderObject_destroy(current);
			current = next;
		}
	}

	free(stack);
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

GLuint BBShaderStack_getProgram (BBShaderStack* stack,
								 const char* vertexShader,
								 const char* fragShader)
{
	BB_ASSERT(stack && vertexShader && fragShader);

	/* Check current program */
	if (stack->currentProgram)
	{
		BB_ASSERT(stack->currentProgram->vertex && stack->currentProgram->vertex->name);
		BB_ASSERT(stack->currentProgram->fragment && stack->currentProgram->fragment->name);
		if (strcmp(stack->currentProgram->vertex->name, vertexShader) == 0 &&
			strcmp(stack->currentProgram->fragment->name, fragShader) == 0)
			return stack->currentProgram->program;
	}

	{
		/* Find program (create new program if not found) */
		BBProgramObject* prog = BBShaderStack_findProgram(stack, vertexShader, fragShader);
		if (!prog)
		{
			BBShaderObject* vertexObj = BBShaderStack_findShader(stack, vertexShader);
			BBShaderObject* fragObj = BBShaderStack_findShader(stack, fragShader);
			if (!vertexObj)
			{
				vertexObj = BBShaderObject_create(vertexShader);
				if (!vertexObj)
					return 0;	/* Vertex shader creation failed */
				BBShaderStack_addShader(stack, vertexObj);
			}
			if (!fragObj)
			{
				fragObj = BBShaderObject_create(fragShader);
				if (!fragObj)
					return 0;	/* Fragment shader creation failed */
				BBShaderStack_addShader(stack, fragObj);
			}
			prog = BBProgramObject_create(vertexObj, fragObj);
			if (!prog)
				return 0;	/* Shader program creation failed */
			BBShaderStack_addProgram(stack, prog);
		}

		BB_ASSERT(prog->program);
		stack->currentProgram = prog;
		return prog->program;
	}
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

BBShaderObject* BBShaderStack_findShader (BBShaderStack* stack, const char* name)
{
	BBShaderObject* current;
	BB_ASSERT(stack && name);

	current = stack->firstShader;
	while (current)
	{
		BB_ASSERT(current->name);
		if (strcmp(current->name, name) == 0)
			return current;

		current = current->next;
	}

	return NULL;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

BBProgramObject* BBShaderStack_findProgram (BBShaderStack* stack,
											 const char* vertexShader,
											 const char* fragShader)
{
	BBProgramObject* current;
	BB_ASSERT(stack && vertexShader && fragShader);

	current = stack->firstProgram;
	while (current)
	{
		BB_ASSERT(current->vertex->name);
		BB_ASSERT(current->fragment->name);
		if (strcmp(current->vertex->name, vertexShader) == 0 &&
			strcmp(current->fragment->name, fragShader) == 0)
			return current;

		current = current->next;
	}

	return NULL;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

void BBShaderStack_addShader (BBShaderStack* stack, BBShaderObject* shaderObj)
{
	BB_ASSERT(stack && shaderObj);
	shaderObj->next = stack->firstShader;
	stack->firstShader = shaderObj;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

void BBShaderStack_addProgram (BBShaderStack* stack, BBProgramObject* programObj)
{
	BB_ASSERT(stack && programObj);
	programObj->next = stack->firstProgram;
	stack->firstProgram = programObj;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

BBShaderObject* BBShaderObject_create (const char* name)
{
	BB_ASSERT(name);
	{
		BBShaderObject* obj = malloc(sizeof(BBShaderObject));
		if (!obj)
			return NULL;

        //printf("--------------------------------------------------------\n");
        //printf("  * shader object name: %s\n", name); // TODO: fixme
        //printf("--------------------------------------------------------\n");

		obj->name		= NULL;
		obj->shader		= 0;
		obj->next		= NULL;

		obj->name = strdup(name);
		if (!obj->name)
		{
			BBShaderObject_destroy(obj);
			return NULL;
		}

		obj->shader = bbCompileShader(obj->name);
#ifndef IMPORT_SUPERPROGRAM
		if (!obj->shader)
		{
			BBShaderObject_destroy(obj);
			return NULL;
		}
#endif

		return obj;
	}
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

void BBShaderObject_destroy (BBShaderObject* obj)
{
	BB_ASSERT(obj);

	if (obj->shader)
		glDeleteShader(obj->shader);

	free(obj->name);
	free(obj);
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

/* unused static function */
#if 0
static void computeProgramName(char* name, BBShaderObject* vertexObj, BBShaderObject* fragObj)
{
	char* vs = vertexObj->name + strlen(vertexObj->name);
	char* fs = fragObj->name + strlen(fragObj->name);

    vs[-2] = 0;
    fs[-2] = 0;

    for ( ; vs[-1] != '/'; --vs )
        ;

    for ( ; fs[-1] != '/'; --fs )
        ;

    strcpy(name, vs);
    strcat(name, "_");
    strcat(name, fs);
    strcat(name, ".sp");
}
#endif

#ifdef IMPORT_SUPERPROGRAM

GLuint bbLinkSuperProgram (char* buffer, int size)
{
	GLuint program = glCreateProgram();

	glBindAttribLocation(program, 0, "position");
	glBindAttribLocation(program, 1, "normal");
	glBindAttribLocation(program, 2, "tex1");
	glBindAttribLocation(program, 3, "tex2");

	glLinkSuperProgram(program, buffer, size);

	return program;
}

#endif

BBProgramObject* BBProgramObject_create (BBShaderObject* vertexObj, BBShaderObject* fragObj)
{
	BB_ASSERT(vertexObj && fragObj);

	{
#ifdef IMPORT_SUPERPROGRAM
        FILE* file;
        char filename[1024];
#endif
#if defined(IMPORT_SUPERPROGRAM) || defined(EXPORT_SUPERPROGRAM)
        int size;
        char* buffer;
#endif

		BBProgramObject* prog = malloc(sizeof(BBProgramObject));
		if (!prog)
			return NULL;

		prog->vertex	= vertexObj;
		prog->fragment	= fragObj;
		prog->program	= 0;
		prog->next		= NULL;

#ifdef IMPORT_SUPERPROGRAM
        computeProgramName(filename, vertexObj, fragObj);

		file = BB_FOPEN(filename, "rb");
		fseek(file, 0, SEEK_END);
		size = ftell(file);
		fseek(file, 0, SEEK_SET);

		buffer = (char*)malloc(size);
		fread(buffer, 1, size, file);
		fclose(file);

		prog->program = bbLinkSuperProgram(buffer, size);
		if (!prog->program)
		{
			BBProgramObject_destroy(prog);
			return NULL;
		}

		free(buffer);
#endif

#if defined(EXPORT_SUPERPROGRAM) || !defined(IMPORT_SUPERPROGRAM)
		prog->program = bbLinkProgram(prog->vertex->shader, prog->fragment->shader);
		if (!prog->program)
		{
			BBProgramObject_destroy(prog);
			return NULL;
		}
#endif

#ifdef EXPORT_SUPERPROGRAM
		glGetSuperProgram(prog->program, NULL, &size);
        buffer = (char*)malloc(size);
        glGetSuperProgram(prog->program, buffer, NULL);

        computeProgramName(filename, vertexObj, fragObj);

		file = fopen(filename, "wb");
        fwrite(buffer, 1, size, file);
        fclose(file);

		free(buffer);
#endif

		return prog;
	}
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

void BBProgramObject_destroy (BBProgramObject* prog)
{
	BB_ASSERT(prog);

	if (prog->program)
		glDeleteProgram(prog->program);

	free(prog);
}
