#include "SurfaceAdopter.h"
#include <windows.h>
#include <ddraw.h>

#if defined (RENDERING_SURFACE_DDRAW)

static DDPIXELFORMAT ddpfOverlayFormats[] = {
  //  {sizeof(DDPIXELFORMAT), DDPF_FOURCC, MAKEFOURCC('Y','U','Y','V'),0,0,0,0,0},		// YUYV
  //  {sizeof(DDPIXELFORMAT), DDPF_FOURCC, MAKEFOURCC('U','Y','V','Y'),0,0,0,0,0},		// UYVY
  //  {sizeof(DDPIXELFORMAT), DDPF_RGB, 0, 32,  0x00FF0000, 0x0000FF00, 0x000000FF, 0}, // 32-bit RGB 5:5:5
  //    {sizeof(DDPIXELFORMAT), DDPF_RGB, 0, 16,  0x03e0, 0x7C00, 0x001F, 0},		    // 16-bit RGB 5:5:5
      {sizeof(DDPIXELFORMAT), DDPF_RGB, 0, 16,  0xF800, 0x07e0, 0x001F, 0}				// 16-bit RGB 5:6:5
};

#define PF_TABLE_SIZE (sizeof(ddpfOverlayFormats) / sizeof(ddpfOverlayFormats[0]))
RECT rs;


