#include "bbPinball.h"
#include "bbGLUtils.h"
#include "bbPinballDefines.h"
#include "bbParserUtils.h"

#define GL_GLEXT_PROTOTYPES
//#include "GLES2/gl2amdext.h"

#if !defined(_WIN32_WCE)
#include <crtdbg.h>
#include <time.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#ifndef BB_DISABLE_SOUNDS
#include <mmsystem.h>
#include <dsound.h>
#endif // !BB_DISABLE_SOUNDS

#ifndef WIN32_LEAN_AND_MEAN
#	define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include "SurfaceAdopter.h"

/* fix warnings about ISO standard */
#ifndef strdup
#define strdup _strdup
#endif

#ifndef WM_MOUSEWHEEL
#	define WM_MOUSEWHEEL 0x20A		/* Don't know why this isn't defined in my environment */
#endif

/*--------------------------------------------------------------------------*/
/* Win32 Global variables
/*--------------------------------------------------------------------------*/

HWND			g_hWnd	= NULL;
HDC				g_hDC	= NULL;
WNDCLASS		g_wc;

EGLDisplay		g_display = EGL_NO_DISPLAY;
EGLConfig		g_config = 0;
EGLContext		g_mainContext = EGL_NO_CONTEXT;
EGLSurface		g_mainSurface = EGL_NO_SURFACE;



#define MAX_RESOLUTIONS 3

GLuint			g_surfaceWidth	= 480;
GLuint			g_surfaceHeight	= 480;
GLuint			g_resolutionIndex = 0;
GLuint			g_resolutionList[MAX_RESOLUTIONS * 2];
GLboolean		g_reload = GL_FALSE;

RenderingSurface* g_RenderingSurface = 0;
int AdjustedWidth = 0;
int AdjustedHeight = 0;


BBPinball*		g_pinball = NULL;

GLint			g_mouseWheelValue = 0;

LARGE_INTEGER	g_timeFreq;
LARGE_INTEGER	g_timeStart;
GLboolean		g_timeInited = GL_FALSE;

#ifndef BB_DISABLE_SOUNDS
BBSoundInterface	g_sound;

IDirectSound8*	g_ds = NULL;

/*--------------------------------------------------------------------------*/
/*
/*--------------------------------------------------------------------------*/

typedef struct WaveFile_s
{
	WAVEFORMATEX* m_pwfx;        // Pointer to WAVEFORMATEX structure
	HMMIO         m_hmmio;       // MM I/O handle for the WAVE
	MMCKINFO      m_ck;          // Multimedia RIFF chunk
	MMCKINFO      m_ckRiff;      // Use in opening a WAVE file
	DWORD         m_dwSize;      // The size of the wave file
} WaveFile;

static WaveFile*	WaveFile_create			();
static GLboolean	WaveFile_open			(WaveFile* wav, LPSTR strFileName);
static void			WaveFile_close			(WaveFile* wav);
static GLboolean	WaveFile_read			(WaveFile* wav, BYTE* pBuffer,
											 DWORD dwSizeToRead, DWORD* pdwSizeRead);
static GLboolean	WaveFile_readMMIO		(WaveFile* wav);
static GLboolean	WaveFile_resetFile		(WaveFile* wav);

/*--------------------------------------------------------------------------*/
/*
/*--------------------------------------------------------------------------*/

WaveFile* WaveFile_create ()
{
	WaveFile* wav = (WaveFile*)malloc(sizeof(WaveFile));
	if (!wav)
		return NULL;

	wav->m_pwfx    = NULL;
	wav->m_hmmio   = NULL;
	wav->m_dwSize  = 0;

	return wav;
}

/*--------------------------------------------------------------------------*/
/*
/*--------------------------------------------------------------------------*/

GLboolean WaveFile_open (WaveFile* wav, LPSTR filename)
{
	if (filename == NULL)
		return GL_FALSE;

	free(wav->m_pwfx);

	wav->m_hmmio = mmioOpen(filename, NULL, MMIO_ALLOCBUF | MMIO_READ);

	if (!WaveFile_readMMIO(wav))
	{
		// ReadMMIO will fail if its an not a wave file
		mmioClose(wav->m_hmmio, 0);
		return GL_FALSE;
	}

	if (!WaveFile_resetFile(wav))
		return GL_FALSE;

	// After the reset, the size of the wav file is m_ck.cksize so store it now
	wav->m_dwSize = wav->m_ck.cksize;

	return GL_TRUE;
}

/*--------------------------------------------------------------------------*/
/*
/*--------------------------------------------------------------------------*/

void WaveFile_close (WaveFile* wav)
{
	if (wav->m_hmmio)
	{
		mmioClose(wav->m_hmmio, 0);
		wav->m_hmmio = NULL;
	}

	free(wav->m_pwfx);
	free(wav);
}

/*--------------------------------------------------------------------------*/
/*
/*--------------------------------------------------------------------------*/

