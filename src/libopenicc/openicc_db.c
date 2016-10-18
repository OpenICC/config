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
#include "xdg_bds.h"

#if HAVE_POSIX
#include <unistd.h>  /* getpid() */
#endif
#include <string.h>  /* strdup() */
#include <stdarg.h>  /* vsnprintf() */
#include <stdio.h>   /* vsnprintf() */

typedef struct openiccDB_s {
  openiccSCOPE_e   scope;
  char * top_key_name;
  openiccConfig_s ** ks;
  int ks_array_reserved_n;
} openiccDB_s;

typedef struct {
  int dummy;
} openiccDummy_s;

typedef struct {
  openiccDummy_s ** array;
  int reserved_n;
} openiccArray_s;

int      openiccArray_Count          ( openiccArray_s    * array )
{
  int count = 0;
  while(array->array[count]) ++count;
  return count;
}

int      openiccArray_Add            ( openiccArray_s    * array,
                                       int                 add )
{
  int count = openiccArray_Count( array );

  if((count + 1) >= array->reserved_n)
  {
    int new_count = array->reserved_n * 5;
    openiccDummy_s ** ptrs;

    if(add > 1)
      new_count = count + add;

    ptrs = calloc( sizeof(openiccDummy_s*), new_count );
    if(!ptrs)
    {
      ERRc_S("%s new_count: %d", _("Could not alloc memory"), new_count );
      return 1;
    }

    memmove( ptrs, array->array, sizeof(openiccDummy_s*) * count );
    free( array->array );
    array->array = ptrs;
    array->reserved_n = new_count;
  }

  return 0;
}

int      openiccArray_Push           ( openiccArray_s    * array )
{
  return openiccArray_Add( array, 1 );
}


int           openiccDB_AddScope     ( openiccDB_s       * db,
                                       const char        * top_key_name,
                                       openiccSCOPE_e      scope )
{
  int error = 0;
  const char * config_file = OPENICC_DB_PREFIX OPENICC_SLASH OPENICC_DB;
  int i;
  /* Locate the directories where the config file is, */
  /* and where we should copy the profile to. */
  int npaths;
  xdg_error er;
  char **paths;

  if ((npaths = xdg_bds(&er, &paths, xdg_conf, xdg_write, 
                        (scope == openiccSCOPE_SYSTEM) ? xdg_local : xdg_user,
                        config_file)) == 0)
  {
    ERRc_S("%s %d", _("Could not find config"), scope );
    return 1;
  }

  if(openicc_debug)
    DBGc_S("%s", _("Paths:"));
  for(i=0; i < npaths; ++i)
    if(openicc_debug)
      DBGc_S("%s", paths[i]);

  for(i = 0; i < npaths; ++i)
  {
    const char * db_file = paths[i];
    /* read JSON input file */
    size_t size = 0;
    char * text = openiccOpenFile( db_file, &size );

    /* parse JSON */
    if(text)
    {
      int count = openiccArray_Count( (openiccArray_s*)&db->ks );
      openiccConfig_s * config = openiccConfig_FromMem( text );
      if(text) free(text); text = NULL;
      openiccConfig_SetInfo ( config, db_file );

      /* reserve enough memory in list array */
      if( openiccArray_Push( (openiccArray_s*)&db->ks ))
      {
        ERRc_S("%s", _("Could not alloc memory") );
        return 1;
      }

      /* add new config to db */
      db->ks[count] = config;
    }
  }

  xdg_free(paths, npaths);

  return error;
}

openiccDB_s * openiccDB_NewFrom      ( const char        * top_key_name,
                                       openiccSCOPE_e      scope )
{
  openiccDB_s * db = calloc( sizeof(openiccDB_s), 1 );

  if(db)
  {
    db->top_key_name = openiccStringCopy( top_key_name, malloc );
    db->scope = scope;
    db->ks_array_reserved_n = 10;
    db->ks = calloc( sizeof(openiccConfig_s *), db->ks_array_reserved_n );
  }

  if(db)
  {
    int error = 0;

    if(!error &&
       (db->scope == openiccSCOPE_USER_SYS || db->scope == openiccSCOPE_USER))
    {
      error = openiccDB_AddScope( db, top_key_name, openiccSCOPE_USER );
    }

    if(!error &&
       (db->scope == openiccSCOPE_USER_SYS || db->scope == openiccSCOPE_SYSTEM))
    {
      error = openiccDB_AddScope( db, top_key_name, openiccSCOPE_SYSTEM );
    }

    if(error)
      ERRc_S("%s: %s %d", _("Could not setup db objetc"), top_key_name, scope );
  }

  return db;
}


void     openiccDB_Release           ( openiccDB_s      ** db )
{
  openiccDB_s * s;

  if(db)
  {
    int count, i;
    s = *db;

    if(!s)
      return;

    free( s->top_key_name );
    count = openiccArray_Count( (openiccArray_s*)&s->ks );
    for(i = 0; i < count; ++i)
      openiccConfig_Release( &s->ks[i] );
    free( s->ks );
    s->ks_array_reserved_n = 0;
    free( s );
    *db = 0;
  }
}

