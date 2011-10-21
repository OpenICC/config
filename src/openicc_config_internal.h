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

#define DBG_UHR_ (double)clock()/(double)CLOCKS_PER_SEC

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct OpeniccConfigs_s {
  char     * json_text;
  yajl_val   yajl;
  char     * dbg_text;
};

void               StringAdd_        ( char             ** text,
                                       const char        * append );
#define STRING_ADD( t, append ) StringAdd_( &t, append )

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */


#endif /* __OPENICC_CONFIG_H__ */
