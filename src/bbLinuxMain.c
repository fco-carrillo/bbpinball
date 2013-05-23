#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <EGL/egl.h>

#include "bbPinballDefines.h"
#include "bbPinball.h"
#include "bbParserUtils.h"

static BBPinball *g_pinball = NULL;

char* getGLError(GLenum error)
{
    switch ( error )
    {
        case GL_INVALID_ENUM:                       return "GL_INVALID_ENUM";
        case GL_INVALID_VALUE:                      return "GL_INVALID_VALUE";
        case GL_INVALID_OPERATION:                  return "GL_INVALID_OPERATION";
        case GL_OUT_OF_MEMORY:                      return "GL_OUT_OF_MEMORY";
        case GL_NO_ERROR:                           return "GL_NO_ERROR";
        case GL_INVALID_FRAMEBUFFER_OPERATION:      return "GL_INVALID_FRAMEBUFFER_OPERATION";
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

bglContext *eglOpen(Config *cfg, int usePBuffer)
{
	/* EGL Setup */

    /*
	static EGLint PbufferAttribs[] =
		{
		    EGL_NONE
		};
     */

    bglContext *context = malloc(sizeof(bglContext));

    if ( context )
    {
        static EGLint ConfigAttribs[] =
        {
            EGL_RED_SIZE,       5,
            EGL_GREEN_SIZE,     6,
            EGL_BLUE_SIZE,      5,
            EGL_ALPHA_SIZE,     0,
            EGL_DEPTH_SIZE,     16,
            EGL_STENCIL_SIZE,   0,
            EGL_NONE
        };

	static EGLint ConfigAttribsPBuffer[] =
       {
           EGL_RED_SIZE,	5,
           EGL_GREEN_SIZE,	6,
           EGL_BLUE_SIZE,	5,
           EGL_ALPHA_SIZE,	0,
           EGL_DEPTH_SIZE,	16,
           EGL_STENCIL_SIZE,	0,
           EGL_SURFACE_TYPE,	EGL_PBUFFER_BIT,
           EGL_NONE
        };

        static EGLint ContextAttribs[] =
        {
            EGL_CONTEXT_CLIENT_VERSION, 2,
            EGL_NONE
        };

        static EGLint SurfaceAttribs[] =
        {
			EGL_NONE
        };

        static EGLint SurfaceAttribsPBuffer[] =
        {
            EGL_WIDTH, 480,
            EGL_HEIGHT, 640,
            EGL_NONE
        };

        EGLint numConfigs;
        EGLint majorVersion;
        EGLint minorVersion;

        //context->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        context->display = eglGetDisplay(fbGetDisplay());
        EGLERR();
        eglBindAPI(EGL_OPENGL_ES_API);

        eglInitialize(context->display, &majorVersion, &minorVersion);
        EGLERR();
        eglGetConfigs(context->display, NULL, 0, &numConfigs);
        EGLERR();
        if (usePBuffer) {
            eglChooseConfig(context->display, ConfigAttribsPBuffer, &context->config, 1, &numConfigs);
        } else {
            eglChooseConfig(context->display, ConfigAttribs, &context->config, 1, &numConfigs);
        }
        
        EGLERR();

        context->context = eglCreateContext(context->display, context->config, NULL, ContextAttribs);
        EGLERR();
        if (usePBuffer) {
            context->surface = eglCreatePbufferSurface(context->display, context->config, SurfaceAttribsPBuffer);
        } else {
			int w, h; 
			fbGetDisplayGeometry(fbGetDisplay(), &w, &h); 
            context->surface = eglCreateWindowSurface(context->display, context->config,\
			fbCreateWindow(fbGetDisplay(), 0, 0, w, h), SurfaceAttribs);
        }
        EGLERR();

        eglMakeCurrent(context->display, context->surface, context->surface, context->context);
        EGLERR();
        GLERR();
    }

    return context;
}

static GLint getCurrentTime()
{
	struct timeval tv = {0};
	struct timezone tz = {0};
	gettimeofday(&tv, &tz);
	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

int kbhit(void)
{
#ifdef AUTO_TEST
	static long count=10;
	if (count-- > 0) return 0;
	return 1;
#else
	struct timeval tv = {0};
	fd_set read_fd;
	FD_ZERO(&read_fd);
	FD_SET(0,&read_fd);
	if(select(1, &read_fd, NULL, NULL, &tv) == -1)
		return 0;
	if(FD_ISSET(0,&read_fd))
		return 1;
	return 0;
#endif
}

/*-------------------------------------------------------------------*//*!
 * \brief       Main procedure.
 *//*-------------------------------------------------------------------*/

int main(int argc, char* argv[])
{
    Config cfg = {0};
    bglContext *context = 0;
    if (argc > 1) {
        if (strcmp(argv[1],"-p")==0) {
            printf("Using pbuffer surface \n");
            context = eglOpen(&cfg,1);
        } else {
            printf("Usage:\n");
            printf("%s    Render to window buffer\n", argv[0]);
            printf("%s -p Render to pixel buffer\n", argv[0]);
            return 0;
        }
    } else {
        context = eglOpen(&cfg,0);
    }
    if (!context)
    {
    	fprintf(stderr, "eglOpen() failed\n");
    	return 0;
    }

    /* Create pinball class */
    {
        BBPinballCreateParams params;
        memset(&params, 0, sizeof(params));

        params.getTime          = getCurrentTime;

        params.display          = context->display;
        params.config           = context->config;
        params.mainContext      = context->context;
        params.mainSurface      = context->surface;

		eglQuerySurface(context->display, context->surface, EGL_WIDTH, (EGLint*) &params.surfaceWidth);
		eglQuerySurface(context->display, context->surface, EGL_HEIGHT, (EGLint*) &params.surfaceHeight);

		bbParseParameters(&params,argc,argv);

        g_pinball = BBPinball_create(&params);
        if (!g_pinball)
        {
        	fprintf(stderr, "BBPinball_create() failed\n");
            return 0;
        }
    }

    while (!kbhit())
    {
		if (BBPinball_main(g_pinball))
			break;
    }

	eglClose(context);

    return 0;
}

void BBPinball_bglSwapBuffer(EGLDisplay dpy, EGLSurface surface)
{
	eglSwapBuffers(dpy, surface);
}

void bglClearDepth(float depth)
{
    glClearDepthf(depth);
}

