/*  @file openicc_config_internal.h
 *
 *  libOpenICC - OpenICC Colour Management Configuration
 *
 *  @par Copyright:
 *            2011 (C) Kai-Uwe Behrmann
 *
 *  @brief    OpenICC Colour Management configuration helpers
 *  @internal
 *  @author   Kai-Uwe Behrmann <ku.b@gmx.de>
 *  @par License:
 *            MIT <http://www.opensource.org/licenses/mit-license.php>
 *  @since    2011/10/21
 */

#ifndef __OPENICC_CONFIG_INTERNAL_H__
#define __OPENICC_CONFIG_INTERNAL_H__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>           /* va_list */
#include <time.h>

#include <yajl/yajl_tree.h>

#include "openicc_config.h"
#include "openicc_version.h"

#ifdef USE_GETTEXT
# include <libintl.h>
# include <locale.h>
# define _(text) dgettext( oy_domain, text )
#else
# define _(text) text
#endif

#if (defined(__APPLE__) && defined(__MACH__)) || defined(__unix__) || (!defined(_WIN32) && !defined(_MSC_VER)) || (defined(_WIN32) && defined(__CYGWIN__)) || defined(__MINGW32__) || defined(__MINGW32)
# include <unistd.h>
# if defined(_POSIX_VERSION)
#  define HAVE_POSIX 1
# endif
#endif

#define DBG_UHR_ (double)clock()/(double)CLOCKS_PER_SEC
#define WARNc1_S( format_,...) openiccMessage_p( openiccMSG_WARN, NULL, \
                                                format_, __VA_ARGS__)
#define WARNc2_S WARNc1_S


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct OpeniccConfigs_s {
  char     * json_text;
  yajl_val   yajl;
  char     * dbg_text;
};

int          openiccStringAddPrintf_ ( char             ** string,
                                       const char        * format,
                                                           ... );
void         openiccStringAdd_       ( char             ** text,
                                       const char        * append );
#define STRING_ADD( t, append ) openiccStringAdd_( &t, append )

extern openiccMessage_f     openiccMessage_p;


char * openiccOpenFile( const char * file_name,
                        size_t     * size_ptr );
char * openiccReadFileSToMem(
                        FILE       * fp,
                        size_t     * size);
size_t openiccWriteFile(const char * file_name,
                        void       * ptr,
                        size_t       size );
char * openiccExtractPathFromFileName_(const char        * file_name );
int    openiccIsDirFull_             ( const char        * name );
char * oyPathGetParent_              ( const char        * name );

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */


#endif /* __OPENICC_CONFIG_H__ */