GLboolean WaveFile_read (WaveFile* wav, BYTE* pBuffer,
						 DWORD dwSizeToRead, DWORD* pdwSizeRead)
{
	MMIOINFO mmioinfoIn; // current status of m_hmmio
	UINT cbDataIn;
	DWORD cT;

	if (wav->m_hmmio == NULL)
		return GL_FALSE;
	if (pBuffer == NULL || pdwSizeRead == NULL)
		return GL_FALSE;

	if (pdwSizeRead != NULL)
		*pdwSizeRead = 0;

	if (mmioGetInfo(wav->m_hmmio, &mmioinfoIn, 0))
		return GL_FALSE;

	cbDataIn = dwSizeToRead;
	if (cbDataIn > wav->m_ck.cksize)
		cbDataIn = wav->m_ck.cksize;

	wav->m_ck.cksize -= cbDataIn;

	for (cT = 0; cT < cbDataIn; cT++)
	{
		// Copy the bytes from the io to the buffer.
		if (mmioinfoIn.pchNext == mmioinfoIn.pchEndRead)
		{
			if (mmioAdvance(wav->m_hmmio, &mmioinfoIn, MMIO_READ))
				return GL_FALSE;

			if (mmioinfoIn.pchNext == mmioinfoIn.pchEndRead)
				return GL_FALSE;
		}

		// Actual copy.
		*((BYTE*)pBuffer+cT) = *((BYTE*)mmioinfoIn.pchNext);
		mmioinfoIn.pchNext++;
	}

	if (mmioSetInfo(wav->m_hmmio, &mmioinfoIn, 0))
		return GL_FALSE;

	if (pdwSizeRead != NULL)
		*pdwSizeRead = cbDataIn;

	return GL_TRUE;
}

/*--------------------------------------------------------------------------*/
/*
/*--------------------------------------------------------------------------*/

GLboolean WaveFile_readMMIO (WaveFile* wav)
{
	MMCKINFO        ckIn;           // chunk info. for general use.
	PCMWAVEFORMAT   pcmWaveFormat;  // Temp PCM structure to load in.

	wav->m_pwfx = NULL;

	if ((mmioDescend(wav->m_hmmio, &wav->m_ckRiff, NULL, 0)))
		return GL_FALSE;

	// Check to make sure this is a valid wave file
	if ((wav->m_ckRiff.ckid != FOURCC_RIFF) ||
		(wav->m_ckRiff.fccType != mmioFOURCC('W', 'A', 'V', 'E')))
		return GL_FALSE;

	// Search the input file for for the 'fmt ' chunk.
	ckIn.ckid = mmioFOURCC('f', 'm', 't', ' ');
	if (mmioDescend(wav->m_hmmio, &ckIn, &wav->m_ckRiff, MMIO_FINDCHUNK))
		return GL_FALSE;

	// Expect the 'fmt' chunk to be at least as large as <PCMWAVEFORMAT>;
	// if there are extra parameters at the end, we'll ignore them
	if (ckIn.cksize < (LONG)sizeof(PCMWAVEFORMAT))
		return GL_FALSE;

	// Read the 'fmt ' chunk into <pcmWaveFormat>.
	if (mmioRead(wav->m_hmmio, (HPSTR)&pcmWaveFormat,
		sizeof(pcmWaveFormat)) != sizeof(pcmWaveFormat))
		return GL_FALSE;

	// Allocate the waveformatex, but if its not pcm format, read the next
	// word, and thats how many extra bytes to allocate.
	if (pcmWaveFormat.wf.wFormatTag == WAVE_FORMAT_PCM)
	{
		wav->m_pwfx = (WAVEFORMATEX*)malloc(sizeof(WAVEFORMATEX));
		if (!wav->m_pwfx)
			return GL_FALSE;

		// Copy the bytes from the pcm structure to the waveformatex structure
		memcpy(wav->m_pwfx, &pcmWaveFormat, sizeof(pcmWaveFormat));
		wav->m_pwfx->cbSize = 0;
	}
	else
	{
		// Read in length of extra bytes.
		WORD cbExtraBytes = 0L;
		if (mmioRead(wav->m_hmmio, (CHAR*)&cbExtraBytes, sizeof(WORD)) != sizeof(WORD))
			return GL_FALSE;

		wav->m_pwfx = (WAVEFORMATEX*)malloc(sizeof(WAVEFORMATEX) + cbExtraBytes);
		if (!wav->m_pwfx)
			return GL_FALSE;

		// Copy the bytes from the pcm structure to the waveformatex structure
		memcpy(wav->m_pwfx, &pcmWaveFormat, sizeof(pcmWaveFormat));
		wav->m_pwfx->cbSize = cbExtraBytes;

		// Now, read those extra bytes into the structure, if cbExtraAlloc != 0.
		if (mmioRead(wav->m_hmmio, (CHAR*)(((BYTE*)&(wav->m_pwfx->cbSize)) + sizeof(WORD)),
			cbExtraBytes) != cbExtraBytes)
		{
			free(wav->m_pwfx);
			return GL_FALSE;
		}
	}

	// Ascend the input file out of the 'fmt ' chunk.
	if (mmioAscend(wav->m_hmmio, &ckIn, 0))
	{
		free(wav->m_pwfx);
		return GL_FALSE;
	}

	return GL_TRUE;
}

/*--------------------------------------------------------------------------*/
/*
/*--------------------------------------------------------------------------*/

GLboolean WaveFile_resetFile (WaveFile* wav)
{
	if (wav->m_hmmio == NULL)
		return GL_FALSE;

	if (mmioSeek(wav->m_hmmio, wav->m_ckRiff.dwDataOffset + sizeof(FOURCC), SEEK_SET) == -1)
		return GL_FALSE;

	// Search the input file for the 'data' chunk.
	wav->m_ck.ckid = mmioFOURCC('d', 'a', 't', 'a');
	if (mmioDescend(wav->m_hmmio, &wav->m_ck, &wav->m_ckRiff, MMIO_FINDCHUNK))
		return GL_FALSE;

	return GL_TRUE;
}

