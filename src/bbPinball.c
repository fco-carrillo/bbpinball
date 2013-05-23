/*--------------------------------------------------------------------------*/
/*  bbPinball.c                                                             */
/*  Author:         Aarni Gratseff / Rocket                                 */
/*  Created:        16/01/2006                                              */
/*  Description:    Pinball main                                            */
/*--------------------------------------------------------------------------*/

#include "bbPinball.h"
#include "bbGLUtils.h"
#include "bbMeshStore.h"
#include "bbMath.h"
#include "bbShaderStack.h"
#include "bbCollisionMap.h"
#include "bbTga.h"

#include <stdio.h>
#include <stdlib.h>
#ifdef __ARM__
#include <memory.h>
#include <string.h>
#endif


#define ENABLE_FPS_COUNTER
#define PRINTF_FPS

// enable USB
//#define USBBLIT 1

// USB
#ifdef USBBLIT
#include "usbBlit.h"
#endif

/*--------------------------------------------------------------------------*/
/* Constants                                                                */
/*--------------------------------------------------------------------------*/

/* DEBUG */

//#define SHOW_DEBUG_TRIANGLE

#ifdef SHOW_DEBUG_TRIANGLE

const GLsizei BB_PINBALL_DEBUG_TRIANGLE_COORD_COUNT = 15;
const GLfloat BB_PINBALL_DEBUG_TRIANGLE_COORDS[15] = { 0.0f, 0.0f, 0.0f,
                            -2.0f, 2.0f, 0.0f, 2.0f, 2.0f, 0.0f,
                            0.0f, 2.0f, -2.0f, 0.0f, 2.0f, 2.0f };
const GLsizei BB_PINBALL_DEBUG_TRIANGLE_INDEX_COUNT = 6;
const GLuint BB_PINBALL_DEBUG_TRIANGLE_INDICES[6] = { 0, 1, 2, 0, 3, 4 };

#endif

/* DEBUG */

const GLfloat BB_PINBALL_FOV = 60.0f;

const GLfloat BB_PINBALL_BUMPER_POWER = 0.1f;
const GLfloat BB_PINBALL_LAUNCH_POWER = 0.5f;

const GLint BB_PINBALL_UPDATE_FREQ = 5;
const GLfloat BB_PINBALL_GRAVITY_ACC = 0.0006f;
const GLfloat BB_PINBALL_TABLE_SURFACE_HEIGHT = 51.202f + 0.05f;
const GLfloat BB_PINBALL_TABLE_SURFACE_REFLECTION_HEIGHT = 51.202f;

const char* BB_PINBALL_CAPTURE_DIR = "../bbPinball/capture/";
const GLint BB_PINBALL_CAPTURE_FRAME_MS = 33;

const GLfloat BB_PINBALL_BALL_RADIUS = 0.9f;
const GLsizei BB_PINBALL_MAX_BALL_COUNT = 4;

#define BB_PINBALL_MAX_PLAYERS          (4)
#define BB_PINBALL_HIGHSCORE_COUNT      (10)

const char* BB_PINBALL_HIGHSCORES_FILENAME = "../bbPinball/data/highscores.txt";

const GLfloat BB_PINBALL_CAMERA_CENTER_Y = 55.0f;
const GLfloat BB_PINBALL_CAMERA_CENTER_Z = 50.0f;

const GLfloat BB_PINBALL_UPPERLEFT_RACKET_POS[] = { 11.6f, 44.5f };
const GLfloat BB_PINBALL_UPPERRIGHT_RACKET_POS[] = { -8.5f, 44.5f };
const GLfloat BB_PINBALL_LOWERLEFT_RACKET_POS[] = { 8.5f, 71.8f };
const GLfloat BB_PINBALL_LOWERRIGHT_RACKET_POS[] = { -5.3f, 71.7f };

const GLfloat BB_PINBALL_LOWER_RACKET_FACING_DIR = BB_PI * 1.7f;
const GLfloat BB_PINBALL_UPPER_RACKET_FACING_DIR = BB_PI * 1.7f;

const GLfloat BB_PINBALL_RACKET_TURN_SPEED = 0.1f;
const GLfloat BB_PINBALL_UPPERRACKET_MAX_ANGLE = BB_PI * 0.25f;
const GLfloat BB_PINBALL_LOWERRACKET_MAX_ANGLE = BB_PI * 0.33f;
const GLfloat BB_PINBALL_RACKET_POWER_MULTIPLIER = 1.2f;

const GLfloat BB_PINBALL_COLLISION_MAP_AREA_TOPLEFT[] = { -18.0f, 86.8f };
const GLfloat BB_PINBALL_COLLISION_MAP_AREA_BOTTOMRIGHT[] = { 18.0f, 18.8f };
const GLfloat BB_PINBALL_BALL_SLOT_Y = 85.0f;

const GLfloat BB_PINBALL_BALL_START_POS[] = { -16.0f, 84.0f };

const GLsizei BB_PINBALL_LOWERRIGHT_RACKET_COLLISION_POINT_COUNT = 8;
const GLfloat BB_PINBALL_LOWERRIGHT_RACKET_COLLISION_POINTS[] = { -1.25f, 74.39f, -1.17f, 74.8f,
                                                                -1.445f, 75.18f, -1.9f, 75.2f,
                                                                -5.73f, 72.5f, -6.0f, 71.8f,
                                                                -5.35f, 71.08f, -4.73f, 71.23f };

const GLsizei BB_PINBALL_LOWERLEFT_RACKET_COLLISION_POINT_COUNT = 8;
const GLfloat BB_PINBALL_LOWERLEFT_RACKET_COLLISION_POINTS[] = { 5.32f, 75.17f, 4.81f, 75.23f,
                                                                4.46f, 74.79f, 4.64f, 74.28f,
                                                                8.03f, 71.21f, 8.83f, 71.09f,
                                                                9.33f, 71.74f, 9.07f, 72.48f };

const GLsizei BB_PINBALL_UPPERRIGHT_RACKET_COLLISION_POINT_COUNT = 8;
const GLfloat BB_PINBALL_UPPERRIGHT_RACKET_COLLISION_POINTS[] = { -5.228f, 47.303f, -5.132f, 47.762f,
                                                                -5.514f, 48.173f, -5.983f, 48.02f,
                                                                -9.061f, 44.98f, -9.195f, 44.311f,
                                                                -8.612f, 43.775f, -7.923f, 43.957f };

const GLsizei BB_PINBALL_UPPERLEFT_RACKET_COLLISION_POINT_COUNT = 8;
const GLfloat BB_PINBALL_UPPERLEFT_RACKET_COLLISION_POINTS[] = { 9.12f, 48.03f, 8.65f, 48.17f,
                                                        8.24f, 47.81f, 8.32f, 47.34f,
                                                        11.02f, 43.98f, 11.76f, 43.78f,
                                                        12.30f, 44.28f, 12.15f, 45.01f };

const GLfloat BB_PINBALL_LOWERRIGHT_RACKET_AREA[] = { -8.0f, 77.0f, 1.0f, 65.0f };
const GLfloat BB_PINBALL_LOWERLEFT_RACKET_AREA[] = { 2.5f, 78.0f, 11.0f, 65.0f };
const GLfloat BB_PINBALL_UPPERRIGHT_RACKET_AREA[] = { -11.0f, 50.0f, -3.25f, 41.4f };
const GLfloat BB_PINBALL_UPPERLEFT_RACKET_AREA[] = { 5.75f, 50.18f, 14.10f, 38.27f };

const GLsizei BB_PINBALL_BALL_DOOR_POINT_COUNT = 2;
const GLfloat BB_PINBALL_BALL_DOOR_POINTS[] = { -16.8f, 50.0f, -14.2f, 52.8f };

const GLfloat BB_PINBALL_BALL_DOOR_MAX_ANGLE = -BB_PI / 4.0f;

const GLsizei BB_PINBALL_LIGHT_COUNT = 21;
const char* BB_PINBALL_LIGHT_MESH_NAMES[] = {
    "LockValo", "Nuolivalo01", "Nuolivalo02",
    "Nuolivalo03", "Nuolivalo04", "Nuolivalo05",
    "Oikea150Valo", "Oikea300Valo", "Oikea500Valo",
    "Vasen150Valo", "Vasen300Valo", "Vasen500Valo",
    "X1Valo", "X2Valo", "X3Valo",
    "X4Valo", "X5Valo", "YlaSwitch01",
    "YlaSwitch02", "YlaSwitch03", "YlaSwitch04" };

const GLint BB_PINBALL_UPPER_LIGHTS_TURN_OFF_DELAY = 500;

/*const GLsizei BB_PINBALL_BUMPER_LIGHT_COUNT = 8;

const char* BB_PINBALL_BUMPER_LIGHT_NAMES[] =
{
    "o_valo_a_07", "o_valo_a_06", "o_valo_a_05", "o_valo_a_00",
    "o_valo_b_07", "o_valo_b_06", "o_valo_b_01", "o_valo_b_05"
};

const GLfloat BB_PINBALL_BUMPER_LIGHT_AREAS[] =
{
    -630.0f, 2700.0f, -210.0f, 1900.0f,
    -210.0f, 2700.0f, 195.0f, 1900.0f,
    195.0f, 2700.0f, 610.0f, 1900.0f,
    610.0f, 2650.0f, 1040.0f, 1900.0f,

    -1370.0f, -1370.0f, -1000.0f, -2000.0f,
    -990.0f, -1590.0f, -620.0f, -2190.0f,
    623.0f, -1525.0f, 1011.0f, -2126.0f,
    1017.0f, -1374.0f, 1385.0f, -1941.0f
};*/

const GLsizei BB_PINBALL_MESHSTORE2_COUNT = 7;
const char* BB_PINBALL_MESHSTORE2_NAMES[] =
{
    "Tuntematon01", "LockPohja", "LockBumper", "Sensorit",
    "Object_001", "Object_007", "YlempiAlusta"
};

/*
const char* BB_PINBALL_TRANSPARENT_MESH_NAMES[] =
{
    "Pleksi",
};
const GLsizei BB_PINBALL_TRANSPARENT_MESH_COUNT = sizeof(BB_PINBALL_TRANSPARENT_MESH_NAMES)/sizeof(BB_PINBALL_TRANSPARENT_MESH_NAMES[0]);
*/

const GLsizei BB_PINBALL_MIDDLEBUMPER_AREA_COUNT = 1;
const GLfloat BB_PINBALL_MIDDLEBUMPER_AREAS[] = { -2.609f, 50.25f, 5.5f, 44.0f };

const GLsizei BB_PINBALL_SIDEBUMPER_AREA_COUNT = 9;
const GLfloat BB_PINBALL_SIDEBUMPER_AREAS[] = { 
    -11.01f, 39.41f, -5.22f, 35.09f,
    -9.50f, 41.08f, -3.32f, 36.64f,
    -7.79f, 42.98f, -2.48f, 37.74f,

    5.21f, 43.18f, 9.78f, 39.42f,
    6.82f, 41.83f, 10.97f, 38.03f,
    9.14f, 39.61f, 13.77f, 35.97f,

    8.72f, 38.00f, 11.58f, 35.30f,
    9.65f, 41.16f, 12.90f, 38.93f,
    5.37f, 39.45f, 10.72f, 36.33f
};

const GLsizei BB_PINBALL_DROPTARGET_COUNT = 16;
const char* BB_PINBALL_DROPTARGET_NAMES[] = {
    "Drop01", "Drop02", "Drop03", "Drop04",
    "Drop05", "Drop06", "Drop07", "",
    "Drop08", "Drop09", "Drop10", "",
    "Drop11", "Drop12", "Drop13", "Drop14" };

const GLfloat BB_PINBALL_DROPTARGET_COLLISION_POINTS[] = {
    13.02f, 55.50f, 12.40f, 56.13f, 11.44f, 55.16f, 12.08f, 54.60f,
    11.97f, 54.25f, 11.29f, 54.95f, 10.39f, 54.04f, 11.04f, 53.43f,
    10.78f, 53.22f, 10.18f, 53.84f, 9.24f, 53.92f, 9.86f, 52.32f,
    9.59f, 52.16f, 9.08f, 52.75f, 8.13f, 51.83f, 8.71f, 51.20f,

    3.63f, 51.86f, 4.31f, 52.15f, 3.85f, 53.26f, 3.16f, 52.98f,
    3.13f, 53.26f, 3.74f, 53.47f, 3.24f, 54.71f, 2.60f, 54.43f,
    2.49f, 53.74f, 3.15f, 54.93f, 2.65f, 56.17f, 2.00f, 55.93f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,

    1.04f, 55.85f, 0.43f, 56.09f, -0.09f, 54.87f, 0.60f, 54.60f,
    0.49f, 54.34f, -0.17f, 54.63f, -0.68f, 53.39f, -0.01f, 53.18f,
    -0.15f, 52.89f, -0.76f, 53.18f, -1.30f, 51.92f, -0.62f, 51.65f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,

    12.49f, 34.28f, 11.72f, 34.81f, 10.93f, 33.73f, 11.64f, 33.21f,
    11.44f, 32.97f, 10.77f, 33.54f, 9.96f, 32.47f, 10.66f, 32.01f,
    10.49f, 31.73f, 9.82f, 32.28f, 9.03f, 31.22f, 9.71f, 30.67f,
    9.55f, 30.46f, 8.88f, 31.03f, 8.08f, 29.95f, 8.76f, 29.42f
};

const GLfloat BB_PINBALL_DROPTARGET_SENSITIVITY = 0.06f;

const GLsizei BB_PINBALL_DROPTARGET_GROUP_SIZE = 4;
const GLsizei BB_PINBALL_DROPTARGET_GROUP_COUNT = 4;
const GLint BB_PINBALL_DROPTARGET_GROUP_RAISE_DELAY = 2000;
const GLfloat BB_PINBALL_DROPTARGET_RAISE_TIME = 140.0f;

const GLfloat BB_PINBALL_DROPTARGET_GROUP_RECTS[] =
{
    6.38f, 57.00f, 14.22f, 50.00f,
    1.65f, 57.06f, 5.13f, 51.17f,
    -2.96f, 57.17f, 1.30f, 50.29f,
    5.77f, 36.74f, 14.17f, 28.21f
};

const GLsizei BB_PINBALL_SENSOR_COUNT = 13;

const GLfloat BB_PINBALL_SENSOR_RECTS[] = {
    2.63f, 28.44f, 4.18f, 25.71f,
    -0.72f, 27.64f, 0.47f, 25.03f,
    -4.19f, 27.99f, -2.94f, 25.55f,
    -7.72f, 29.72f, -6.43f, 27.40f,

    14.89f, 45.53f, 16.22f, 42.89f,
    14.85f, 53.77f, 16.26f, 50.95f,

    -13.50f, 53.15f, -10.70f, 51.86f,

    14.96f, 65.66f, 16.41f, 63.11f,
    12.68f, 65.88f, 14.03f, 63.40f,
    9.46f, 67.23f, 10.71f, 64.98f,

    -7.55f, 67.20f, -6.23f, 64.85f,
    -10.76f, 65.79f, -9.41f, 63.51f,
    -13.15f, 65.67f, -11.78f, 63.03f
};

const GLfloat BB_PINBALL_BALL_LOCK_POS[] = { -7.64f, 51.47f };
const GLfloat BB_PINBALL_BALL_LOCK_RADIUS = 1.0f;
const GLint BB_PINBALL_BALL_LOCK_DURATION = 1200;
const GLfloat BB_PINBALL_BALL_LOCK_DEPTH = 0.5f;

const char* BB_PINBALL_TOUCHIE_MESH_NAME = "Touchie";
const GLfloat BB_PINBALL_TOUCHIE_POS[] = { 1.56f, 57.36f };
const GLfloat BB_PINBALL_TOUCHIE_RADIUS = 1.0f;
const GLfloat BB_PINBALL_TOUCHIE_MOVE_ANGLE = -BB_PI / 2.0f;
const GLfloat BB_PINBALL_TOUCHIE_MAX_MOVE = 0.6f;
const GLfloat BB_PINBALL_TOUCHIE_MOVE_SPEED = 15.0f;

const GLuint BB_PINBALL_SCORE_BUMPER_MIDDLE = 200;
const GLuint BB_PINBALL_SCORE_BUMPER_SIDES = 100;

const GLuint BB_PINBALL_SCORE_DROPTARGET = 100;
const GLuint BB_PINBALL_SCORE_TOUCHIE = 300;
const GLuint BB_PINBALL_SCORE_LOCK = 400;
const GLuint BB_PINBALL_SCORE_LIGHT_SWITCH = 150;
const GLuint BB_PINBALL_SCORE_LEFT_RAMP = 300;
const GLuint BB_PINBALL_SCORE_RIGHT_RAMP = 300;

const char* BB_PINBALL_RECORD_FILENAME = "../bbPinball/data/record.dat";

const GLsizei BB_PINBALL_LED_NUMBER_COUNT = 7;
const char* BB_PINBALL_LED_MESH_NAMES[] =
{
    "o_d_1_1", "o_d_1_2", "o_d_1_3", "o_d_1_4", "o_d_1_5", "o_d_1_6", "o_d_1_7",
    "o_d_2_1", "o_d_2_2", "o_d_2_3", "o_d_2_4", "o_d_2_5", "o_d_2_6", "o_d_2_7",
    "o_d_3_1", "o_d_3_2", "o_d_3_3", "o_d_3_4", "o_d_3_5", "o_d_3_6", "o_d_3_7",
    "o_d_4_1", "o_d_4_2", "o_d_4_3", "o_d_4_4", "o_d_4_5", "o_d_4_6", "o_d_4_7"
};

const GLsizei BB_PINBALL_BOARD_OBJECTS_COUNT = 9;
const char* BB_PINBALL_BOARD_OBJECTS_NAMES[] =
{
    "TornienKatot", "Object_022", "KeskiTotsa", "OikeaTotsa", "VasenTotsa",
    "TapitBumpers", "Object_023", "Lautasivut", "Object_012"
};

const char* BB_PINBALL_OUTER_OBJECTS_NAMES[] = {
#if 0 // MIKEY_REFLECT_THE_WALLS
	"Object_005", // SeinaA == WallA
	"Object_006", // SeinaB == WallB
#endif
	"Paaty",
	"Taulu",
	"Paaty2"
};
const GLsizei BB_PINBALL_OUTER_OBJECTS_COUNT = sizeof(BB_PINBALL_OUTER_OBJECTS_NAMES)/sizeof(BB_PINBALL_OUTER_OBJECTS_NAMES[0]);

/*--------------------------------------------------------------------------*/
/* Ball structs                                                             */
/*--------------------------------------------------------------------------*/

typedef struct BBBall_s
{
    GLfloat pos[3];
    GLfloat lastPos[3];
    GLfloat vel[3];
} BBBall;

typedef struct BBBallCollision_s
{
    BBBall* ball;
    GLfloat vec[3];
} BBBallCollision;

/*--------------------------------------------------------------------------*/
/* Record structs                                                           */
/*--------------------------------------------------------------------------*/

#define BB_PINBALL_RECORD_VALUE_COUNT           (1 + 16 + 1 + 21 + 4 + 6 + 4 + 1)
#define BB_PINBALL_RECORD_CHUNK_FRAME_COUNT     8192

typedef struct BBRecordFrame_s
{
    GLfloat     values[BB_PINBALL_RECORD_VALUE_COUNT];
} BBRecordFrame;

typedef struct BBRecordChunk_s BBRecordChunk;

struct BBRecordChunk_s
{
    BBRecordFrame   frames[BB_PINBALL_RECORD_CHUNK_FRAME_COUNT];
    GLint           count;
    BBRecordChunk*  next;
};

BBRecordChunk*      BBRecordChunk_create        (void);
void                BBRecordChunk_destroy       (BBRecordChunk* chunk);

/*--------------------------------------------------------------------------*/
/*  Sound effects                                                           */
/*--------------------------------------------------------------------------*/

typedef enum
{
    BB_SFX_MENU_UP = 1,
    BB_SFX_MENU_DOWN,
    BB_SFX_MENU_SELECT,

    BB_SFX_BALL_LAUNCH,
    BB_SFX_RACKET1,
    BB_SFX_RACKET1B,
    BB_SFX_RACKET2,
    BB_SFX_RACKET2B,
    BB_SFX_DROP1,
    BB_SFX_DROPGROUPUP,
    BB_SFX_ELECTRIC1,
    BB_SFX_ELECTRIC2,
    BB_SFX_BUMPER1,
    BB_SFX_BUMPER2,
    BB_SFX_BUMPER3,
    BB_SFX_BUMPER4,
    BB_SFX_BALL_LOCK,
    BB_SFX_BALL_RELEASE,
    BB_SFX_TOUCHIE,

    BB_SFX_MUSIC,
    BB_SFX_MAX
} BBSoundEffect;

const char* const BB_SFX_FILENAMES[] =
{
    "menuUp.wav",
    "menuDown.wav",
    "menuSelect.wav",
    "ballLaunch.wav",
    "racket1.wav",
    "racket1b.wav",
    "racket2.wav",
    "racket2b.wav",
    "drop1.wav",
    "dropGroupUp.wav",
    "electric1.wav",
    "electric2.wav",
    "bumper1.wav",
    "bumper2.wav",
    "bumper3.wav",
    "bumper4.wav",
    "ballLock.wav",
    "ballRelease.wav",
    "touchie.wav",

    "Olof_Gustafsson-Weight_for_Rocket.wav",
};

/*--------------------------------------------------------------------------*/
/*  Player struct                                                           */
/*--------------------------------------------------------------------------*/

struct BBPlayer_s
{
    GLboolean               freeLaunch;
    GLuint                  ballsLeft;
    GLuint                  ballsLeftLastChange;
    GLuint                  score;
    GLboolean               highscoreChecked;

    GLboolean               cycleArrowLights;
    GLuint                  scoreMultiplier;

    GLboolean*              dropTargetDownList;
    GLint*                  dropTargetGroupDown;
    GLboolean*              lightOnList;

    GLint                   leftLightsOffCount;
    GLint                   rightLightsOffCount;
};

typedef struct BBPlayer_s BBPlayer;

static void BBPlayer_init                   (BBPlayer* player);
static void BBPlayer_defaultValues          (BBPlayer* player);

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