void LogErrorMesssage(HRESULT hErrValue)
{
	switch(hErrValue)
	{
	case DDERR_CANTCREATEDC:
		printf("DDERR_CANTCREATEDC \n");
		break;
	case DDERR_CANTLOCKSURFACE:
		printf("DDERR_CANTLOCKSURFACE \n");
		break;
	case DDERR_CLIPPERISUSINGHWND:
		printf("DDERR_CLIPPERISUSINGHWND \n");
		break;
	case DDERR_COLORKEYNOTSET:
		printf("DDERR_COLORKEYNOTSET \n");
		break;
	case DDERR_CURRENTLYNOTAVAIL:
		printf("DDERR_CURRENTLYNOTAVAIL \n");
		break;
	case DDERR_DCALREADYCREATED:
		printf("DDERR_DCALREADYCREATED \n");
		break;
	case DDERR_DEVICEDOESNTOWNSURFACE:
		printf("DDERR_DEVICEDOESNTOWNSURFACE \n");
		break;
	case DDERR_DIRECTDRAWALREADYCREATED:
		printf("DDERR_DIRECTDRAWALREADYCREATED \n");
		break;
	case DDERR_EXCLUSIVEMODEALREADYSET:
		printf("DDERR_EXCLUSIVEMODEALREADYSET \n");
		break;
	case DDERR_GENERIC:
		printf("DDERR_GENERIC \n");
		break;
	case DDERR_HEIGHTALIGN:
		printf("DDERR_HEIGHTALIGN \n");
		break;
	case DDERR_IMPLICITLYCREATED:
		printf("DDERR_IMPLICITLYCREATED \n");
		break;
	case DDERR_INCOMPATIBLEPRIMARY:
		printf("DDERR_INCOMPATIBLEPRIMARY \n");
		break;
	case DDERR_INVALIDCAPS:
		printf("DDERR_INVALIDCAPS \n");
		break;
	case DDERR_INVALIDCLIPLIST:
		printf("DDERR_INVALIDCLIPLIST \n");
		break;
	case DDERR_INVALIDMODE:
		printf("DDERR_INVALIDMODE \n");
		break;
	case DDERR_INVALIDOBJECT:
		printf("DDERR_INVALIDOBJECT \n");
		break;
	case DDERR_INVALIDPARAMS:
		printf("DDERR_INVALIDPARAMS \n");
		break;
	case DDERR_INVALIDPIXELFORMAT:
		printf("DDERR_INVALIDPIXELFORMAT \n");
		break;
	case DDERR_INVALIDPOSITION:
		printf("DDERR_INVALIDPOSITION \n");
		break;
	case DDERR_INVALIDRECT:
		printf("DDERR_INVALIDRECT \n");
		break;
	case DDERR_LOCKEDSURFACES:
		printf("DDERR_LOCKEDSURFACES \n");
		break;
	case DDERR_MOREDATA:
		printf("DDERR_MOREDATA \n");
		break;
	case DDERR_NOALPHAHW:
		printf("case DDERR_NOALPHAHW \n");
		break;
	case DDERR_NOBLTHW			:
		printf("DDERR_NOBLTHW \n");
		break;
	case DDERR_NOCLIPLIST:
		printf("DDERR_NOCLIPLIST \n");
		break;
	case DDERR_NOCLIPPERATTACHED:
		printf("DDERR_NOCLIPPERATTACHED \n");
		break;
	case DDERR_NOCOLORCONVHW:
		printf("DDERR_NOCOLORCONVHW \n");
		break;
	case DDERR_NOCOLORKEYHW:
		printf("DDERR_NOCOLORKEYHW \n");
		break;
	case DDERR_NOCOOPERATIVELEVELSET:
		printf("DDERR_NOCOOPERATIVELEVELSET \n");
		break;
	case DDERR_NODC:
		printf("DDERR_NODC \n");
		break;
	case DDERR_NOFLIPHW:
		printf("DDERR_NOFLIPHW \n");
		break;
	case DDERR_NOOVERLAYDEST:
		printf("DDERR_NOOVERLAYDEST \n");
		break;
	case DDERR_NOOVERLAYHW:
		printf("DDERR_NOOVERLAYHW \n");
		break;
	case DDERR_NOPALETTEATTACHED:
		printf("DDERR_NOPALETTEATTACHED \n");
		break;
	case DDERR_NOPALETTEHW:
		printf("DDERR_NOPALETTEHW \n");
		break;
	case DDERR_NORASTEROPHW:
		printf("DDERR_NORASTEROPHW \n");
		break;
	case DDERR_NOSTRETCHHW:
		printf("DDERR_NOSTRETCHHW \n");
		break;
	case DDERR_NOTAOVERLAYSURFACE:
		printf("DDERR_NOTAOVERLAYSURFACE \n");
		break;
	case DDERR_NOTFLIPPABLE:
		printf("DDERR_NOTFLIPPABLE \n");
		break;
	case DDERR_NOTFOUND:
		printf("DDERR_NOTFOUND \n");
		break;
	case DDERR_NOTLOCKED:
		printf("DDERR_NOTLOCKED \n");
		break;
	case DDERR_NOTPALETTIZED:
		printf("DDERR_NOTPALETTIZED \n");
		break;
	case DDERR_NOVSYNCHW:
		printf("DDERR_NOVSYNCHW \n");
		break;
	case DDERR_NOZOVERLAYHW:
		printf("DDERR_NOZOVERLAYHW \n");
		break;
	case DDERR_OUTOFCAPS:
		printf("DDERR_OUTOFCAPS \n");
		break;
	case DDERR_OUTOFMEMORY:
		printf("DDERR_OUTOFMEMORY \n");
		break;
	case DDERR_OUTOFVIDEOMEMORY:
		printf("DDERR_OUTOFVIDEOMEMORY \n");
		break;
	case DDERR_OVERLAPPINGRECTS:
		printf("DDERR_OVERLAPPINGRECTS \n");
		break;
	case DDERR_OVERLAYNOTVISIBLE:
		printf("DDERR_OVERLAYNOTVISIBLE \n");
		break;
	case DDERR_PALETTEBUSY:
		printf("DDERR_PALETTEBUSY \n");
		break;
	case DDERR_PRIMARYSURFACEALREADYEXISTS:
		printf("DDERR_PRIMARYSURFACEALREADYEXISTS \n");
		break;
	case DDERR_REGIONTOOSMALL:
		printf("DDERR_REGIONTOOSMALL \n");
		break;
	case DDERR_SURFACEBUSY:
		printf("DDERR_SURFACEBUSY \n");
		break;
	case DDERR_SURFACELOST:
		printf("DDERR_SURFACELOST \n");
		break;
	case DDERR_TOOBIGHEIGHT:
		printf("DDERR_TOOBIGHEIGHT \n");
		break;
	case DDERR_TOOBIGSIZE:
		printf("DDERR_TOOBIGSIZE \n");
		break;
	case DDERR_TOOBIGWIDTH:
		printf("DDERR_TOOBIGWIDTH \n");
		break;
	case DDERR_UNSUPPORTED:
		printf("DDERR_UNSUPPORTED \n");
		break;
	case DDERR_UNSUPPORTEDFORMAT:
		printf("DDERR_UNSUPPORTEDFORMAT \n");
		break;
	case DDERR_VERTICALBLANKINPROGRESS:
		printf("DDERR_VERTICALBLANKINPROGRESS \n");
		break;
	case DDERR_VIDEONOTACTIVE:
		printf("DDERR_VIDEONOTACTIVE \n");
		break;
	case DDERR_WASSTILLDRAWING:
		printf("DDERR_WASSTILLDRAWING \n");
		break;
	case DDERR_WRONGMODE:
		printf("DDERR_WRONGMODE \n");
		break;
	default:
		printf("default error\n");
	}
}