/*--------------------------------------------------------------------------*/
/*
/*--------------------------------------------------------------------------*/

#define MAX_SOUNDS	256

typedef struct DXSound_s
{
	char*					filename;
	IDirectSoundBuffer*		m_apDSBuffer;
	DWORD					m_dwDSBufferSize;
	WaveFile*				m_pWaveFile;
} DXSound;

GLuint				g_soundCount;
DXSound*			g_soundList[MAX_SOUNDS];

GLuint				g_soundLastError = 0;

static DXSound*		DXSound_create				(const char* filename);
static GLboolean	DXSound_load				(DXSound* sound);
static void			DXSound_destroy				(DXSound* sound);

static GLboolean	DXSound_restoreBuffer		(DXSound* sound, GLboolean* pbWasRestored);
static GLboolean	DXSound_fillBufferWithSound	(DXSound* sound, GLboolean bRepeatWavIfBufferLarger);

static GLboolean	DXSound_play				(DXSound* sound, GLfloat volume, GLboolean repeat);
static GLboolean	DXSound_stop				(DXSound* sound);

/*--------------------------------------------------------------------------*/
/*
/*--------------------------------------------------------------------------*/

DXSound* DXSound_create (const char* filename)
{
	DXSound* sound;

	sound = (DXSound*)malloc(sizeof(DXSound));
	if (!sound)
	{
		g_soundLastError = BB_SOUND_ERR_OUT_OF_MEMORY;
		return NULL;
	}

	sound->filename = NULL;
	sound->m_apDSBuffer = NULL;
	sound->m_dwDSBufferSize = 0;
	sound->m_pWaveFile = NULL;

	sound->filename = strdup(filename);
	if (!sound->filename)
	{
		g_soundLastError = BB_SOUND_ERR_OUT_OF_MEMORY;
		DXSound_destroy(sound);
		return NULL;
	}

	if (!DXSound_load(sound))
	{
		DXSound_destroy(sound);
		return NULL;
	}

	return sound;
}

GLboolean DXSound_load (DXSound* sound)
{
	IDirectSoundBuffer*	apDSBuffer		= NULL;
	DWORD				dwDSBufferSize	= 0;
	WaveFile*			pWaveFile		= NULL;
	DSBUFFERDESC		dsbd;

	if (g_ds == NULL)
		goto LFail;

	pWaveFile = WaveFile_create();
	if (!pWaveFile)
		goto LFail;

	WaveFile_open(pWaveFile, sound->filename);

	if (pWaveFile->m_dwSize == 0)
		goto LFail;

	// Make the DirectSound buffer the same size as the wav file
	dwDSBufferSize = pWaveFile->m_dwSize;

	// Create the direct sound buffer, and only request the flags needed
	// since each requires some overhead and limits if the buffer can
	// be hardware accelerated
	memset(&dsbd, 0, sizeof(DSBUFFERDESC));
	dsbd.dwSize				= sizeof(DSBUFFERDESC);
	dsbd.dwFlags			= DSBCAPS_CTRLVOLUME;
	dsbd.dwBufferBytes		= dwDSBufferSize;
    dsbd.guid3DAlgorithm	= GUID_NULL;
	dsbd.lpwfxFormat		= pWaveFile->m_pwfx;

	// DirectSound is only guarenteed to play PCM data.  Other
	// formats may or may not work depending the sound card driver.
	if (FAILED(IDirectSound8_CreateSoundBuffer(g_ds, &dsbd, &apDSBuffer, NULL)))
		goto LFail;

	sound->m_apDSBuffer = apDSBuffer;
	sound->m_dwDSBufferSize = dwDSBufferSize;
	sound->m_pWaveFile = pWaveFile;

    DXSound_fillBufferWithSound(sound, GL_FALSE);

	return GL_TRUE;

	LFail:
	// Cleanup
	free(pWaveFile);
	free(apDSBuffer);
	return GL_FALSE;
}

/*--------------------------------------------------------------------------*/
/*
/*--------------------------------------------------------------------------*/

void DXSound_destroy (DXSound* sound)
{
	if (sound->m_apDSBuffer)
		IDirectSoundBuffer_Release(sound->m_apDSBuffer);

	if (sound->m_pWaveFile)
		WaveFile_close(sound->m_pWaveFile);

	free(sound->filename);
	free(sound);
}

/*--------------------------------------------------------------------------*/
/*
/*--------------------------------------------------------------------------*/

GLboolean DXSound_restoreBuffer (DXSound* sound, GLboolean* pbWasRestored)
{
	DWORD dwStatus;

	if (g_ds == NULL)
		return GL_FALSE;

	if (pbWasRestored)
		*pbWasRestored = GL_FALSE;

	if (FAILED(IDirectSoundBuffer_GetStatus(sound->m_apDSBuffer, &dwStatus)))
		return GL_FALSE;

	if (dwStatus & DSBSTATUS_BUFFERLOST)
	{
		HRESULT hr;
		// Since the app could have just been activated, then
		// DirectSound may not be giving us control yet, so
		// the restoring the buffer may fail.
		// If it does, sleep until DirectSound gives us control.

		do
		{
			hr = IDirectSoundBuffer_Restore(sound->m_apDSBuffer);
			if (hr == DSERR_BUFFERLOST)
				Sleep(10);
		}
		while ((hr = IDirectSoundBuffer_Restore(sound->m_apDSBuffer)) == DSERR_BUFFERLOST);

		if (pbWasRestored != NULL)
			*pbWasRestored = TRUE;
	}

	return GL_TRUE;
}

