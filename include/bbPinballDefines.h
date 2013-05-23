/*--------------------------------------------------------------------------*/
/*	bbDefs.h                                                                */
/*	Author:			Aarni Gratseff / Rocket                                 */
/*	Created:		16/01/2006                                              */
/*	Description:	Common definitions and macros                           */
/*--------------------------------------------------------------------------*/

#ifndef BBPINBALLDEFINES_H
#define BBPINBALLDEFINES_H

#define BB_DEVEL	/* Enables error messages */

#define BB_DISABLE_SOUNDS /* disable sounds */

#if defined(__SYMBIAN32__) || defined(_WIN32_WCE)

#include <stdio.h>
#include <string.h>

#ifdef USE_RAMFILESYS
#include "../../ramfilesys/myfilesys.h"
#endif

static FILE* BB_FOPEN(const char* filename, char* params)
{
    FILE *fh = NULL;
#ifndef USE_RAMFILESYS

#if defined(__SYMBIAN32__)
	char temp_name[128] = "/bbData/";
#else
	char temp_name1[128] = "\\NANDFlash\\bbData\\";
	char temp_name2[128] = "\\SDMemory\\bbData\\";
#endif

#endif // !USE_RAMFILESYS
	char* stripped_filename = strrchr(filename,'/');
	if (stripped_filename == NULL)
	{
	    stripped_filename = (char *)filename;
	}
	else
	{
	    stripped_filename++;
	}
	
#ifndef USE_RAMFILESYS
        strcat(temp_name1,stripped_filename);
	fh = fopen(temp_name1,params);
        if (fh)
            return(fh);

        strcat(temp_name2,stripped_filename);
	return fopen(temp_name2,params);
#else
	return fopen(stripped_filename, params);
#endif
}

#else

#define BB_FOPEN fopen

#endif

/*--------------------------------------------------------------------------*/
/* Compiler and platform identification                                     */
/*--------------------------------------------------------------------------*/

#define BB_COMPILER_MSVC		1
#define BB_COMPILER_GCC			2
#define BB_COMPILER_ARMCC       3

#define BB_PLATFORM_WIN32		1
#define BB_PLATFORM_ARM         2

#if defined(_MSC_VER)
#	define BB_COMPILER		BB_COMPILER_MSVC
#	ifdef _MSC_VER
#		if (_MSC_VER < 1300)
#			define BB_COMPILER_VER		6
#		else
#			define BB_COMPILER_VER		7
#		endif
#	endif
#endif

#if defined(GCC)
#	define BB_COMPILER		BB_COMPILER_GCC
#endif

#if defined(WIN32) || defined(_WIN32)
#	define BB_PLATFORM		BB_PLATFORM_WIN32
#elif defined(__ARM__)
#   define BB_COMPILER      BB_COMPILER_ARMCC
#   define BB_PLATFORM      BB_PLATFORM_ARM
#endif

#if defined _DEBUG
#	define BB_DEBUG
#endif

/* Unknown compiler version */

#ifndef BB_COMPILER_VER
#	define BB_COMPILER_VER		0
#endif

/* Errors */

#ifndef BB_COMPILER
#	error BB_COMPILER not defined.
#endif

#ifndef BB_PLATFORM
#	error BB_PLATFORM not defined.
#endif

/*--------------------------------------------------------------------------*/
/* Macros                                                                   */
/*--------------------------------------------------------------------------*/

#if (BB_COMPILER == BB_COMPILER_MSVC && BB_PLATFORM == BB_PLATFORM_WIN32)
#	include <assert.h>

#	define BB_INLINE			__forceinline

#	ifdef BB_DEBUG
#		define BB_ASSERT(c)		assert(c);
#ifndef	_WIN32_WCE
#		define BB_BREAKPOINT	__asm int 3
#endif
#	endif
#endif

#if BB_COMPILER == BB_COMPILER_GCC || BB_COMPILER == BB_COMPILER_ARMCC
#	define BB_INLINE			inline
#endif

