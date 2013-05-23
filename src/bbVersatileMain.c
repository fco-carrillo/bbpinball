/*-------------------------------------------------------------------------*
 *
 * (C) ATI Technologies (Finland)Oy
 * All Rights Reserved.
 *
 * This file consists of unpublished, proprietary source code of
 * ATI Technologies (Finland)Oy, and is considered Confidential Information for
 * purposes of non-disclosure agreement. Disclosure outside the terms
 * outlined in signed agreement may result in irrepairable harm to
 * ATI Technologies (Finland)Oy and legal action against the party in breach.
 * 
 * Description: ATI Technologies (Finland)OpenGl ES Sample application
 * 
 * $Author: $ 
 * $Revision: $ 
 * $DateTime: $ 
 * $Id: $ 
 * 
 *-------------------------------------------------------------------------*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bbPinballDefines.h"
#include "bbPinball.h"
#include "bbParserUtils.h"
#if defined(__VERSATILE__)
#include "bb_misc.h"
#endif
//#include "base.h"
//#include "context.h"

#ifdef PERFMONITOR
/* Performance monitor */
#define ES20_VERSION
#include "perfmonitor.h"
#include "monitors_arm11.h"
#endif

#define SAMPLE_WINDOW_WIDTH 640//320
#define SAMPLE_WINDOW_HEIGHT 480//240


BBPinball *g_pinball = NULL;

char* getGLError(GLenum error)
{
    switch ( error )
    {
        case GL_INVALID_ENUM:                       return "GL_INVALID_ENUM";
        case GL_INVALID_VALUE:                      return "GL_INVALID_VALUE";
        case GL_INVALID_OPERATION:                  return "GL_INVALID_OPERATION";
#if defined (CONTEXT_WGL)
        case GL_STACK_OVERFLOW:                     return "GL_STACK_OVERFLOW";
        case GL_STACK_UNDERFLOW:                    return "GL_STACK_UNDERFLOW";
#endif
        case GL_OUT_OF_MEMORY:                      return "GL_OUT_OF_MEMORY";
        case GL_NO_ERROR:                           return "GL_NO_ERROR";
#if defined (CONTEXT_WGL)
        case GL_INVALID_FRAMEBUFFER_OPERATION_EXT:  return "GL_INVALID_FRAMEBUFFER_OPERATION_EXT";
#else
        case GL_INVALID_FRAMEBUFFER_OPERATION:      return "GL_INVALID_FRAMEBUFFER_OPERATION";
#endif
    }
    return "GL_NO_ERROR";
}

char* getEGLError(EGLint error)
{
	switch ( error )
	{
		case EGL_SUCCESS:            	return "EGL_SUCCESS";
		case EGL_NOT_INITIALIZED:    	return "EGL_NOT_INITIALIZED";
		case EGL_BAD_ACCESS:         	return "EGL_BAD_ACCESS";
		case EGL_BAD_ALLOC:          	return "EGL_BAD_ALLOC";
		case EGL_BAD_ATTRIBUTE:      	return "EGL_BAD_ATTRIBUTE";
		case EGL_BAD_CONFIG:         	return "EGL_BAD_CONFIG";
		case EGL_BAD_CONTEXT:        	return "EGL_BAD_CONTEXT";
		case EGL_BAD_CURRENT_SURFACE:	return "EGL_BAD_CURRENT_SURFACE";
		case EGL_BAD_DISPLAY:        	return "EGL_BAD_DISPLAY";
		case EGL_BAD_MATCH:          	return "EGL_BAD_MATCH";
		case EGL_BAD_NATIVE_PIXMAP:  	return "EGL_BAD_NATIVE_PIXMAP";
		case EGL_BAD_NATIVE_WINDOW:  	return "EGL_BAD_NATIVE_WINDOW";
		case EGL_BAD_PARAMETER:      	return "EGL_BAD_PARAMETER";
		case EGL_BAD_SURFACE:        	return "EGL_BAD_SURFACE";
		case EGL_CONTEXT_LOST:       	return "EGL_CONTEXT_LOST";
	}
	return "EGL_SUCCESS";        
}

void eglClose(bglContext* context)
{
    /* Destroy all EGL resources */
    if ( context )
    {
        eglMakeCurrent(context->display, NULL, NULL, NULL);
        eglDestroyContext(context->display, context->context);
        eglDestroySurface(context->display, context->surface);
        eglTerminate(context->display);

        free(context);
    }
}