/*--------------------------------------------------------------------------*/
/*
/*--------------------------------------------------------------------------*/

GLboolean DXSound_fillBufferWithSound (DXSound* sound, GLboolean bRepeatWavIfBufferLarger)
{
	void*	pDSLockedBuffer      = NULL; // Pointer to locked buffer memory
	DWORD   dwDSLockedBufferSize = 0;    // Size of the locked DirectSound buffer
	DWORD   dwWavDataRead        = 0;    // Amount of data read from the wav file

	if (g_ds == NULL)
		return GL_FALSE;

	// Make sure we have focus, and we didn't just switch in from
	// an app which had a DirectSound device
	if (!DXSound_restoreBuffer(sound, NULL))
		return GL_FALSE;

	// Lock the buffer down
	if (FAILED(IDirectSoundBuffer_Lock(sound->m_apDSBuffer, 0, sound->m_dwDSBufferSize,
										&pDSLockedBuffer, &dwDSLockedBufferSize,
										NULL, NULL, 0L)))
		return GL_FALSE;

	// Reset the wave file to the beginning
	if (!WaveFile_resetFile(sound->m_pWaveFile))
		return GL_FALSE;

	if (!WaveFile_read(sound->m_pWaveFile, (BYTE*)pDSLockedBuffer,
					dwDSLockedBufferSize,
					&dwWavDataRead ))
		return GL_FALSE;

	if (dwWavDataRead == 0)
	{
		// Wav is blank, so just fill with silence
		FillMemory((BYTE*) pDSLockedBuffer,
				dwDSLockedBufferSize,
				(BYTE)(sound->m_pWaveFile->m_pwfx->wBitsPerSample == 8 ? 128 : 0));
	}
	else if (dwWavDataRead < dwDSLockedBufferSize)
	{
		// If the wav file was smaller than the DirectSound buffer,
		// we need to fill the remainder of the buffer with data
		if (bRepeatWavIfBufferLarger)
		{
			// Reset the file and fill the buffer with wav data
			DWORD dwReadSoFar = dwWavDataRead;    // From previous call above.
			while (dwReadSoFar < dwDSLockedBufferSize)
			{
				// This will keep reading in until the buffer is full
				// for very short files
				if (!WaveFile_resetFile(sound->m_pWaveFile))
					return GL_FALSE;

				if (!WaveFile_read(sound->m_pWaveFile, (BYTE*)pDSLockedBuffer + dwReadSoFar,
									dwDSLockedBufferSize - dwReadSoFar,
									&dwWavDataRead))
					return GL_FALSE;

				dwReadSoFar += dwWavDataRead;
			}
		}
		else
		{
			// Don't repeat the wav file, just fill in silence
			FillMemory((BYTE*) pDSLockedBuffer + dwWavDataRead,
						dwDSLockedBufferSize - dwWavDataRead,
						(BYTE)(sound->m_pWaveFile->m_pwfx->wBitsPerSample == 8 ? 128 : 0));
		}
	}

	// Unlock the buffer, we don't need it anymore.
	IDirectSoundBuffer_Unlock(sound->m_apDSBuffer, pDSLockedBuffer, dwDSLockedBufferSize, NULL, 0);

	return GL_TRUE;
}

/*--------------------------------------------------------------------------*/
/*
/*--------------------------------------------------------------------------*/

GLboolean DXSound_play (DXSound* sound, GLfloat volume, GLboolean repeat)
{
    GLboolean	bRestored;
	DWORD		currPos;
	DWORD		dwStatus;
//	HRESULT res;

	if (sound->m_apDSBuffer == NULL)
		return GL_FALSE;

    // Restore the buffer if it was lost
    if (!DXSound_restoreBuffer(sound, &bRestored))
		return GL_FALSE;

    if (bRestored)
    {
        if (!DXSound_fillBufferWithSound(sound, GL_FALSE))
			return GL_FALSE;
    }

	IDirectSoundBuffer_SetVolume(sound->m_apDSBuffer, (unsigned int)(DSBVOLUME_MAX * volume + DSBVOLUME_MIN * (1.0f - volume)));
//	res = IDirectSoundBuffer_SetVolume(sound->m_apDSBuffer, DSBVOLUME_MIN);
//	IDirectSoundBuffer_SetFrequency(sound->m_apDSBuffer, DSBFREQUENCY_ORIGINAL);
//	IDirectSoundBuffer_SetPan(sound->m_apDSBuffer, DSBPAN_LEFT);

/*	switch (res)
	{
	case DSERR_CONTROLUNAVAIL:
		res = 0;
		break;
	case DSERR_GENERIC:
		res = 1;
		break;
	case DSERR_INVALIDPARAM:
		res = 2;
		break;
	case DSERR_PRIOLEVELNEEDED:
		res = 3;
		break;
	default:
		break;
	}*/

	if (FAILED(IDirectSoundBuffer_GetStatus(sound->m_apDSBuffer, &dwStatus)))
		return GL_FALSE;

	if (FAILED(IDirectSoundBuffer_GetCurrentPosition(sound->m_apDSBuffer, &currPos, NULL)))
		return GL_FALSE;

	if ((dwStatus & DSBSTATUS_PLAYING) && currPos > 0 && currPos < sound->m_dwDSBufferSize)
	{
		if (FAILED(IDirectSoundBuffer_SetCurrentPosition(sound->m_apDSBuffer, 0)))
			return GL_FALSE;
	}
	else
	{
		unsigned int flags = 0;

		if (repeat)
		{
			flags |= DSBPLAY_LOOPING;
		}

		if (FAILED(IDirectSoundBuffer_Play(sound->m_apDSBuffer, 0, 0, flags)))
			return GL_FALSE;
	}

	return GL_TRUE;
}

