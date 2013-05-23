/*--------------------------------------------------------------------------*/
/*	bbGLUtils.h                                                             */
/*	Author:			Aarni Gratseff / Rocket                                 */
/*	Created:		17/01/2006                                              */
/*	Description:	Various GL utilities                                    */
/*--------------------------------------------------------------------------*/

#include "bbGLUtils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

GLuint bbCompileShader (const char* filename)
{
	GLuint fileLength;
	char* code;
	GLuint obj;
	GLenum type = 0;
	FILE* f;

	BB_ASSERT(filename);

	f = BB_FOPEN(filename, "rb");
	if (!f)
		return 0;

	fseek(f, 0, SEEK_END);
	fileLength = ftell(f);

	code = malloc(sizeof(char) * (fileLength + 1));
	if (!code)
	{
		fclose(f);
		return 0;
	}

	fseek(f, 0, SEEK_SET);
	fread(code, 1, fileLength, f);
	fclose(f);
	code[fileLength]=0;

	if (filename[strlen(filename) - 1] == 'v')
		type = GL_VERTEX_SHADER;
	else if (filename[strlen(filename) - 1] == 'f')
		type = GL_FRAGMENT_SHADER;
	else
		BB_BREAKPOINT;

	obj = glCreateShader(type);
	if (!obj)
	{
		free(code);
		return 0;
	}

#ifdef USE_OPENGLES2
	{
		const char *shaderStrings[3];
		shaderStrings[0] = type == GL_FRAGMENT_SHADER ? "precision mediump float;\n" : "";
#if 0
		shaderStrings[1] = type == GL_FRAGMENT_SHADER ? "#define FALSE_COLOR\n" : "";
#else
		shaderStrings[1] = type == GL_FRAGMENT_SHADER ? "" : "";
#endif
		shaderStrings[2] = code;
	
		glShaderSource(obj, 3, shaderStrings, NULL);
	}
#else
	glShaderSource(obj, 1, &code, NULL);
#endif
	glCompileShader(obj);
	free(code);

#ifdef BB_DEVEL
	{
		/* Print compilation log to console */
		GLint loglen;
		glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &loglen);
		if (loglen > 1)
		{
			char* infoLog = (char*)malloc(loglen);
			if (infoLog)
			{
				GLint charsWritten;
				glGetShaderInfoLog(obj, loglen, &charsWritten, infoLog);
				printf("[%s]: %s\n", filename, infoLog);
				free(infoLog);
			}
		}
	}
#endif

	{
		GLint compiled;
		glGetShaderiv(obj, GL_COMPILE_STATUS, &compiled);
		if (!compiled)
		{
			glDeleteShader(obj);
			return 0;
		}
	}
	
	return obj;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

GLuint bbLinkProgram (GLuint vertexShader, GLuint fragShader)
{
	GLuint program;
    GLint  link_status = 0;
	BB_ASSERT(vertexShader && fragShader);

	program = glCreateProgram();
	if (!program)
		return 0;

	glAttachShader(program, vertexShader);
	glAttachShader(program, fragShader);

	glBindAttribLocation(program, 0, "position");
	glBindAttribLocation(program, 1, "normal");
	glBindAttribLocation(program, 2, "tex1");
	glBindAttribLocation(program, 3, "tex2");

	glLinkProgram(program);

/*	GLint linked;*/
	glGetProgramiv(program, GL_LINK_STATUS, &link_status);
	if(link_status == GL_FALSE)
	{
      printf("program linking failed.\n");		
	}

#ifdef BB_DEVEL
	{
		/* Print link log to console */
		GLint infologLength;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infologLength);
		if (infologLength > 1)
		{
			char *infoLog = (char*)malloc(infologLength);
			if (infoLog)
			{
				GLint charsWritten;
				glGetProgramInfoLog(program, infologLength, &charsWritten, infoLog);
				printf("Shader program [0x%x/0x%x]:\n%s\n\n", vertexShader, fragShader, infoLog);
				free(infoLog);
			}
		}
		else
		{
			printf("Shader program [0x%x/0x%x]:\n%s\n\n", vertexShader, fragShader, "No infolog available!");
		}
	}
#endif

	return program;
}