int UpdateSurface(HWND hWnd, RenderingSurface* pRenderingSurface)
{	
	POINT DDDestOrigin;
	// Set the flags we'll send to UpdateOverlay
    DWORD dwUpdateFlags = DDOVER_SHOW;
	DDOVERLAYFX                 ovfx;
	HRESULT                     hRet;
	RECT rd;	

    // Does the overlay hardware support source color keying?
    // If so, we can hide the black background around the image.
    // This probably won't work with YUV formats
    memset(&ovfx, 0, sizeof(ovfx));
    ovfx.dwSize = sizeof(ovfx);

	//We are not doing this, since it will cause transparent wherever black.

#if 0
	if (ddcaps.dwOverlayCaps & DDOVERLAYCAPS_CKEYSRC)
    {
        dwUpdateFlags |= DDOVER_KEYSRCOVERRIDE;

        // Create an overlay FX structure so we can specify a source color key.
        // This information is ignored if the DDOVER_SRCKEYOVERRIDE flag 
        // isn't set.
        ovfx.dckSrcColorkey.dwColorSpaceLowValue=0; // black as the color key
        ovfx.dckSrcColorkey.dwColorSpaceHighValue=0;
    }
#endif
    

	//get the rect of my target window on my screen
	DDDestOrigin.x = 0, DDDestOrigin.y = 0;
	ClientToScreen(hWnd, &DDDestOrigin);
	GetClientRect(hWnd, &rd);
	OffsetRect(&rd, DDDestOrigin.x, DDDestOrigin.y);

	// Update the overlay parameters.
	hRet = pRenderingSurface->pDDSOverlay->lpVtbl->UpdateOverlay(pRenderingSurface->pDDSOverlay, NULL, pRenderingSurface->pDDSPrimary, &rd, dwUpdateFlags, &ovfx);
	if (hRet != DD_OK)
	{
		hRet = pRenderingSurface->pDDSOverlay->lpVtbl->UpdateOverlay(pRenderingSurface->pDDSOverlay, NULL, pRenderingSurface->pDDSPrimary, &rs, dwUpdateFlags, &ovfx);
		if (hRet != DD_OK)
		{
			printf("Unable to show overlay surface!");
			LogErrorMesssage(hRet);
			return FALSE;
		}
	}
	return TRUE;
}
void ReleaseAllSurfaces(RenderingSurface* pRenderingSurface)
{	
    if (pRenderingSurface->pDDSOverlay != NULL)
    {
        // Use UpdateOverlay() with the DDOVER_HIDE flag to remove an overlay 
	    // from the display.
	    pRenderingSurface->pDDSOverlay->lpVtbl->UpdateOverlay(pRenderingSurface->pDDSOverlay,NULL, pRenderingSurface->pDDSPrimary, NULL, DDOVER_HIDE, NULL);
        pRenderingSurface->pDDSOverlay->lpVtbl->Release(pRenderingSurface->pDDSOverlay);
        pRenderingSurface->pDDSOverlay = NULL;
    }

    if (pRenderingSurface->pDDSPrimary != NULL)
    {
        pRenderingSurface->pDDSPrimary->lpVtbl->Release(pRenderingSurface->pDDSPrimary);
        pRenderingSurface->pDDSPrimary = NULL;
    }

    if (pRenderingSurface->pDD != NULL)
    {
        pRenderingSurface->pDD->lpVtbl->Release(pRenderingSurface->pDD);
        pRenderingSurface->pDD = NULL;
    }

	if(pRenderingSurface != NULL)
	{
		free(pRenderingSurface);
		pRenderingSurface = NULL;
	}
}

