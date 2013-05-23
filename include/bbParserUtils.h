/*--------------------------------------------------------------------------*/
/*	bbParserUtils.h                                                         */
/*	Author:			Aarni Gratseff / Rocket                                 */
/*	Created:		23/01/2006                                              */
/*	Description:	Parsering utilities header                              */
/*--------------------------------------------------------------------------*/

#ifndef BBPARSERUTILS_H
#define BBPARSERUTILS_H

#include "bbPinball.h"

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

void        bbParseParameters   (BBPinballCreateParams* params, int argc, char* argv[]);
char*		bbReadFileToBuffer  (const char* filename);

void		bbSkipWhite			(char** stream);
char*		bbParseString		(char** stream, const char* stopChars);

#endif /* BBPARSERUTILS_H */
