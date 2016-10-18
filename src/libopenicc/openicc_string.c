/*  @file openicc_string.c
 *
 *  libOpenICC - OpenICC Colour Management Configuration
 *
 *  @par Copyright:
 *            2011-2016 (C) Kai-Uwe Behrmann
 *
 *  @brief    OpenICC Colour Management configuration helpers
 *  @internal
 *  @author   Kai-Uwe Behrmann <ku.b@gmx.de>
 *  @par License:
 *            MIT <http://www.opensource.org/licenses/mit-license.php>
 *  @since    2011/06/27
 */

#include "openicc_config_internal.h"

#if HAVE_POSIX
#include <unistd.h>  /* getpid() */
#endif
#include <string.h>  /* strdup() */
#include <stdarg.h>  /* vsnprintf() */
#include <stdio.h>   /* vsnprintf() */


static char *  openiccStringAppend_  ( const char        * text,
                                       const char        * append )
{
  char * text_copy = NULL;
  int text_len = 0, append_len = 0;

  if(text)
    text_len = strlen(text);

  if(append)
    append_len = strlen(append);

  if(text_len || append_len)
  {
    text_copy = calloc(sizeof(char), text_len + append_len + 1);

    if(text_len)
      memcpy( text_copy, text, text_len );

    if(append_len)
      memcpy( &text_copy[text_len], append, append_len );
    text_copy[text_len+append_len] = '\000';
  }

  return text_copy;
}


/** @internal 
 *  @brief   printf style string add
 *
 *  @version OpenICC: 0.1.0
 *  @date    2009/02/07
 *  @since   2009/02/07 (OpenICC: 0.1.0)
 */
int          openiccStringAddPrintf  ( char             ** string,
                                       const char        * format,
                                                           ... )
{
  char * text_copy = NULL;
  char * text = 0;
  va_list list;
  int len;
  size_t sz = strlen(format) * 2;

  text = malloc( sz );
  if(!text)
  {
    WARNc_S(
     "Could not allocate 256 byte of memory.",0);
    return 1;
  }

  text[0] = 0;

  va_start( list, format);
  len = vsnprintf( text, sz, format, list );
  va_end  ( list );

  if (len >= sz)
  {
    text = realloc( text, (len+1)*sizeof(char) );
    va_start( list, format);
    len = vsnprintf( text, len+1, format, list );
    va_end  ( list );
  }


  text_copy = openiccStringAppend_(*string, text);

  if(string && *string)
    free(*string);

  *string = text_copy;

  free(text);

  return 0;
}


void               openiccStringAdd_ ( char             ** text,
                                       const char        * append )
{
  char * text_copy = NULL;

  text_copy = openiccStringAppend_(*text, append);

  if(text && *text)
    free(*text);

  *text = text_copy;

  return;
}

char *       openiccStringCopy       ( const char        * text,
                                       openiccAlloc_f      alloc )
{
  char * text_copy = NULL;
    
  if(text)
  {
    text_copy = alloc( strlen(text) + 1 );
    if(text_copy == NULL)
    {
      WARNc_S("could not allocate enough memory: %d", strlen(text) + 1 );
      return NULL;
    }

    strcpy( text_copy, text );
  }
  return text_copy;
}

