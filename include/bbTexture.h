/*--------------------------------------------------------------------------*/
/*	bbTexture.h                                                             */
/*	Author:			Aarni Gratseff / Rocket                                 */
/*	Created:		24/01/2006                                              */
/*	Description:	Texture classes header                                  */
/*--------------------------------------------------------------------------*/

#ifndef BBTEXTURE_H
#define BBTEXTURE_H

#include "bbPinballDefines.h"

/*--------------------------------------------------------------------------*/
/* Forward declarations                                                     */
/*--------------------------------------------------------------------------*/

typedef struct BBTexture_s BBTexture;
typedef struct BBTextureStore_s BBTextureStore;

/*--------------------------------------------------------------------------*/
/* Texture public methods                                                   */
/*--------------------------------------------------------------------------*/

BBTexture*			BBTexture_create			(const char* name,
												 GLsizei width, GLsizei height,
												 GLenum type, GLenum channels,
												 const void** data);

void				BBTexture_destroy			(BBTexture* texture);

const char*			BBTexture_getName			(BBTexture* texture);
GLenum				BBTexture_getType			(BBTexture* texture);

void				BBTexture_activate			(BBTexture* texture, GLuint channel);
void				BBTexture_deactivate		(BBTexture* texture, GLuint channel);

/*--------------------------------------------------------------------------*/
/* Texture store public methods                                             */
/*--------------------------------------------------------------------------*/

BBTextureStore*		BBTextureStore_create		(void);
void				BBTextureStore_destroy		(BBTextureStore* texStore);

BBTexture*			BBTextureStore_load			(BBTextureStore* texStore, const char* name, int startLodLevel);
BBTexture*			BBTextureStore_loadCube		(BBTextureStore* texStore, const char* name, int startLodLevel);

#endif /* BBTEXTURE_H */
