#ifndef AGLOB_H

/* Provide a system independent glob type function */

/*************************************************************************
 Copyright 2011 Graeme W. Gill

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.

 *************************************************************************/
#if !defined(WIN32) && !defined(WIN64)
#include <glob.h>
#endif

#ifdef __cplusplus
	extern "C" {
#endif

typedef struct {
#if defined(WIN32) || defined(WIN64)
	char *base;				/* Base path */
    struct _finddata_t ffs;
	long ff;
	int first;
#else	/* UNIX */
	glob_t g;
	int rv;			/* glob return value */
	size_t ix;
#endif
	int merr;		/* NZ on malloc error */
} aglob;


/* Create the aglob for files matching the given path and pattern. */
/* Return nz on malloc error */
int aglob_create(aglob *g, char *spath);

/* Return an allocated string of the next match. */
/* Return NULL if no more matches */
char *aglob_next(aglob *g);

/* Free the aglob once we're done with it */
void aglob_cleanup(aglob *g);

#ifdef __cplusplus
	}
#endif

#define AGLOB_H
#endif /* AGLOB_H */