bglContext* eglOpen(Config *cfg){
    
        /* EGL Setup */

static EGLint s_pbufferAttribs[] =
    {
        EGL_WIDTH,  0,
        EGL_HEIGHT, 0,
        EGL_NONE
    };
                        
    bglContext* context = malloc(sizeof(bglContext));
    if ( context )
    {
        static EGLint s_configAttribs[] =
        {
            EGL_RED_SIZE,       5,
            EGL_GREEN_SIZE,     6,
            EGL_BLUE_SIZE,      5,
            EGL_ALPHA_SIZE,     0,
            EGL_DEPTH_SIZE,     16,
            EGL_STENCIL_SIZE,   0,
			//EGL_SAMPLE_BUFFERS, 1,
			//EGL_SAMPLES,        4,
           
            EGL_NONE
        };
        
        static const EGLint ContextAttribs[] =
        {
            EGL_CONTEXT_CLIENT_VERSION, 2,
            EGL_NONE
        };
        
        EGLint numConfigs;
        EGLint majorVersion;
        EGLint minorVersion;
        
        /*s_configAttribs[1] = cfg->rBits;
        s_configAttribs[3] = cfg->gBits;
        s_configAttribs[5] = cfg->bBits;
        s_configAttribs[7] = cfg->aBits;
        s_configAttribs[9] = cfg->depthBits;
        s_configAttribs[11] = cfg->stencilBits;*/

        s_pbufferAttribs[1]= cfg->winW;
        s_pbufferAttribs[3]= cfg->winH;

        context->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        EGLERR();

        eglBindAPI(EGL_OPENGL_ES_API);

        eglInitialize(context->display, &majorVersion, &minorVersion);
        EGLERR();
        eglGetConfigs(context->display, NULL, 0, &numConfigs);
        EGLERR();
        eglChooseConfig(context->display, s_configAttribs, &context->config, 1, &numConfigs);
        EGLERR();
        
        context->context = eglCreateContext(context->display, context->config, NULL, ContextAttribs);
        EGLERR();
        context->surface = eglCreatePbufferSurface(context->display, context->config, s_pbufferAttribs);
        EGLERR();

        eglMakeCurrent(context->display, context->surface, context->surface, context->context);
        EGLERR();
        GLERR();
    }
        
    return context;
}

#if defined(__SYMBIAN32__)

extern int getCurrentTime(void);

#else

static GLint getCurrentTime()
{
#if defined(PERFMONITOR)
    static int tickcount = 0; 
    tickcount += 30;
    return tickcount;
#else
    return (GLint)GetTickCount();
#endif
}

#endif

/*-------------------------------------------------------------------*//*!
 * \brief       Main procedure.
 *//*-------------------------------------------------------------------*/

#if defined(__VERSATILE__)
int main(int argc, char* argv[])
{
#else
int SymbianMain()
{
    int argc = 0;
    char** argv = NULL;
#endif
    Config cfg = {0};
    bglContext* context;
#ifdef PERFMONITOR
    FILE* perf = fopen("pinball_perf.pmo","wb");

    if(perf == NULL)
    {
        printf("ERROR: couldn't open perf monitor file");
//        si_fail("Failed to open perfomance monitor file");
    }
#endif
    
	cfg.rBits = 5;
	cfg.gBits = 6;
	cfg.bBits = 5;
	cfg.aBits = 0;
	cfg.bitsPerPixel = 16;
	cfg.depthBits = 16;
	cfg.stencilBits = 0;
	cfg.sampleBuffers = 0;
	cfg.samples = 0;
	cfg.winW = SAMPLE_WINDOW_WIDTH;
	cfg.winH = SAMPLE_WINDOW_HEIGHT;
	
    context = eglOpen(&cfg);

    /* Create pinball class */
    {
        BBPinballCreateParams params;
        memset(&params, 0, sizeof(params));

        params.getTime          = getCurrentTime;

        params.display          = context->display;
        params.config           = context->config;
        params.mainContext      = context->context;
        params.mainSurface      = context->surface;

        params.surfaceWidth     = SAMPLE_WINDOW_WIDTH;
        params.surfaceHeight    = SAMPLE_WINDOW_HEIGHT;

		bbParseParameters(&params,argc,argv);

        g_pinball = BBPinball_create(&params);
        if (!g_pinball)
        {
            return 0;
        }
    }

#ifdef PERFMONITOR
    pmRegisterMonitor(&cycles_arm11); /* add a core cycle counter */
    pmRegisterMonitor(&branchmiss_arm11); /* add a branch prediction miss counter */
    pmRegisterMonitor(&iexec_arm11); /* add a instructions executed counter */
    pmInitialize(perf, "Pinball demo");
    pmStart();
    
#define CALIB() \
    pmProbe(ENTRY_PROBE_ID(calibration)); \
    pmProbe(EXIT_PROBE_ID(calibration));

    CALIB();
    CALIB();
    CALIB();
    CALIB();
    CALIB();
    CALIB();
#endif

    for(;;)
    {
#ifdef PERFMONITOR
        pmProbe(ENTRY_PROBE_ID(mainProbe));
#endif
		if (BBPinball_main(g_pinball))
			break;
#ifdef PERFMONITOR
        pmProbe(EXIT_PROBE_ID(mainProbe));
#endif
    }
    
#ifdef PERFMONITOR
    pmStop();
    pmFinalize();
#endif

    return 0;
}

void bglSwapBuffer(bglContext* context)
{
    if ( context )
    {
        eglSwapBuffers(context->display, context->surface);
    }
}

void bglClearDepth(float depth)
{
    glClearDepthf(depth);
}

char* strdup(const char *s)
{
    char *result = (char*)malloc(strlen(s) + 1);
    if (result == (char*)0x0)
	return (char*)0x0;
    strcpy(result, s);
    return result;
}