void BBPlayer_init (BBPlayer* player)
{
    BB_ASSERT(player);

    player->freeLaunch          = GL_FALSE;
    player->ballsLeft           = 3;
    player->ballsLeftLastChange = 0;
    player->score               = 0;
    player->highscoreChecked    = GL_FALSE;

    player->cycleArrowLights    = GL_FALSE;
    player->scoreMultiplier     = 1;

    player->dropTargetDownList  = NULL;
    player->dropTargetGroupDown = NULL;

    player->lightOnList         = NULL;

    player->leftLightsOffCount  = 0;
    player->rightLightsOffCount = 0;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

void BBPlayer_defaultValues (BBPlayer* player)
{
    GLint i;
    BB_ASSERT(player);

    player->freeLaunch          = GL_FALSE;
    player->ballsLeft           = 3;
    player->ballsLeftLastChange = 0;
    player->score               = 0;
    player->highscoreChecked    = GL_FALSE;

    player->cycleArrowLights    = GL_FALSE;
    player->scoreMultiplier     = 1;

    for (i = 0; i < BB_PINBALL_DROPTARGET_COUNT; i++)
        player->dropTargetDownList[i] = GL_FALSE;

    player->dropTargetDownList[7] = GL_TRUE;
    player->dropTargetDownList[11] = GL_TRUE;

    for (i = 0; i < BB_PINBALL_DROPTARGET_GROUP_COUNT; i++)
        player->dropTargetGroupDown[i] = (i == 1 || i == 2) ? 1 : 0;

    /* Lights */

    for (i = 0; i < BB_PINBALL_LIGHT_COUNT; i++)
        player->lightOnList[i] = GL_FALSE;

    player->lightOnList[0] = GL_TRUE; /* touchie */

    player->lightOnList[6] = GL_TRUE; /* right 150 */
    player->lightOnList[7] = GL_TRUE; /* right 300 */
    player->lightOnList[8] = GL_TRUE; /* right 500 */
    player->lightOnList[9] = GL_TRUE; /* left 150 */
    player->lightOnList[10] = GL_TRUE; /* left 300 */
    player->lightOnList[11] = GL_TRUE; /* left 500 */

    player->lightOnList[12] = GL_TRUE; /* 1x multiplier */

    player->leftLightsOffCount  = 0;
    player->rightLightsOffCount = 0;
}

/*--------------------------------------------------------------------------*/
/*  Highscore struct                                                        */
/*--------------------------------------------------------------------------*/

struct BBHighscore_s
{
    char name[4];
    GLuint score;
};

typedef struct BBHighscore_s BBHighscore;

/*--------------------------------------------------------------------------*/
/*  Pinball struct                                                          */
/*--------------------------------------------------------------------------*/

struct BBPinball_s
{
#ifdef USBBLIT
    USB*                    usb;
#endif

    BBPinballCreateParams   params;
    GLboolean               quit;

    GLint                   currentTime;
    GLint                   lastUpdate;
    GLint                   lastCameraUpdate;
    GLfloat                 timeDelta;

    int                     singleFrame;
    int                     maxFrames;
	int						hideHUD;
    char*                   capturePrefix;
    int                     startLodLevel;
    int                     useVBO;
    int                     useIBO;

    /* Gfx related */

    GLfloat                 outputMultiplier;

    BBShaderStack*          shaderStack;
    BBTextureStore*         texStore;
    BBMaterialStore*        matStore;

    BBMeshStore*            meshStore;
    BBMeshStore*            meshStore2;
//    BBMeshStore*            transparentMeshStore;
    BBMeshStore*            boardObjectStore;
    BBMeshStore*            outerObjectStore;

    BBMesh*                 voidGlass;
    BBMesh*                 glassMesh;
    BBMesh*                 boardMesh;

    BBMesh*                 ballMesh;
    BBMesh*                 ballShadowMesh;

    BBMesh*                 lowerRightRacketMesh;
    BBMesh*                 lowerLeftRacketMesh;
    BBMesh*                 upperRightRacketMesh;
    BBMesh*                 upperLeftRacketMesh;

    BBMesh*                 lowerRightRacketShadowMesh;
    BBMesh*                 lowerLeftRacketShadowMesh;
    BBMesh*                 upperRightRacketShadowMesh;
    BBMesh*                 upperLeftRacketShadowMesh;

    BBMesh*                 ballDoorMesh;
    GLfloat                 ballDoorAngle;

    BBMesh**                dropTargetMeshes;
    GLfloat*                dropTargetPosList;

    BBMesh*                 touchieMesh;
    GLfloat                 touchieMove;

    BBMesh**                lightMeshes;
    GLfloat*                lightAlphaList;

/*  BBMesh**                bumperLightMeshes;
    GLfloat*                bumperLightValues;*/

    BBMesh*                 hudMesh;
    BBMesh*                 smallFontMesh;
    BBMesh*                 textShadowMesh;
    BBMesh*                 startScreenMesh;
    BBMesh*                 startScreen2Mesh;
    BBMesh*                 startScreenBackMesh;

    BBMesh**                ledMeshes;
    GLint                   ledNumber;

    GLfloat                 hudPos[2];
    GLfloat                 hudScale[2];
    GLfloat                 hudTexPos[2];
    GLfloat                 hudTexScale[2];

    GLfloat                 hudColor[4];

    GLint                   menuFadeTime;
    GLfloat                 menuAlpha;
    GLint                   menuSelection;
    GLint                   menuInactivityTimer;
    GLboolean               menuInactive;

    GLint                   nextLightCycleIndex;
    GLint                   nextLightCycleTime;

    GLboolean               highscoreCheckRunning;
    GLboolean               highscoreCheckNext;
    GLint                   highscoreCheckCurrentPlayer;
    char                    highscoreCheckName[4];
    GLint                   highscoreCheckNamePos;
    GLint                   highscoreCheckScrollTime;

    GLint                   highscoreScrollTime;
    GLfloat                 highscoreScroll;
    BBHighscore             highscores[BB_PINBALL_HIGHSCORE_COUNT];

    GLfloat                 lookAt[3];
    GLfloat                 eye[3];
    GLfloat                 cameraDist;
    GLfloat                 horzAngle;
    GLfloat                 vertAngle;
    GLfloat                 viewMatrix[4 * 4];
    GLfloat                 projMatrix[4 * 4];

    GLboolean               ballCamera;
    GLboolean               scorePeek;

    /* DEBUG */
#ifdef SHOW_DEBUG_TRIANGLE
    BBMesh*                 debugTriangle;
    GLfloat                 debugPos[3];
#endif
    /* DEBUG */

    /* Temporary gfx vars */

    GLfloat                 lightCurrentAlpha;
    GLfloat                 currentTexBlendValue;
    GLfloat                 worldMatrix[4 * 4];

    /* Gfx & physics related */

    GLfloat                 lowerLeftRacketAngle;
    GLfloat                 lowerRightRacketAngle;
    GLfloat                 upperLeftRacketAngle;
    GLfloat                 upperRightRacketAngle;

    GLfloat                 lowerRightRacketMatrix[4 * 4];
    GLfloat                 lowerLeftRacketMatrix[4 * 4];
    GLfloat                 upperRightRacketMatrix[4 * 4];
    GLfloat                 upperLeftRacketMatrix[4 * 4];

    /* Physics & game related */

    BBCollisionMap*         collisionMap;

    GLint*                  dropTargetGroupRaiseTime;
    GLint                   turnUpperLightsOffTime;

    GLboolean*              sensorDownList;

    GLboolean               touchieDown;

    GLboolean               ballLocked;
    GLint                   ballReleaseTime;

    GLuint                  currentPlayerIndex;
    GLuint                  numPlayers;

    GLint                   showPlayerTime;

    BBPlayer                players[BB_PINBALL_MAX_PLAYERS];
    BBPlayer*               player;

    GLfloat                 lowerLeftRacketAngleVel;
    GLfloat                 lowerRightRacketAngleVel;
    GLfloat                 upperLeftRacketAngleVel;
    GLfloat                 upperRightRacketAngleVel;

    GLboolean               gameRunning;
    GLboolean               ballInPlay;
    GLint                   ballLaunchTime;
    BBBall                  ball;

//  GLsizei                 ballCollisionCount;
//  BBBallCollision*        ballCollisions;

    GLboolean               ballDoorClosed;

    GLfloat                 nudgeVel;

    /* Keys & interactivity */

    /* Pointer position */
    GLint                   pointerX;
    GLint                   pointerY;

    /* Pointer keys */
    GLboolean               leftDown;
    GLboolean               middleDown;
    GLboolean               rightDown;

    /* Arrow keys */
    GLboolean               moveBallUp;
    GLboolean               moveBallDown;
    GLboolean               moveBallLeft;
    GLboolean               moveBallRight;

    /* Nudge */
    GLboolean               nudge;

    /* Racket keys and angles */
    GLboolean               leftRacketKeyDown;
    GLboolean               rightRacketKeyDown;

    /* Screen capture */
    GLubyte*                captureBuffer;
    GLboolean               captureOn;
    GLint                   captureFrame;

    /* Record & playback */
    GLboolean               recordOn;
    GLboolean               playbackOn;
    GLint                   recordStartTime;
    GLint                   playbackStartTime;

    BBRecordChunk*          firstRecordChunk;
    BBRecordChunk*          currentRecordChunk;
    GLint                   currentRecordPos;
    GLint                   recordChunkCount;

#ifndef BB_DISABLE_SOUNDS
    BBSoundInterface        sound;

    GLuint                  sfxList[BB_SFX_MAX];
    int                     frameSFX;

    GLboolean               musicPlaying;
#endif // !BB_DISABLE_SOUNDS
};

/*--------------------------------------------------------------------------*/
/*  Pinball private methods                                                 */
/*--------------------------------------------------------------------------*/

static GLboolean    BBPinball_init                  (BBPinball* pinball);

static void         BBPinball_calculateRacketMatrix (BBPinball* pinball, GLfloat* matrix,
                                                     const GLfloat* pos, GLfloat angle);

static void         BBPinball_drawRacket            (BBPinball* pinball, BBMesh* mesh,
                                                     BBMesh* shadowMesh, const GLfloat* matrix,
                                                     GLboolean mirror);

static void         BBPinball_drawText              (BBPinball* pinball, const char* string, GLfloat x, GLfloat y, GLboolean shadow);
static void         BBPinball_draw                  (BBPinball* pinball);

static void         BBPinball_update                (BBPinball* pinball);

static void         BBPinball_updateCamera          (BBPinball* pinball);
/*static BBBall*      BBPinball_checkBallCollision    (BBPinball* pinball,
                                                     BBBall* current,
                                                     const GLfloat* pos);*/

static GLboolean    BBPinball_vectorCollision       (BBPinball* pinball, const GLfloat* coords, GLsizei count,
                                                    const GLfloat* matrix, const GLfloat* oldPoint,
                                                    const GLfloat* newPoint, GLfloat* normal, GLboolean close);

/*static GLboolean    BBPinball_convexVectorCollision (BBPinball* pinball, const GLfloat* coords, GLsizei count,
                                                    const GLfloat* matrix, const GLfloat* point, GLfloat* normal);*/

static GLboolean    BBPinball_lineCollision         (BBPinball* pinball,
                                                    const GLfloat* coords, GLsizei count, const GLfloat* matrix,
                                                    const GLfloat* point1, const GLfloat* point2,
                                                    GLfloat* normal, GLfloat* intersection);

static void         BBPinball_ballPhysics           (BBPinball* pinball, BBBall* ball);

static GLboolean    BBPinball_isInsideAreas         (BBPinball* pinball, const GLfloat* pos,
                                                     const GLfloat* areas, GLsizei count);

static void         BBPinball_scrollLights          (BBPinball* pinball, GLboolean right);

static GLboolean    BBPinball_changePlayer          (BBPinball* pinball, GLuint playerIndex);
static void         BBPinball_newGame               (BBPinball* pinball/*, GLsizei playerCount*/);
static void         BBPinball_endGame               (BBPinball* pinball, GLboolean doHighscoreCheck);

static GLboolean    BBPinball_storeFrame            (BBPinball* pinball);
static GLboolean    BBPinball_playbackFrame         (BBPinball* pinball, GLboolean reset);

static GLboolean    BBPinball_saveRecording         (BBPinball* pinball);
static GLboolean    BBPinball_loadRecording         (BBPinball* pinball);
static void         BBPinball_dumpRecording         (BBPinball* pinball);

static void         BBPinball_playSFX               (BBPinball* pinball, GLuint index, GLfloat volume, GLboolean repeat);

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

BBPinball* BBPinball_create (BBPinballCreateParams* params)
{
    BB_ASSERT(params);
    {
        GLuint i;

        BBPinball* pinball = malloc(sizeof(BBPinball));
        if (!pinball)
            return NULL;

#ifdef USBBLIT
        pinball->usb                    = NULL;
#endif

        pinball->params                 = *params;
        pinball->quit                   = GL_FALSE;

        pinball->currentTime            = -1;
        pinball->lastUpdate             = 0;
        pinball->lastCameraUpdate       = 0;
        pinball->timeDelta              = 0.0f;

        pinball->singleFrame            = params->singleFrame;
        pinball->maxFrames              = params->maxFrames;
		pinball->hideHUD				= params->hideHUD;
        pinball->capturePrefix          = params->capturePrefix;
        pinball->startLodLevel          = params->startLodLevel;
        pinball->useVBO                 = params->useVBO;
        pinball->useIBO                 = params->useIBO;

        /* Gfx related */

        pinball->outputMultiplier       = 1.0f;

        pinball->shaderStack            = NULL;
        pinball->texStore               = NULL;
        pinball->matStore               = NULL;

        pinball->meshStore              = NULL;
        pinball->meshStore2             = NULL;
//        pinball->transparentMeshStore   = NULL;
        pinball->boardObjectStore       = NULL;
        pinball->outerObjectStore       = NULL;

        pinball->voidGlass              = NULL;
        pinball->glassMesh              = NULL;
        pinball->boardMesh              = NULL;

        pinball->ballMesh               = NULL;
        pinball->ballShadowMesh         = NULL;

        pinball->lowerRightRacketMesh   = NULL;
        pinball->lowerLeftRacketMesh    = NULL;
        pinball->upperRightRacketMesh   = NULL;
        pinball->upperLeftRacketMesh    = NULL;

        pinball->lowerRightRacketShadowMesh = NULL;
        pinball->lowerLeftRacketShadowMesh  = NULL;
        pinball->upperRightRacketShadowMesh = NULL;
        pinball->upperLeftRacketShadowMesh  = NULL;

        pinball->ballDoorMesh           = NULL;
        pinball->ballDoorAngle          = BB_PINBALL_BALL_DOOR_MAX_ANGLE;

        pinball->dropTargetMeshes       = NULL;
        pinball->dropTargetPosList      = NULL;

        pinball->touchieMesh            = NULL;
        pinball->touchieMove            = 0.0f;

        pinball->lightMeshes            = NULL;
        pinball->lightAlphaList         = NULL;

/*      pinball->bumperLightMeshes      = NULL;
        pinball->bumperLightValues      = NULL;*/

        pinball->hudMesh                = NULL;
        pinball->smallFontMesh          = NULL;
        pinball->textShadowMesh         = NULL;
        pinball->startScreenMesh        = NULL;
        pinball->startScreen2Mesh       = NULL;
        pinball->startScreenBackMesh    = NULL;

        pinball->ledMeshes              = NULL;
        pinball->ledNumber              = 0;

        pinball->hudPos[0]              = 0.0f;
        pinball->hudPos[1]              = 0.0f;

        pinball->hudScale[0]            = 1.0f;
        pinball->hudScale[1]            = 1.0f;

        pinball->hudTexPos[0]           = 0.0f;
        pinball->hudTexPos[1]           = 0.0f;

        pinball->hudTexScale[0]         = 1.0f;
        pinball->hudTexScale[1]         = 1.0f;

        pinball->hudColor[0]            = 1.0f;
        pinball->hudColor[1]            = 1.0f;
        pinball->hudColor[2]            = 1.0f;
        pinball->hudColor[3]                = 1.0f;

        pinball->menuAlpha              = 1.0f;
        pinball->menuFadeTime           = 0;
        pinball->menuSelection          = 0;
        pinball->menuInactivityTimer    = 0;
        pinball->menuInactive           = GL_TRUE;

        pinball->nextLightCycleIndex    = 0;
        pinball->nextLightCycleTime     = 0;

        pinball->highscoreCheckRunning  = GL_FALSE;
        pinball->highscoreCheckNext     = GL_FALSE;
        pinball->highscoreCheckCurrentPlayer    = 0;
        pinball->highscoreCheckName[0]  = 'A';
        pinball->highscoreCheckName[1]  = 'M';
        pinball->highscoreCheckName[2]  = 'D';
        pinball->highscoreCheckName[3]  = '\0';
        pinball->highscoreCheckNamePos  = 0;
        pinball->highscoreCheckScrollTime = 0;

        pinball->highscoreScrollTime    = 0;
        pinball->highscoreScroll        = 0.0f;

        for (i = 0; i < BB_PINBALL_HIGHSCORE_COUNT; i++)
        {
            pinball->highscores[i].name[0] = 'A';
            pinball->highscores[i].name[1] = 'M';
            pinball->highscores[i].name[2] = 'D';
            pinball->highscores[i].name[3] = '\0';
            pinball->highscores[i].score = 1000 - i * 100;
        }

        pinball->lookAt[0]              = 0.0f;
        pinball->lookAt[1]              = BB_PINBALL_CAMERA_CENTER_Y;
        pinball->lookAt[2]              = BB_PINBALL_CAMERA_CENTER_Z;

        pinball->eye[0]                 = 0.0f;
        pinball->eye[1]                 = 0.0f;
        pinball->eye[2]                 = 0.0f;

        pinball->cameraDist             = 80.0f;
        pinball->horzAngle              = 0.0f;
        pinball->vertAngle              = 1.0f;

        bbMatrixIdentity(pinball->viewMatrix);
        bbMatrixIdentity(pinball->projMatrix);

        pinball->ballCamera             = GL_TRUE;
        pinball->scorePeek              = GL_FALSE;

/* DEBUG */
#ifdef SHOW_DEBUG_TRIANGLE
        pinball->debugTriangle          = NULL;
        pinball->debugPos[0]            = 0.0f;
        pinball->debugPos[1]            = 0.0f;
        pinball->debugPos[2]            = 0.0f;
#endif
/* DEBUG */

        /* Temporary gfx vars */

        pinball->lightCurrentAlpha      = 0.0f;
        pinball->currentTexBlendValue   = 0.0f;
        bbMatrixIdentity(pinball->worldMatrix);

        /* Gfx & physics related */

        pinball->lowerLeftRacketAngle       = 0.0f;
        pinball->lowerRightRacketAngle      = 0.0f;
        pinball->upperLeftRacketAngle       = 0.0f;
        pinball->upperRightRacketAngle      = 0.0f;

        bbMatrixIdentity(pinball->lowerRightRacketMatrix);
        bbMatrixIdentity(pinball->lowerLeftRacketMatrix);
        bbMatrixIdentity(pinball->upperRightRacketMatrix);
        bbMatrixIdentity(pinball->upperLeftRacketMatrix);

        /* Physics & game related */

        pinball->collisionMap           = NULL;

        pinball->dropTargetGroupRaiseTime   = NULL;
        pinball->turnUpperLightsOffTime = 0;

        pinball->sensorDownList         = NULL;

        pinball->touchieDown            = GL_FALSE;

        pinball->ballLocked             = GL_FALSE;
        pinball->ballReleaseTime        = 0;

        pinball->currentPlayerIndex     = 0;
        pinball->numPlayers             = 2;

        pinball->showPlayerTime         = 0;

        for (i = 0; i < BB_PINBALL_MAX_PLAYERS; i++)
            BBPlayer_init(&pinball->players[i]);

        pinball->player                 = &pinball->players[0];

        pinball->lowerLeftRacketAngleVel    = 0.0f;
        pinball->lowerRightRacketAngleVel   = 0.0f;
        pinball->upperLeftRacketAngleVel    = 0.0f;
        pinball->upperRightRacketAngleVel   = 0.0f;

        pinball->gameRunning            = GL_FALSE;
        pinball->ballInPlay             = GL_FALSE;
        pinball->ballLaunchTime         = 0;

        pinball->ball.pos[0]            = BB_PINBALL_BALL_START_POS[0];
        pinball->ball.pos[1]            = BB_PINBALL_BALL_START_POS[1];
        pinball->ball.pos[2]            = 0.0f;
        pinball->ball.vel[0]            = 0.0f;
        pinball->ball.vel[1]            = 0.0f;
        pinball->ball.vel[2]            = 0.0f;

//      pinball->ballCollisionCount     = 0;
//      pinball->ballCollisions         = NULL;

        pinball->ballDoorClosed         = GL_FALSE;

        pinball->nudgeVel               = 0.0f;

        /* Keys & interactivity */

        /* Pointer position */
        pinball->pointerX               = -1;
        pinball->pointerY               = -1;

        /* Pointer keys */
        pinball->leftDown               = GL_FALSE;
        pinball->middleDown             = GL_FALSE;
        pinball->rightDown              = GL_FALSE;

        /* Arrow keys */
        pinball->moveBallUp             = GL_FALSE;
        pinball->moveBallDown           = GL_FALSE;
        pinball->moveBallLeft           = GL_FALSE;
        pinball->moveBallRight          = GL_FALSE;

        /* Nudge */
        pinball->nudge                  = GL_FALSE;

        /* Racket keys and angles */
        pinball->leftRacketKeyDown      = GL_FALSE;
        pinball->rightRacketKeyDown     = GL_FALSE;

        /* Screen capture */
        pinball->captureBuffer          = NULL;
        pinball->captureOn              = pinball->capturePrefix ? GL_TRUE : GL_FALSE;
        pinball->captureFrame           = 0;

        pinball->recordOn               = GL_FALSE;
        pinball->playbackOn             = GL_TRUE;
        pinball->recordStartTime        = 0;
        pinball->playbackStartTime      = 0;

        pinball->firstRecordChunk       = NULL;
        pinball->currentRecordChunk     = NULL;
        pinball->currentRecordPos       = 0;
        pinball->recordChunkCount       = 0;

#ifndef BB_DISABLE_SOUNDS
        if (params->sound)
        {
            pinball->sound.load         = params->sound->load;
            pinball->sound.free         = params->sound->free;
            pinball->sound.play         = params->sound->play;
            pinball->sound.stop         = params->sound->stop;
        }
        else
        {
            memset(&pinball->sound, 0, sizeof(BBSoundInterface));
        }

        pinball->frameSFX               = -1;

        pinball->musicPlaying           = GL_FALSE;
#endif // !BB_DISABLE_SOUNDS
        if (!BBPinball_init(pinball))
        {
            BBPinball_destroy(pinball);
            return NULL;
        }

        return pinball;
    }
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

GLboolean BBPinball_init (BBPinball* pinball)
{
    GLint i;
    BB_ASSERT(pinball);

// USB
#ifdef USBBLIT
    pinball->usb = createUsbInterface(320 * 240);
    if (!pinball->usb)
        return GL_FALSE;
#endif

    /*--------------------------------------------------------------------------*/
    /* Gfx part                                                                 */
    /*--------------------------------------------------------------------------*/

    /* Shader stack creation */

    pinball->shaderStack = BBShaderStack_create();
    if (!pinball->shaderStack)
        return GL_FALSE;

    /* Texture store creation */

    pinball->texStore = BBTextureStore_create();
    if (!pinball->texStore)
        return GL_FALSE;

    /* Material store creation & loading */

    pinball->matStore = BBMaterialStore_create();
    if (!pinball->matStore)
        return GL_FALSE;

    if (!BBMaterialStore_load(pinball->matStore,
                            pinball->shaderStack,
                            pinball->texStore,
                            "../bbPinball/data/materials.txt",pinball->startLodLevel))
        return GL_FALSE;

    /* Mesh store creation & loading */

    pinball->meshStore = BBMeshStore_create(pinball->matStore);
    if (!pinball->meshStore)
        return GL_FALSE;

    if (!BBMeshStore_load(pinball->meshStore, "../bbPinball/meshes/uus.msh",pinball->useVBO,pinball->useIBO))
        return GL_FALSE;

    /* Create mesh store for meshStore 2. meshes */

    pinball->meshStore2 = BBMeshStore_create(pinball->matStore);
    if (!pinball->meshStore2)
        return GL_FALSE;

    for (i = 0; i < BB_PINBALL_MESHSTORE2_COUNT; i++)
        if (!BBMeshStore_exchangeByName(pinball->meshStore, pinball->meshStore2, BB_PINBALL_MESHSTORE2_NAMES[i]))
            return GL_FALSE;

    /* Create mesh store for transparent meshes and exchange transparent meshes from earlier created store */

/*
    pinball->transparentMeshStore = BBMeshStore_create(pinball->matStore);
    if (!pinball->transparentMeshStore)
        return GL_FALSE;

	for (i = 0; i < BB_PINBALL_TRANSPARENT_MESH_COUNT; i++)
        if (!BBMeshStore_exchangeByName(pinball->meshStore, pinball->transparentMeshStore, BB_PINBALL_TRANSPARENT_MESH_NAMES[i]))
            return GL_FALSE;
*/

    /* Mesh store for board's objects */

    pinball->boardObjectStore = BBMeshStore_create(pinball->matStore);
    if (!pinball->boardObjectStore)
        return GL_FALSE;

    for (i = 0; i < BB_PINBALL_BOARD_OBJECTS_COUNT; i++)
        if (!BBMeshStore_exchangeByName(pinball->meshStore, pinball->boardObjectStore, BB_PINBALL_BOARD_OBJECTS_NAMES[i]))
            return GL_FALSE;

    /* Mesh store for board's objects */

    pinball->outerObjectStore = BBMeshStore_create(pinball->matStore);
    if (!pinball->outerObjectStore)
        return GL_FALSE;

    for (i = 0; i < BB_PINBALL_OUTER_OBJECTS_COUNT; i++)
        if (!BBMeshStore_exchangeByName(pinball->meshStore, pinball->outerObjectStore, BB_PINBALL_OUTER_OBJECTS_NAMES[i]))
            return GL_FALSE;

    /* Get meshes / remove from store */

    pinball->voidGlass = BBMeshStore_getMeshByName(pinball->meshStore, "VoidPleksi");
    if (!pinball->voidGlass)
        return GL_FALSE;
    BBMeshStore_removeFromList(pinball->meshStore, pinball->voidGlass);

    pinball->glassMesh = BBMeshStore_getMeshByName(pinball->meshStore, "Pleksi");
    if (!pinball->glassMesh)
        return GL_FALSE;
    BBMeshStore_removeFromList(pinball->meshStore, pinball->glassMesh);

    pinball->boardMesh = BBMeshStore_getMeshByName(pinball->meshStore, "Pohjalauta");
    if (!pinball->boardMesh)
        return GL_FALSE;
    BBMeshStore_removeFromList(pinball->meshStore, pinball->boardMesh);

    /* Ball */

    pinball->ballMesh = BBMeshStore_getMeshByName(pinball->meshStore, "Pallo");
    if (!pinball->ballMesh)
        return GL_FALSE;
    BBMeshStore_removeFromList(pinball->meshStore, pinball->ballMesh);

    pinball->ballShadowMesh = BBMeshStore_getMeshByName(pinball->meshStore, "PallonVarjo");
    if (!pinball->ballShadowMesh)
        return GL_FALSE;
    BBMeshStore_removeFromList(pinball->meshStore, pinball->ballShadowMesh);

    /* Rackets */

    pinball->lowerRightRacketMesh = BBMeshStore_getMeshByName(pinball->meshStore, "MailaOikeaAla");
    if (!pinball->lowerRightRacketMesh)
        return GL_FALSE;
    BBMeshStore_removeFromList(pinball->meshStore, pinball->lowerRightRacketMesh);

    pinball->lowerLeftRacketMesh = BBMeshStore_getMeshByName(pinball->meshStore, "MailaVasenAla");
    if (!pinball->lowerLeftRacketMesh)
        return GL_FALSE;
    BBMeshStore_removeFromList(pinball->meshStore, pinball->lowerLeftRacketMesh);

    pinball->upperRightRacketMesh = BBMeshStore_getMeshByName(pinball->meshStore, "MailaOikeaYla");
    if (!pinball->upperRightRacketMesh)
        return GL_FALSE;
    BBMeshStore_removeFromList(pinball->meshStore, pinball->upperRightRacketMesh);

    pinball->upperLeftRacketMesh = BBMeshStore_getMeshByName(pinball->meshStore, "MailaVasenYla");
    if (!pinball->upperLeftRacketMesh)
        return GL_FALSE;
    BBMeshStore_removeFromList(pinball->meshStore, pinball->upperLeftRacketMesh);

    /* Rackets' shadows */

    pinball->lowerRightRacketShadowMesh = BBMeshStore_getMeshByName(pinball->meshStore, "MailanVarjoOikeaAla");
    if (!pinball->lowerRightRacketShadowMesh)
        return GL_FALSE;
    BBMeshStore_removeFromList(pinball->meshStore, pinball->lowerRightRacketShadowMesh);

    pinball->lowerLeftRacketShadowMesh = BBMeshStore_getMeshByName(pinball->meshStore, "MailanVarjoVasenAla");
    if (!pinball->lowerLeftRacketShadowMesh)
        return GL_FALSE;
    BBMeshStore_removeFromList(pinball->meshStore, pinball->lowerLeftRacketShadowMesh);

    pinball->upperRightRacketShadowMesh = BBMeshStore_getMeshByName(pinball->meshStore, "MailanVarjoOikeaYla");
    if (!pinball->upperRightRacketShadowMesh)
        return GL_FALSE;
    BBMeshStore_removeFromList(pinball->meshStore, pinball->upperRightRacketShadowMesh);

    pinball->upperLeftRacketShadowMesh = BBMeshStore_getMeshByName(pinball->meshStore, "MailanVarjoVasenYla");
    if (!pinball->upperLeftRacketShadowMesh)
        return GL_FALSE;
    BBMeshStore_removeFromList(pinball->meshStore, pinball->upperLeftRacketShadowMesh);

    /* Ball door */

    pinball->ballDoorMesh = BBMeshStore_getMeshByName(pinball->meshStore, "Balldoor");
    if (!pinball->ballDoorMesh)
        return GL_FALSE;
    BBMeshStore_removeFromList(pinball->meshStore, pinball->ballDoorMesh);

    /* Drop targets */

    pinball->dropTargetMeshes = malloc(sizeof(BBMesh*) * BB_PINBALL_DROPTARGET_COUNT);
    if (!pinball->dropTargetMeshes)
        return GL_FALSE;

    for (i = 0; i < BB_PINBALL_DROPTARGET_COUNT; i++)
        pinball->dropTargetMeshes[i] = NULL;

    pinball->dropTargetPosList = malloc(sizeof(GLfloat) * BB_PINBALL_DROPTARGET_COUNT);
    if (!pinball->dropTargetPosList)
        return GL_FALSE;

    for (i = 0; i < BB_PINBALL_DROPTARGET_COUNT; i++)
    {
        if (strlen(BB_PINBALL_DROPTARGET_NAMES[i]) > 0)
        {
            pinball->dropTargetMeshes[i] = BBMeshStore_getMeshByName(pinball->meshStore, BB_PINBALL_DROPTARGET_NAMES[i]);
            if (!pinball->dropTargetMeshes[i])
                return GL_FALSE;
            BBMeshStore_removeFromList(pinball->meshStore, pinball->dropTargetMeshes[i]);
        }
        else
        {
            pinball->dropTargetMeshes[i] = NULL;
        }
    }

    /* Touchie */

    pinball->touchieMesh = BBMeshStore_getMeshByName(pinball->meshStore, BB_PINBALL_TOUCHIE_MESH_NAME);
    if (!pinball->touchieMesh)
        return GL_FALSE;
    BBMeshStore_removeFromList(pinball->meshStore, pinball->touchieMesh);

    /* Lights */

    pinball->lightMeshes = malloc(sizeof(BBMesh*) * BB_PINBALL_LIGHT_COUNT);
    if (!pinball->lightMeshes)
        return GL_FALSE;

    for (i = 0; i < BB_PINBALL_LIGHT_COUNT; i++)
    {
        pinball->lightMeshes[i] = BBMeshStore_getMeshByName(pinball->meshStore, BB_PINBALL_LIGHT_MESH_NAMES[i]);
        if (!pinball->lightMeshes[i])
            return GL_FALSE;
        BBMeshStore_removeFromList(pinball->meshStore, pinball->lightMeshes[i]);
    }

    pinball->lightAlphaList = malloc(sizeof(GLfloat) * BB_PINBALL_LIGHT_COUNT);
    if (!pinball->lightAlphaList)
        return GL_FALSE;

    for (i = 0; i < BB_PINBALL_LIGHT_COUNT; i++)
        pinball->lightAlphaList[i] = 0.0f;

    /* Bumper lights */

/*  pinball->bumperLightMeshes = malloc(sizeof(BBMesh*) * BB_PINBALL_BUMPER_LIGHT_COUNT);
    if (!pinball->bumperLightMeshes)
        return GL_FALSE;

    for (i = 0; i < BB_PINBALL_BUMPER_LIGHT_COUNT; i++)
        pinball->bumperLightMeshes[i] = NULL;

    for (i = 0; i < BB_PINBALL_BUMPER_LIGHT_COUNT; i++)
    {
        pinball->bumperLightMeshes[i] = BBMeshStore_getMeshByName(pinball->meshStore, BB_PINBALL_BUMPER_LIGHT_NAMES[i]);
        if (!pinball->bumperLightMeshes[i])
            return GL_FALSE;
        BBMeshStore_removeFromList(pinball->meshStore, pinball->bumperLightMeshes[i]);
    }

    pinball->bumperLightValues = malloc(sizeof(GLfloat) * BB_PINBALL_BUMPER_LIGHT_COUNT);
    if (!pinball->bumperLightValues)
        return GL_FALSE;

    for (i = 0; i < BB_PINBALL_BUMPER_LIGHT_COUNT; i++)
        pinball->bumperLightValues[i] = 0.0f;
*/
    pinball->ledMeshes = malloc(sizeof(BBMesh*) * BB_PINBALL_LED_NUMBER_COUNT * 4);
    if (!pinball->ledMeshes)
        return GL_FALSE;

    for (i = 0; i < BB_PINBALL_LED_NUMBER_COUNT * 4; i++)
        pinball->ledMeshes[i] = NULL;

    for (i = 0; i < BB_PINBALL_LED_NUMBER_COUNT * 4; i++)
    {
        pinball->ledMeshes[i] = BBMeshStore_getMeshByName(pinball->meshStore, BB_PINBALL_LED_MESH_NAMES[i]);
        if (!pinball->ledMeshes[i])
            return GL_FALSE;
        BBMeshStore_removeFromList(pinball->meshStore, pinball->ledMeshes[i]);
    }

    /* HUD */

    pinball->hudMesh = bbCreateRect(0.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        "m_hud", pinball->matStore, pinball->useVBO,pinball->useIBO);

    pinball->smallFontMesh = bbCreateRect(0.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        "m_smallfont", pinball->matStore, pinball->useVBO,pinball->useIBO);

    pinball->textShadowMesh = bbCreateRect(0.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        "m_textshadow", pinball->matStore, pinball->useVBO,pinball->useIBO);

    pinball->startScreenMesh = bbCreateRect(0.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        "m_startscreen", pinball->matStore, pinball->useVBO,pinball->useIBO);

    pinball->startScreen2Mesh = bbCreateRect(0.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        "m_startscreen2", pinball->matStore, pinball->useVBO,pinball->useIBO);

    pinball->startScreenBackMesh = bbCreateRect(0.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        "m_startscreenback", pinball->matStore, pinball->useVBO,pinball->useIBO);

    /* Calculate camera position */

    BBPinball_updateCamera(pinball);

    /* Setup projection matrix */

    {
        GLfloat aspect = (GLfloat)pinball->params.surfaceWidth / (GLfloat)pinball->params.surfaceHeight;
        bbMatrixProjection(BB_PINBALL_FOV, aspect, 10.0f, 200.0f, pinball->projMatrix);
    }

/* DEBUG */
#ifdef SHOW_DEBUG_TRIANGLE
    pinball->debugTriangle = BBMesh_create(pinball->matStore, "debugTriangle", pinball->useVBO, pinball->useIBO);
    if (!pinball->debugTriangle)
        return GL_FALSE;

    BBMesh_loadVertices(pinball->debugTriangle, BB_PINBALL_DEBUG_TRIANGLE_COORDS, sizeof(GLfloat) * BB_PINBALL_DEBUG_TRIANGLE_COORD_COUNT, BB_VERTEX_DECLARATION_POSITION);
    BBMesh_loadIndices(pinball->debugTriangle, BB_PINBALL_DEBUG_TRIANGLE_INDICES, BB_PINBALL_DEBUG_TRIANGLE_INDEX_COUNT);
    BBMesh_addSubMesh(pinball->debugTriangle, 0, BB_PINBALL_DEBUG_TRIANGLE_INDEX_COUNT, "default");
#endif
/* DEBUG */

    /*--------------------------------------------------------------------------*/
    /* Physics part                                                             */
    /*--------------------------------------------------------------------------*/

    /* Load collision map */

    {
        BBTga* tga = BBTga_load("../bbPinball/data/colmap.tga");
        if (!tga)
            return GL_FALSE;

        pinball->collisionMap = BBCollisionMap_create();
        if (!pinball->collisionMap)
        {
            BBTga_destroy(tga);
            return GL_FALSE;
        }

        BBCollisionMap_load(pinball->collisionMap, tga->data, tga->width, tga->height);
        BBTga_destroy(tga);

        BBCollisionMap_setArea(pinball->collisionMap,
            BB_PINBALL_COLLISION_MAP_AREA_TOPLEFT[0],
            BB_PINBALL_COLLISION_MAP_AREA_TOPLEFT[1],
            BB_PINBALL_COLLISION_MAP_AREA_BOTTOMRIGHT[0],
            BB_PINBALL_COLLISION_MAP_AREA_BOTTOMRIGHT[1]);
    }

    /* Drop targets */

    pinball->dropTargetGroupRaiseTime = malloc(sizeof(GLint) * BB_PINBALL_DROPTARGET_GROUP_COUNT);
    if (!pinball->dropTargetGroupRaiseTime)
        return GL_FALSE;

    {
        GLuint p;
        for (p = 0; p < BB_PINBALL_MAX_PLAYERS; p++)
        {
            pinball->players[p].dropTargetDownList = malloc(sizeof(GLboolean) * BB_PINBALL_DROPTARGET_COUNT);
            if (!pinball->players[p].dropTargetDownList)
                return GL_FALSE;

            pinball->players[p].dropTargetGroupDown = malloc(sizeof(GLint) * BB_PINBALL_DROPTARGET_GROUP_COUNT);
            if (!pinball->players[p].dropTargetGroupDown)
                return GL_FALSE;

            pinball->players[p].lightOnList = malloc(sizeof(GLboolean) * BB_PINBALL_LIGHT_COUNT);
            if (!pinball->players[p].lightOnList)
                return GL_FALSE;

            BBPlayer_defaultValues(&pinball->players[p]);
        }
    }

    for (i = 0; i < BB_PINBALL_DROPTARGET_COUNT; i++)
    {
        pinball->dropTargetPosList[i] = 0.0f;
    }

    for (i = 0; i < BB_PINBALL_DROPTARGET_GROUP_COUNT; i++)
    {
        pinball->dropTargetGroupRaiseTime[i] = 0;
    }

    /* Sensors */

    pinball->sensorDownList = malloc(sizeof(GLboolean) * BB_PINBALL_SENSOR_COUNT);
    if (!pinball->sensorDownList)
        return GL_FALSE;

    for (i = 0; i < BB_PINBALL_SENSOR_COUNT; i++)
        pinball->sensorDownList[i] = GL_FALSE;

    /* Allocate balls */

    {
        /* Buffer for ball-ball collisions */

/*      pinball->ballCollisions = malloc(sizeof(BBBallCollision));
        if (!pinball->ballCollisions)
            return GL_FALSE;*/
    }

    /* Screen capture */

    pinball->captureBuffer = malloc(sizeof(GLubyte) * pinball->params.surfaceWidth * pinball->params.surfaceHeight * 4);
    if (!pinball->captureBuffer)
        return GL_FALSE;

    BBPinball_loadRecording(pinball);

#ifndef BB_DISABLE_SOUNDS
    for (i = 0; i < BB_SFX_MAX - 1; i++)
    {
        const char PATH[] = "../bbPinball/sounds/";
        char fullFilename[512];
        const size_t PATH_LENGTH = strlen(PATH);
        memcpy(fullFilename, PATH, PATH_LENGTH);
        memcpy(fullFilename + PATH_LENGTH, BB_SFX_FILENAMES[i], strlen(BB_SFX_FILENAMES[i]) + 1);
        pinball->sfxList[i] = pinball->sound.load(fullFilename);
        if (pinball->sfxList[i] == BB_SOUND_NONE)
        {
#ifdef BB_DEBUG
            printf("Error: Unable to load sound %s\n", BB_SFX_FILENAMES[i]);
#endif
        }
    }
#endif // !BB_DISABLE_SOUNDS

    {
        FILE* f = BB_FOPEN(BB_PINBALL_HIGHSCORES_FILENAME, "rb");
        if (f)
        {
            for (i = 0; i < 10; i++)
            {
                fread(&pinball->highscores[i].score, sizeof(GLuint), 1, f); 
                fread(&pinball->highscores[i].name, sizeof(char), 3, f); 
            }
            fclose(f);
        }
    }

  if (pinball->playbackOn)
  {
        pinball->playbackStartTime = pinball->params.getTime();
        BBPinball_playbackFrame(pinball, GL_TRUE);
  }

    glViewport(0, 0, pinball->params.surfaceWidth, pinball->params.surfaceHeight);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepthf(1.0f);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    return GL_TRUE;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

void BBPinball_destroy (BBPinball* pinball)
{
    GLint i;
    BB_ASSERT(pinball);

    /*{
        FILE* f = BB_FOPEN(BB_PINBALL_HIGHSCORES_FILENAME, "wb");
        if (f)
        {
            for (i = 0; i < 10; i++)
            {
                fwrite(&pinball->highscores[i].score, sizeof(GLuint), 1, f); 
                fwrite(&pinball->highscores[i].name, sizeof(char), 3, f); 
            }
            fclose(f);
        }
    }*/

    BBPinball_saveRecording(pinball);
    BBPinball_dumpRecording(pinball);

    /* Screen capture */
    free(pinball->captureBuffer);

    /* Physics */

//  free(pinball->ballCollisions);
//  free(pinball->balls);

    free(pinball->sensorDownList);

    for (i = 0; i < BB_PINBALL_MAX_PLAYERS; i++)
    {
        free(pinball->players[i].lightOnList);
        free(pinball->players[i].dropTargetGroupDown);
        free(pinball->players[i].dropTargetDownList);
    }

    free(pinball->dropTargetGroupRaiseTime);

    if (pinball->collisionMap)
        BBCollisionMap_destroy(pinball->collisionMap);

    /* Gfx */

/* DEBUG */
#ifdef SHOW_DEBUG_TRIANGLE
    if (pinball->debugTriangle)
        BBMesh_destroy(pinball->debugTriangle);
#endif
/* DEBUG */

    /* HUD */
    if (pinball->startScreenBackMesh)
        BBMesh_destroy(pinball->startScreenBackMesh);

    if (pinball->startScreenMesh)
        BBMesh_destroy(pinball->startScreenMesh);

    if (pinball->startScreen2Mesh)
        BBMesh_destroy(pinball->startScreen2Mesh);

    if (pinball->textShadowMesh)
        BBMesh_destroy(pinball->textShadowMesh);

    if (pinball->smallFontMesh)
        BBMesh_destroy(pinball->smallFontMesh);

    if (pinball->hudMesh)
        BBMesh_destroy(pinball->hudMesh);

    /* LED */

    if (pinball->ledMeshes)
    {
        for (i = 0; i < BB_PINBALL_LED_NUMBER_COUNT * 4; i++)
        {
            if (pinball->ledMeshes[i])
                BBMesh_destroy(pinball->ledMeshes[i]);
        }
        free(pinball->ledMeshes);
    }

    /* Bumper lights */

/*  free(pinball->bumperLightValues);

    if (pinball->bumperLightMeshes)
    {
        for (i = 0; i < BB_PINBALL_BUMPER_LIGHT_COUNT; i++)
        {
            if (pinball->bumperLightMeshes[i])
                BBMesh_destroy(pinball->bumperLightMeshes[i]);
        }
        free(pinball->bumperLightMeshes);
    }*/

    /* Lights */

    free(pinball->lightAlphaList);

    if (pinball->lightMeshes)
    {
        for (i = 0; i < BB_PINBALL_LIGHT_COUNT; i++)
        {
            if (pinball->lightMeshes[i])
                BBMesh_destroy(pinball->lightMeshes[i]);
        }
        free(pinball->lightMeshes);
    }

    /* Touchie */

    if (pinball->touchieMesh)
        BBMesh_destroy(pinball->touchieMesh);

    /* Drop targets */

    free(pinball->dropTargetPosList);

    if (pinball->dropTargetMeshes)
    {
        for (i = 0; i < BB_PINBALL_DROPTARGET_COUNT; i++)
        {
            if (pinball->dropTargetMeshes[i])
                BBMesh_destroy(pinball->dropTargetMeshes[i]);
        }
        free(pinball->dropTargetMeshes);
    }

    /* Ball door */

    if (pinball->ballDoorMesh)
        BBMesh_destroy(pinball->ballDoorMesh);

    /* Rackets' shadows */

    if (pinball->upperLeftRacketShadowMesh)
        BBMesh_destroy(pinball->upperLeftRacketShadowMesh);
    if (pinball->upperRightRacketShadowMesh)
        BBMesh_destroy(pinball->upperRightRacketShadowMesh);
    if (pinball->lowerLeftRacketShadowMesh)
        BBMesh_destroy(pinball->lowerLeftRacketShadowMesh);
    if (pinball->lowerRightRacketShadowMesh)
        BBMesh_destroy(pinball->lowerRightRacketShadowMesh);

    /* Rackets */

    if (pinball->upperLeftRacketMesh)
        BBMesh_destroy(pinball->upperLeftRacketMesh);
    if (pinball->upperRightRacketMesh)
        BBMesh_destroy(pinball->upperRightRacketMesh);
    if (pinball->lowerLeftRacketMesh)
        BBMesh_destroy(pinball->lowerLeftRacketMesh);
    if (pinball->lowerRightRacketMesh)
        BBMesh_destroy(pinball->lowerRightRacketMesh);

    if (pinball->ballShadowMesh)
        BBMesh_destroy(pinball->ballShadowMesh);
    if (pinball->ballMesh)
        BBMesh_destroy(pinball->ballMesh);

    if (pinball->boardMesh)
        BBMesh_destroy(pinball->boardMesh);
    if (pinball->voidGlass)
        BBMesh_destroy(pinball->voidGlass);
    if (pinball->glassMesh)
        BBMesh_destroy(pinball->glassMesh);

    if (pinball->outerObjectStore)
        BBMeshStore_destroy(pinball->outerObjectStore);

    if (pinball->boardObjectStore)
        BBMeshStore_destroy(pinball->boardObjectStore);

//    if (pinball->transparentMeshStore)
//        BBMeshStore_destroy(pinball->transparentMeshStore);

    if (pinball->meshStore2)
        BBMeshStore_destroy(pinball->meshStore2);

    if (pinball->meshStore)
        BBMeshStore_destroy(pinball->meshStore);

    if (pinball->matStore)
        BBMaterialStore_destroy(pinball->matStore);

    if (pinball->texStore)
        BBTextureStore_destroy(pinball->texStore);

    if (pinball->shaderStack)
        BBShaderStack_destroy(pinball->shaderStack);

// USB
#ifdef USBBLIT
    if (pinball->usb)
        destroyUsbInterface(pinball->usb);
#endif

    free(pinball);
}

static void cacheUniformSetup(GLuint program, BBPinballUniformSetupCache* cache)
{
    cache->projMatLoc = glGetUniformLocation(program, "projMatrix");

    cache->worldMatLoc = glGetUniformLocation(program, "worldMatrix");
    cache->worldNormalMatLoc = glGetUniformLocation(program, "worldNormalMatrix");
    cache->viewMatLoc = glGetUniformLocation(program, "viewMatrix");
    cache->viewNormalMatLoc = glGetUniformLocation(program, "viewNormalMatrix");

    cache->lightIntensityLoc = glGetUniformLocation(program, "lightIntensity");
    cache->lightPosLoc = glGetUniformLocation(program, "lightPos");
    cache->eyePosLoc = glGetUniformLocation(program, "eyePos");
    cache->eyePosOSLoc = glGetUniformLocation(program, "eyePosOS");

    cache->texLoc = glGetUniformLocation(program, "texSampler");
    cache->tex2Loc = glGetUniformLocation(program, "texSampler2");
    cache->tex3Loc = glGetUniformLocation(program, "texSampler3");
    cache->tex4Loc = glGetUniformLocation(program, "texSampler4");
    cache->texBlendLoc = glGetUniformLocation(program, "texBlendValue");

    cache->lightAlphaLoc = glGetUniformLocation(program, "lightAlpha");
    cache->ledNumberLoc = glGetUniformLocation(program, "ledNumber");
    cache->outputMultiplierLoc = glGetUniformLocation(program, "outputMultiplier");
    cache->outputMultiplier2Loc = glGetUniformLocation(program, "outputMultiplier2");
    cache->outputMultiplier3Loc = glGetUniformLocation(program, "outputMultiplier3");

    cache->hudPosLoc = glGetUniformLocation(program, "hudPos");
    cache->hudScaleLoc = glGetUniformLocation(program, "hudScale");
    cache->hudTexPosLoc = glGetUniformLocation(program, "hudTexPos");
    cache->hudTexScaleLoc = glGetUniformLocation(program, "hudTexScale");
    cache->hudColorLoc = glGetUniformLocation(program, "hudColor");

    cache->valid = 1;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

static void uniformSetup (void* userPtr, GLuint program, BBPinballUniformSetupCache* cache)
{
    BBPinball* pinball = (BBPinball*)userPtr;
    BB_ASSERT(pinball);

if( pinball->worldMatrix[0] != 1.0f )
	pinball->worldMatrix[0] = pinball->worldMatrix[0];



    if (!cache->valid)
    {
        cacheUniformSetup(program,cache);
        glUniform1i(cache->texLoc, 0);
        glUniform1i(cache->tex2Loc, 1);
        glUniform1i(cache->tex3Loc, 2);
        glUniform1i(cache->tex4Loc, 3);

        glUniform1f(cache->texBlendLoc, pinball->currentTexBlendValue);

        if (cache->lightPosLoc >= 0)
        {
            float xyz[3] = { 0.f, 100.f, 20.f };
            glUniform3fv(cache->lightPosLoc, 1, xyz);
        }
    }

    {
        if (cache->projMatLoc >= 0)
        {
            GLfloat matrix[4 * 4];

            if ( cache->worldMatLoc >= 0 )
            {
                glUniformMatrix4fv(cache->worldMatLoc, 1, 0, pinball->worldMatrix);
            }

            if (cache->worldNormalMatLoc >= 0)
            {
                GLfloat worldNoTrans[4 * 4];
                memcpy(worldNoTrans, pinball->worldMatrix, 4 * 4 * sizeof(GLfloat));
                worldNoTrans[0 + 3 * 4] = 0.0f;
                worldNoTrans[1 + 3 * 4] = 0.0f;
                worldNoTrans[2 + 3 * 4] = 0.0f;
                glUniformMatrix4fv(cache->worldNormalMatLoc, 1, 0, worldNoTrans);
            }

            bbMatrixMultiply(pinball->viewMatrix, pinball->worldMatrix, matrix);

            if ( cache->viewMatLoc >= 0 )
            {
                glUniformMatrix4fv(cache->viewMatLoc, 1, 0, matrix);
            }

            if (cache->viewNormalMatLoc >= 0)
            {
                GLfloat viewNoTrans[4 * 4];
                memcpy(viewNoTrans, pinball->viewMatrix, 4 * 4 * sizeof(GLfloat));
                viewNoTrans[0 + 3 * 4] = 0.0f;
                viewNoTrans[1 + 3 * 4] = 0.0f;
                viewNoTrans[2 + 3 * 4] = 0.0f;
                glUniformMatrix4fv(cache->viewNormalMatLoc, 1, 0, viewNoTrans);
            }

            bbMatrixMultiply(pinball->projMatrix, matrix, matrix);

            glUniformMatrix4fv(cache->projMatLoc, 1, 0, matrix);
        }
    }

    if (cache->lightIntensityLoc >= 0 && cache->currentTime != pinball->currentTime)
    {
        cache->currentTime = pinball->currentTime;
        glUniform1f(cache->lightIntensityLoc, sinf(pinball->currentTime / 800.0f) * 0.125f + 1.0f);
    }

    if ( cache->eyePosLoc >= 0 )
    {
        glUniform3fv(cache->eyePosLoc, 1, pinball->eye);
    }

    if ( cache->eyePosOSLoc >= 0 )
	{ 
		float vEyeOS[3];
		vEyeOS[0] = pinball->eye[0] - pinball->worldMatrix[3*4+0];
		vEyeOS[1] = pinball->eye[1] - pinball->worldMatrix[3*4+1];
		vEyeOS[2] = pinball->eye[2] - pinball->worldMatrix[3*4+2];
		glUniform3fv(cache->eyePosOSLoc, 1, vEyeOS);
	}

    if ( cache->lightAlphaLoc >= 0 )
    {
        glUniform1f(cache->lightAlphaLoc, pinball->lightCurrentAlpha);
    }
    if ( cache->ledNumberLoc >= 0 && cache->ledNumber != pinball->ledNumber )
    {
        cache->ledNumber = pinball->ledNumber;
        glUniform1f(cache->ledNumberLoc, (float)pinball->ledNumber);
    }

    if ( cache->outputMultiplierLoc >= 0 )
    {
        glUniform1f(cache->outputMultiplierLoc, pinball->outputMultiplier);
    }

    if ( cache->outputMultiplier2Loc >= 0 )
    {
	    float outputMult2;
	    if( pinball->outputMultiplier == 1.0f )
		    outputMult2 = 1.0f;
	    else
		    outputMult2	= pinball->outputMultiplier*pinball->outputMultiplier*0.4f;
	
	    glUniform1f(cache->outputMultiplier2Loc, outputMult2);
    }

    if ( cache->outputMultiplier3Loc >= 0 )
    {
	    float outputMult3;
	    if( pinball->outputMultiplier == 1.0f )
		    outputMult3 = 1.0f;
	    else
		    outputMult3	= pinball->outputMultiplier*pinball->outputMultiplier*0.3f;
	
	    glUniform1f(cache->outputMultiplier3Loc, outputMult3);
    }
}

static void uniformSetupHUD (void* userPtr, GLuint program, BBPinballUniformSetupCache* cache)
{
    BBPinball* pinball = (BBPinball*)userPtr;
    BB_ASSERT(pinball);

    if (!cache->valid)
    {
        cacheUniformSetup(program,cache);
    }

    glUniform2fv(cache->hudPosLoc, 1, pinball->hudPos);
    glUniform2fv(cache->hudScaleLoc, 1, pinball->hudScale);
    glUniform2fv(cache->hudTexPosLoc, 1, pinball->hudTexPos);
    glUniform2fv(cache->hudTexScaleLoc, 1, pinball->hudTexScale);
    glUniform4fv(cache->hudColorLoc, 1, pinball->hudColor);
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

static void bbMatrixMirrorY (GLfloat* mat, GLfloat y)
{
    GLfloat temp[4 * 4];

    bbMatrixTranslation(0.0f, y, 0.0f, temp);
    bbMatrixMultiply(mat, temp, mat);

    bbMatrixScaling(1.0f, -1.0f, 1.0f, temp);
    bbMatrixMultiply(mat, temp, mat);

    bbMatrixTranslation(0.0f, -y, 0.0f, temp);
    bbMatrixMultiply(mat, temp, mat);
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

void BBPinball_calculateRacketMatrix (BBPinball* pinball, GLfloat* matrix,
                                      const GLfloat* pos, GLfloat angle)
{
    GLfloat trans[4 * 4];
    BB_ASSERT(pinball && matrix && pos);

    bbMatrixRotationY(angle, matrix);
    bbMatrixTranslation(pos[0], 0.0f, pos[1], trans);
    bbMatrixMultiply(trans, matrix, matrix);
    bbMatrixTranslation(-pos[0], 0.0f, -pos[1], trans);
    bbMatrixMultiply(matrix, trans, matrix);
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

void BBPinball_drawRacket (BBPinball* pinball, BBMesh* mesh, BBMesh* shadowMesh, const GLfloat* matrix, GLboolean mirror)
{
    BB_ASSERT(pinball);

    memcpy(pinball->worldMatrix, matrix, sizeof(GLfloat) * 4 * 4);

    if (mirror)
    {
        bbMatrixMirrorY(pinball->worldMatrix, BB_PINBALL_TABLE_SURFACE_REFLECTION_HEIGHT);
    }

    if (mesh)
    {
        BBMesh_draw(mesh, uniformSetup, pinball, GL_FALSE, GL_FALSE);
    }

    if (shadowMesh)
    {
        glDepthMask(GL_FALSE);
        BBMesh_draw(shadowMesh, uniformSetup, pinball, GL_TRUE, GL_FALSE);
        glDepthMask(GL_TRUE);
    }
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

const GLfloat SMALL_FONT_CHAR_WIDTH = 24.0f;
const GLfloat SMALL_FONT_CHAR_HEIGHT = 32.0f;
const GLfloat SMALL_FONT_TEXTURE_WIDTH = 1024.0f;
const GLfloat SMALL_FONT_TEXTURE_HEIGHT = 32.0f;
const GLfloat SMALL_FONT_SCREEN_SIZE_X = 24.0f / 1024.0f * 1.1f;
const GLfloat SMALL_FONT_SCREEN_SIZE_Y = 32.0f / 768.0f * 1.1f;

void BBPinball_drawText (BBPinball* pinball, const char* string, GLfloat x, GLfloat y, GLboolean shadow)
{
    GLint i = 0;
    GLint len = (GLint)strlen(string);
    GLint p = 0;

    BB_ASSERT(pinball);

    if (shadow)
    {
        GLfloat alpha = pinball->hudColor[3];
        pinball->hudColor[3] = alpha * 0.6f;

        while (p < len)
        {
            char c = string[p++];

            if (c != ' ')
            {
                pinball->hudPos[0] = x + SMALL_FONT_SCREEN_SIZE_X * i - SMALL_FONT_SCREEN_SIZE_X * 0.5f;
                pinball->hudPos[1] = y - SMALL_FONT_SCREEN_SIZE_Y * 0.25f;
                pinball->hudScale[0] = SMALL_FONT_SCREEN_SIZE_X * 2.0f;
                pinball->hudScale[1] = SMALL_FONT_SCREEN_SIZE_Y * 1.5f;

                pinball->hudTexPos[0] = 0.0f;
                pinball->hudTexPos[1] = 0.0f;
                pinball->hudTexScale[0] = 1.0f;
                pinball->hudTexScale[1] = 1.0f;

                BBMesh_draw(pinball->textShadowMesh, uniformSetupHUD, pinball, GL_TRUE, GL_FALSE);
            }

            i++;
        }

        pinball->hudColor[3] = alpha;

        i = 0;
        p = 0;
    }

    while (p < len)
    {
        GLint n = 0;
        char c = string[p++];

        if (c >= 48 && c <= 58)
        {
            n = c - 47 + 26;
        }
        else if (c >= 65 && c <= 90)
        {
            n = c - 64;
        }
        else
        {
            i++;
            continue;
        }

        pinball->hudPos[0] = x + SMALL_FONT_SCREEN_SIZE_X * i;
        pinball->hudPos[1] = y;
        pinball->hudScale[0] = SMALL_FONT_SCREEN_SIZE_X;
        pinball->hudScale[1] = SMALL_FONT_SCREEN_SIZE_Y;

        pinball->hudTexPos[0] = (n * SMALL_FONT_CHAR_WIDTH) / SMALL_FONT_TEXTURE_WIDTH - 0.5f / SMALL_FONT_TEXTURE_WIDTH;
        pinball->hudTexPos[1] = -1.0f / SMALL_FONT_TEXTURE_HEIGHT;
        pinball->hudTexScale[0] = SMALL_FONT_CHAR_WIDTH / SMALL_FONT_TEXTURE_WIDTH;
        pinball->hudTexScale[1] = SMALL_FONT_CHAR_HEIGHT / SMALL_FONT_TEXTURE_HEIGHT;

        BBMesh_draw(pinball->smallFontMesh, uniformSetupHUD, pinball, GL_TRUE, GL_FALSE);

        i++;
    }
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

static void BBPinball_drawLedScreen (BBPinball* pinball)
{
    GLsizei i;
    GLsizei p;

    glDepthMask(GL_FALSE);

    for (p = 0; p < 4; p++)
    {
        GLint score = pinball->players[p].score;
        for (i = 0; i < BB_PINBALL_LED_NUMBER_COUNT; i++)
        {
            pinball->ledNumber = score % 10;
            BBMesh_draw(pinball->ledMeshes[i + p * BB_PINBALL_LED_NUMBER_COUNT], uniformSetup, pinball, GL_TRUE, GL_FALSE);
            score /= 10;
        }
    }

    glDepthMask(GL_TRUE);
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

const GLfloat TOP_GLASS_Y = 55.28f;

#if defined(__VERSATILE__) && !defined(PERFMONITOR)
#define PRINT_PERF \
    endTick = pinball->params.getTime(); \
    blockTimes[block++] = endTick - startTick; \
    startTick = pinball->params.getTime();
#else
#define PRINT_PERF
#endif


int g_MeshNum;


void BBPinball_draw (BBPinball* pinball)
{
	g_MeshNum = 0;
#if defined(__VERSATILE__) && !defined(PERFMONITOR)
    unsigned int blockTimes[256];
    unsigned int totalTime;
    unsigned int startTick, endTick, block = 0;
    startTick = pinball->params.getTime();
#endif
    
    BB_ASSERT(pinball);

#ifndef BB_DISABLE_SOUNDS
    if (!pinball->musicPlaying)
    {
        pinball->musicPlaying = GL_TRUE;
        BBPinball_playSFX(pinball, BB_SFX_MUSIC, 0.85f, GL_TRUE);
    }
#endif // !BB_DISABLE_SOUNDS

//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClear(GL_DEPTH_BUFFER_BIT);

    if (pinball->gameRunning)
    {
        if (pinball->ballDoorClosed)
            pinball->ballDoorAngle += (0.0f - pinball->ballDoorAngle) * pinball->timeDelta / 60.0f;
        else
            pinball->ballDoorAngle += (BB_PINBALL_BALL_DOOR_MAX_ANGLE - pinball->ballDoorAngle) * pinball->timeDelta / 60.0f;
    }

    {
        GLint i;
        GLfloat ballDoorMatrix[4 * 4];
        GLfloat touchieMatrix[4 * 4];

        {
            const GLfloat doorPivot[2] = { -14.2f, 52.9f };
            GLfloat trans[4 * 4];
            bbMatrixRotationY(pinball->ballDoorAngle, ballDoorMatrix);
            bbMatrixTranslation(doorPivot[0], 0.0f, doorPivot[1], trans);
            bbMatrixMultiply(trans, ballDoorMatrix, ballDoorMatrix);
            bbMatrixTranslation(-doorPivot[0], 0.0f, -doorPivot[1], trans);
            bbMatrixMultiply(ballDoorMatrix, trans, ballDoorMatrix);
        }

        {
            GLfloat x = cosf(BB_PINBALL_TOUCHIE_MOVE_ANGLE) * pinball->touchieMove;
            GLfloat y = sinf(BB_PINBALL_TOUCHIE_MOVE_ANGLE) * pinball->touchieMove;
            bbMatrixTranslation(x, 0.0f, y, touchieMatrix);
        }
        
        PRINT_PERF

//#define BOARD_REFLECTIONS
#ifdef BOARD_REFLECTIONS
        glCullFace(GL_FRONT);
        pinball->outputMultiplier = pinball->viewMatrix[5] * 1.0f;

        {
            BBMesh* mesh = BBMeshStore_getFirstMesh(pinball->boardObjectStore);
            bbMatrixMirrorY(pinball->worldMatrix, BB_PINBALL_TABLE_SURFACE_REFLECTION_HEIGHT);

            while (mesh)
            {
                BBMesh_draw(mesh, uniformSetup, pinball, GL_FALSE, GL_FALSE);
                mesh = BBMeshStore_getNextMesh(pinball->boardObjectStore, mesh);
            }
        }

        /* Draw rackets */

        BBPinball_drawRacket(pinball, pinball->lowerRightRacketMesh, NULL, pinball->lowerRightRacketMatrix, GL_TRUE);
        BBPinball_drawRacket(pinball, pinball->lowerLeftRacketMesh, NULL, pinball->lowerLeftRacketMatrix, GL_TRUE);
        BBPinball_drawRacket(pinball, pinball->upperRightRacketMesh, NULL, pinball->upperRightRacketMatrix, GL_TRUE);
        BBPinball_drawRacket(pinball, pinball->upperLeftRacketMesh, NULL, pinball->upperLeftRacketMatrix, GL_TRUE);

        memcpy(pinball->worldMatrix, ballDoorMatrix, sizeof(GLfloat) * 4 * 4);
        bbMatrixMirrorY(pinball->worldMatrix, BB_PINBALL_TABLE_SURFACE_REFLECTION_HEIGHT);
        BBMesh_draw(pinball->ballDoorMesh, uniformSetup, pinball, GL_FALSE, GL_FALSE);

        /* Draw drop targets */

        for (i = 0; i < BB_PINBALL_DROPTARGET_COUNT; i++)
        {
//          bbMatrixTranslation(0.0f, pinball->dropTargetPosList[i], 0.0f, pinball->worldMatrix);
//          bbMatrixMirrorY(pinball->worldMatrix, BB_PINBALL_TABLE_SURFACE_REFLECTION_HEIGHT);
            if (bbAlmostZero(pinball->dropTargetPosList[i]))
                BBMesh_draw(pinball->dropTargetMeshes[i], uniformSetup, pinball, GL_FALSE, GL_FALSE);
        }

        memcpy(pinball->worldMatrix, touchieMatrix, sizeof(GLfloat) * 4 * 4);
        bbMatrixMirrorY(pinball->worldMatrix, BB_PINBALL_TABLE_SURFACE_REFLECTION_HEIGHT);
        BBMesh_draw(pinball->touchieMesh, uniformSetup, pinball, GL_FALSE, GL_FALSE);

        /* Draw ball */

        bbMatrixTranslation(pinball->ball.pos[0], pinball->ball.pos[2] + BB_PINBALL_TABLE_SURFACE_HEIGHT + BB_PINBALL_BALL_RADIUS, pinball->ball.pos[1], pinball->worldMatrix);
        bbMatrixMirrorY(pinball->worldMatrix, BB_PINBALL_TABLE_SURFACE_REFLECTION_HEIGHT);
        BBMesh_draw(pinball->ballMesh, uniformSetup, pinball, GL_FALSE, GL_FALSE);

        glCullFace(GL_BACK);
        bbMatrixIdentity(pinball->worldMatrix);
        pinball->outputMultiplier = 1.0f;
#endif


#if 1 // MIKEY_DRAW_TABLE
//        BBMesh_draw(pinball->boardMesh, uniformSetup, pinball, GL_TRUE, GL_FALSE);
        BBMesh_draw(pinball->boardMesh, uniformSetup, pinball, GL_FALSE, GL_FALSE);

        {
            BBMesh* mesh = BBMeshStore_getFirstMesh(pinball->meshStore);
            while (mesh)
            {
                BBMesh_draw(mesh, uniformSetup, pinball, GL_FALSE, GL_FALSE);
                mesh = BBMeshStore_getNextMesh(pinball->meshStore, mesh);
            }
        }

        {
            BBMesh* mesh = BBMeshStore_getFirstMesh(pinball->boardObjectStore);
            while (mesh)
            {
                BBMesh_draw(mesh, uniformSetup, pinball, GL_FALSE, GL_FALSE);
                mesh = BBMeshStore_getNextMesh(pinball->boardObjectStore, mesh);
            }
        }
        {
            BBMesh* mesh = BBMeshStore_getFirstMesh(pinball->outerObjectStore);
            while (mesh)
            {
                BBMesh_draw(mesh, uniformSetup, pinball, GL_FALSE, GL_FALSE);
                mesh = BBMeshStore_getNextMesh(pinball->outerObjectStore, mesh);
            }
        }

        {
            BBMesh* mesh = BBMeshStore_getFirstMesh(pinball->meshStore2);
            while (mesh)
            {
                BBMesh_draw(mesh, uniformSetup, pinball, GL_FALSE, GL_FALSE);
                mesh = BBMeshStore_getNextMesh(pinball->meshStore2, mesh);
            }
        }

        PRINT_PERF

        /* Draw balldoors */

        memcpy(pinball->worldMatrix, ballDoorMatrix, sizeof(GLfloat) * 4 * 4);
        BBMesh_draw(pinball->ballDoorMesh, uniformSetup, pinball, GL_FALSE, GL_FALSE);

        /* Draw balls */

        /*      for (i = 0; i < pinball->ballCount; i++)
        {
        bbMatrixTranslation(pinball->balls[i].pos[0], pinball->balls[i].pos[2], pinball->balls[i].pos[1], pinball->worldMatrix);
        BBMesh_draw(pinball->ballMesh, uniformSetup, pinball, GL_FALSE);
        glDepthMask(GL_FALSE);
        BBMesh_draw(pinball->ballShadowMesh, uniformSetup, pinball, GL_TRUE);
        glDepthMask(GL_TRUE);
        }*/

        bbMatrixTranslation(pinball->ball.pos[0], pinball->ball.pos[2] + BB_PINBALL_TABLE_SURFACE_HEIGHT + BB_PINBALL_BALL_RADIUS, pinball->ball.pos[1], pinball->worldMatrix);
		{
			static GLint bDrawBall = 1;
			static GLint bDrawBallShadow = 1;
			if(bDrawBall)
					BBMesh_draw(pinball->ballMesh, uniformSetup, pinball, GL_TRUE, GL_FALSE);
					glDepthMask(GL_FALSE);
			if(bDrawBallShadow)
					BBMesh_draw(pinball->ballShadowMesh, uniformSetup, pinball, GL_TRUE, GL_FALSE);
		}
		glDepthMask(GL_TRUE);

        PRINT_PERF

        /* Draw rackets */

        BBPinball_drawRacket(pinball, NULL, pinball->lowerRightRacketShadowMesh, pinball->lowerRightRacketMatrix, GL_FALSE);
        BBPinball_drawRacket(pinball, NULL, pinball->lowerLeftRacketShadowMesh, pinball->lowerLeftRacketMatrix, GL_FALSE);
        BBPinball_drawRacket(pinball, NULL, pinball->upperRightRacketShadowMesh, pinball->upperRightRacketMatrix, GL_FALSE);
        BBPinball_drawRacket(pinball, NULL, pinball->upperLeftRacketShadowMesh, pinball->upperLeftRacketMatrix, GL_FALSE);

        BBPinball_drawRacket(pinball, pinball->lowerRightRacketMesh, NULL, pinball->lowerRightRacketMatrix, GL_FALSE);
        BBPinball_drawRacket(pinball, pinball->lowerLeftRacketMesh, NULL, pinball->lowerLeftRacketMatrix, GL_FALSE);
        BBPinball_drawRacket(pinball, pinball->upperRightRacketMesh, NULL, pinball->upperRightRacketMatrix, GL_FALSE);
        BBPinball_drawRacket(pinball, pinball->upperLeftRacketMesh, NULL, pinball->upperLeftRacketMatrix, GL_FALSE);

        PRINT_PERF

        /* Draw drop targets */

        memcpy(pinball->worldMatrix, touchieMatrix, sizeof(GLfloat) * 4 * 4);
        BBMesh_draw(pinball->touchieMesh, uniformSetup, pinball, GL_FALSE, GL_FALSE);

        for (i = 0; i < BB_PINBALL_DROPTARGET_COUNT; i++)
        {
            bbMatrixTranslation(0.0f, pinball->dropTargetPosList[i], 0.0f, pinball->worldMatrix);
            BBMesh_draw(pinball->dropTargetMeshes[i], uniformSetup, pinball, GL_FALSE, GL_FALSE);
        }
        
        PRINT_PERF

        /* DEBUG */
#ifdef SHOW_DEBUG_TRIANGLE
        bbMatrixTranslation(pinball->debugPos[0], pinball->debugPos[1], pinball->debugPos[2], pinball->worldMatrix);
        glDisable(GL_CULL_FACE);
        BBMesh_draw(pinball->debugTriangle, uniformSetup, pinball, GL_FALSE, GL_FALSE);
        glEnable(GL_CULL_FACE);
#endif
        /* DEBUG */

        /* Reset transformation */

        bbMatrixIdentity(pinball->worldMatrix);

        /* LED */

        BBPinball_drawLedScreen(pinball);
        
        PRINT_PERF

        /* Draw bumper lights */

/*      for (i = 0; i < BB_PINBALL_BUMPER_LIGHT_COUNT; i++)
        {
            pinball->currentTexBlendValue = pinball->bumperLightValues[i];
            BBMesh_draw(pinball->bumperLightMeshes[i], uniformSetup, pinball, GL_FALSE);
        }*/

        /* Draw lights */

        glDepthFunc(GL_LEQUAL);

        {
            float timeDelta = pinball->timeDelta / 100.0f;
            
            for (i = 0; i < BB_PINBALL_LIGHT_COUNT; i++)
            {
                if (pinball->gameRunning)
                {
                    if (pinball->player->lightOnList[i])
                        pinball->lightAlphaList[i] += (1.0f - pinball->lightAlphaList[i]) * timeDelta;
                    else
                        pinball->lightAlphaList[i] += (0.0f - pinball->lightAlphaList[i]) * timeDelta;
                }
    
                pinball->lightCurrentAlpha = pinball->lightAlphaList[i];
    
                BBMesh_draw(pinball->lightMeshes[i], uniformSetup, pinball, GL_TRUE, GL_FALSE);
            }
        }

#endif

        glDepthFunc(GL_LESS);

        glClear(GL_DEPTH_BUFFER_BIT);


        PRINT_PERF

        /* Top mirror */

		glDepthFunc(GL_ALWAYS);
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        BBMesh_draw(pinball->voidGlass, uniformSetup, pinball, GL_FALSE, GL_FALSE);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glDepthFunc(GL_GREATER);


        bbMatrixMirrorY(pinball->worldMatrix, TOP_GLASS_Y);
        glCullFace(GL_FRONT);
        pinball->outputMultiplier = pinball->viewMatrix[5];

/*      {
            GLint i;
            for (i = 0; i < 16; i++)
                printf("%f ", pinball->viewMatrix[i]);
            printf("\n");
        }*/

        {
            BBMesh* mesh = BBMeshStore_getFirstMesh(pinball->outerObjectStore);

            // why rocket science wanted to render this specific mesh first? seems to work fine in other order too =D
            //BBMesh_draw(mesh, uniformSetup, pinball, GL_FALSE, GL_TRUE);
            //mesh = BBMeshStore_getNextMesh(pinball->outerObjectStore, mesh);

            BBPinball_drawLedScreen(pinball);

            while (mesh)
            {
#if 1
                BBMesh_draw(mesh, uniformSetup, pinball, GL_FALSE, GL_TRUE);
#endif
                mesh = BBMeshStore_getNextMesh(pinball->outerObjectStore, mesh);
            }
        }

        glDepthFunc(GL_LESS);
        glCullFace(GL_BACK);
        bbMatrixIdentity(pinball->worldMatrix);
        pinball->outputMultiplier = 1.0f;

        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        
        PRINT_PERF

		/* Draw meshes from transparent list */
        {
/*
            BBMesh* mesh = BBMeshStore_getFirstMesh(pinball->transparentMeshStore);
            while (mesh)
            {
                BBMesh_draw(mesh, uniformSetup, pinball, GL_TRUE, GL_FALSE);
                mesh = BBMeshStore_getNextMesh(pinball->transparentMeshStore, mesh);
            }
*/

			BBMesh_draw(pinball->glassMesh, uniformSetup, pinball, GL_TRUE, GL_FALSE);
        }

        PRINT_PERF

//      glDepthMask(GL_TRUE);
//      glEnable(GL_DEPTH_TEST);

        /* HUD */

//      glDisable(GL_DEPTH_TEST);

        if (!pinball->playbackOn) // Hide HUD when playback is on
        {
            const GLint MENU_INACTIVITY_TIMEOUT = 10000;
            if (pinball->menuInactive)
            {
                GLint startScreenTime = ((pinball->currentTime - pinball->menuInactivityTimer) + 2000) % 30000;
                if (pinball->currentTime < 4000 && pinball->singleFrame == -1 && !pinball->hideHUD)
                {
                    pinball->hudPos[0] = 0.0f;
                    pinball->hudPos[1] = 0.0f;
                    pinball->hudScale[0] = 1.0f;
                    pinball->hudScale[1] = 1.0f;

                    pinball->hudColor[0] = 0.0f;
                    pinball->hudColor[1] = 0.0f;
                    pinball->hudColor[2] = 0.0f;
                    pinball->hudColor[3] = 1.0f;

                    BBMesh_draw(pinball->hudMesh, uniformSetupHUD, pinball, GL_TRUE, GL_FALSE);

                    pinball->hudTexScale[0] = 1.0f;
                    pinball->hudTexScale[1] = 1.0f;
                    pinball->hudTexPos[0] = 0.0f;
                    pinball->hudTexPos[1] = 0.0f;

                    pinball->hudScale[0] = 0.75f;
                    pinball->hudScale[1] = 1.0f;
                    pinball->hudPos[0] = 0.5f - pinball->hudScale[0] / 2.0f;
                    pinball->hudPos[1] = 0.5f - pinball->hudScale[1] / 2.0f;

                    pinball->hudColor[0] = 1.0f;
                    pinball->hudColor[1] = 1.0f;
                    pinball->hudColor[2] = 1.0f;
                    pinball->hudColor[3] = 1.0f - (pinball->currentTime - 3000) / 1000.0f;

                    BBMesh_draw(pinball->startScreen2Mesh, uniformSetupHUD, pinball, GL_TRUE, GL_FALSE);

                    pinball->hudColor[3] = 1.0f;
                }
                else if (startScreenTime < 10000 && !pinball->hideHUD)
                {
                    GLfloat a = 1.0f;

                    pinball->hudTexScale[0] = 1.0f;
                    pinball->hudTexScale[1] = 1.0f;
                    pinball->hudTexPos[0] = 0.0f;
                    pinball->hudTexPos[1] = 0.0f;

                    pinball->hudPos[0] = 0.0f;
                    pinball->hudPos[1] = 0.0f;
                    pinball->hudScale[0] = 1.0f;
                    pinball->hudScale[1] = 1.0f;

                    if (startScreenTime < 2000)
                    {
                        a = startScreenTime / 2000.0f;
                    }
                    else if (startScreenTime > 8000)
                    {
                        a = 1.0f - (startScreenTime - 8000) / 2000.0f;
                    }

                    pinball->hudColor[0] = 0.125f;
                    pinball->hudColor[1] = 0.25f;
                    pinball->hudColor[2] = 0.5f;
                    pinball->hudColor[3] = a * 0.5f;

                    BBMesh_draw(pinball->startScreenBackMesh, uniformSetupHUD, pinball, GL_TRUE, GL_FALSE);

                    pinball->hudColor[0] = 1.0f;
                    pinball->hudColor[1] = 1.0f;
                    pinball->hudColor[2] = 1.0f;
                    pinball->hudColor[3] = a * 2.0f;

                    BBMesh_draw(pinball->startScreenMesh, uniformSetupHUD, pinball, GL_TRUE, GL_FALSE);

                    pinball->hudColor[3] = 1.0f;
                }
            }
            else if (!pinball->gameRunning && pinball->currentTime - pinball->menuInactivityTimer >= MENU_INACTIVITY_TIMEOUT)
            {
                pinball->menuInactive = GL_TRUE;
                pinball->menuInactivityTimer = pinball->currentTime;
            }

            if (pinball->menuFadeTime)
            {
                GLboolean in = GL_FALSE;
                GLint f = pinball->menuFadeTime;

                if (f < 0)
                {
                    in = GL_TRUE;
                    f = -f;
                }

                if (in)
                {
                    pinball->menuAlpha = (pinball->currentTime - f) / 1000.0f;
                    if (pinball->menuAlpha > 1.0f)
                    {
                        pinball->menuAlpha = 1.0f;
                    }
                }
                else
                {
                    pinball->menuAlpha = 1.0f - (pinball->currentTime - f) / 1000.0f;
                    if (pinball->menuAlpha < 0.0f)
                    {
                        pinball->menuAlpha = 0.0f;
                    }
                }

                if (pinball->currentTime >= f + 1000)
                    pinball->menuFadeTime = 0;
            }

            if (pinball->menuAlpha > 0.0f && !pinball->menuInactive)
            {
                const char* START_GAME_STRING = "START GAME";
                const char* PLAYERS_STRING = "PLAYERS";
                const char* HIGHSCORES_STRING = "HIGHSCORES";

                GLint i;

                if (pinball->highscoreScrollTime)
                {
                    GLboolean out = GL_FALSE;
                    GLint f = pinball->highscoreScrollTime;
                    GLfloat t;

                    if (f < 0)
                    {
                        out = GL_TRUE;
                        f = -f;
                    }

                    t = (pinball->currentTime - f) / 1000.0f;

                    if (out)
                    {
                        pinball->highscoreScroll = 1.0f - t;
                        if (pinball->highscoreScroll <= 0.0f)
                        {
                            pinball->highscoreScroll = 0.0f;
                            pinball->highscoreScrollTime = 0;
                        }
                    }
                    else
                    {
                        pinball->highscoreScroll = t;
                        if (pinball->highscoreScroll >= 1.0f)
                        {
                            pinball->highscoreScroll = 1.0f;
                            pinball->highscoreScrollTime = 0;
                        }
                    }
                }

                if (pinball->highscoreCheckScrollTime)
                {
                    GLfloat t = (pinball->currentTime - pinball->highscoreCheckScrollTime) / 1000.0f;

                    pinball->highscoreScroll = -1.0f + t;
                    if (pinball->highscoreScroll >= 0.0f)
                    {
                        pinball->highscoreScroll = 0.0f;
                        pinball->highscoreCheckScrollTime = 0;
                    }
                }

                pinball->hudColor[3] = pinball->menuAlpha;

                if (pinball->highscoreCheckRunning)
                {
                    if (pinball->highscoreCheckNext)
                    {
                        GLint highestIndex = -1;
                        GLuint highest = 0;
                        GLuint p;
                        for (p = 0; p < pinball->numPlayers; p++)
                        {
                            if (!pinball->players[p].highscoreChecked && highest < pinball->players[p].score)
                            {
                                highest = pinball->players[p].score;
                                highestIndex = p;
                            }
                        }
                        if (highestIndex >= 0 && highest > pinball->highscores[9].score)
                        {
                            pinball->highscoreCheckCurrentPlayer = highestIndex;
                            pinball->highscoreCheckNamePos = 0;
                            pinball->highscoreScroll = -1.0f;
                            pinball->highscoreCheckNext = GL_FALSE;
                        }
                        else
                        {
                            pinball->highscoreCheckRunning = GL_FALSE;
                            if (pinball->highscoreScroll == -1.0f)
                                pinball->highscoreCheckScrollTime = pinball->currentTime;
                        }
                    }
                }

                if (pinball->highscoreScroll < 0.0f)
                {
                    const char* HIGHSCORE_STRING = "HIGHSCORE";
                    char text[16];
                    char unlitName[4];
                    char litName[4];
                    GLfloat a;
                    BBPinball_drawText(pinball, HIGHSCORE_STRING, 0.5f - SMALL_FONT_SCREEN_SIZE_X * strlen(HIGHSCORE_STRING) / 2.0f, 0.5f - SMALL_FONT_SCREEN_SIZE_Y / 2.0f - 1.0f - pinball->highscoreScroll, GL_TRUE);

                    sprintf(text, "PLAYER %d", pinball->highscoreCheckCurrentPlayer + 1);
                    BBPinball_drawText(pinball, text, 0.5f - SMALL_FONT_SCREEN_SIZE_X * strlen(text) / 2.0f, 0.55f - SMALL_FONT_SCREEN_SIZE_Y / 2.0f - 1.0f - pinball->highscoreScroll, GL_TRUE);

                    sprintf(text, "%d", pinball->players[pinball->highscoreCheckCurrentPlayer].score);
                    BBPinball_drawText(pinball, text, 0.5f - SMALL_FONT_SCREEN_SIZE_X * strlen(text) / 2.0f, 0.6f - SMALL_FONT_SCREEN_SIZE_Y / 2.0f - 1.0f - pinball->highscoreScroll, GL_TRUE);

                    unlitName[3] = '\0';
                    litName[3] = '\0';

                    for (i = 0; i < 3; i++)
                    {
                        if (i == pinball->highscoreCheckNamePos)
                        {
                            litName[i] = pinball->highscoreCheckName[i];
                            unlitName[i] = ' ';
                        }
                        else
                        {
                            unlitName[i] = pinball->highscoreCheckName[i];
                            litName[i] = ' ';
                        }
                    }

                    a = sinf(pinball->currentTime / 200.0f) * 0.25f + 0.75f;

                    pinball->hudColor[0] = a;
                    pinball->hudColor[1] = a;
                    pinball->hudColor[2] = a;
                    BBPinball_drawText(pinball, litName, 0.5f - SMALL_FONT_SCREEN_SIZE_X * strlen(litName) / 2.0f, 0.65f - SMALL_FONT_SCREEN_SIZE_Y / 2.0f - 1.0f - pinball->highscoreScroll, GL_TRUE);

                    pinball->hudColor[0] = 0.3f;
                    pinball->hudColor[1] = 0.3f;
                    pinball->hudColor[2] = 0.3f;
                    BBPinball_drawText(pinball, unlitName, 0.5f - SMALL_FONT_SCREEN_SIZE_X * strlen(unlitName) / 2.0f, 0.65f - SMALL_FONT_SCREEN_SIZE_Y / 2.0f - 1.0f - pinball->highscoreScroll, GL_TRUE);

                    pinball->hudColor[0] = 1.0f;
                    pinball->hudColor[1] = 1.0f;
                    pinball->hudColor[2] = 1.0f;
                }

                if (pinball->highscoreScroll > -1.0f && pinball->highscoreScroll < 1.0f)
                {
                    GLfloat alpha = (1.0f - bbAbs(pinball->highscoreScroll)) * pinball->menuAlpha;
                    pinball->hudTexScale[0] = 1.0f;
                    pinball->hudTexScale[1] = 1.0f;
                    pinball->hudTexPos[0] = 0.0f;
                    pinball->hudTexPos[1] = 0.0f;

                    pinball->hudPos[0] = 0.0f;
                    pinball->hudPos[1] = 0.0f;
                    pinball->hudScale[0] = 1.0f;
                    pinball->hudScale[1] = 1.0f;

                    pinball->hudColor[0] = 0.125f;
                    pinball->hudColor[1] = 0.25f;
                    pinball->hudColor[2] = 0.5f;
                    pinball->hudColor[3] = alpha * 0.5f;

                    BBMesh_draw(pinball->startScreenBackMesh, uniformSetupHUD, pinball, GL_TRUE, GL_FALSE);

                    pinball->hudColor[0] = 1.0f;
                    pinball->hudColor[1] = 1.0f;
                    pinball->hudColor[2] = 1.0f;
//                  pinball->hudColor[3] = a * 2.0f;
                    pinball->hudColor[3] = alpha;

                    BBMesh_draw(pinball->startScreenMesh, uniformSetupHUD, pinball, GL_TRUE, GL_FALSE);

                    pinball->hudPos[0] = 0.0f;
                    pinball->hudPos[1] = 0.7725f;
                    pinball->hudScale[0] = 1.0f;
                    pinball->hudScale[1] = 0.2f;

                    pinball->hudColor[0] = 0.0f;
                    pinball->hudColor[1] = 0.0f;
                    pinball->hudColor[2] = 0.0f;
                    BBMesh_draw(pinball->hudMesh, uniformSetupHUD, pinball, GL_TRUE, GL_FALSE);

                    pinball->hudColor[0] = 1.0f;
                    pinball->hudColor[1] = 1.0f;
                    pinball->hudColor[2] = 1.0f;

                    for (i = 0; i < 3; i++)
                    {
                        char text[16];
                        GLfloat yPos = 0.0f;

                        switch (i)
                        {
                        case 0:
                            yPos = -0.06f;
                            sprintf(text, "%s", START_GAME_STRING);
                            break;
                        case 1:
                            sprintf(text, "%s %d", PLAYERS_STRING, pinball->numPlayers);
                            break;
                        case 2:
                            yPos = 0.06f;
                            sprintf(text, "%s", HIGHSCORES_STRING);
                            break;
                        }

                        if (pinball->menuSelection != i)
                        {
                            pinball->hudColor[0] = 0.25f;
                            pinball->hudColor[1] = 0.25f;
                            pinball->hudColor[2] = 0.25f;
                        }

                        BBPinball_drawText(pinball, text, 0.5f - SMALL_FONT_SCREEN_SIZE_X * strlen(text) / 2.0f, 0.85f + yPos, GL_FALSE);

                        if (pinball->menuSelection != i)
                        {
                            pinball->hudColor[0] = 1.0f;
                            pinball->hudColor[1] = 1.0f;
                            pinball->hudColor[2] = 1.0f;
                        }
                    }
                    pinball->hudColor[3] = pinball->menuAlpha;
                }

                if (pinball->highscoreScroll > 0.0f)
                {
                    pinball->hudPos[0] = 0.275f;
                    pinball->hudPos[1] = 1.25f - pinball->highscoreScroll;
                    pinball->hudScale[0] = 0.45f;
                    pinball->hudScale[1] = BB_PINBALL_HIGHSCORE_COUNT * SMALL_FONT_SCREEN_SIZE_Y;

                    pinball->hudColor[0] = 0.0f;
                    pinball->hudColor[1] = 0.0f;
                    pinball->hudColor[2] = 0.0f;
                    pinball->hudColor[3] = 0.5f;
                    BBMesh_draw(pinball->hudMesh, uniformSetupHUD, pinball, GL_TRUE, GL_FALSE);
                    pinball->hudColor[3] = 1.0f;

                    for (i = 0; i < BB_PINBALL_HIGHSCORE_COUNT; i++)
                    {
                        GLfloat f;

                        char scoreText[16];
                        sprintf(scoreText, "%d", pinball->highscores[i].score);

                        f = sinf(pinball->currentTime / 400.0f + i * 0.9f) * 0.5f + 0.5f;

                        pinball->hudColor[0] = 0.5f + 0.5f * f;
                        pinball->hudColor[1] = 0.5f + 1.0f * f;
                        pinball->hudColor[2] = 0.5f + 1.0f * f;

                        BBPinball_drawText(pinball, pinball->highscores[i].name, 0.3f, i * SMALL_FONT_SCREEN_SIZE_Y + 0.005f + 1.25f - pinball->highscoreScroll, GL_TRUE);
                        BBPinball_drawText(pinball, scoreText, 0.7f - SMALL_FONT_SCREEN_SIZE_X * strlen(scoreText), i * SMALL_FONT_SCREEN_SIZE_Y + 0.005f + 1.25f - pinball->highscoreScroll, GL_TRUE);
                    }

                    pinball->hudColor[0] = 1.0f;
                    pinball->hudColor[1] = 1.0f;
                    pinball->hudColor[2] = 1.0f;
                }
            }
            
            if (pinball->showPlayerTime)
            {
                GLint step = -1;
                GLfloat t = (pinball->currentTime - pinball->showPlayerTime) / 500.0f;

                if (t < 1.0f)
                    step = 0;
                else if (t >= 1.0f && t < 3.0f)
                    step = 1;
                else if (t >= 3.0f)
                    step = 2;

                switch (step)
                {
                case 0:
                    pinball->hudColor[3] = t;
                    break;
                case 1:
                    pinball->hudColor[3] = 1.0f;
                    break;
                case 2:
                    pinball->hudColor[3] = 4.0f - t;
                    break;
                default:
                    pinball->hudColor[3] = 0.0f;
                    pinball->showPlayerTime = 0;
                    break;
                }

                if (pinball->hudColor[3] > 0.0f)
                {
                    char text[16];
                    sprintf(text, "PLAYER %d", pinball->currentPlayerIndex + 1);
                    BBPinball_drawText(pinball, text, 0.5f - SMALL_FONT_SCREEN_SIZE_X * strlen(text) / 2.0f, 0.47f - SMALL_FONT_SCREEN_SIZE_Y / 2.0f, GL_TRUE);

                    sprintf(text, "BALLS LEFT %d", pinball->player->ballsLeftLastChange);
                    BBPinball_drawText(pinball, text, 0.5f - SMALL_FONT_SCREEN_SIZE_X * strlen(text) / 2.0f, 0.53f - SMALL_FONT_SCREEN_SIZE_Y / 2.0f, GL_TRUE);
                }
            }
        }
/*      else if (startScreenTime < 10000)
        {
            GLfloat a = 1.0f;

            pinball->hudTexScale[0] = 1.0f;
            pinball->hudTexScale[1] = 1.0f;
            pinball->hudTexPos[0] = 0.0f;
            pinball->hudTexPos[1] = 0.0f;

            pinball->hudPos[0] = 0.0f;
            pinball->hudPos[1] = 0.0f;
            pinball->hudScale[0] = 1.0f;
            pinball->hudScale[1] = 1.0f;

            if (startScreenTime < 2000)
            {
                a = startScreenTime / 2000.0f;
            }
            else if (startScreenTime > 8000)
            {
                a = 1.0f - (startScreenTime - 8000) / 2000.0f;
            }

            pinball->hudColor[0] = pinball->hudColor[3] * 0.125f;
            pinball->hudColor[1] = pinball->hudColor[3] * 0.25f;
            pinball->hudColor[2] = pinball->hudColor[3] * 0.5f;
            pinball->hudColor[3] = a * 0.5f;

            BBMesh_draw(pinball->startScreenBackMesh, uniformSetupHUD, pinball, GL_TRUE, GL_FALSE);

            pinball->hudColor[0] = 1.0f;
            pinball->hudColor[1] = 1.0f;
            pinball->hudColor[2] = 1.0f;
            pinball->hudColor[3] = a * 2.0f;

            BBMesh_draw(pinball->startScreenMesh, uniformSetupHUD, pinball, GL_TRUE, GL_FALSE);

            pinball->hudColor[3] = 1.0f;
        }*/
        
        PRINT_PERF

#if defined(__VERSATILE__) && !defined(PERFMONITOR)
        totalTime = 0;
        for (i = 0; i < block; ++i)
        {
            printf("Block %d: %d ms\n", i + 1, blockTimes[i]);
            totalTime += blockTimes[i];
        }
        
        printf("Total time: %d ms\n", totalTime);
#endif

        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
    }
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

void symbianPrintf(char* string);

GLboolean BBPinball_main (BBPinball* pinball)
{
#if defined(__VERSATILE__) && !defined(PERFMONITOR)
    static int frame = 0;
    static int totalFrameTime = 0;
    unsigned int startTick, endTick;
    
    startTick = pinball->params.getTime();
#endif

#ifdef ENABLE_FPS_COUNTER
    static int framecount = 0;
    static float fps = 0;
    static unsigned  long long old_tick = 0;
    
    unsigned long long tick = pinball->params.getTime();

    ++ framecount;
   if ( tick - old_tick > 0 )
    {
#if defined(PRINTF_FPS) && defined(__SYMBIAN32__)
        char buffer[64];
#endif   
        fps = 0.2f * fps + 0.8f * (float)framecount * 1000.f / (tick - old_tick);

        framecount = 0;
        old_tick = tick;        
        pinball->players[3].score = (int)fps;
        
#ifdef PRINTF_FPS
#if defined(__SYMBIAN32__)
		sprintf(buffer,"fps: %.2f\n",fps);
        symbianPrintf(buffer);
#else
		printf("fps: %.2f\n",fps);
#endif
#endif
    }else{
          printf("time zone is too small %d - %d\n", tick, old_tick);
    }
#endif
    
    
    BB_ASSERT(pinball);

    if (pinball->currentTime == -1)
    {
        pinball->currentTime = pinball->params.getTime();
        pinball->lastUpdate = pinball->currentTime;
        pinball->lastCameraUpdate = pinball->currentTime;
        pinball->menuInactivityTimer = pinball->currentTime;
    }

    /*if (pinball->captureOn || !pinball->gameRunning || pinball->recordOn)
    {
        GLint time = pinball->params.getTime();
        while (time - pinball->currentTime < BB_PINBALL_CAPTURE_FRAME_MS)
        {
            time = pinball->params.getTime();
        }

        pinball->timeDelta = (GLfloat)BB_PINBALL_CAPTURE_FRAME_MS;
        pinball->currentTime += BB_PINBALL_CAPTURE_FRAME_MS;
    }
    else*/
    {
        GLint time = pinball->params.getTime();
        pinball->timeDelta = (GLfloat)(time - pinball->currentTime);
        pinball->timeDelta = pinball->timeDelta > 100 ? 100 : pinball->timeDelta;
        pinball->currentTime = time;
    }

    if (!pinball->gameRunning && !pinball->highscoreCheckRunning)
    {
        if (!BBPinball_playbackFrame(pinball, GL_FALSE))
            BBPinball_playbackFrame(pinball, GL_TRUE);

        BBPinball_calculateRacketMatrix(pinball, pinball->lowerRightRacketMatrix, BB_PINBALL_LOWERRIGHT_RACKET_POS, pinball->lowerRightRacketAngle);
        BBPinball_calculateRacketMatrix(pinball, pinball->lowerLeftRacketMatrix, BB_PINBALL_LOWERLEFT_RACKET_POS, pinball->lowerLeftRacketAngle);
        BBPinball_calculateRacketMatrix(pinball, pinball->upperRightRacketMatrix, BB_PINBALL_UPPERRIGHT_RACKET_POS, pinball->upperRightRacketAngle);
        BBPinball_calculateRacketMatrix(pinball, pinball->upperLeftRacketMatrix, BB_PINBALL_UPPERLEFT_RACKET_POS, pinball->upperLeftRacketAngle);
    }
    else
    {
        BBPinball_update(pinball);
        if (pinball->recordOn)
            BBPinball_storeFrame(pinball);
    }

    while (pinball->lastCameraUpdate < pinball->currentTime)
    {
        if (pinball->ballCamera)
        {
            GLfloat delta[3];
            GLfloat eyeDelta[3];
            const GLfloat lookAtSpeed = 32.0f;
            const GLfloat eyeSpeed = 32.0f;

            if (pinball->gameRunning)
            {
                if (!pinball->ballInPlay)
                {
                    GLfloat t = pinball->currentTime / 6000.0f;

                    delta[0] = 0.0f;
                    delta[1] = BB_PINBALL_CAMERA_CENTER_Y;
                    delta[2] = BB_PINBALL_CAMERA_CENTER_Z - 10.0f;

                    eyeDelta[0] = 0.0f;
                    eyeDelta[1] = BB_PINBALL_CAMERA_CENTER_Y + 22.0f + sinf(t) * 8.0f;
                    eyeDelta[2] = BB_PINBALL_CAMERA_CENTER_Z + 50.0f;
                }
                else
                {
                    if (pinball->scorePeek)
                    {
                        delta[0] = 0.0f;
                        delta[1] = BB_PINBALL_CAMERA_CENTER_Y + 10.0f;
                        delta[2] = BB_PINBALL_CAMERA_CENTER_Z - 10.0f;

                        eyeDelta[0] = 0.0f;
                        eyeDelta[1] = BB_PINBALL_CAMERA_CENTER_Y + 22.0f;
                        eyeDelta[2] = BB_PINBALL_CAMERA_CENTER_Z + 20.0f;
                    }
                    else
                    {
                        BBBall* ball = &pinball->ball;
                        GLfloat dist = 1.0f + bbClamp(bbVec3Length(ball->vel) / 0.3f, 0.0f, 1.5f);

                        delta[0] = ball->pos[0] + ball->vel[0] * dist * 4.0f;
                        delta[1] = ball->pos[2] + ball->vel[2] * dist * 4.0f + BB_PINBALL_TABLE_SURFACE_HEIGHT;
                        delta[2] = ball->pos[1] + ball->vel[1] * dist * 4.0f;

                        eyeDelta[0] = ball->pos[0];
                        eyeDelta[1] = ball->pos[2] + 24.0f * dist + BB_PINBALL_TABLE_SURFACE_HEIGHT;
                        eyeDelta[2] = ball->pos[1] + 12.0f * dist;
                    }
                }
            }
            else
            {
                if ((pinball->currentTime & (16384 - 1)) < 8192)
                {
                    GLfloat t = pinball->currentTime / 7000.0f + BB_PI * 3.0f / 2.0f;

                    t = t - sinf(t * 2.0f) * 0.4f;

                    delta[0] = 0.0f;
                    delta[1] = BB_PINBALL_CAMERA_CENTER_Y - 20.0f;
                    delta[2] = BB_PINBALL_CAMERA_CENTER_Z - 35.0f + bbAbs(cosf(t)) * 30.0f;

                    eyeDelta[0] = sinf(t) * 55.0f - 10.0f;
                    eyeDelta[1] = BB_PINBALL_CAMERA_CENTER_Y + 40.0f - bbAbs(cosf(t)) * 20.0f;
                    eyeDelta[2] = BB_PINBALL_CAMERA_CENTER_Z + 90.0f - bbAbs(cosf(t)) * 70.0f;
                }
                else
                {
                    BBBall* ball = &pinball->ball;
                    GLfloat dist = 1.0f + bbClamp(bbVec3Length(ball->vel) / 0.3f, 0.0f, 1.5f);

                    delta[0] = ball->pos[0] + ball->vel[0] * dist * 4.0f;
                    delta[1] = ball->pos[2] + ball->vel[2] * dist * 4.0f + BB_PINBALL_TABLE_SURFACE_HEIGHT;
                    delta[2] = ball->pos[1] + ball->vel[1] * dist * 4.0f;

                    eyeDelta[0] = ball->pos[0];
                    eyeDelta[1] = ball->pos[2] + 24.0f * dist + BB_PINBALL_TABLE_SURFACE_HEIGHT;
                    eyeDelta[2] = ball->pos[1] + 12.0f * dist;
                }
            }

            bbVec3Sub(delta, pinball->lookAt, delta);

            delta[0] *= 1.0f / lookAtSpeed;
            delta[1] *= 1.0f / lookAtSpeed;
            delta[2] *= 1.0f / lookAtSpeed;

            bbVec3Add(pinball->lookAt, delta, pinball->lookAt);

            bbVec3Sub(eyeDelta, pinball->eye, eyeDelta);

            eyeDelta[0] *= 1.0f / eyeSpeed;
            eyeDelta[1] *= 1.0f / eyeSpeed;
            eyeDelta[2] *= 1.0f / eyeSpeed;

            bbVec3Add(pinball->eye, eyeDelta, pinball->eye);
        }
        pinball->lastCameraUpdate += 10;
    }

/*      {
        GLfloat vec[2];
        vec[0] = cosf(BB_PI - BB_PINBALL_LOWER_RACKET_FACING_DIR - pinball->leftRacketAngle) * 4.0f;
        vec[1] = sinf(BB_PI - BB_PINBALL_LOWER_RACKET_FACING_DIR - pinball->leftRacketAngle) * 4.0f;
        bbVec2Add(BB_PINBALL_LOWERLEFT_RACKET_POS, vec, vec);
        pinball->debugPos[0] = vec[0];
        pinball->debugPos[1] = 51.0f;
        pinball->debugPos[2] = vec[1];
    }*/

    {
        GLfloat eye[3];
        GLfloat lookAt[3];
        memcpy(eye, pinball->eye, sizeof(GLfloat) * 3);
        memcpy(lookAt, pinball->lookAt, sizeof(GLfloat) * 3);
        eye[0] += pinball->nudgeVel;
        lookAt[0] += pinball->nudgeVel;
        bbMatrixView(eye, lookAt, 0.0f, pinball->viewMatrix);
    }
    // render, capture and swap only if not waiting for single frame or we have reached that specific frame
    if ( pinball->singleFrame == -1 || pinball->singleFrame == pinball->currentRecordPos )
    {
        BBPinball_draw(pinball);

        if (pinball->captureOn)
        {
            char name[1024];

            glReadPixels(0, 0, pinball->params.surfaceWidth, pinball->params.surfaceHeight, GL_RGBA, GL_UNSIGNED_BYTE, pinball->captureBuffer);

            {
                GLubyte* buf = pinball->captureBuffer;
                GLint c = pinball->params.surfaceWidth * pinball->params.surfaceHeight;
                while (c--)
                {
                    GLubyte t = buf[0];
                    buf[0] = buf[2];
                    buf[2] = t;
                    buf[3] = 0xff;  // full alpha
                    buf += 4;
                }
            }

            sprintf(name, "%s_%.08x.tga", pinball->capturePrefix ? pinball->capturePrefix : BB_PINBALL_CAPTURE_DIR, pinball->captureFrame);

            BBTga_save(name, pinball->params.surfaceWidth, pinball->params.surfaceHeight, 4, pinball->captureBuffer, GL_FALSE);
            pinball->captureFrame++;
        }

        BBPinball_bglSwapBuffer(pinball->params.display, pinball->params.mainSurface);

        if ( pinball->singleFrame == pinball->currentRecordPos || pinball->currentRecordPos >= pinball->maxFrames )
        {
            // if we are having playback and want to just export single frame, then exit when it is done
            exit(0);
        }
    }

#ifdef USBBLIT
    if ( ((pinball->params.surfaceHeight == 320 && pinball->params.surfaceWidth == 240) ||
          (pinball->params.surfaceHeight == 240) && (pinball->params.surfaceWidth == 320)) && pinball->usb && pinball->usb->isOk)
    {
        static unsigned char    pixbuf0[320 * 240 * 3];
        static unsigned short   pixbuf1[320 * 240];
        glReadPixels(0, 0, pinball->params.surfaceWidth, pinball->params.surfaceHeight, 0x80E0, GL_UNSIGNED_BYTE, pixbuf0);
        {
            unsigned int y, y2;
            unsigned char* pix = pixbuf0;
            for (y = 0, y2 = pinball->params.surfaceHeight; y < pinball->params.surfaceHeight; ++y, --y2)
            {
                unsigned int x, i;
                for (x = 0, i = 0; x < pinball->params.surfaceWidth; ++x, i += 3)
                {
                    unsigned int r8, g8, b8;
                    unsigned int r5, g6, b5;

                    b8 = pix[0] & 0xff;
                    g8 = pix[1] & 0xff;
                    r8 = pix[2] & 0xff;
                    pix += 3;

                    r5 = r8 >> 3;
                    g6 = g8 >> 2;
                    b5 = b8 >> 3;

                    if (pinball->params.surfaceWidth == 240)
                        pixbuf1[x + y2 * 240] = (r5 << 11) | (g6 << 5) | b5;
                    else
                        pixbuf1[(320 - x) + (240 - y2) * 320] = (r5 << 11) | (g6 << 5) | b5; 
                }
            }
        }

        usbBlit(pinball->usb, pixbuf1, 320 * 240, 0);
    }
#endif

#if defined(__VERSATILE__) && !defined(PERFMONITOR)
    endTick = pinball->params.getTime();
    ++frame;
    totalFrameTime += endTick - startTick;
    printf("Time: %d ms, FPS: %.2f, avg FPS: %.2f\n", (endTick - startTick), 1000.0f / (float)(endTick - startTick), (float)frame / ((float)totalFrameTime / 1000.0f));
#endif

    return pinball->quit;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

void BBPinball_update (BBPinball* pinball)
{
    GLint i;
    GLboolean touchieNotTouched = GL_TRUE;
    GLboolean addNudge = GL_FALSE;

    BB_ASSERT(pinball);

/*  if (pinball->gameRunning)
    {
        char text[16];
        sprintf(text, "%d", pinball->player->score);
        pinball->hudColor[3] = 1.0f;
        BBPinball_drawText(pinball, text, 0.01f, 0.95f, GL_TRUE);
    }
    else*/
    if (!pinball->gameRunning && pinball->nextLightCycleTime <= pinball->currentTime)
    {
        pinball->nextLightCycleTime = pinball->currentTime + 250;

        for (i = 0; i < BB_PINBALL_LIGHT_COUNT; i++)
            pinball->player->lightOnList[i] = GL_FALSE;

        if ((pinball->nextLightCycleIndex % 4) == 0)
            pinball->player->lightOnList[0] = GL_TRUE;
        pinball->player->lightOnList[(pinball->nextLightCycleIndex % 5) + 1] = GL_TRUE;
        pinball->player->lightOnList[(pinball->nextLightCycleIndex % 3) + 6] = GL_TRUE;
        pinball->player->lightOnList[(pinball->nextLightCycleIndex % 3) + 9] = GL_TRUE;
        pinball->player->lightOnList[(pinball->nextLightCycleIndex % 5) + 12] = GL_TRUE;
        pinball->player->lightOnList[(pinball->nextLightCycleIndex % 4) + 17] = GL_TRUE;

        pinball->nextLightCycleIndex++;
    }

    if (pinball->nudge)
    {
        pinball->nudge = GL_FALSE;
        addNudge = GL_TRUE;
        pinball->nudgeVel = 0.5f + (rand() & 65535) * 0.1f / 65535.0f;
        if ((rand() & 255) >= 127)
            pinball->nudgeVel = -pinball->nudgeVel;
    }

    while (pinball->lastUpdate < pinball->currentTime)
    {
        if (pinball->leftRacketKeyDown)
        {
            pinball->lowerLeftRacketAngleVel = (-BB_PINBALL_LOWERRACKET_MAX_ANGLE - pinball->lowerLeftRacketAngle) * BB_PINBALL_RACKET_TURN_SPEED;
            pinball->upperLeftRacketAngleVel = (-BB_PINBALL_UPPERRACKET_MAX_ANGLE - pinball->upperLeftRacketAngle) * BB_PINBALL_RACKET_TURN_SPEED;
        }
        else
        {
            pinball->lowerLeftRacketAngleVel = (0.0f - pinball->lowerLeftRacketAngle) * BB_PINBALL_RACKET_TURN_SPEED;
            pinball->upperLeftRacketAngleVel = (0.0f - pinball->upperLeftRacketAngle) * BB_PINBALL_RACKET_TURN_SPEED;
        }

        pinball->lowerLeftRacketAngle += pinball->lowerLeftRacketAngleVel;
        pinball->upperLeftRacketAngle += pinball->upperLeftRacketAngleVel;

        if (pinball->rightRacketKeyDown)
        {
            pinball->lowerRightRacketAngleVel = (BB_PINBALL_LOWERRACKET_MAX_ANGLE - pinball->lowerRightRacketAngle) * BB_PINBALL_RACKET_TURN_SPEED;
            pinball->upperRightRacketAngleVel = (BB_PINBALL_UPPERRACKET_MAX_ANGLE - pinball->upperRightRacketAngle) * BB_PINBALL_RACKET_TURN_SPEED;
        }
        else
        {
            pinball->lowerRightRacketAngleVel = (0.0f - pinball->lowerRightRacketAngle) * BB_PINBALL_RACKET_TURN_SPEED;
            pinball->upperRightRacketAngleVel = (0.0f - pinball->upperRightRacketAngle) * BB_PINBALL_RACKET_TURN_SPEED;
        }

        pinball->lowerRightRacketAngle += pinball->lowerRightRacketAngleVel;
        pinball->upperRightRacketAngle += pinball->upperRightRacketAngleVel;

        BBPinball_calculateRacketMatrix(pinball, pinball->lowerRightRacketMatrix, BB_PINBALL_LOWERRIGHT_RACKET_POS, pinball->lowerRightRacketAngle);
        BBPinball_calculateRacketMatrix(pinball, pinball->lowerLeftRacketMatrix, BB_PINBALL_LOWERLEFT_RACKET_POS, pinball->lowerLeftRacketAngle);
        BBPinball_calculateRacketMatrix(pinball, pinball->upperRightRacketMatrix, BB_PINBALL_UPPERRIGHT_RACKET_POS, pinball->upperRightRacketAngle);
        BBPinball_calculateRacketMatrix(pinball, pinball->upperLeftRacketMatrix, BB_PINBALL_UPPERLEFT_RACKET_POS, pinball->upperLeftRacketAngle);

        for (i = 0; i < 1; i++)
        {
            BBBall* ball = &pinball->ball;
            BBPinball_ballPhysics(pinball, ball);
            if (addNudge)
                ball->vel[0] += pinball->nudgeVel * 0.25f;

            if (pinball->ballInPlay && ball->pos[1] > BB_PINBALL_BALL_SLOT_Y)
            {
                if (pinball->player->freeLaunch)
                {
#ifdef BB_DEVEL
                    printf("Game: freeLaunch = false\n");
#endif
                    pinball->player->freeLaunch = GL_FALSE;

                    pinball->ball.pos[0] = BB_PINBALL_BALL_START_POS[0];
                    pinball->ball.pos[1] = BB_PINBALL_BALL_START_POS[1];
                    pinball->ball.pos[2] = 0.0f;

                    pinball->ballLaunchTime = pinball->currentTime;
                    BBPinball_playSFX(pinball, BB_SFX_BALL_LAUNCH, 1.0f, GL_FALSE);
                }
                else
                {
                    pinball->ball.pos[0] = BB_PINBALL_BALL_START_POS[0];
                    pinball->ball.pos[1] = BB_PINBALL_BALL_START_POS[1];
                    pinball->ball.pos[2] = 0.0f;
                    pinball->ball.vel[0] = 0.0f;
                    pinball->ball.vel[1] = 0.0f;
                    pinball->ball.vel[2] = 0.0f;
                    pinball->ballInPlay = GL_FALSE;

                    if (!BBPinball_changePlayer(pinball, pinball->currentPlayerIndex + 1))
                    {
                        BBPinball_endGame(pinball, GL_TRUE);
                    }
                }
            }

            if (pinball->ballLaunchTime && pinball->currentTime > pinball->ballLaunchTime + 1410)
            {
                BBBall* ball = &pinball->ball;
                ball->pos[0] = BB_PINBALL_BALL_START_POS[0];
                ball->pos[1] = BB_PINBALL_BALL_START_POS[1];
                ball->pos[2] = 0.0f;
                ball->vel[0] = 0.0f;
                ball->vel[1] = -(1.0f + (rand() & 0x7fff) / 65536.0f) * BB_PINBALL_LAUNCH_POWER;
                ball->vel[2] = 0.0f;

                pinball->ballDoorClosed = GL_FALSE;

                pinball->player->ballsLeft--;

                pinball->ballLaunchTime = 0;
            }

            if (!pinball->ballLocked &&
                pinball->currentTime - pinball->ballReleaseTime > BB_PINBALL_BALL_LOCK_DURATION)
            {
                GLfloat lockDistVec[2];
                bbVec2Sub(ball->pos, BB_PINBALL_BALL_LOCK_POS, lockDistVec);
                if (bbVec2Length(lockDistVec) < BB_PINBALL_BALL_LOCK_RADIUS)
                {
                    pinball->ballLocked = GL_TRUE;
                    pinball->ballReleaseTime = pinball->currentTime + BB_PINBALL_BALL_LOCK_DURATION;
                    pinball->player->score += BB_PINBALL_SCORE_LOCK * pinball->player->scoreMultiplier;
                    pinball->player->lightOnList[0] = GL_FALSE;
                    BBPinball_playSFX(pinball, BB_SFX_BALL_LOCK, 1.0f, GL_FALSE);
                }
            }

            if (pinball->ballLocked)
            {
                if (pinball->currentTime >= pinball->ballReleaseTime)
                {
                    ball->pos[2] = 0.0f;
                    pinball->ballLocked = GL_FALSE;
                    BBPinball_playSFX(pinball, BB_SFX_BALL_RELEASE, 1.0f, GL_FALSE);
                }
                else
                {
                    memcpy(ball->pos, BB_PINBALL_BALL_LOCK_POS, sizeof(GLfloat) * 2);
                    ball->pos[2] = -BB_PINBALL_BALL_LOCK_DEPTH;
                    ball->vel[0] = 0.0f;
                    ball->vel[1] = 0.0f;
                    ball->vel[2] = 0.0f;
                }
            }

            {
                GLfloat touchieDistVec[2];
                bbVec2Sub(ball->pos, BB_PINBALL_TOUCHIE_POS, touchieDistVec);
                if (bbVec2Length(touchieDistVec) < BB_PINBALL_TOUCHIE_RADIUS)
                    touchieNotTouched = GL_FALSE;
            }
        }

        if (touchieNotTouched)
        {
            pinball->touchieDown = GL_FALSE;
            pinball->touchieMove += (0.0f - pinball->touchieMove) / BB_PINBALL_TOUCHIE_MOVE_SPEED;
        }
        else
        {
            pinball->touchieMove += (BB_PINBALL_TOUCHIE_MAX_MOVE - pinball->touchieMove) / BB_PINBALL_TOUCHIE_MOVE_SPEED;
            if (!pinball->touchieDown)
            {
                pinball->touchieDown = GL_TRUE;
                pinball->player->score += BB_PINBALL_SCORE_TOUCHIE;
                BBPinball_playSFX(pinball, BB_SFX_TOUCHIE, 1.0f, GL_FALSE);
            }
        }

        if (pinball->ball.pos[1] < 48.0f)
            pinball->ballDoorClosed = GL_TRUE;

        for (i = 0; i < BB_PINBALL_DROPTARGET_COUNT; i++)
        {
            if (pinball->player->dropTargetDownList[i])
                pinball->dropTargetPosList[i] += (-1.8f - pinball->dropTargetPosList[i]) * pinball->timeDelta / BB_PINBALL_DROPTARGET_RAISE_TIME;
            else
                pinball->dropTargetPosList[i] += (0.0f - pinball->dropTargetPosList[i]) * pinball->timeDelta / BB_PINBALL_DROPTARGET_RAISE_TIME;
        }

        for (i = 0; i < BB_PINBALL_DROPTARGET_GROUP_COUNT; i++)
        {
            if (pinball->player->dropTargetGroupDown[i] == BB_PINBALL_DROPTARGET_GROUP_SIZE)
            {
                /* Whole group down */
                pinball->dropTargetGroupRaiseTime[i] = pinball->currentTime + BB_PINBALL_DROPTARGET_GROUP_RAISE_DELAY;
                pinball->player->dropTargetGroupDown[i] = (i == 1 || i == 2) ? 1 : 0;

                pinball->player->cycleArrowLights = GL_TRUE;
                BBPinball_playSFX(pinball, BB_SFX_ELECTRIC2, 1.0f, GL_FALSE);

#ifdef BB_DEVEL
                printf("Game: Extra ball\n");
#endif
                pinball->player->ballsLeft++;
            }
            if (pinball->dropTargetGroupRaiseTime[i] && pinball->currentTime > pinball->dropTargetGroupRaiseTime[i])
            {
                GLint t;
                for (t = 0; t < BB_PINBALL_DROPTARGET_GROUP_SIZE; t++)
                    pinball->player->dropTargetDownList[t + i * BB_PINBALL_DROPTARGET_GROUP_SIZE] = GL_FALSE;

                pinball->player->dropTargetDownList[7] = GL_TRUE;
                pinball->player->dropTargetDownList[11] = GL_TRUE;

                pinball->dropTargetGroupRaiseTime[i] = 0;

                BBPinball_playSFX(pinball, BB_SFX_DROPGROUPUP, 1.0f, GL_FALSE);
            }
        }

        for (i = 0; i < BB_PINBALL_SENSOR_COUNT; i++)
        {
            GLint b;
            GLboolean down = GL_FALSE;

            for (b = 0; b < 1; b++)
            {
                if (BBPinball_isInsideAreas(pinball, pinball->ball.pos, BB_PINBALL_SENSOR_RECTS + i * 4, 1))
                {
                    down = GL_TRUE;
                    break;
                }
            }

            if (pinball->sensorDownList[i] != down)
            {
                pinball->sensorDownList[i] = down;

                if (!down)
                {
                    switch (i)
                    {
                    case 0:
                    case 1:
                    case 2:
                    case 3:
                        if (pinball->player->lightOnList[i + 17] == GL_FALSE)
                            BBPinball_playSFX(pinball, BB_SFX_ELECTRIC2, 1.0f, GL_FALSE);

                        pinball->player->lightOnList[i + 17] = GL_TRUE;
                        pinball->player->score += BB_PINBALL_SCORE_LIGHT_SWITCH;

                        if (!pinball->turnUpperLightsOffTime &&
                            pinball->player->lightOnList[17] &&
                            pinball->player->lightOnList[18] &&
                            pinball->player->lightOnList[19] &&
                            pinball->player->lightOnList[20])
                        {
                            pinball->turnUpperLightsOffTime = pinball->currentTime +
                                BB_PINBALL_UPPER_LIGHTS_TURN_OFF_DELAY;
                        }
                        break;
                    case 4:
                    case 5:
                        pinball->player->score += BB_PINBALL_SCORE_LEFT_RAMP;
                        break;
                    case 6:
                        {
                            if (pinball->player->cycleArrowLights)
                            {
                                GLint i;
                                pinball->player->cycleArrowLights = GL_FALSE;
                                for (i = 0; i < 5; i++)
                                    pinball->player->lightOnList[i + 1] = GL_FALSE;

                                if (pinball->player->lightOnList[0] == GL_FALSE)
                                    BBPinball_playSFX(pinball, BB_SFX_ELECTRIC2, 1.0f, GL_FALSE);

                                pinball->player->lightOnList[0] = GL_TRUE; /* lock */
                            }

                            pinball->player->score += BB_PINBALL_SCORE_RIGHT_RAMP;
                        }
                        break;

                    case 7:
                        if (pinball->player->lightOnList[11])
                        {
                            BBPinball_playSFX(pinball, BB_SFX_ELECTRIC1, 1.0f, GL_FALSE);
                            pinball->player->lightOnList[11] = GL_FALSE; /* left 500 */
                            pinball->player->leftLightsOffCount++;
                            pinball->player->score += 500;
                        }
                        break;
                    case 8:
                        if (pinball->player->lightOnList[10])
                        {
                            BBPinball_playSFX(pinball, BB_SFX_ELECTRIC1, 1.0f, GL_FALSE);
                            pinball->player->lightOnList[10] = GL_FALSE; /* left 300 */
                            pinball->player->leftLightsOffCount++;
                            pinball->player->score += 300;
                        }
                        break;
                    case 9:
                        if (pinball->player->lightOnList[9])
                        {
                            BBPinball_playSFX(pinball, BB_SFX_ELECTRIC1, 1.0f, GL_FALSE);
                            pinball->player->lightOnList[9] = GL_FALSE; /* left 150 */
                            pinball->player->leftLightsOffCount++;
                            pinball->player->score += 150;
                        }
                        break;
                    case 10:
                        if (pinball->player->lightOnList[6])
                        {
                            BBPinball_playSFX(pinball, BB_SFX_ELECTRIC1, 1.0f, GL_FALSE);
                            pinball->player->lightOnList[6] = GL_FALSE; /* right 150 */
                            pinball->player->rightLightsOffCount++;
                            pinball->player->score += 150;
                        }
                        break;
                    case 11:
                        if (pinball->player->lightOnList[7])
                        {
                            BBPinball_playSFX(pinball, BB_SFX_ELECTRIC1, 1.0f, GL_FALSE);
                            pinball->player->lightOnList[7] = GL_FALSE; /* right 300 */
                            pinball->player->rightLightsOffCount++;
                            pinball->player->score += 300;
                        }
                        break;
                    case 12:
                        if (pinball->player->lightOnList[8])
                        {
                            BBPinball_playSFX(pinball, BB_SFX_ELECTRIC1, 1.0f, GL_FALSE);
                            pinball->player->lightOnList[8] = GL_FALSE; /* right 500 */
                            pinball->player->rightLightsOffCount++;
                            pinball->player->score += 500;
                        }
                        break;

                    default:
                        break;
                    }
                }
            }
        }

        if (pinball->player->leftLightsOffCount == 3)
        {
            pinball->player->leftLightsOffCount = 0;
            pinball->player->lightOnList[9] = GL_TRUE; /* left 500 */
            pinball->player->lightOnList[10] = GL_TRUE; /* left 300 */
            pinball->player->lightOnList[11] = GL_TRUE; /* left 150 */
            if (!pinball->player->freeLaunch)
            {
                pinball->player->freeLaunch = GL_TRUE;
#ifdef BB_DEVEL
                printf("Game: freeLaunch = true\n");
#endif
            }
        }

        if (pinball->player->rightLightsOffCount == 3)
        {
            pinball->player->rightLightsOffCount = 0;
            pinball->player->lightOnList[6] = GL_TRUE; /* right 500 */
            pinball->player->lightOnList[7] = GL_TRUE; /* right 300 */
            pinball->player->lightOnList[8] = GL_TRUE; /* right 150 */
            if (!pinball->player->freeLaunch)
            {
                pinball->player->freeLaunch = GL_TRUE;
#ifdef BB_DEVEL
                printf("Game: freeLaunch = true\n");
#endif
            }
        }

        pinball->nudgeVel = -pinball->nudgeVel;
        pinball->nudgeVel *= 0.95f;

        addNudge = GL_FALSE;
        pinball->lastUpdate += BB_PINBALL_UPDATE_FREQ;
//      pinball->lastUpdate = pinball->currentTime;
    }

    if (pinball->turnUpperLightsOffTime && pinball->currentTime > pinball->turnUpperLightsOffTime)
    {
        pinball->turnUpperLightsOffTime = 0;

        if (pinball->player->scoreMultiplier < 5)
        {
            pinball->player->lightOnList[12 + pinball->player->scoreMultiplier] = GL_TRUE;
            pinball->player->scoreMultiplier++;
        }

        pinball->player->lightOnList[17] = GL_FALSE;
        pinball->player->lightOnList[18] = GL_FALSE;
        pinball->player->lightOnList[19] = GL_FALSE;
        pinball->player->lightOnList[20] = GL_FALSE;

        BBPinball_playSFX(pinball, BB_SFX_ELECTRIC1, 1.0f, GL_FALSE);
    }

    if (pinball->player->cycleArrowLights)
    {
        GLint i;
        for (i = 0; i < 5; i++)
            pinball->player->lightOnList[i + 1] = sinf((GLfloat)i - pinball->currentTime / 200.0f) > 0.5f;
    }

/*  {
        GLint i;
        for (i = 0; i < BB_PINBALL_BUMPER_LIGHT_COUNT; i++)
            pinball->bumperLightValues[i] *= 0.95f;
    }*/
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

void BBPinball_keyEvent (BBPinball* pinball, GLboolean down, GLint key)
{
    BB_ASSERT(pinball);

    switch (key)
    {
    case BB_KEYCODE_LEFT_MOUSE:
        pinball->leftDown = down;
        break;
    case BB_KEYCODE_MIDDLE_MOUSE:
        pinball->middleDown = down;
        break;
    case BB_KEYCODE_RIGHT_MOUSE:
        pinball->rightDown = down;
        break;
    case BB_KEYCODE_WHEEL_FORWARD:
        if (down && !pinball->ballCamera)
        {
            pinball->cameraDist -= 4.0f;
            BBPinball_updateCamera(pinball);
        }
        break;
    case BB_KEYCODE_WHEEL_BACKWARD:
        if (down && !pinball->ballCamera)
        {
            pinball->cameraDist += 4.0f;
            BBPinball_updateCamera(pinball);
        }
        break;
    case BB_KEYCODE_CHANGE_CAMERA:
        if (down)
        {
            pinball->ballCamera = !pinball->ballCamera;
            if (!pinball->ballCamera)
                BBPinball_updateCamera(pinball);
        }
        break;

/**/
	case BB_KEYCODE_TOGGLE_PLAYBACK:
        if (down && !pinball->recordOn)
        {
            pinball->playbackOn = !pinball->playbackOn;
            if (pinball->playbackOn)
            {
                pinball->playbackStartTime = pinball->params.getTime();
                BBPinball_playbackFrame(pinball, GL_TRUE);
            }
            else
            {
                pinball->lastUpdate = pinball->currentTime;
                pinball->lastCameraUpdate = pinball->currentTime;
            }
        }
        break;
/**/

    case BB_KEYCODE_ESCAPE:
        if (down)
        {
            if (pinball->gameRunning)
            {
                BBPinball_endGame(pinball, GL_FALSE);
            }
            else
            {
                pinball->quit = GL_TRUE;
            }
        }
        break;
    }

    if (!pinball->gameRunning)
    {
        if (pinball->menuInactive)
        {
            switch (key)
            {
            case BB_KEYCODE_LAUNCH_BALL:
            case BB_KEYCODE_UP_ARROW:
            case BB_KEYCODE_LEFT_ARROW:
            case BB_KEYCODE_DOWN_ARROW:
            case BB_KEYCODE_RIGHT_ARROW:
                if (down)
                {
                    pinball->menuInactive = GL_FALSE;
                    pinball->menuInactivityTimer = pinball->currentTime;
                    pinball->menuAlpha = 1.0f;
                }
                break;
            }
        }
        else if (pinball->highscoreCheckRunning)
        {
            switch (key)
            {
            case BB_KEYCODE_UP_ARROW:
                if (down)
                {
                    if (!pinball->menuInactive)
                    {
                        pinball->menuInactivityTimer = pinball->currentTime;
                    }
                    BBPinball_playSFX(pinball, BB_SFX_MENU_UP, 1.0f, GL_FALSE);
                    pinball->highscoreCheckName[pinball->highscoreCheckNamePos]--;
                    if (pinball->highscoreCheckName[pinball->highscoreCheckNamePos] < 65)
                        pinball->highscoreCheckName[pinball->highscoreCheckNamePos] = 90;
                }
                break;
            case BB_KEYCODE_DOWN_ARROW:
                if (down)
                {
                    if (!pinball->menuInactive)
                    {
                        pinball->menuInactivityTimer = pinball->currentTime;
                    }
                    BBPinball_playSFX(pinball, BB_SFX_MENU_DOWN, 1.0f, GL_FALSE);
                    pinball->highscoreCheckName[pinball->highscoreCheckNamePos]++;
                    if (pinball->highscoreCheckName[pinball->highscoreCheckNamePos] > 90)
                        pinball->highscoreCheckName[pinball->highscoreCheckNamePos] = 65;
                }
                break;
            case BB_KEYCODE_LAUNCH_BALL:
                if (down)
                {
                    if (!pinball->menuInactive)
                    {
                        pinball->menuInactivityTimer = pinball->currentTime;
                    }
                    if (pinball->highscoreCheckNamePos < 3)
                    {
                        pinball->highscoreCheckNamePos++;
                        BBPinball_playSFX(pinball, BB_SFX_MENU_SELECT, 1.0f, GL_FALSE);
                        if (pinball->highscoreCheckNamePos == 3)
                        {
                            pinball->players[pinball->highscoreCheckCurrentPlayer].highscoreChecked = GL_TRUE;
                            pinball->highscoreCheckNext = GL_TRUE;

                            {
                                GLint i;
                                GLint o;
                                for (i = 0; i < 10; i++)
                                {
                                    if (pinball->highscores[i].score < pinball->players[pinball->highscoreCheckCurrentPlayer].score)
                                    {
                                        break;
                                    }
                                }
                                BB_ASSERT(i != 10);
                                for (o = 9; o > i; o--)
                                {
                                    pinball->highscores[o].score = pinball->highscores[o - 1].score;
                                    memcpy(pinball->highscores[o].name, pinball->highscores[o - 1].name, 3);
                                }
                                pinball->highscores[i].score = pinball->players[pinball->highscoreCheckCurrentPlayer].score;
                                memcpy(pinball->highscores[i].name, pinball->highscoreCheckName, 3);
                            }
                        }
                    }
                }
                break;
            }
        }
        else if (pinball->highscoreScroll > 0.0f)
        {
            switch (key)
            {
            case BB_KEYCODE_LAUNCH_BALL:
                if (down)
                {
                    if (!pinball->menuInactive)
                    {
                        pinball->menuInactivityTimer = pinball->currentTime;
                    }
                    if (!pinball->highscoreScrollTime)
                    {
                        BBPinball_playSFX(pinball, BB_SFX_MENU_SELECT, 1.0f, GL_FALSE);
                        pinball->highscoreScrollTime = -pinball->currentTime;
                    }
                }
                break;
            }
        }
        else
        {
            switch (key)
            {
            case BB_KEYCODE_LAUNCH_BALL:
                if (down)
                {
                    if (!pinball->menuInactive)
                    {
                        pinball->menuInactivityTimer = pinball->currentTime;
                    }
                    BBPinball_playSFX(pinball, BB_SFX_MENU_SELECT, 1.0f, GL_FALSE);
                    switch (pinball->menuSelection)
                    {
                    case 0:
                        pinball->gameRunning = GL_TRUE;
                        pinball->menuFadeTime = pinball->currentTime;
                        BBPinball_newGame(pinball);
                        break;
                    case 1:
                        pinball->numPlayers++;
                        if (pinball->numPlayers > 4)
                            pinball->numPlayers = 1;
                        break;
                    case 2:
                        if (!pinball->highscoreScrollTime)
                            pinball->highscoreScrollTime = pinball->currentTime;
                        break;
                    }
                }
                break;
            case BB_KEYCODE_UP_ARROW:
//          case BB_KEYCODE_LEFT_ARROW:
                if (down)
                {
                    if (!pinball->menuInactive)
                    {
                        pinball->menuInactivityTimer = pinball->currentTime;
                    }
                    if (pinball->menuSelection > 0)
                    {
                        pinball->menuSelection--;
                        BBPinball_playSFX(pinball, BB_SFX_MENU_UP, 0.8f, GL_FALSE);
                    }
                }
                break;
            case BB_KEYCODE_DOWN_ARROW:
//          case BB_KEYCODE_RIGHT_ARROW:
                if (down)
                {
                    if (!pinball->menuInactive)
                    {
                        pinball->menuInactivityTimer = pinball->currentTime;
                    }
                    if (pinball->menuSelection < 2)
                    {
                        pinball->menuSelection++;
                        BBPinball_playSFX(pinball, BB_SFX_MENU_DOWN, 0.8f, GL_FALSE);
                    }
                }
                break;
            }
        }
    }
    else
    {
        switch (key)
        {
        case BB_KEYCODE_UP_ARROW:
            pinball->moveBallUp = down;
            break;
        case BB_KEYCODE_DOWN_ARROW:
            pinball->moveBallDown = down;
            break;
        case BB_KEYCODE_LEFT_ARROW:
            pinball->moveBallLeft = down;
            break;
        case BB_KEYCODE_RIGHT_ARROW:
            pinball->moveBallRight = down;
            break;

        case BB_KEYCODE_LAUNCH_BALL:
            if (!pinball->ballInPlay)
            {
                if (pinball->ballLaunchTime == 0 && down && pinball->player->ballsLeft > 0)
                {
                    pinball->ball.pos[0] = BB_PINBALL_BALL_START_POS[0];
                    pinball->ball.pos[1] = BB_PINBALL_BALL_START_POS[1];
                    pinball->ball.pos[2] = 0.0f;

                    pinball->ballLaunchTime = pinball->currentTime;
                    pinball->ballInPlay = GL_TRUE;

                    BBPinball_playSFX(pinball, BB_SFX_BALL_LAUNCH, 1.0f, GL_FALSE);
                }
            }
            else
            {
                pinball->scorePeek = down;
            }
            break;

        case BB_KEYCODE_LEFT_RACKET:
            if (pinball->leftRacketKeyDown != down)
            {
                if (down)
                {
                    BBPinball_scrollLights(pinball, GL_FALSE);
                    BBPinball_playSFX(pinball, BB_SFX_RACKET1, 1.0f, GL_FALSE);
                }
                else
                {
                    BBPinball_playSFX(pinball, BB_SFX_RACKET1B, 1.0f, GL_FALSE);
                }
                pinball->leftRacketKeyDown = down;
            }
            break;
        case BB_KEYCODE_RIGHT_RACKET:
            if (pinball->rightRacketKeyDown != down)
            {
                if (down)
                {
                    BBPinball_scrollLights(pinball, GL_TRUE);
                    BBPinball_playSFX(pinball, BB_SFX_RACKET2, 1.0f, GL_FALSE);
                }
                else
                {
                    BBPinball_playSFX(pinball, BB_SFX_RACKET2B, 1.0f, GL_FALSE);
                }
                pinball->rightRacketKeyDown = down;
            }
            break;

        case BB_KEYCODE_NUDGE:
            if (down)
                pinball->nudge = GL_TRUE;
            break;

        case BB_KEYCODE_TOGGLE_CAPTURE:
            if (down)
                pinball->captureOn = !pinball->captureOn;
            break;

        case BB_KEYCODE_START_RECORD:
            if (down)
            {
#ifndef BB_DISABLE_SOUNDS
                pinball->frameSFX = -1;
#endif // !BB_DISABLE_SOUNDS
                pinball->recordOn = GL_TRUE;
                pinball->playbackOn = GL_FALSE;
                pinball->recordStartTime = pinball->params.getTime();
                BBPinball_dumpRecording(pinball);
            }
            break;

        case BB_KEYCODE_STOP_RECORD:
            if (down)
            {
                pinball->recordOn = GL_FALSE;
            }
            break;
        }
    }
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

void BBPinball_ptrEvent (BBPinball* pinball, GLint x, GLint y)
{
    BB_ASSERT(pinball);

    if (!pinball->ballCamera && x != -1 && y != -1 && pinball->pointerX != -1 && pinball->pointerY != -1)
    {
        GLfloat deltaX = (GLfloat)(x - pinball->pointerX);
        GLfloat deltaY = (GLfloat)(y - pinball->pointerY);

        if (pinball->leftDown)
        {
            pinball->horzAngle += deltaX / 100.0f;
            pinball->vertAngle += deltaY / 100.0f;
            BBPinball_updateCamera(pinball);
        }

        if (pinball->middleDown)
        {
            GLfloat dir[2];
            dir[0] = cosf(pinball->horzAngle) * 0.5f;
            dir[1] = sinf(pinball->horzAngle) * 0.5f;
            pinball->lookAt[0] += dir[0] * deltaX - dir[1] * deltaY;
            pinball->lookAt[2] -= dir[0] * deltaY + dir[1] * deltaX;
            BBPinball_updateCamera(pinball);
        }
    }

    pinball->pointerX = x;
    pinball->pointerY = y;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

void BBPinball_updateCamera (BBPinball* pinball)
{
    GLfloat dir[3];
    
    BB_ASSERT(pinball);

    dir[0] = sinf(pinball->horzAngle);
    dir[1] = pinball->vertAngle;
    dir[2] = cosf(pinball->horzAngle);

    bbVec3Normalize(dir);

    pinball->eye[0] = pinball->lookAt[0] + dir[0] * pinball->cameraDist;
    pinball->eye[1] = pinball->lookAt[1] + dir[1] * pinball->cameraDist;
    pinball->eye[2] = pinball->lookAt[2] + dir[2] * pinball->cameraDist;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

/*BBBall* BBPinball_checkBallCollision (BBPinball* pinball,
                                        BBBall* current,
                                        const GLfloat* pos)
{
  GLint i;
    BB_ASSERT(pinball);

    for (i = 0; i < 1; i++)
    {
        if (&pinball->balls[i] == current)
            continue;

        {
            GLfloat d[3];
            GLfloat len;

            d[0] = pos[0] - pinball->balls[i].pos[0];
            d[1] = pos[1] - pinball->balls[i].pos[1];
            d[2] = pos[2] - pinball->balls[i].pos[2];

            len = bbVec3Length(d);
            if (len <= BB_PINBALL_BALL_RADIUS)
                return &pinball->balls[i];
        }
    }
    return NULL;
}
*/

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

GLboolean BBPinball_vectorCollision (BBPinball* pinball, const GLfloat* coords, GLsizei count,
                                     const GLfloat* matrix, const GLfloat* oldPoint,
                                     const GLfloat* newPoint, GLfloat* normal, GLboolean close)
{
    GLint i = 0;
    GLfloat startVec[2];
    GLfloat deltaNewOld[2];
    GLfloat endVec[2];
    GLfloat closest[2];
    GLint startIndex = i * 2;
    GLint endIndex = startIndex + 2;
    GLfloat deltaLen;

    BB_ASSERT(pinball && coords && oldPoint && newPoint && normal);

    bbVec2Sub(newPoint, oldPoint, deltaNewOld);
    deltaLen = bbVec2Length(deltaNewOld);

    startVec[0] = coords[startIndex];
    startVec[1] = coords[startIndex + 1];

    if (matrix)
        bbVec2Transform(startVec, matrix, startVec, 0, 2);

    while (i < count)
    {
        endVec[0] = coords[endIndex];
        endVec[1] = coords[endIndex + 1];

        if (matrix)
            bbVec2Transform(endVec, matrix, endVec, 0, 2);

        if (bbVec2ClosestPoint(startVec, endVec, newPoint, closest))
        {
            GLfloat distVec[2];
            GLfloat dist;
            bbVec2Sub(closest, oldPoint, distVec);
            dist = bbVec2Length(distVec);
            bbVec2CrossProduct(endVec, startVec, normal);
            if (dist < deltaLen && bbVec2DotProduct(deltaNewOld, normal) < 0.0f)
            {
                bbVec2Normalize(normal);
                return GL_TRUE;
            }
        }

        i++;
        if (i < count)
        {
            startIndex += 2;
            endIndex += 2;
            if (endIndex >= count * 2)
            {
                if (!close)
                    return GL_FALSE;

                endIndex = 0;
            }

            memcpy(startVec, endVec, sizeof(GLfloat) * 2);
        }
    }

    return GL_FALSE;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

/*
#define BB_MAX_VECTOR_COLLISION_SIDES 16

GLboolean BBPinball_convexVectorCollision (BBPinball* pinball, const GLfloat* coords, GLsizei count,
                                            const GLfloat* matrix, const GLfloat* point, GLfloat* normal)
{
    GLint i = 0;
    GLfloat startVec[2];
    GLfloat endVec[2];
    GLint startIndex = i * 2;
    GLint endIndex = startIndex + 2;
    GLfloat closest[2];
    GLfloat normals[BB_MAX_VECTOR_COLLISION_SIDES][2];
    GLfloat dists[BB_MAX_VECTOR_COLLISION_SIDES];
    GLboolean discards[BB_MAX_VECTOR_COLLISION_SIDES];
    GLint closestIndex = -1;
    GLfloat closestDist = 100000.0f;

    BB_ASSERT(pinball && coords && matrix && point && normal);

    startVec[0] = coords[startIndex];
    startVec[1] = coords[startIndex + 1];

    if (matrix)
        bbVec2Transform(startVec, matrix, startVec, 0, 2);

    while (i < count)
    {
        GLfloat d[2];

        endVec[0] = coords[endIndex];
        endVec[1] = coords[endIndex + 1];

        if (matrix)
            bbVec2Transform(endVec, matrix, endVec, 0, 2);

        discards[i] = !bbVec2ClosestPoint(startVec, endVec, point, closest);

        bbVec2Sub(closest, point, d);

        bbVec2CrossProduct(endVec, startVec, normals[i]);

        if (bbVec2DotProduct(normals[i], d) < 0.0f)
            return GL_FALSE;

        dists[i] = bbVec2Length(d);

        i++;
        if (i < count)
        {
            startIndex += 2;
            endIndex += 2;
            if (endIndex >= count * 2)
                endIndex = 0;

            memcpy(startVec, endVec, sizeof(GLfloat) * 2);
        }
    }

    for (i = 0; i < count; i++)
        if (!discards[i] && dists[i] < closestDist)
        {
            closestIndex = i;
            closestDist = dists[i];
        }

    BB_ASSERT(closestIndex != -1);

    memcpy(normal, normals[closestIndex], sizeof(GLfloat) * 2);
    bbVec2Normalize(normal);

    return GL_TRUE;
}
*/

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

GLboolean BBPinball_lineCollision (BBPinball* pinball,
                                const GLfloat* coords, GLsizei count, const GLfloat* matrix,
                                const GLfloat* point1, const GLfloat* point2,
                                GLfloat* normal, GLfloat* intersection)
{
    GLint i = 0;
    GLfloat startVec[2];
    GLfloat endVec[2];
    GLint startIndex = i * 2;
    GLint endIndex = startIndex + 2;
    GLfloat closestDist = 0.0f;
    GLboolean collided = GL_FALSE;

    GLfloat a2;
    GLfloat b2;
    GLfloat c2;

    BB_ASSERT(pinball && coords && matrix && point1 && point2 && normal && intersection);

    startVec[0] = coords[startIndex];
    startVec[1] = coords[startIndex + 1];

    a2 = point2[1] - point1[1];
    b2 = point1[0] - point2[0];
    c2 = point2[0] * point1[1] - point1[0] * point2[1];

    if (matrix)
        bbVec2Transform(startVec, matrix, startVec, 0, 2);

    while (i < count)
    {
        endVec[0] = coords[endIndex];
        endVec[1] = coords[endIndex + 1];

        if (matrix)
            bbVec2Transform(endVec, matrix, endVec, 0, 2);

        {
            GLfloat inter[2];

            GLfloat a1 = endVec[1] - startVec[1];
            GLfloat b1 = startVec[0] - endVec[0];
            GLfloat c1 = endVec[0] * startVec[1] - startVec[0] * endVec[1];

            GLfloat denom = a1 * b2 - a2 * b1;
            if (bbAlmostZero(denom))
            {
                // Lines are paraller
                return GL_FALSE;
            }

            inter[0] = (b1 * c2 - b2 * c1) / denom;
            inter[1] = (a2 * c1 - a1 * c2) / denom;

            {
                GLfloat lineVec[2];
                GLfloat ballVec[2];
                GLfloat interToLineVec[2];
                GLfloat interToBallVec[2];
                GLfloat currDist;

                bbVec2Sub(endVec, startVec, lineVec);
                bbVec2Sub(point2, point1, ballVec);
                bbVec2Sub(inter, startVec, interToLineVec);
                bbVec2Sub(inter, point1, interToBallVec);

                currDist = bbVec2Length(interToBallVec);

                if (bbVec2DotProduct(lineVec, interToLineVec) > 0.0f &&
                    bbVec2DotProduct(ballVec, interToBallVec) > 0.0f &&
                    bbVec2Length(interToLineVec) < bbVec2Length(lineVec) &&
                    currDist < bbVec2Length(ballVec))
                {
                    if (!collided || currDist < closestDist)
                    {
                        GLfloat currNormal[2];
                        bbVec2CrossProduct(endVec, startVec, currNormal);

                        if (bbVec2DotProduct(currNormal, interToBallVec) < 0.0f)
                        {
                            closestDist = currDist;
                            collided = GL_TRUE;

                            intersection[0] = inter[0];
                            intersection[1] = inter[1];

                            normal[0] = currNormal[0];
                            normal[1] = currNormal[1];
                            bbVec2Normalize(normal);
                        }
                    }
                }
            }
        }

        i++;
        if (i < count)
        {
            startIndex += 2;
            endIndex += 2;
            if (endIndex >= count * 2)
                endIndex = 0;

            memcpy(startVec, endVec, sizeof(GLfloat) * 2);
        }
    }

    return collided;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

void BBPinball_ballPhysics (BBPinball* pinball, BBBall* ball)
{
    GLboolean collision = GL_FALSE;
//  BBBall* collidedBall;
    GLfloat res[2];
    GLfloat moveVec[2];
    GLint d = 0;
//    GLint ballCollisionDivider = 0;
    GLfloat racketColVec[2];
//  GLfloat racketIntersectionSum[2];
    GLint racketColCount = 0;
    GLfloat bumperVec[2];
    GLint bumperColCount = 0;
    GLfloat ballTotalVelocity = bbVec2Length(ball->vel);
    GLboolean middleBumper = GL_FALSE;

    const GLfloat BALL_MOVE_VELOCITY = 0.006f;

    const GLfloat RACKET_PUSH_VELOCITY = 0.0025f;

    res[0] = 0.0f;
    res[1] = 0.0f;

    memcpy(moveVec, ball->vel, sizeof(GLfloat) * 2);

    racketColVec[0] = 0.0f;
    racketColVec[1] = 0.0f;

    bumperVec[0] = 0.0f;
    bumperVec[1] = 0.0f;

//  racketIntersectionSum[0] = 0.0f;
//  racketIntersectionSum[1] = 0.0f;

//  pinball->ballCollisionCount = 0;

    while (d < 32)
    {
        GLboolean dirCollision = GL_FALSE;
        GLfloat vec[2];
        GLfloat pos[2];
        GLfloat a = BB_PI * 2.0f * d / 32.0f;
        GLfloat intersection[2];

        vec[0] = cosf(a);
        vec[1] = sinf(a);

        pos[0] = ball->pos[0] + vec[0] * BB_PINBALL_BALL_RADIUS;
        pos[1] = ball->pos[1] + vec[1] * BB_PINBALL_BALL_RADIUS;

/*      collidedBall = BBPinball_checkBallCollision(pinball, ball, pos);
        if (collidedBall)
        {
            GLint i;
            GLint lastIndex = pinball->ballCollisionCount;
            GLboolean add = GL_TRUE;
            BBBallCollision* bc;

            for (i = 0; i < lastIndex; i++)
            {
                if (pinball->ballCollisions[i].ball == collidedBall)
                {
                    bc = &pinball->ballCollisions[i];
                    add = GL_FALSE;
                    break;
                }
            }

            if (add)
            {
                bc = &pinball->ballCollisions[lastIndex];
                bc->ball = collidedBall;
                bc->vec[0] = 0.0f;
                bc->vec[1] = 0.0f;
                bc->vec[2] = 0.0f;
                pinball->ballCollisionCount++;
            }

            bbVec3Add(bc->vec, vec, bc->vec);
            ballCollisionDivider++;
            dirCollision = GL_TRUE;
        }
*/
        {
            GLint i;
            for (i = 0; i < BB_PINBALL_DROPTARGET_COUNT; i++)
            {
                if (BBPinball_isInsideAreas(pinball, ball->pos,
                    BB_PINBALL_DROPTARGET_GROUP_RECTS + (i / BB_PINBALL_DROPTARGET_GROUP_SIZE) * 4, 1))
                {
                    GLfloat normal[2];
                    if (!pinball->player->dropTargetDownList[i] &&
                        BBPinball_vectorCollision(pinball,
                        BB_PINBALL_DROPTARGET_COLLISION_POINTS + i * 8,
                        4, NULL, ball->pos, pos, normal, GL_FALSE))
                    {
                        if (ballTotalVelocity > BB_PINBALL_DROPTARGET_SENSITIVITY &&
                            bbVec2DotProduct(normal, ball->vel) < 0.0f)
                        {
                            pinball->player->dropTargetDownList[i] = GL_TRUE;
                            pinball->player->dropTargetGroupDown[i / BB_PINBALL_DROPTARGET_GROUP_SIZE]++;
                            pinball->player->score += BB_PINBALL_SCORE_DROPTARGET;
                            BBPinball_playSFX(pinball, BB_SFX_DROP1, 1.0f, GL_FALSE);
                        }
                        dirCollision = GL_TRUE;
                        break;
                    }
                }
            }
        }

        if (!dirCollision && BBCollisionMap_getPoint(pinball->collisionMap, pos) != 0)
        {
            if (BBPinball_isInsideAreas(pinball, ball->pos, BB_PINBALL_MIDDLEBUMPER_AREAS, BB_PINBALL_MIDDLEBUMPER_AREA_COUNT))
            {
                bbVec2Sub(bumperVec, vec, bumperVec);
                bumperColCount++;
                middleBumper = GL_TRUE;
            }
            else if (BBPinball_isInsideAreas(pinball, ball->pos, BB_PINBALL_SIDEBUMPER_AREAS, BB_PINBALL_SIDEBUMPER_AREA_COUNT))
            {
                bbVec2Sub(bumperVec, vec, bumperVec);
                bumperColCount++;
            }

            dirCollision = GL_TRUE;
        }

        if (!dirCollision)
        {
            GLfloat normal[2];

            if (BBPinball_isInsideAreas(pinball, ball->pos, BB_PINBALL_LOWERRIGHT_RACKET_AREA, 1) &&
                BBPinball_lineCollision(pinball,
                BB_PINBALL_LOWERRIGHT_RACKET_COLLISION_POINTS,
                BB_PINBALL_LOWERRIGHT_RACKET_COLLISION_POINT_COUNT,
                pinball->lowerRightRacketMatrix, ball->lastPos, pos, normal, intersection))
            {
                GLfloat distFromNutVec[2];
                GLfloat power;
                bbVec2Sub(BB_PINBALL_LOWERRIGHT_RACKET_POS, ball->pos, distFromNutVec);
                power = bbVec2Length(distFromNutVec);
                dirCollision = GL_TRUE;
                if (power > 0.0f)
                {
                    GLfloat powerVec[2];
                    powerVec[0] = cosf(BB_PINBALL_LOWER_RACKET_FACING_DIR - pinball->lowerRightRacketAngle);
                    powerVec[1] = sinf(BB_PINBALL_LOWER_RACKET_FACING_DIR - pinball->lowerRightRacketAngle);
                    power *= bbAbs(powerVec[0] * normal[0] + powerVec[1] * normal[1]) *
                        pinball->lowerRightRacketAngleVel * BB_PINBALL_RACKET_POWER_MULTIPLIER;
                    racketColVec[0] += normal[0] * (power + RACKET_PUSH_VELOCITY);
                    racketColVec[1] += normal[1] * (power + RACKET_PUSH_VELOCITY);
//                  racketIntersectionSum[0] += intersection[0] - vec[0];
//                  racketIntersectionSum[1] += intersection[1] - vec[1];
                    racketColCount++;
                }
            }
            else if (BBPinball_isInsideAreas(pinball, ball->pos, BB_PINBALL_LOWERLEFT_RACKET_AREA, 1) &&
                BBPinball_lineCollision(pinball,
                BB_PINBALL_LOWERLEFT_RACKET_COLLISION_POINTS,
                BB_PINBALL_LOWERLEFT_RACKET_COLLISION_POINT_COUNT,
                pinball->lowerLeftRacketMatrix, ball->lastPos, pos, normal, intersection))
            {
                GLfloat distFromNutVec[2];
                GLfloat power;
                bbVec2Sub(BB_PINBALL_LOWERLEFT_RACKET_POS, ball->pos, distFromNutVec);
                power = bbVec2Length(distFromNutVec);
                dirCollision = GL_TRUE;
                if (power > 0.0f)
                {
                    GLfloat powerVec[2];
                    powerVec[0] = cosf(BB_PI - BB_PINBALL_LOWER_RACKET_FACING_DIR - pinball->lowerLeftRacketAngle);
                    powerVec[1] = sinf(BB_PI - BB_PINBALL_LOWER_RACKET_FACING_DIR - pinball->lowerLeftRacketAngle);
                    power *= bbAbs(powerVec[0] * normal[0] + powerVec[1] * normal[1]) *
                            pinball->lowerLeftRacketAngleVel * BB_PINBALL_RACKET_POWER_MULTIPLIER;
                    racketColVec[0] -= normal[0] * (power - RACKET_PUSH_VELOCITY);
                    racketColVec[1] -= normal[1] * (power - RACKET_PUSH_VELOCITY);
//                  racketIntersectionSum[0] += intersection[0] - vec[0];
//                  racketIntersectionSum[1] += intersection[1] - vec[1];
                    racketColCount++;
                }
            }
            else if (BBPinball_isInsideAreas(pinball, ball->pos, BB_PINBALL_UPPERRIGHT_RACKET_AREA, 1) &&
                BBPinball_lineCollision(pinball,
                BB_PINBALL_UPPERRIGHT_RACKET_COLLISION_POINTS,
                BB_PINBALL_UPPERRIGHT_RACKET_COLLISION_POINT_COUNT,
                pinball->upperRightRacketMatrix, ball->lastPos, pos, normal, intersection))
            {
                GLfloat distFromNutVec[2];
                GLfloat power;
                bbVec2Sub(BB_PINBALL_UPPERRIGHT_RACKET_POS, ball->pos, distFromNutVec);
                power = bbVec2Length(distFromNutVec);
                dirCollision = GL_TRUE;
                if (power > 0.0f)
                {
                    GLfloat powerVec[2];
                    powerVec[0] = cosf(BB_PINBALL_UPPER_RACKET_FACING_DIR - pinball->upperRightRacketAngle);
                    powerVec[1] = sinf(BB_PINBALL_UPPER_RACKET_FACING_DIR - pinball->upperRightRacketAngle);
                    power *= bbAbs(powerVec[0] * normal[0] + powerVec[1] * normal[1]) *
                        pinball->upperRightRacketAngleVel * BB_PINBALL_RACKET_POWER_MULTIPLIER;
                    racketColVec[0] += normal[0] * (power + RACKET_PUSH_VELOCITY);
                    racketColVec[1] += normal[1] * (power + RACKET_PUSH_VELOCITY);
//                  racketIntersectionSum[0] += intersection[0] - vec[0];
//                  racketIntersectionSum[1] += intersection[1] - vec[1];
                    racketColCount++;
                }
            }
            else if (BBPinball_isInsideAreas(pinball, ball->pos, BB_PINBALL_UPPERLEFT_RACKET_AREA, 1) &&
                BBPinball_lineCollision(pinball,
                BB_PINBALL_UPPERLEFT_RACKET_COLLISION_POINTS,
                BB_PINBALL_UPPERLEFT_RACKET_COLLISION_POINT_COUNT,
                pinball->upperLeftRacketMatrix, ball->lastPos, pos, normal, intersection))
            {
                GLfloat distFromNutVec[2];
                GLfloat power;
                bbVec2Sub(BB_PINBALL_UPPERLEFT_RACKET_POS, ball->pos, distFromNutVec);
                power = bbVec2Length(distFromNutVec);
                dirCollision = GL_TRUE;
                if (power > 0.0f)
                {
                    GLfloat powerVec[2];
                    powerVec[0] = cosf(BB_PI - BB_PINBALL_UPPER_RACKET_FACING_DIR - pinball->upperLeftRacketAngle);
                    powerVec[1] = sinf(BB_PI - BB_PINBALL_UPPER_RACKET_FACING_DIR - pinball->upperLeftRacketAngle);
                    power *= bbAbs(powerVec[0] * normal[0] + powerVec[1] * normal[1]) *
                        pinball->upperLeftRacketAngleVel * BB_PINBALL_RACKET_POWER_MULTIPLIER;
                    racketColVec[0] -= normal[0] * (power - RACKET_PUSH_VELOCITY);
                    racketColVec[1] -= normal[1] * (power - RACKET_PUSH_VELOCITY);
//                  racketIntersectionSum[0] += intersection[0] - vec[0];
//                  racketIntersectionSum[1] += intersection[1] - vec[1];
                    racketColCount++;
                }
            }
            else if (pinball->ballDoorClosed && BBPinball_vectorCollision(pinball, BB_PINBALL_BALL_DOOR_POINTS,
                BB_PINBALL_BALL_DOOR_POINT_COUNT, NULL, ball->pos, pos, normal, GL_FALSE))
            {
                dirCollision = GL_TRUE;
            }
        }

        if (dirCollision)
        {
            bbVec2Sub(res, vec, res);
            collision = GL_TRUE;
        }

        d++;
    }

    if (collision && bbVec2DotProduct(moveVec, res) < 0.0f)
    {
        GLfloat vel = bbVec2Length(moveVec);
        GLfloat outDir[2];
//      GLfloat hitAngleCoeff;
//      GLint i;

        const GLfloat OUT_VELOCITY_MULTIPLIER = 0.7f;
        const GLfloat OUT_VELOCITY_MULTIPLIER_INV = 0.275f;

        moveVec[0] /= vel;
        moveVec[1] /= vel;

        bbVec2Normalize(res);

//      hitAngleCoeff = (1.0f + bbVec2DotProduct(moveVec, res)) * 0.1f + 0.9f;

        vel *= OUT_VELOCITY_MULTIPLIER;

        bbVec2Reflect(moveVec, res, outDir);

        ball->vel[0] = ball->vel[0] * OUT_VELOCITY_MULTIPLIER_INV - outDir[0] * vel;
        ball->vel[1] = ball->vel[1] * OUT_VELOCITY_MULTIPLIER_INV - outDir[1] * vel;

/*      for (i = 0; i < pinball->ballCollisionCount; i++)
        {
            BBBallCollision* bc = &pinball->ballCollisions[i];
            bc->ball->vel[0] += (bc->vec[0] * vel) / ballCollisionDivider;
            bc->ball->vel[1] += (bc->vec[1] * vel) / ballCollisionDivider;
        }*/
        if (racketColCount)
        {
//          ball->pos[0] = racketIntersectionSum[0] / racketColCount;
//          ball->pos[1] = racketIntersectionSum[1] / racketColCount;
            ball->vel[0] += racketColVec[0] / racketColCount;
            ball->vel[1] += racketColVec[1] / racketColCount;
        }
        if (bumperColCount)
        {
            int r;

            ball->vel[0] += bumperVec[0] * BB_PINBALL_BUMPER_POWER / bumperColCount;
            ball->vel[1] += bumperVec[1] * BB_PINBALL_BUMPER_POWER / bumperColCount;

            if (middleBumper)
                pinball->player->score += BB_PINBALL_SCORE_BUMPER_MIDDLE;
            else
                pinball->player->score += BB_PINBALL_SCORE_BUMPER_SIDES;

            r = (rand() >> 6) & 3;

            switch (r)
            {
            case 0:
                BBPinball_playSFX(pinball, BB_SFX_BUMPER1, 1.0f, GL_FALSE);
                break;
            case 1:
                BBPinball_playSFX(pinball, BB_SFX_BUMPER2, 1.0f, GL_FALSE);
                break;
            case 2:
                BBPinball_playSFX(pinball, BB_SFX_BUMPER3, 1.0f, GL_FALSE);
                break;
            case 3:
                BBPinball_playSFX(pinball, BB_SFX_BUMPER4, 1.0f, GL_FALSE);
                break;
            }
        }

/*      for (i = 0; i < BB_PINBALL_BUMPER_LIGHT_COUNT; i++)
            if (BBPinball_isInsideAreas(pinball, ball->pos, BB_PINBALL_BUMPER_LIGHT_AREAS + i * 4, 1))
                pinball->bumperLightValues[i] = 1.0f;*/
    }

    ball->lastPos[0] = ball->pos[0];
    ball->lastPos[1] = ball->pos[1];
    ball->pos[0] += ball->vel[0];
    ball->pos[1] += ball->vel[1];

    ball->vel[1] += BB_PINBALL_GRAVITY_ACC;

//  if (ball->pos[1] < BB_PINBALL_BALL_SLOT_Y)
//      ball->pos[2] = BB_PINBALL_BALL_RADIUS + (ball->pos[1] - BB_PINBALL_BALL_SLOT_Y) / 2.0f; /* 22,5 degree slope */
//  else
//      ball->pos[2] = BB_PINBALL_BALL_RADIUS;

    if (pinball->moveBallUp)
        ball->vel[1] -= BALL_MOVE_VELOCITY;
    if (pinball->moveBallDown)
        ball->vel[1] += BALL_MOVE_VELOCITY;
    if (pinball->moveBallLeft)
        ball->vel[0] += BALL_MOVE_VELOCITY;
    if (pinball->moveBallRight)
        ball->vel[0] -= BALL_MOVE_VELOCITY;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

GLboolean BBPinball_isInsideAreas (BBPinball* pinball, const GLfloat* pos,
                                 const GLfloat* areas, GLsizei count)
{
    GLint i;
    BB_ASSERT(pinball && pos && areas && count);

    for (i = 0; i < count; i++)
    {
        GLfloat left = areas[i * 4];
        GLfloat top = areas[i * 4 + 1];
        GLfloat right = areas[i * 4 + 2];
        GLfloat bottom = areas[i * 4 + 3];
        if (pos[0] >= left && pos[0] <= right && pos[1] <= top && pos[1] >= bottom)
            return GL_TRUE;
    }

    return GL_FALSE;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

void BBPinball_scrollLights (BBPinball* pinball, GLboolean right)
{
    GLint i;
    GLint start;
    GLint stop;
    GLint d;
    BB_ASSERT(pinball);

    if (right)
    {
        start = 3;
        stop = 0;
        d = -1;
    }
    else
    {
        start = 0;
        stop = 3;
        d = 1;
    }

    for (i = start; i != stop; i += d)
    {
        GLint ci = 17 + i;
        GLint li = ci - 1;

        if (li < 17)
            li += 4;

        bbSwapInt(pinball->player->lightOnList[ci], pinball->player->lightOnList[li])
    }
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

GLboolean BBPinball_changePlayer (BBPinball* pinball, GLuint playerIndex)
{
    GLint i;
    GLuint rep = 0;
    BB_ASSERT(pinball);

    for (i = 0; i < BB_PINBALL_DROPTARGET_GROUP_COUNT; i++)
    {
        pinball->dropTargetGroupRaiseTime[i] = 0;
    }

    pinball->turnUpperLightsOffTime = 0;

    do
    {
        pinball->currentPlayerIndex = playerIndex;
        if (pinball->currentPlayerIndex >= pinball->numPlayers)
            pinball->currentPlayerIndex = 0;

        pinball->player = &pinball->players[pinball->currentPlayerIndex];

        rep++;
        if (rep > pinball->numPlayers)
            return GL_FALSE;
    }
    while (pinball->player->ballsLeft == 0);

    pinball->player->ballsLeftLastChange = pinball->player->ballsLeft;

#ifdef BB_DEVEL
    printf("Game: Current player changed to %d\n", pinball->currentPlayerIndex);
#endif

    pinball->showPlayerTime = pinball->currentTime;

    return GL_TRUE;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

void BBPinball_newGame (BBPinball* pinball/*, GLsizei playerCount*/)
{
    GLint i;
    BB_ASSERT(pinball);
//  BB_ASSERT(playerCount >= 1 && playerCount <= 4);

//  pinball->numPlayers = playerCount;

    for (i = 0; i < BB_PINBALL_MAX_PLAYERS; i++)
    {
        BBPlayer_defaultValues(&pinball->players[i]);
    }

    BBPinball_changePlayer(pinball, 0);

    pinball->ballInPlay             = GL_FALSE;
    pinball->ballLaunchTime         = 0;

    pinball->ball.pos[0]            = BB_PINBALL_BALL_START_POS[0];
    pinball->ball.pos[1]            = BB_PINBALL_BALL_START_POS[1];
    pinball->ball.pos[2]            = 0.0f;
    pinball->ball.vel[0]            = 0.0f;
    pinball->ball.vel[1]            = 0.0f;
    pinball->ball.vel[2]            = 0.0f;

    pinball->ballDoorClosed         = GL_FALSE;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

void BBPinball_endGame (BBPinball* pinball, GLboolean doHighscoreCheck)
{
#ifdef BB_DEVEL
    printf("Game: Game ended.\n");
#endif
    pinball->gameRunning = GL_FALSE;

    pinball->moveBallUp = GL_FALSE;
    pinball->moveBallDown = GL_FALSE;
    pinball->moveBallLeft = GL_FALSE;
    pinball->moveBallRight = GL_FALSE;
    pinball->leftRacketKeyDown = GL_FALSE;
    pinball->rightRacketKeyDown = GL_FALSE;
    pinball->nudge = GL_FALSE;
    pinball->scorePeek = GL_FALSE;

    pinball->menuFadeTime = -pinball->currentTime;

    pinball->menuInactive = GL_FALSE;
    pinball->menuInactivityTimer = pinball->currentTime;

    pinball->highscoreCheckRunning = doHighscoreCheck;
    pinball->highscoreCheckNext = doHighscoreCheck;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

GLboolean BBPinball_storeFrame (BBPinball* pinball)
{
    BBRecordFrame* frame;
    GLint i;
    GLint c = 0;
    BB_ASSERT(pinball);

    if (!pinball->firstRecordChunk)
    {
        pinball->firstRecordChunk = BBRecordChunk_create();
        if (!pinball->firstRecordChunk)
            return GL_FALSE;

        pinball->currentRecordChunk = pinball->firstRecordChunk;
    }

    if (pinball->currentRecordPos >= BB_PINBALL_RECORD_CHUNK_FRAME_COUNT)
    {
        pinball->currentRecordChunk->next = BBRecordChunk_create();
        if (!pinball->currentRecordChunk->next)
            return GL_FALSE;

        pinball->currentRecordChunk = pinball->currentRecordChunk->next;
        pinball->currentRecordPos = 0;
    }

    frame = &pinball->currentRecordChunk->frames[pinball->currentRecordPos];
    pinball->currentRecordPos++;
    pinball->currentRecordChunk->count++;

    frame->values[c++] = pinball->ballDoorAngle;

    for (i = 0; i < BB_PINBALL_DROPTARGET_COUNT; i++)
        frame->values[c++] = pinball->dropTargetPosList[i];

    frame->values[c++] = pinball->touchieMove;

    for (i = 0; i < BB_PINBALL_LIGHT_COUNT; i++)
        frame->values[c++] = pinball->lightAlphaList[i];

/*  for (i = 0; i < BB_PINBALL_BUMPER_LIGHT_COUNT; i++)
        frame->values[c++] = pinball->bumperLightValues[i];*/

    frame->values[c++] = pinball->lowerLeftRacketAngle;
    frame->values[c++] = pinball->lowerRightRacketAngle;
    frame->values[c++] = pinball->upperLeftRacketAngle;
    frame->values[c++] = pinball->upperRightRacketAngle;

    frame->values[c++] = pinball->ball.pos[0];
    frame->values[c++] = pinball->ball.pos[1];
    frame->values[c++] = pinball->ball.pos[2];
    frame->values[c++] = pinball->ball.vel[0];
    frame->values[c++] = pinball->ball.vel[1];
    frame->values[c++] = pinball->ball.vel[2];

    frame->values[c++] = *((float*)((int*)(&pinball->players[0].score)));
    frame->values[c++] = *((float*)((int*)(&pinball->players[1].score)));
    frame->values[c++] = *((float*)((int*)(&pinball->players[2].score)));
    frame->values[c++] = *((float*)((int*)(&pinball->players[3].score)));

#ifndef BB_DISABLE_SOUNDS
    frame->values[c++] = *((float*)((int*)(&pinball->frameSFX)));
    pinball->frameSFX = -1;
#endif // !BB_DISABLE_SOUNDS

    return GL_TRUE;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

GLboolean BBPinball_playbackFrame (BBPinball* pinball, GLboolean reset)
{
    BBRecordFrame* frame;
    GLint i;
    GLint c = 0;
    BB_ASSERT(pinball);
    BB_ASSERT(!pinball->gameRunning);
    BB_ASSERT(!pinball->recordOn);

    if (!pinball->firstRecordChunk)
        return GL_FALSE;

    if (!pinball->currentRecordChunk || reset)
    {
        pinball->currentRecordChunk = pinball->firstRecordChunk;
        pinball->currentRecordPos = 0;
        if (reset)
            return GL_TRUE;
    }
    else if (pinball->currentRecordPos >= pinball->currentRecordChunk->count)
    {
        pinball->currentRecordChunk = pinball->currentRecordChunk->next;
        pinball->currentRecordPos = 0;
    }

    if (!pinball->currentRecordChunk)
        return GL_FALSE;

    frame = &pinball->currentRecordChunk->frames[pinball->currentRecordPos];
    pinball->currentRecordPos++;

    pinball->ballDoorAngle = frame->values[c++];

    for (i = 0; i < BB_PINBALL_DROPTARGET_COUNT; i++)
        pinball->dropTargetPosList[i] = frame->values[c++];

    pinball->touchieMove = frame->values[c++];

    for (i = 0; i < BB_PINBALL_LIGHT_COUNT; i++)
        pinball->lightAlphaList[i] = frame->values[c++];

//  for (i = 0; i < BB_PINBALL_BUMPER_LIGHT_COUNT; i++)
//      pinball->bumperLightValues[i] = frame->values[c++];

    pinball->lowerLeftRacketAngle = frame->values[c++];
    pinball->lowerRightRacketAngle = frame->values[c++];
    pinball->upperLeftRacketAngle = frame->values[c++];
    pinball->upperRightRacketAngle = frame->values[c++];

    pinball->ball.pos[0] = frame->values[c++];
    pinball->ball.pos[1] = frame->values[c++];
    pinball->ball.pos[2] = frame->values[c++];
    pinball->ball.vel[0] = frame->values[c++];
    pinball->ball.vel[1] = frame->values[c++];
    pinball->ball.vel[2] = frame->values[c++];

    pinball->players[0].score = *((int*)((float*)(&frame->values[c++])));
    pinball->players[1].score = *((int*)((float*)(&frame->values[c++])));
    pinball->players[2].score = *((int*)((float*)(&frame->values[c++])));
#ifndef ENABLE_FPS_COUNTER
    pinball->players[3].score = *((int*)((float*)(&frame->values[c++])));
#endif

    {
        int sfx = *((int*)((float*)(&frame->values[c++])));
        if (sfx >= 0)
            BBPinball_playSFX(pinball, sfx, 1.0f, GL_FALSE);
    }

    return GL_TRUE;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

GLboolean BBPinball_saveRecording (BBPinball* pinball)
{
    /*FILE* f;
    GLubyte a = 1;
    GLsizei i;
    BBRecordChunk* current;

    BB_ASSERT(pinball);

    if (!pinball->firstRecordChunk)
        return GL_FALSE;

    f = BB_FOPEN(BB_PINBALL_RECORD_FILENAME, "wb");
    if (!f)
        return GL_FALSE;

    current = pinball->firstRecordChunk;
    while (current)
    {
        fwrite(&current->count, sizeof(GLint), 1, f);
        for (i = 0; i < current->count; i++)
        {
            fwrite(current->frames[i].values, sizeof(GLfloat), BB_PINBALL_RECORD_VALUE_COUNT, f);
        }
        current = current->next;
        if (current)
            fwrite(&a, 1, 1, f);
        else
        {
            a = 0;
            fwrite(&a, 1, 1, f);
        }
    }

    fclose(f);*/

    return GL_TRUE;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

GLboolean BBPinball_loadRecording (BBPinball* pinball)
{
    FILE* f;
    GLubyte end = 0;
    BB_ASSERT(pinball);

    BBPinball_dumpRecording(pinball);

    f = BB_FOPEN(BB_PINBALL_RECORD_FILENAME, "rb");
    if (!f)
        return GL_FALSE;

    while (!end)
    {
        GLint count = 0;
        BBRecordChunk* chunk = BBRecordChunk_create();
        if (!chunk)
        {
            fclose(f);
            return GL_FALSE;
        }

        fread(&count, sizeof(GLint), 1, f);

        fread(&chunk->frames, sizeof(GLfloat), count * BB_PINBALL_RECORD_VALUE_COUNT, f);
        chunk->count = count;

        if (pinball->currentRecordChunk)
        {
            pinball->currentRecordChunk->next = chunk;
            pinball->currentRecordChunk = chunk;
        }
        else
        {
            pinball->firstRecordChunk = chunk;
            pinball->currentRecordChunk = chunk;
        }

        fread(&end, 1, 1, f);
        end = !end;
    }

    fclose(f);

    return GL_TRUE;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

void BBPinball_dumpRecording (BBPinball* pinball)
{
    BBRecordChunk* current;
    BB_ASSERT(pinball);

    current = pinball->firstRecordChunk;
    while (current)
    {
        BBRecordChunk* next = current->next;
        BBRecordChunk_destroy(current);
        current = next;
    }

    pinball->firstRecordChunk = NULL;
    pinball->currentRecordChunk = NULL;
    pinball->currentRecordPos = 0;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

BBRecordChunk* BBRecordChunk_create ()
{
    BBRecordChunk* chunk = malloc(sizeof(BBRecordChunk));
    if (!chunk)
        return NULL;

    chunk->next = NULL;
    chunk->count = 0;

    return chunk;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

void BBRecordChunk_destroy (BBRecordChunk* chunk)
{
    BB_ASSERT(chunk);
    free(chunk);
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

void BBPinball_playSFX (BBPinball* pinball, GLuint index, GLfloat volume, GLboolean repeat)
{
    if (index == 0 || index >= BB_SFX_MAX)
        return;

#ifndef BB_DISABLE_SOUNDS
    if (pinball->recordOn && pinball->frameSFX == -1)
        pinball->frameSFX = index;

    pinball->sound.play(pinball->sfxList[index - 1], volume, repeat);
#endif // !BB_DISABLE_SOUNDS
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/