#define BB_UNREF(v)				((void*)v)
#ifdef __ARM__
#define BB_NULL_STATEMENT
#else
#define BB_NULL_STATEMENT		BB_UNREF(0)
#endif

#ifndef BB_BREAKPOINT
#	define BB_BREAKPOINT		BB_NULL_STATEMENT
#endif

#ifndef BB_ASSERT
#	define BB_ASSERT(c)			BB_NULL_STATEMENT
#endif

#ifndef BB_INLINE
#	define BB_INLINE
#endif

/*--------------------------------------------------------------------------*/
/* Basic types (use GL types instead)                                       */
/*--------------------------------------------------------------------------*/
/*
typedef signed int				BBbool;

typedef signed int				BBint32;
typedef signed short			BBint16;
typedef signed char				BBint8;

typedef unsigned int			BBuint32;
typedef unsigned short			BBuint16;
typedef unsigned char			BBuint8;

typedef float					BBfloat;
typedef double					BBdouble;*/

#if (BB_COMPILER == BB_COMPILER_MSVC)
	typedef signed __int64		BBint64;
	typedef unsigned __int64	BBuint64;
#elif (BB_COMPILER == BB_COMPILER_GCC || BB_COMPILER == BB_COMPILER_ARMCC)
	typedef signed long long	BBint64;
	typedef unsigned long long	BBuint64;
#endif

/*--------------------------------------------------------------------------*/
/* Constants                                                                */
/*--------------------------------------------------------------------------*/
/*
#define	BB_TRUE					1
#define	BB_FALSE				0
*/
#define	BB_UINT32_MAX			0xffffffff
#define	BB_UINT16_MAX			0xffff
#define	BB_UINT8_MAX			0xff

#define	BB_INT32_MAX			0x7fffffff
#define	BB_INT16_MAX			0x7fff
#define	BB_INT8_MAX				0x7f

#define	BB_INT32_MIN			(-0x7fffffff - 1)
#define	BB_INT16_MIN			-0x8000
#define	BB_INT8_MIN				-0x80

#define	BB_PI					3.14159265f

/*
#define	BB_FIXED_BITS			16
#define	BB_FIXED_ONE			(1 << BB_FIXED_BITS)
#define	BB_FIXED_TWO			(2 << BB_FIXED_BITS)
#define	BB_FIXED_LOWMASK		(BB_FIXED_ONE - 1)
#define	BB_FIXED_HIGHMASK		(BB_FIXED_LOWMASK << BB_FIXED_BITS)

#define	BB_FIXED_PI				(BBuint32)(BB_PI * 65536)
*/

#ifdef USE_OPENGLES2
#include "EGL/egl.h"
#include "GLES2/gl2.h"
#include "GLES2/gl2ext.h"

typedef struct eglContext_s
{
	EGLDisplay	display;
	EGLConfig   config;
	EGLContext	context;
	EGLSurface	surface;
} bglContext;

char* getEGLError(EGLint error);
#define EGLERR() do { EGLint error = eglGetError(); if (error != EGL_SUCCESS) { printf("EGL ERROR (%s) %s %d", getEGLError(error), __FILE__, __LINE__); } } while (0)

#else
#include "../../gles2wrap/egl12.h"
#include "../../gles2wrap/gles2.h"
#endif

typedef struct 
{
    int    winW;
    int    winH;
	int    rBits;
	int    gBits;
	int    bBits;
	int    aBits;
	int    bitsPerPixel;
	int    depthBits;
	int    stencilBits;
	int    sampleBuffers;
	int    samples;
} Config;

/* Macro: insert GLERR(); to check if GL returned rerror at a certain point. */
#define GLERR() do { GLenum error = glGetError(); if (error != GL_NO_ERROR) { printf("GL ERROR (%s) %s %d", getGLError(error), __FILE__, __LINE__); } } while (0)

#endif /* BBPINBALLDEFINES_H */
