/*  @file openicc_db.c
 *
 *  libOpenICC - OpenICC Colour Management Configuration
 *
 *  @par Copyright:
 *            2015 (C) Kai-Uwe Behrmann
 *
 *  @brief    OpenICC Colour Management configuration helpers
 *  @internal
 *  @author   Kai-Uwe Behrmann <ku.b@gmx.de>
 *  @par License:
 *            MIT <http://www.opensource.org/licenses/mit-license.php>
 *  @since    2015/08/29
 */

#include "openicc_config_internal.h"
#include "openicc_db.h"

#if HAVE_POSIX
#include <unistd.h>  /* getpid() */
#endif
#include <string.h>  /* strdup() */
#include <stdarg.h>  /* vsnprintf() */
#include <stdio.h>   /* vsnprintf() */

typedef struct openiccDB_s {
  openiccSCOPE_e   scope;
  const char     * top_key_name;
  yajl_val      ** ks;
} openiccDB_s;

openiccDB_s * openiccDB_newFrom      ( const char        * top_key_name,
                                       openiccSCOPE_e      scope )
{
  openiccDB_s * db = calloc( sizeof(openiccDB_s), 1 );

  if(db)
  {
    db->top_key_name = openiccStringCopy( top_key_name, malloc );
    db->scope = scope;
  }

  return db;

}