HRESULT RestoreAllSurfaces(HWND	hWnd, RenderingSurface* pRenderingSurface)
{
    HRESULT hRet;

    // Try Restoring the primary surface.

    hRet = pRenderingSurface->pDDSPrimary->lpVtbl->Restore(pRenderingSurface->pDDSPrimary);
    if (hRet != DD_OK)
	    return hRet;

    // Try Restoring the overlay surface.
    hRet = pRenderingSurface->pDDSOverlay->lpVtbl->Restore(pRenderingSurface->pDDSOverlay);
    if (hRet != DD_OK)
	    return hRet;

    // Show the overlay.
    UpdateSurface(hWnd, pRenderingSurface);    
	return hRet;
}


EGLNativeDisplayType GetDisplayType ()
{
	return EGL_DIRECTDRAW_DISPLAY;
}

RenderingSurface* CreateRenderingSurface(HWND hWnd, int width, int height)
{
	DDSURFACEDESC               ddsd;
    DDCAPS                      ddcaps;
    HRESULT                     hRet;	
	int i = 0;
	RenderingSurface* pRenderingSurface = NULL;

	pRenderingSurface = malloc(sizeof(RenderingSurface));
	if(!pRenderingSurface)
	{
		return 0;
	}
	 
	hRet = DirectDrawCreate(NULL, &pRenderingSurface->pDD, NULL);
    if (hRet != DD_OK)
	{
		printf("DirectDrawCreate FAILED");
        return 0;
	}

	// Get normal mode.

    hRet = pRenderingSurface->pDD->lpVtbl->SetCooperativeLevel(pRenderingSurface->pDD, hWnd, DDSCL_NORMAL);
    if (hRet != DD_OK)
	{
		printf("SetCooperativeLevel FAILED");
        return 0; 
	}

    // Get a primary surface interface pointer (only needed for init.)
    memset(&ddsd, 0, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    ddsd.dwFlags = DDSD_CAPS;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
    hRet = pRenderingSurface->pDD->lpVtbl->CreateSurface(pRenderingSurface->pDD, &ddsd, &pRenderingSurface->pDDSPrimary, NULL);
    if (hRet != DD_OK)
	{
		printf("CreateSurface FAILED");
		return 0;
	}

    // See if we can support overlays.
    memset(&ddcaps, 0, sizeof(ddcaps));
    ddcaps.dwSize = sizeof(ddcaps);
    hRet = pRenderingSurface->pDD->lpVtbl->GetCaps(pRenderingSurface->pDD, &ddcaps, NULL);
    if (hRet != DD_OK)
	{
		printf("GetCaps FAILED");
		return 0;
	}

    if (ddcaps.dwOverlayCaps == 0)
	{
        printf("Overlays are not supported in hardware!");
		return 0;
	}

    // Get alignment info to compute our overlay surface size.	
    rs.left = 0;
    rs.top = 0;
    rs.right = width;
    rs.bottom = height;
    if (ddcaps.dwAlignSizeSrc != 0)
	    rs.right += rs.right % ddcaps.dwAlignSizeSrc;
    
    // Create the overlay flipping surface. We will attempt the pixel formats
    // in our table one at a time until we find one that jives.
    memset(&ddsd, 0, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    ddsd.ddsCaps.dwCaps = DDSCAPS_OVERLAY | DDSCAPS_FLIP | DDSCAPS_VIDEOMEMORY;
    ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_BACKBUFFERCOUNT | 
                   DDSD_PIXELFORMAT;
	ddsd.dwWidth = rs.right;
	ddsd.dwHeight = rs.bottom;
    ddsd.dwBackBufferCount = 1;

    
    do
	{
	    ddsd.ddpfPixelFormat = ddpfOverlayFormats[i];
	    hRet = pRenderingSurface->pDD->lpVtbl->CreateSurface(pRenderingSurface->pDD,&ddsd, &pRenderingSurface->pDDSOverlay, NULL);
    } while (hRet != DD_OK && (++i < PF_TABLE_SIZE));

    if (hRet != DD_OK)
	{
        printf("Unable to create overlay surface!");
		return 0;
	}   
	
	hRet = pRenderingSurface->pDDSOverlay->lpVtbl->Blt(pRenderingSurface->pDDSOverlay, NULL, NULL, NULL, DDBLT_COLORFILL, NULL);
	if (hRet != DD_OK)
	{
		printf("Unable to blit");
		return 0;
	}
	UpdateSurface(hWnd, pRenderingSurface);
	
	printf("pRenderingSurface->pDDSOverlay =%x \n", pRenderingSurface->pDDSOverlay);
	return	pRenderingSurface;
}

unsigned int GetSurface(RenderingSurface* pRenderingSurface)
{
	if(pRenderingSurface)
	{
		return (unsigned int)pRenderingSurface->pDDSOverlay;
	}
	return 0;
}

void FlipSurface(RenderingSurface* pRenderingSurface)
{
	while (TRUE)
	{
		HRESULT hRet = pRenderingSurface->pDDSOverlay->lpVtbl->Flip(pRenderingSurface->pDDSOverlay, NULL, 0);
		if (hRet == DD_OK)
			break;
		if (hRet == DDERR_SURFACELOST)
		{
			hRet = RestoreAllSurfaces(NULL, pRenderingSurface);
			if (hRet != DD_OK)
				break;
			printf(" DDERR_SURFACELOST");
			break;
		}
		if (hRet != DDERR_WASSTILLDRAWING)
		{
			printf("DDERR_WASSTILLDRAWING");			
			break;
		}
	}
}

void HandleWindowMessages(HWND	hWnd, UINT	uMsg, WPARAM	wParam, LPARAM	lParam, RenderingSurface* pSurface)
{
	switch (uMsg)
	{
		case WM_MOVE:
		case WM_SIZE:
			UpdateSurface(hWnd, pSurface);
			break;		
		case WM_DESTROY:			
			ReleaseAllSurfaces(pSurface);
			break;
	}	
}
#else
EGLNativeDisplayType GetDisplayType ()
{
	return EGL_DEFAULT_DISPLAY;
}

unsigned int GetSurface(RenderingSurface* pRenderingSurface)
{
	return (unsigned int) pRenderingSurface->hWnd;
}
RenderingSurface* CreateRenderingSurface (HWND hWnd, int width, int height)
{
	RenderingSurface* pRenderingSurface = malloc(sizeof(RenderingSurface));
	pRenderingSurface->hWnd = hWnd;
	return pRenderingSurface;
}

void FlipSurface(RenderingSurface* pRenderingSurface)
{
}
void ReleaseAllSurfaces(RenderingSurface* pRenderingSurface)
{
	if(pRenderingSurface)
	{
		free(pRenderingSurface);
		pRenderingSurface = NULL;
	}
}
void HandleWindowMessages(HWND	hWnd, UINT	uMsg, WPARAM	wParam, LPARAM	lParam, RenderingSurface* pRenderingSurface)
{
}
#endif

//******************************************************DDraw***************/
