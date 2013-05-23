/*--------------------------------------------------------------------------*/
/*	bbPinball.h                                                             */
/*	Author:			Aarni Gratseff / Rocket                                 */
/*	Created:		16/01/2006                                              */
/*	Description:	Pinball main header                                     */
/*--------------------------------------------------------------------------*/

#ifndef BBPINBALL_H
#define BBPINBALL_H

#include "bbPinballDefines.h"

/*--------------------------------------------------------------------------*/
/* Callback function definitions                                            */
/*--------------------------------------------------------------------------*/

typedef	GLint (*GetTimeFunc)	();

#ifndef BB_DISABLE_SOUNDS
/*--------------------------------------------------------------------------*/
/* Sound interface                                                          */
/*--------------------------------------------------------------------------*/

#define BB_SOUND_NONE			0

typedef enum
{
	BB_SOUND_ERR_INVALID = 1,
	BB_SOUND_ERR_FILE_NOT_FOUND,
	BB_SOUND_ERR_OUT_OF_MEMORY,

	BB_SOUND_ERR_MAX
} BBSoundError;

typedef GLuint	(*LoadSoundFunc)	(const char* filename);
typedef void	(*FreeSoundFunc)	(GLuint sound);

typedef GLuint	(*PlaySoundFunc)	(GLuint sound, GLfloat volume, GLboolean repeat);
typedef GLuint	(*StopSoundFunc)	(GLuint sound);

typedef struct BBSoundInterface_s
{
	LoadSoundFunc	load;
	FreeSoundFunc	free;

	PlaySoundFunc	play;
	StopSoundFunc	stop;
} BBSoundInterface;

#endif // !BB_DISABLE_SOUNDS

/*--------------------------------------------------------------------------*/
/* Key codes enumeration                                                    */
/*--------------------------------------------------------------------------*/

typedef enum
{
	BB_KEYCODE_LEFT_MOUSE = 1,
	BB_KEYCODE_MIDDLE_MOUSE,
	BB_KEYCODE_RIGHT_MOUSE,
	BB_KEYCODE_WHEEL_FORWARD,
	BB_KEYCODE_WHEEL_BACKWARD,
	BB_KEYCODE_UP_ARROW,
	BB_KEYCODE_DOWN_ARROW,
	BB_KEYCODE_LEFT_ARROW,
	BB_KEYCODE_RIGHT_ARROW,

	BB_KEYCODE_LAUNCH_BALL,
	BB_KEYCODE_CHANGE_CAMERA,
	BB_KEYCODE_LEFT_RACKET,
	BB_KEYCODE_RIGHT_RACKET,
	BB_KEYCODE_NUDGE,

	BB_KEYCODE_TOGGLE_CAPTURE,

	BB_KEYCODE_START_RECORD,
	BB_KEYCODE_STOP_RECORD,

	BB_KEYCODE_TOGGLE_PLAYBACK,

	BB_KEYCODE_ESCAPE,
	BB_KEYCODE_MAX
} BBKeyCode;

/*--------------------------------------------------------------------------*/
/* Pinball class creation parameters struct                                 */
/*--------------------------------------------------------------------------*/

typedef struct BBPinballCreateParams_s
{
	GetTimeFunc		getTime;

	EGLDisplay	display;
	EGLConfig	config;
	EGLContext	mainContext;
	EGLSurface	mainSurface;

	GLuint		surfaceWidth;
	GLuint		surfaceHeight;

	int			singleFrame;
	int			maxFrames;
	int			hideHUD;
	char*		capturePrefix;
	int			startLodLevel;
	int			useVBO;
	int			useIBO;

#ifndef BB_DISABLE_SOUNDS
	BBSoundInterface*	sound;
#endif // !BB_DISABLE_SOUNDS
} BBPinballCreateParams;

/*--------------------------------------------------------------------------*/
/* Forward declarations                                                     */
/*--------------------------------------------------------------------------*/

typedef struct BBPinball_s BBPinball;

/*--------------------------------------------------------------------------*/
/* BBPinball public methods                                                 */
/*--------------------------------------------------------------------------*/

BBPinball*		BBPinball_create	(BBPinballCreateParams* params);
void			BBPinball_destroy	(BBPinball* pinball);

GLboolean		BBPinball_main		(BBPinball* pinball);

void			BBPinball_keyEvent	(BBPinball* pinball, GLboolean down, GLint key);
void			BBPinball_ptrEvent	(BBPinball* pinball, GLint x, GLint y);

void			BBPinball_bglSwapBuffer	(EGLDisplay dpy, EGLSurface surface);

#endif /* BBPINBALL_H */
