#ifndef __SURFACE_ADOPTER_H
#define __SURFACE_ADOPTER_H

#include <EGL/egl.h>
#include <ddraw.h>

typedef struct RenderingSurface_s RenderingSurface;
#ifdef __cplusplus
 extern "C" {
 #endif 

	struct RenderingSurface_s
	{
#if defined (RENDERING_SURFACE_DDRAW)
		LPDIRECTDRAW                pDD;        // DirectDraw object
		LPDIRECTDRAWSURFACE         pDDSPrimary; // Primary Surface.
		LPDIRECTDRAWSURFACE         pDDSOverlay; // The overlay primary.
#endif
		HWND hWnd;
	};

 EGLNativeDisplayType GetDisplayType ();
 unsigned int GetSurface(RenderingSurface* pRenderingSurface);
 RenderingSurface* CreateRenderingSurface (HWND hWnd, int width, int height);
 void FlipSurface(RenderingSurface* pRenderingSurface);
 void ReleaseAllSurfaces(RenderingSurface* pRenderingSurface);
 void HandleWindowMessages(HWND	hWnd, UINT	uMsg, WPARAM	wParam, LPARAM	lParam, RenderingSurface* pRenderingSurface);

#ifdef __cplusplus
 }
 #endif 
#endif //__SURFACE_ADOPTER_H