/*--------------------------------------------------------------------------*/
/*
/*--------------------------------------------------------------------------*/

GLboolean DXSound_stop (DXSound* sound)
{
	return GL_TRUE;
}

#endif // !BB_DISABLE_SOUNDS

/*--------------------------------------------------------------------------*/
/*
/*--------------------------------------------------------------------------*/

static GLint getCurrentTime ()
{
#if !defined(_WIN32_WCE)
	LARGE_INTEGER	currentTime;

	QueryPerformanceCounter(&currentTime);
	if (!g_timeInited)
	{
		QueryPerformanceFrequency(&g_timeFreq);
		g_timeStart = currentTime;
		g_timeInited = GL_TRUE;
	}

	return (GLint)((currentTime.QuadPart - g_timeStart.QuadPart) * 1000.0 / g_timeFreq.QuadPart);
#else
	return (GLint)GetTickCount();
#endif
}

/*--------------------------------------------------------------------------*/
/*
/*--------------------------------------------------------------------------*/

static GLint translateKey (GLint key)
{
	switch (key)
	{
	case VK_UP:
		return BB_KEYCODE_UP_ARROW;
	case VK_DOWN:
		return BB_KEYCODE_DOWN_ARROW;
	case VK_LEFT:
		return BB_KEYCODE_LEFT_ARROW;
	case VK_RIGHT:
		return BB_KEYCODE_RIGHT_ARROW;

	case VK_ESCAPE:
		return BB_KEYCODE_ESCAPE;

	case VK_F2:
		return BB_KEYCODE_START_RECORD;
	case VK_F3:
		return BB_KEYCODE_STOP_RECORD;
	case VK_F4:
		return BB_KEYCODE_TOGGLE_PLAYBACK;

	case VK_F5:
		return BB_KEYCODE_TOGGLE_CAPTURE;

	case VK_RETURN:
		return BB_KEYCODE_LAUNCH_BALL;
	case 'C':
		return BB_KEYCODE_CHANGE_CAMERA;

	case 'A':
	case VK_LSHIFT:
		return BB_KEYCODE_LEFT_RACKET;
	case 'L':
	case VK_RSHIFT:
		return BB_KEYCODE_RIGHT_RACKET;

	case VK_SPACE:
		return BB_KEYCODE_NUDGE;

	default:
		return 0;
	}
}

/*--------------------------------------------------------------------------*/
/*
/*--------------------------------------------------------------------------*/

