/*--------------------------------------------------------------------------*/
/*	bbMeshStore.h                                                           */
/*	Author:			Aarni Gratseff / Rocket                                 */
/*	Created:		19/01/2006                                              */
/*	Description:	Mesh store class header                                 */
/*--------------------------------------------------------------------------*/

#ifndef BBMESHSTORE_H
#define BBMESHSTORE_H

#include "bbPinballDefines.h"
#include "bbMesh.h"
#include "bbMaterial.h"

/*--------------------------------------------------------------------------*/
/* Forward declarations                                                     */
/*--------------------------------------------------------------------------*/

typedef struct BBMeshStore_s BBMeshStore;

/*--------------------------------------------------------------------------*/
/* Mesh store public methods                                                */
/*--------------------------------------------------------------------------*/

BBMeshStore*	BBMeshStore_create			(BBMaterialStore* matStore);
void			BBMeshStore_destroy			(BBMeshStore* store);

GLboolean		BBMeshStore_load			(BBMeshStore* store, const char* filename, int useVBO, int useIBO);

GLboolean		BBMeshStore_exchange		(BBMeshStore* store, BBMeshStore* target, BBMesh* mesh);
GLboolean		BBMeshStore_exchangeByName	(BBMeshStore* store, BBMeshStore* target, const char* name);
GLboolean		BBMeshStore_removeFromList	(BBMeshStore* store, BBMesh* mesh);

BBMesh*			BBMeshStore_getFirstMesh	(BBMeshStore* store);
BBMesh*			BBMeshStore_getNextMesh		(BBMeshStore* store, BBMesh* current);
BBMesh*			BBMeshStore_getMeshByName	(BBMeshStore* store, const char* name);

#endif /* BBMESHSTORE_H */
