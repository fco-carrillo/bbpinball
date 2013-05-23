/*--------------------------------------------------------------------------*/
/*	bbMaterial.h                                                            */
/*	Author:			Aarni Gratseff / Rocket                                 */
/*	Created:		23/01/2006                                              */
/*	Description:	Material classes header                                 */
/*--------------------------------------------------------------------------*/

#ifndef BBMATERIAL_H
#define BBMATERIAL_H

#include "bbPinballDefines.h"

#include "bbShaderStack.h"
#include "bbTexture.h"

#define BB_MATERIAL_MAX_TEXTURES 4

/*--------------------------------------------------------------------------*/
/* Pinball uniform setup cache struct                                       */
/*--------------------------------------------------------------------------*/

typedef struct
{
	GLint valid;
    int currentTime;
    int ledNumber;

	GLint projMatLoc;

	GLint worldMatLoc;
	GLint worldNormalMatLoc;
	GLint viewMatLoc;
	GLint viewNormalMatLoc;

	GLint lightIntensityLoc;
	GLint lightPosLoc;
	GLint eyePosLoc;
	GLint eyePosOSLoc;

	GLint texLoc;
	GLint tex2Loc;
	GLint tex3Loc;
	GLint tex4Loc;
	GLint texBlendLoc;

	GLint lightAlphaLoc;
	GLint ledNumberLoc;
	GLint outputMultiplierLoc;
	GLint outputMultiplier2Loc;
	GLint outputMultiplier3Loc;

	GLint hudPosLoc;
	GLint hudScaleLoc;
	GLint hudTexPosLoc;
	GLint hudTexScaleLoc;
	GLint hudColorLoc;
} BBPinballUniformSetupCache;


/*--------------------------------------------------------------------------*/
/* Material struct                                                          */
/*--------------------------------------------------------------------------*/

typedef struct BBMaterial_s
{
	char*			name;
	GLuint			program;
	BBPinballUniformSetupCache uniformSetupCache;
	BBTexture*		textures[BB_MATERIAL_MAX_TEXTURES];

	GLboolean		blend;
	GLboolean		additive;
	GLboolean		tone;

	struct BBMaterial_s*		next;
} BBMaterial;

/*--------------------------------------------------------------------------*/
/* Forward declarations                                                     */
/*--------------------------------------------------------------------------*/

typedef struct BBMaterialStore_s BBMaterialStore;

/*--------------------------------------------------------------------------*/
/* Material public methods                                                  */
/*--------------------------------------------------------------------------*/

GLuint					BBMaterial_activate					(BBMaterial* material, GLboolean forceAdditive);
void					BBMaterial_deactivate				(BBMaterial* material, GLboolean forceAdditive);
GLboolean				BBMaterial_isTransparent			(BBMaterial* material);

/*--------------------------------------------------------------------------*/
/* Material store public methods                                            */
/*--------------------------------------------------------------------------*/

BBMaterialStore*		BBMaterialStore_create				(void);
void					BBMaterialStore_destroy				(BBMaterialStore* matStore);

GLboolean				BBMaterialStore_load				(BBMaterialStore* matStore,
															 BBShaderStack* shaderStack,
															 BBTextureStore* texStore,
															 const char* filename,
															 int startLodLevel);

BBMaterial*				BBMaterialStore_getMaterial			(BBMaterialStore* matStore, const char* name);

#endif /* BBMATERIAL_H */