LONG WINAPI mainWndProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	//Call this always, adopter needs to do some tricks
	HandleWindowMessages(hWnd, uMsg, wParam, lParam, g_RenderingSurface);

	switch (uMsg)
	{
		case WM_KEYUP:
		case WM_SYSKEYUP:
			{
				GLint tkey = translateKey((GLint)wParam);
				if (tkey)
				{
					BBPinball_keyEvent(g_pinball, GL_FALSE, tkey);
					return 0;
				}
				break;
			}
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			{
				GLint tkey = translateKey((GLint)wParam);
				if (tkey)
				{
                    BBPinball_keyEvent(g_pinball, GL_TRUE, tkey);
					return 0;
				}
				else if (wParam == VK_F12)
				{
					g_resolutionIndex = (g_resolutionIndex + 1) % MAX_RESOLUTIONS;
					g_reload = GL_TRUE;
					return 0;
				}
				break;
			}

#if !defined(_WIN32_WCE)
		case WM_LBUTTONDOWN:
			BBPinball_keyEvent(g_pinball, GL_TRUE, BB_KEYCODE_LEFT_MOUSE);
			return 0;
		case WM_LBUTTONUP:
			BBPinball_keyEvent(g_pinball, GL_FALSE, BB_KEYCODE_LEFT_MOUSE);
			return 0;
		case WM_MBUTTONDOWN:
			BBPinball_keyEvent(g_pinball, GL_TRUE, BB_KEYCODE_MIDDLE_MOUSE);
			return 0;
		case WM_MBUTTONUP:
			BBPinball_keyEvent(g_pinball, GL_FALSE, BB_KEYCODE_MIDDLE_MOUSE);
			return 0;
		case WM_RBUTTONDOWN:
			BBPinball_keyEvent(g_pinball, GL_TRUE, BB_KEYCODE_RIGHT_MOUSE);
			return 0;
		case WM_RBUTTONUP:
			BBPinball_keyEvent(g_pinball, GL_FALSE, BB_KEYCODE_RIGHT_MOUSE);
			return 0;

		case WM_MOUSEWHEEL:
			g_mouseWheelValue += (short)HIWORD(wParam);
			while (g_mouseWheelValue >= 120)
			{
				BBPinball_keyEvent(g_pinball, GL_TRUE, BB_KEYCODE_WHEEL_FORWARD);
				BBPinball_keyEvent(g_pinball, GL_FALSE, BB_KEYCODE_WHEEL_FORWARD);
				g_mouseWheelValue -= 120;
			}
			while (g_mouseWheelValue <= -120)
			{
				BBPinball_keyEvent(g_pinball, GL_TRUE, BB_KEYCODE_WHEEL_BACKWARD);
				BBPinball_keyEvent(g_pinball, GL_FALSE, BB_KEYCODE_WHEEL_BACKWARD);
				g_mouseWheelValue += 120;
			}
			return 0;

		case WM_MOUSEMOVE:
			BBPinball_ptrEvent(g_pinball, LOWORD(lParam), HIWORD(lParam));
			return 0;

		case WM_MOUSELEAVE:
			BBPinball_ptrEvent(g_pinball, -1, -1);
			return 0;
#endif

		case WM_CREATE:
		{
			int majorVersion, minorVersion;
			EGLint num_cfgs = 0;
			int attrib[] =
			{
#if 0
				EGL_RED_SIZE,   5,
				EGL_GREEN_SIZE, 5,
				EGL_BLUE_SIZE,  5,
                EGL_ALPHA_SIZE, 1,
				EGL_DEPTH_SIZE, 16,
				EGL_STENCIL_SIZE, 0,
#else
				EGL_RED_SIZE,   5,
				EGL_GREEN_SIZE, 6,
				EGL_BLUE_SIZE,  5,
                EGL_ALPHA_SIZE, 0,
				EGL_DEPTH_SIZE, 16,
				EGL_STENCIL_SIZE, 0,

#endif

/*				EGL_SAMPLE_BUFFERS, 1,
				EGL_SAMPLES, 16,
*/
				EGL_NONE
			};

			EGLint ContextAttribs[] =
			{
				EGL_CONTEXT_CLIENT_VERSION, 2,
				EGL_NONE
			};

			g_hDC = GetDC(hWnd);

			g_display = eglGetDisplay(GetDisplayType());
			if (!g_display)
				return EGL_FALSE;

			eglBindAPI(EGL_OPENGL_ES_API);

			if (!eglInitialize(g_display, &majorVersion, &minorVersion))
				return EGL_FALSE;

			if (!eglChooseConfig(g_display, attrib, &g_config, 1, &num_cfgs ) || (num_cfgs < 1))
				return EGL_FALSE;

			g_mainContext = eglCreateContext(g_display, g_config, 0, ContextAttribs);
			g_RenderingSurface = CreateRenderingSurface(hWnd, 480, 480); 
			g_mainSurface = eglCreateWindowSurface(g_display, g_config, (EGLNativeWindowType)GetSurface(g_RenderingSurface), 0);

			eglMakeCurrent(g_display, g_mainSurface, g_mainSurface, g_mainContext);

/*			glEnable(GL_MULTISAMPLE);
			glSampleCoverage(0, GL_FALSE);
*/			return 0;
		}

		case WM_CLOSE:
            PostQuitMessage(0);
			return 0;

#if !defined(_WIN32_WCE)
		case WM_SIZE:
			return 0;

		case WM_PAINT:
			ValidateRect(hWnd, NULL);
			return 0;

		case WM_DESTROY:
			return 0;
#endif
	}
	return (LONG)DefWindowProc(hWnd, uMsg, wParam, lParam);	
}

/*--------------------------------------------------------------------------*/
/*
/*--------------------------------------------------------------------------*/

#define APP_NAME "AMD Pinball"

static HWND initializeWindow (HINSTANCE hInstance, int nCmdShow)
{
	HWND		hWnd;
	//int			wWidth;
	//int			wHeight;
    RECT      rect;    

	/* Get actual window size */
	//wWidth = GetSystemMetrics(SM_CXDLGFRAME) * 2 + g_surfaceWidth;
	//wHeight = GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYDLGFRAME) * 2 + g_surfaceHeight;
    rect.left   = 0;
    rect.top    = 0;
    rect.right  = g_surfaceWidth;
    rect.bottom = g_surfaceHeight;

#if !defined(_WIN32_WCE)
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, FALSE);
#else
    AdjustWindowRectEx(&rect, WS_OVERLAPPED | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, FALSE, 0);
#endif

    AdjustedWidth  = rect.right - rect.left;
    AdjustedHeight = rect.bottom - rect.top;


	/* Register the frame class */
//	g_wc.cbSize		 = sizeof(g_wc);
	g_wc.style		 = CS_HREDRAW | CS_VREDRAW;
#if !defined(_WIN32_WCE)
	g_wc.style		 |= CS_OWNDC;
#endif
	g_wc.lpfnWndProc	 = (WNDPROC)mainWndProc;
	g_wc.cbClsExtra	 = 0;
	g_wc.cbWndExtra	 = 0;
	g_wc.hInstance	 = hInstance;
	g_wc.hIcon		 = NULL;//LoadIcon (hInstance, szAppName);
//	g_wc.hIconSm		 = NULL;
	g_wc.hCursor		 = LoadCursor(NULL,IDC_ARROW);
	g_wc.hbrBackground = NULL;//(HBRUSH)(COLOR_WINDOW+1);
#if !defined(_WIN32_WCE)
	g_wc.lpszMenuName  = APP_NAME;
#else
    g_wc.lpszMenuName  = NULL;
#endif
	g_wc.lpszClassName = TEXT(APP_NAME);	

	if (!RegisterClass(&g_wc))
		return NULL;

#if !defined(_WIN32_WCE)
	hWnd = CreateWindow(TEXT(APP_NAME), TEXT(APP_NAME),
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
		0, 0, AdjustedWidth, AdjustedHeight, NULL, NULL, hInstance, NULL);
#else
	hWnd = CreateWindow(TEXT(APP_NAME), TEXT(APP_NAME),
        WS_OVERLAPPED | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		0, 0, AdjustedWidth, AdjustedHeight, NULL, NULL, hInstance, NULL);
#endif

	/* Make sure the window was created */
	if (!hWnd)
		return NULL;

	ShowWindow(hWnd, nCmdShow);
	while (	ShowCursor(FALSE) >= 0 );
	UpdateWindow(hWnd);
	InvalidateRgn(hWnd, NULL, TRUE);
	return hWnd;
}

/*--------------------------------------------------------------------------*/
/*
/*--------------------------------------------------------------------------*/

static void deinitWindow ()
{
	eglMakeCurrent(g_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	eglDestroyContext(g_display, g_mainContext);
	eglDestroySurface(g_display, g_mainSurface);
	eglTerminate(g_display);
	g_display = EGL_NO_DISPLAY;
	g_config = 0;
	g_mainContext = EGL_NO_CONTEXT;
	g_mainSurface = EGL_NO_SURFACE;

	if (g_hDC)
		ReleaseDC(g_hWnd, g_hDC);

	UnregisterClass(TEXT(APP_NAME), GetModuleHandle(NULL));
	
	ReleaseAllSurfaces(g_RenderingSurface);

//	DestroyWindow(g_hWnd);
	g_hDC = NULL;
	g_hWnd = NULL;
}

/*--------------------------------------------------------------------------*/
/*
/*--------------------------------------------------------------------------*/

static void messagePump ()
{
	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
//	Sleep(1);
}

/*--------------------------------------------------------------------------*/
/*
/*--------------------------------------------------------------------------*/

#ifndef BB_DISABLE_SOUNDS
static GLuint loadSound (const char* filename)
{
	DXSound* sound = NULL;

	if (g_soundCount >= 256)
		return BB_SOUND_NONE;

	sound = DXSound_create(filename);
	if (!sound)
		return BB_SOUND_NONE;

	g_soundList[g_soundCount] = sound;
	g_soundCount++;

	return g_soundCount;
}

/*--------------------------------------------------------------------------*/
/*
/*--------------------------------------------------------------------------*/

static void freeSound (GLuint sound)
{
	if (sound == BB_SOUND_NONE || sound > MAX_SOUNDS)
		return;

	sound--;

	if (!g_soundList[sound])
		return;

	DXSound_destroy(g_soundList[sound]);
	g_soundList[sound] = NULL;
}

/*--------------------------------------------------------------------------*/
/*
/*--------------------------------------------------------------------------*/

static GLuint playSound (GLuint sound, GLfloat volume, GLboolean repeat)
{
	if (sound == BB_SOUND_NONE || sound > MAX_SOUNDS)
		return BB_SOUND_ERR_INVALID;

	sound--;

	if (!g_soundList[sound])
		return BB_SOUND_ERR_INVALID;

	return DXSound_play(g_soundList[sound], volume, repeat);
}

/*--------------------------------------------------------------------------*/
/*
/*--------------------------------------------------------------------------*/

static GLuint stopSound (GLuint sound)
{
	if (sound == BB_SOUND_NONE || sound > MAX_SOUNDS)
		return BB_SOUND_ERR_INVALID;

	sound--;

	if (!g_soundList[sound])
		return BB_SOUND_ERR_INVALID;

	return DXSound_stop(g_soundList[sound]);
}

/*--------------------------------------------------------------------------*/
/*
/*--------------------------------------------------------------------------*/

static int initDirectSound (DWORD coopLevel)
{
    LPDIRECTSOUNDBUFFER pDSBPrimary = NULL;
    DSBUFFERDESC dsbd;
    WAVEFORMATEX wfx;

	const unsigned int PRIMARY_CHANNELS = 2;
	const unsigned int PRIMARY_FREQ = 44100;
	const unsigned int PRIMARY_BITRATE = 16;

	if (g_ds)
		return 3;

	// Create IDirectSound using the primary sound device
	if (FAILED(DirectSoundCreate8(NULL, &g_ds, NULL)))
		return 1;

	// Set DirectSound coop level
	if (FAILED(IDirectSound8_SetCooperativeLevel(g_ds, g_hWnd, coopLevel)))
		return 2;

	// Get the primary buffer
	memset(&dsbd, 0, sizeof(DSBUFFERDESC));
	dsbd.dwSize        = sizeof(DSBUFFERDESC);
	dsbd.dwFlags       = DSBCAPS_PRIMARYBUFFER;
	dsbd.dwBufferBytes = 0;
	dsbd.lpwfxFormat   = NULL;

	if(FAILED(IDirectSound8_CreateSoundBuffer(g_ds, &dsbd, &pDSBPrimary, NULL)))
		return 4;

	memset(&wfx, 0, sizeof(WAVEFORMATEX));
	wfx.wFormatTag      = (WORD) WAVE_FORMAT_PCM;
	wfx.nChannels       = (WORD) PRIMARY_CHANNELS;
	wfx.nSamplesPerSec  = (DWORD) PRIMARY_FREQ;
	wfx.wBitsPerSample  = (WORD) PRIMARY_BITRATE;
	wfx.nBlockAlign     = (WORD) (wfx.wBitsPerSample / 8 * wfx.nChannels);
	wfx.nAvgBytesPerSec = (DWORD) (wfx.nSamplesPerSec * wfx.nBlockAlign);

	if (FAILED(IDirectSoundBuffer8_SetFormat(pDSBPrimary, &wfx)))
		return 5;

	if (pDSBPrimary)
		IDirectSoundBuffer8_Release(pDSBPrimary);

	return 0;
}

/*--------------------------------------------------------------------------*/
/*
/*--------------------------------------------------------------------------*/

static void deinitDirectSound ()
{
	if (g_ds)
	{
		IDirectSound8_Release(g_ds);
		g_ds = NULL;
	}
}

#endif // !BB_DISABLE_SOUNDS

/*--------------------------------------------------------------------------*/
/*
/*--------------------------------------------------------------------------*/

static GLint initMain (int argc, char* argv[])
{
	g_hWnd = initializeWindow(GetModuleHandle(NULL), SW_SHOW);
	if (!g_hWnd)
	{
		MessageBox(NULL, TEXT("Unable to create window, quitting"), TEXT("Fatal error"),
			MB_OK | MB_SETFOREGROUND | MB_ICONSTOP);
		return 1;
	}

#ifndef BB_DISABLE_SOUNDS
	if (initDirectSound(DSSCL_PRIORITY))
	{
		MessageBox(NULL, "Unable to create direct sound, quitting", "Fatal error",
			MB_OK | MB_SETFOREGROUND | MB_ICONSTOP);
		return 2;
	}

	memset(&g_soundList, 0, sizeof(g_soundList));

	g_sound.load = loadSound;
	g_sound.free = freeSound;
	g_sound.play = playSound;
	g_sound.stop = stopSound;
#endif // !BB_DISABLE_SOUNDS

	/* Create pinball class */

	{
		BBPinballCreateParams params;

		memset(&params, 0, sizeof(params));

		params.getTime			= getCurrentTime;

		params.display			= g_display;
		params.config			= g_config;;
		params.mainContext		= g_mainContext;
		params.mainSurface		= g_mainSurface;

		params.surfaceWidth		= g_surfaceWidth;
		params.surfaceHeight	= g_surfaceHeight;

		bbParseParameters(&params,argc,argv);

		g_pinball = BBPinball_create(&params);
		if (!g_pinball)
		{
			MessageBox(NULL, TEXT("Unable to create pinball, quitting"), TEXT("Fatal error"),
				MB_OK | MB_SETFOREGROUND | MB_ICONSTOP);
			return 3;
		}
	}

	return 0;
}

static void deinitMain ()
{
	BBPinball_destroy(g_pinball);
	g_pinball = NULL;

#ifndef BB_DISABLE_SOUNDS
	{
		GLuint i;
		for (i = 0; i < MAX_SOUNDS; i++)
		{
			if (g_soundList[i])
			{
				DXSound_destroy(g_soundList[i]);
				g_soundList[i] = NULL;
			}
		}
	}

	deinitDirectSound();
#endif // !BB_DISABLE_SOUNDS

	deinitWindow();
}

//int main (int argc, char* argv[])
int WINAPI WinMain(HINSTANCE    hInstance,
                   HINSTANCE    hPrevInstance,
#if defined(_WIN32_WCE)
                   LPWSTR       lpCmdLine,
#else
                   LPSTR        lpCmdLine,
#endif
                   int          nCmdShow)
{
    int argc = 1;
    char* argv[] = { "bbPinball.exe" };
    
	GLsizei i;

#if !defined(_WIN32_WCE)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
//	_CrtSetBreakAlloc(53);
#endif

	for (i = 0; i < MAX_RESOLUTIONS; i++)
	{
		g_resolutionList[i * 2] = 320;
		g_resolutionList[i * 2 + 1] = 240;
	}

	{
		FILE* f = BB_FOPEN("../bbPinball/data/config.txt", "rb");
		if (f)
		{
			char buf[256];

			for (i = 0; i < MAX_RESOLUTIONS; i++)
			{
				fgets(buf, 256, f);
				g_resolutionList[i * 2] = atoi(buf);

				fgets(buf, 256, f);
				g_resolutionList[i * 2 + 1] = atoi(buf);
			}

			fclose(f);
		}
	}

	g_surfaceWidth = g_resolutionList[0];
	g_surfaceHeight = g_resolutionList[1];

	srand(GetTickCount());

	if (initMain(argc,argv))
		return 0;

	/* Run */

	while (1)
	{
		if (BBPinball_main(g_pinball))
			break;

		messagePump();

		if (g_reload)
		{
			deinitMain();
			g_surfaceWidth = g_resolutionList[g_resolutionIndex * 2];
			g_surfaceHeight = g_resolutionList[g_resolutionIndex * 2 + 1];
			if (initMain(argc,argv) != 0)
				break;
			g_reload = GL_FALSE;
		}
	}

	/* Destroy */
	deinitMain();
	return 0;
}

void BBPinball_bglSwapBuffer(EGLDisplay dpy, EGLSurface surface)
{
	eglSwapBuffers(dpy, surface);
	FlipSurface(g_RenderingSurface);
}