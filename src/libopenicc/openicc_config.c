/*  @file openicc_config.c
 *
 *  libOpenICC - OpenICC Colour Management Configuration
 *
 *  @par Copyright:
 *            2011-2015 (C) Kai-Uwe Behrmann
 *
 *  @brief    OpenICC Colour Management configuration helpers
 *  @internal
 *  @author   Kai-Uwe Behrmann <ku.b@gmx.de>
 *  @par License:
 *            MIT <http://www.opensource.org/licenses/mit-license.php>
 *  @since    2011/06/27
 */

#include "openicc_config_internal.h"

int openicc_debug = 0;
#ifndef HAVE_OPENICC
int level_PROG = 0;
#endif
int openicc_backtrace = 0;

#if HAVE_POSIX
#include <unistd.h>  /* getpid() */
#endif
#include <string.h>  /* strdup() */
#include <stdarg.h>  /* vsnprintf() */
#include <stdio.h>   /* vsnprintf() */


/**
 *  @brief   load configurations from in memory JSON text
 *  
 */
openiccConfig_s *  openiccConfig_FromMem( const char       * data )
{
  openiccConfig_s * config = NULL;
  if(data && data[0])
  {
    config = calloc( sizeof(openiccConfig_s), 1 );
    if(!config)
    {
      ERRc_S( "could not allocate %d bytes",
               (int)sizeof(openiccConfig_s));
      return config;
    }

    config->json_text = strdup( (char*)data );
    config->yajl = yajl_tree_parse( data, NULL, 0 );
    if(!config->yajl)
    {
      char * msg = malloc(1024);
      config->yajl = yajl_tree_parse( data, msg, 1024 );
      WARNc_S( "%s\n", msg?msg:"" );
      if( msg ) free(msg);
      openiccConfig_Release( &config );
    }
  }

  return config;
}

/**
 *  @brief   release the data base object
 */
void               openiccConfig_Release (
                                       openiccConfig_s  ** config )
{
  openiccConfig_s * c = 0;
  if(config)
  {
    c = *config;
    if(c)
    {
      if(c->json_text)
        free(c->json_text);
      else
        WARNcc_S( c, "expected openiccConfig_s::json_text", 0 );
      if(c->yajl)
        yajl_tree_free(c->yajl);
      else
        WARNcc_S( c, "expected openiccConfig_s::yajl",0 );
      if(c->dbg_text)
        free(c->dbg_text);
      else
        WARNcc_S( c, "expected openiccConfig_s::dbg_text",0 );
      free(c);
    }
    *config = NULL;
  }
}

static const char * dev_cl[] = {
                OPENICC_DEVICE_MONITOR,
                OPENICC_DEVICE_SCANNER,
                OPENICC_DEVICE_PRINTER ,
                OPENICC_DEVICE_CAMERA , NULL };
/**
 *  @brief   get default device class
 */
const char** const openiccConfigGetDeviceClasses (
                                       const char       ** device_classes,
                                       int               * count )
{
  int device_classes_n = 0;

  if(device_classes)
    while(device_classes[device_classes_n++]) ;
  else
  {
    device_classes_n = 4;
    device_classes = dev_cl;
  }

  *count = device_classes_n;

  return device_classes;
}

/**
 *  @brief count devices in data base object
 *
 *  @param[in]     config              the data base object
 *  @param[in]     device_classes      the device class filter
 *  @return                            count of matching device configurations
 */
int                openiccConfig_CountDevices (
                                       openiccConfig_s   * config,
                                       const char       ** device_classes )
{
  int n = 0;

  if(config)
  {
    const char * base_path[] = {"org","freedesktop","openicc","device",0};
    yajl_val base = yajl_tree_get( config->yajl, base_path, yajl_t_object );
    if(base)
    {
      yajl_val dev_class;
      {
        int i = 0, device_classes_n = 0;

        device_classes = openiccConfigGetDeviceClasses( device_classes,
                                       &device_classes_n );

        for(i = 0; i < device_classes_n; ++i)
        {
          const char * obj_key[] = { device_classes[i], 0 };
          dev_class = yajl_tree_get( base, obj_key, yajl_t_array );
          if(dev_class)
            n += dev_class->u.array.len;
        }
      }
    } else
      WARNcc_S( config, "could not find " OPENICC_DEVICE_PATH " %s",
                config->dbg_text ? config->dbg_text : "" );
  }

  return n;
}

/**
 *  @brief   get keys and their values
 *
 *  @param[in]     config              the data base object
 *  @param[in]     device_classes      the device class filter
 *  @param[in]     pos                 the device position
 *  @param[out]    keys                a zero terminated list of device keys
 *  @param[out]    values              a zero terminated list of device values
 *  @param[in]     alloc               user allocation function
 */
const char *       openiccConfig_DeviceGet (
                                       openiccConfig_s   * config,
                                       const char       ** device_classes,
                                       int                 pos,
                                       char            *** keys,
                                       char            *** values,
                                       openiccAlloc_f      alloc )
{
  int n = 0;
  const char * actual_device_class = 0;

  if(config)
  {
    const char * base_path[] = {"org","freedesktop","openicc","device",0};
    yajl_val base = yajl_tree_get( config->yajl, (const char**)base_path,
                                   yajl_t_object );
    if(base)
    {
      yajl_val dev_class;
      {
        int i = 0, device_classes_n = 0;

        device_classes = openiccConfigGetDeviceClasses( device_classes,
                                       &device_classes_n );

        for(i = 0; i < device_classes_n; ++i)
        {
          const char * obj_key[] = { device_classes[i], 0 };
          int j = 1;
          yajl_val device = 0;
          dev_class = yajl_tree_get( base, obj_key, yajl_t_array );
          if(dev_class)
          {
            int elements = dev_class->u.array.len;
            for(j = 0; j < elements; ++j)
            {
              device = dev_class->u.array.values[j];
              if(n == pos)
              {
                actual_device_class = device_classes[i];
                if(YAJL_IS_OBJECT( device ))
                {
                  int count = device->u.object.len;
                  *keys = alloc( sizeof(char*) * (count + 1) );
                  *values = alloc( sizeof(char*) * (count + 1) );
                  if(*keys) memset(*keys, 0, sizeof(char*) * (count + 1) );
                  if(*values) memset(*values, 0, sizeof(char*) * (count + 1) );
                  for(i = 0; i < count; ++i)
                  {
                    if(device->u.object.keys[i] && device->u.object.keys[i][0])
                    {
                      (*keys)[i] = alloc( sizeof(char) *
                                       (strlen(device->u.object.keys[i]) + 1) );
                      strcpy( (*keys)[i], device->u.object.keys[i] );
                    }
                    if(device->u.object.values[i])
                    {
                      char * t = 0;
                      const char * tmp = NULL, * tmp2 = NULL;
                      switch(device->u.object.values[i]->type)
                      {
                        case yajl_t_string:
                             tmp = device->u.object.values[i]->u.string; break;
                        case yajl_t_number:
                             tmp = device->u.object.values[i]->u.number.r;break;
                        case yajl_t_array:
                             {
                               int k = 0,
                                   n = device->u.object.values[i]->u.array.len;
                               openiccStringAdd_( &t, "[" );
                               for(k = 0; k < n; ++k)
                               {
                                 if(device->u.object.values[i]->
                                    u.array.values[k]->type == yajl_t_string)
                                   tmp2 = device->u.object.values[i]->
                                         u.array.values[k]->u.string;
                                 else
                                 if(device->u.object.values[i]->
                                    u.array.values[k]->type == yajl_t_number)
                                   tmp2 = device->u.object.values[i]->
                                         u.array.values[k]->u.number.r;

                                 if(tmp2)
                                 {
                                   if(k != 0)
                                   openiccStringAdd_( &t, "," );
                                   openiccStringAdd_( &t, "\"" );
                                   openiccStringAdd_( &t, tmp2 );
                                   openiccStringAdd_( &t, "\"" );
                                   tmp = t;
                                 }
                               }
                               openiccStringAdd_( &t, "]" );
                               tmp = t;
                             }
                             break;
                        default:
                             tmp = "no string or number"; break;
                      }
                      if(!tmp)
                        tmp = "no value found";
                      (*values)[i] = alloc( sizeof(char) * (strlen(tmp) + 1) );
                      strcpy( (*values)[i], tmp );
                    }
                  }
                }
                break;
              }
              if(device)
                ++n;
            }
          }
        }
      }
    } else
      WARNcc_S( config, "could not find " OPENICC_DEVICE_PATH " %s",
                config->dbg_text ? config->dbg_text : "" );
  }

  return actual_device_class;
}

/**
 *  @brief   add a string for debugging and error messages
 */
void               openiccConfig_SetInfo (
                                       openiccConfig_s   * config,
                                       const char        * debug_info )
{
  if(config && debug_info)
  {
    if(config->dbg_text)
      free(config->dbg_text);
    config->dbg_text = strdup( (char*)debug_info );
  }
}

/**
 *  @brief   obtain a JSON string
 * 
 *  @param[in]     config              a data base object
 *  @param[in]     device_classes      a zero terminated list of device class
 *                                     strings
 *  @param[in]     pos                 device position in list
 *  @param[in]     flags               - OPENICC_CONFIGS_SKIP_HEADER
 *                                     - OPENICC_CONFIGS_SKIP_FOOTER
 *  @param[in]     device_class        the last written device class
 *  @param[out]    json                the resulting JSON string allocated by
 *                                     alloc
 *  @param[in]     alloc               user allocation function
 *  @return                            device class
 */
const char *       openiccConfig_DeviceGetJSON (
                                       openiccConfig_s   * config,
                                       const char       ** device_classes,
                                       int                 pos,
                                       int                 flags,
                                       const char        * device_class,
                                       char             ** json,
                                       openiccAlloc_f      alloc )
{
  char            ** keys = 0;
  char            ** values = 0;
  int j, n = 0;
  char * txt = 0;

  const char * d = openiccConfig_DeviceGet( config, device_classes, pos,
                                            &keys, &values, malloc );

  if(alloc)
    txt = alloc(4096);
  else
    txt = calloc( sizeof(char), 4096 );

  if(txt)
    txt[0] = '\000';
  else
  {
    ERRcc_S( config, "could not allocate 4096 bytes",0 );
    return txt;
  }

  if(!(flags & OPENICC_CONFIGS_SKIP_HEADER))
    sprintf( txt, OPENICC_DEVICE_JSON_HEADER, d );
  else if(device_class)
  {
    if(d != device_class)
      sprintf( &txt[strlen(txt)],
  "\n          ],\n          \"%s\": [{\n", d);
    else
        /* end the previous JSON array field and open the next one */
      sprintf( &txt[strlen(txt)], ",\n            {\n");
  }

    n = 0; if(keys) while(keys[n]) ++n;
    for( j = 0; j < n; ++j )
    {
      char * val = values[j];
      if(val[0] != '[')
        sprintf( &txt[strlen(txt)],
  "              \"%s\": \"%s\"", keys[j], val);
      else
        sprintf( &txt[strlen(txt)],
  "              \"%s\": %s", keys[j], val);
      if(j < n-1)
        sprintf( &txt[strlen(txt)], ",");
      sprintf( &txt[strlen(txt)], "\n");
      free(keys[j]);
      free(val);
    }
    if(keys) free(keys); if(values) free(values);

    /* close the object */
  if(!(flags & OPENICC_CONFIGS_SKIP_FOOTER))
    sprintf( &txt[strlen(txt)], "            }\n" OPENICC_DEVICE_JSON_FOOTER);
  else
    sprintf( &txt[strlen(txt)], "            }" );

  *json = txt;

  return d;
}

/**
 *  @brief   find out the device class of a given data base entry
 *
 *  @param[in]     config              a data base entry object
 *  @param[in]     alloc               user allocation function
 */
char *             openiccConfig_DeviceClassGet (
                                       openiccConfig_s   * config,
                                       openiccAlloc_f      alloc )
{
  char * device_class = 0;

  if(config)
  {
    const char * base_path[] = {"org","freedesktop","openicc","device",0};
    yajl_val base = yajl_tree_get( config->yajl, (const char**)base_path,
                                   yajl_t_object );
    if(base && YAJL_IS_OBJECT( base ))
    {
      yajl_val v = base;

      if(v->u.object.keys[0] && v->u.object.keys[0][0])
      {
        device_class = alloc( sizeof(char) *
                              (strlen(v->u.object.keys[0]) + 1) );
        strcpy( device_class, v->u.object.keys[0] );
      }

    } else
      WARNcc_S( config, "could not find " OPENICC_DEVICE_PATH " %s",
                config->dbg_text ? config->dbg_text : "" );
  }

  return device_class;
}

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

/** @func    openiccMessageFormat
 *  @brief   default function to form a message string
 *
 *  This default message function is used as a message formatter.
 *  The resulting string can be placed anywhere, e.g. in a GUI.
 *
 *  @see the openiccMessageFunc() needs just to replaxe the fprintf with your 
 *  favourite GUI call.
 *
 *  @version OpenICC: 0.1.0
 *  @date    2011/01/15
 *  @since   2008/04/03 (OpenICC: 0.1.0)
 */
int                openiccMessageFormat (
                                       char             ** message_text,
                                       int                 code,
                                       openiccConfig_s   * context_object,
                                       const char        * string )
{
  char * text = 0, * t = 0;
  int i;
  const char * type_name = "";
#ifdef HAVE_POSIX
  pid_t pid = 0;
#else
  int pid = 0;
#endif
  FILE * fp = 0;
  const char * id_text = 0;
  char * id_text_tmp = 0;
  openiccConfig_s * c = (openiccConfig_s*) context_object;

  if(code == openiccMSG_DBG && !openicc_debug)
    return 0;

  if(c)
  {
    type_name = "openiccConfig_s";
    id_text = c->dbg_text;
    if(id_text)
      id_text_tmp = strdup(id_text);
    id_text = id_text_tmp;
  }

  text = calloc( sizeof(char), 256 );

# define MAX_LEVEL 20
  if(level_PROG < 0)
    level_PROG = 0;
  if(level_PROG > MAX_LEVEL)
    level_PROG = MAX_LEVEL;
  for (i = 0; i < level_PROG; i++)
    sprintf( &text[strlen(text)], " ");

  STRING_ADD( t, text );

  text[0] = 0;

  switch(code)
  {
    case openiccMSG_WARN:
         STRING_ADD( t, _("WARNING") );
         break;
    case openiccMSG_ERROR:
         STRING_ADD( t, _("!!! ERROR"));
         break;
  }

  /* reduce output for non core messages */
  if( (openiccMSG_ERROR <= code && code <= 399) )
  {
    openiccStringAddPrintf( &t,
                        " %03f: ", DBG_UHR_);
    openiccStringAddPrintf( &t,
                        "%s%s%s%s ", type_name,
             id_text ? "=\"" : "", id_text ? id_text : "", id_text ? "\"" : "");
  }

  STRING_ADD( t, string );

  if(openicc_backtrace)
  {
#   define TMP_FILE "/tmp/openicc_gdb_temp." OPENICC_VERSION_NAME "txt"
#ifdef HAVE_POSIX
    pid = (int)getpid();
#endif
    fp = fopen( TMP_FILE, "w" );

    if(fp)
    {
      fprintf(fp, "attach %d\n", pid);
      fprintf(fp, "thread 1\nbacktrace\n"/*thread 2\nbacktrace\nthread 3\nbacktrace\n*/"detach" );
      fclose(fp);
      fprintf( stderr, "GDB output:\n" );
      i = system("gdb -batch -x " TMP_FILE);
    } else
      fprintf(stderr,
      OI_DBG_FORMAT_"Could not open " TMP_FILE "\n",OI_DBG_ARGS_);
  }

  free( text ); text = 0;
  if(id_text_tmp) free(id_text_tmp); id_text_tmp = 0;

  *message_text = t;

  return 0;
}

/** @func    openiccMessageFunc
 *  @brief   default message function to console
 *
 *  The default message function is used as a message printer to the console 
 *  from library start.
 *
 *  @param         code                a message code understood be your message
 *                                     handler or openiccMSG_e
 *  @param         context_object      a openiccConfig_s is expected
 *  @param         format              the text format string for following args
 *  @param         ...                 the variable args fitting to format
 *  @return                            0 - success; 1 - error
 *
 *  @version OpenICC: 0.1.0
 *  @date    2009/07/20
 *  @since   2008/04/03 (OpenICC: 0.1.0)
 */
int  openiccMessageFunc              ( openiccMSG_e        code,
                                       openiccConfig_s   * context_object,
                                       const char        * format,
                                       ... )
{
  char * text = 0, * msg = 0;
  int error = 0;
  va_list list;
  size_t sz = 0;
  int len = 0;
  openiccConfig_s * c = context_object;


  va_start( list, format);
  len = vsnprintf( text, sz, format, list);
  va_end  ( list );

  {
    text = calloc( sizeof(char), len+2 );
    if(!text)
    {
      fprintf(stderr,
      OI_DBG_FORMAT_"Could not allocate 256 byte of memory.\n",OI_DBG_ARGS_);
      return 1;
    }
    va_start( list, format);
    len = vsnprintf( text, len+1, format, list);
    va_end  ( list );
  }

  error = openiccMessageFormat( &msg, code, c, text );

  if(msg)
    fprintf( stderr, "%s\n", msg );

  free( text ); text = 0;
  free( msg ); msg = 0;

  return error;
}

openiccMessage_f     openiccMessage_p = openiccMessageFunc;

/** @func    openiccMessageFuncSet
 *  @brief
 *
 *  @version OpenICC: 0.1.0
 *  @date    2011/10/21
 *  @since   2008/04/03 (OpenICC: 0.1.0)
 */
int            openiccMessageFuncSet ( openiccMessage_f    message_func )
{
  if(message_func)
    openiccMessage_p = message_func;
  return 0;
}

/** @func    openiccVersion
 *  @brief
 *
 *  @version OpenICC: 0.1.0
 *  @date    2015/08/27
 *  @since   2015/08/27 (OpenICC: 0.1.0)
 */
int            openiccVersion        ( void )
{
  return OPENICC_VERSION;
}

const char * openicc_domain_path = OPENICC_LOCALEDIR;
int openicc_i18n_init = 0;
/** @func    openiccInit
 *  @brief   init the library; optionally
 *
 *  Additionally use setlocale() somewhere in your application.
 *
 *  return -1 for no USE_GETTEXT defined, otherwise 1
 *
 *  @version OpenICC: 0.1.0
 *  @date    2015/08/27
 *  @since   2015/08/27 (OpenICC: 0.1.0)
 */
int            openiccInit           ( void )
{
#ifdef USE_GETTEXT
  if(!openicc_i18n_init)
  {
    char * temp = 0;
    ++openicc_i18n_init;

    if(getenv("OI_LOCALEDIR") && strlen(getenv("OI_LOCALEDIR")))
      openicc_domain_path = strdup(getenv("OI_LOCALEDIR"));

    openiccStringAdd_( &temp, "NLSPATH=");
    openiccStringAdd_( &temp, openicc_domain_path);
    putenv(temp); /* Solaris */

    bindtextdomain( "OpenICC", openicc_domain_path );
    if(openicc_debug)
      WARNc_S("bindtextdomain() to \"%s\"", openicc_domain_path );
  }
  return openicc_i18n_init;
#endif
  return -1;
}

/** \addtogroup path_names
 *  @{
 */

/**
 *  @brief get Path Name for Installation 
 *
 *  Note: Not all combinations return a path name. Some make no sense.
 *  So be careful and test the result.
 *
 *  ::openiccPATH_MODULE + ::openiccSCOPE_USER and ::openiccPATH_MODULE + ::openiccSCOPE_OPENICC are
 *  supported. ::openiccPATH_SCRIPT gives no result at all.
 *
 *  @version OpenICC: 0.1.0
 *  @date    2015/08/28
 *  @since   2015/02/08 (OpenICC: 0.1.0)
 */
char *       openiccGetInstallPath   ( openiccPATH_TYPE_e  type,
                                       openiccSCOPE_e      scope,
                                       openiccAlloc_f      allocFunc )
{
  char * path = NULL;
#define C(p) openiccStringCopy(p,allocFunc);
  switch (type)
  {
    case openiccPATH_ICC:
      switch((int)scope)
      {
        case openiccSCOPE_USER:
          path = C( OS_ICC_USER_DIR );
          break;
        case openiccSCOPE_SYSTEM:
          path = C( OS_ICC_SYSTEM_DIR ) ;
          break;
        case openiccSCOPE_OPENICC:
          path = C( OPENICC_SYSCOLORDIR OPENICC_SLASH OPENICC_ICCDIRNAME );
          break;
        case openiccSCOPE_MACHINE:
          path = C( OS_ICC_MACHINE_DIR );
        break;
        default:
          path = NULL;
      }
      break;
    case openiccPATH_POLICY:
    {
      switch((int)scope)
      {
        case openiccSCOPE_USER:
          path = C( OS_SETTINGS_USER_DIR );
          break;
        case openiccSCOPE_SYSTEM:
          path = C( OS_SETTINGS_SYSTEM_DIR );
          break;
        case openiccSCOPE_OPENICC:
          path = C( OPENICC_SYSCOLORDIR OPENICC_SLASH OPENICC_SETTINGSDIRNAME);
          break;
        case openiccSCOPE_MACHINE:
          path = C( OS_SETTINGS_MACHINE_DIR );
        break;
      }
      break;
    }
    case openiccPATH_MODULE:
    {
      switch((int)scope)
      {
        case openiccSCOPE_USER:
        {
          char * t = NULL;
          openiccStringAddPrintf( &t,
                             "~/.local/lib%s/" OPENICC_CMMSUBPATH, strstr(OPENICC_LIBDIR, "lib64") ? "64":"");
          path = C( t );
          if(t) free(t); t = NULL;
          break;
        }
        case openiccSCOPE_OPENICC:
          path = C( OPENICC_CMMDIR );
          break;
        default:
          path = NULL;
      }
      break;
    }
    case openiccPATH_CACHE:
    {
      switch((int)scope)
      {
        case openiccSCOPE_USER:
          path = C( OS_DL_CACHE_USER_DIR );
          break;
        case openiccSCOPE_SYSTEM:
          path = C( OS_DL_CACHE_SYSTEM_DIR );
          break;
        default:
          path = NULL;
      }
      break;
    }
    default:
      path = NULL;
  }

  return path;
}

/*  @} *//* path_names */


char * openiccOpenFile( const char * file_name,
                        size_t   * size_ptr )
{
  FILE * fp = NULL;
  size_t size = 0, s = 0;
  char * text = NULL;

    if(file_name)
    {
      fp = fopen(file_name,"rb");
      if(fp)
      {
        fseek(fp,0L,SEEK_END); 
        size = ftell (fp);
        rewind(fp);
        text = malloc(size+1);
        s = fread(text, sizeof(char), size, fp);
        text[size] = '\000';
        if(s != size)
          WARNc_S( "Error: fread %lu but should read %lu",
                  (long unsigned int) s, (long unsigned int)size);
        fclose( fp );
      } else
      {
        WARNc_S( "Error: Could not open file - \"%s\"", file_name);
      }
    }

  if(size_ptr)
    *size_ptr = size;

  return text;
}

char *       openiccReadFileSToMem   ( FILE              * fp,
                                       size_t            * size)
{
  size_t mem_size = 256;
  char* mem = malloc(mem_size),
        c;

  if (fp && size)
  {
    *size = 0;
    do
    {
      c = getc(fp);
      if(*size >= mem_size)
      {
        mem_size *= 2;
        mem = realloc( mem, mem_size );
      }
      mem[(*size)++] = c;
    } while(!feof(fp));

    --*size;

    if(mem)
      mem[*size] = 0;
    else
    {
      free (mem);
      *size = 0;
    }
  }

  return mem;
}

#include <errno.h>
#include <sys/stat.h> /* mkdir() */
char* openiccExtractPathFromFileName_ (const char* file_name)
{
  char *path_name = 0;
  char *ptr;

  path_name = strdup( file_name );
  ptr = strrchr (path_name, '/');
  if(ptr)
    ptr[0] = 0;
  else
    strcpy( path_name, "." );
  return path_name;
}
int openiccIsDirFull_ (const char* name)
{
  struct stat status;
  int r = 0;

  memset(&status,0,sizeof(struct stat));
  r = stat (name, &status);

  if(r != 0 && openicc_debug > 1)
  switch (errno)
  {
    case EACCES:       WARNc_S("Permission denied: %s", name); break;
    case EIO:          WARNc_S("EIO : %s", name); break;
    case ENAMETOOLONG: WARNc_S("ENAMETOOLONG : %s", name); break;
    case ENOENT:       WARNc_S("A component of the name/file_name does not exist, or the file_name is an empty string: \"%s\"", name); break;
    case ENOTDIR:      WARNc_S("ENOTDIR : %s", name); break;
#ifdef HAVE_POSIX
    case ELOOP:        WARNc_S("Too many symbolic links encountered while traversing the name: %s", name); break;
    case EOVERFLOW:    WARNc_S("EOVERFLOW : %s", name); break;
#endif
    default:           WARNc_S("%s : %s", strerror(errno), name); break;
  }
  r = !r &&
       ((status.st_mode & S_IFMT) & S_IFDIR);

  return r;
}

char * openiccPathGetParent_ (const char* name)
{
  char *parentDir = 0, *ptr = 0;

  parentDir = strdup( name );
  ptr = strrchr( parentDir, '/');
  if (ptr)
  {
    if (ptr[1] == 0) /* ending dir separator */
    {
      ptr[0] = 0;
      if (strrchr( parentDir, '/'))
      {
        ptr = strrchr (parentDir, '/');
        ptr[0+1] = 0;
      }
    }
    else
      ptr[0+1] = 0;
  }

  return parentDir;
}


int openiccMakeDir_ (const char* path)
{
  const char * full_name = path;
  char * path_parent = 0,
       * path_name = 0;
  int rc = !full_name;

#ifdef HAVE_POSIX
  mode_t mode = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH; /* 0755 */
#endif

  if(full_name)
    path_name = openiccExtractPathFromFileName_(full_name);
  if(path_name)
  {
    if(!openiccIsDirFull_(path_name))
    {
      path_parent = openiccPathGetParent_(path_name);
      if(!openiccIsDirFull_(path_parent))
      {
        rc = openiccMakeDir_(path_parent);
        free( path_parent );
      }

      rc = mkdir (path_name
#ifdef HAVE_POSIX
                            , mode
#endif
                                  );
      if(rc && openicc_debug > 1)
      switch (errno)
      {
        case EACCES:       WARNc_S("Permission denied: %s", path); break;
        case EIO:          WARNc_S("EIO : %s", path); break;
        case ENAMETOOLONG: WARNc_S("ENAMETOOLONG : %s", path); break;
        case ENOENT:       WARNc_S("A component of the path/file_name does not exist, or the file_name is an empty string: \"%s\"", path); break;
        case ENOTDIR:      WARNc_S("ENOTDIR : %s", path); break;
#ifdef HAVE_POSIX
        case ELOOP:        WARNc_S("Too many symbolic links encountered while traversing the path: %s", path); break;
        case EOVERFLOW:    WARNc_S("EOVERFLOW : %s", path); break;
#endif
        default:           WARNc_S("%s : %s", strerror(errno), path); break;
      }
    }
    free( path_name );;
  }

  return rc;
}


size_t openiccWriteFile(const char * filename,
                        void       * mem,
                        size_t       size )
{
  FILE *fp = 0;
  const char * full_name = filename;
  int r = !filename;
  size_t written_n = 0;
  char * path = 0;

  if(!r)
  {
    path = openiccExtractPathFromFileName_( full_name );
    r = openiccMakeDir_( path );
  }

  if(!r)
  {
    fp = fopen(full_name, "wb");
    if ((fp != 0)
     && mem
     && size)
    {
#if 0
      do {
        r = fputc ( block[pt++] , fp);
      } while (--size);
#else
      written_n = fwrite( mem, 1, size, fp );
      if(written_n != size)
        r = errno;
#endif
    } else 
      if(mem && size)
        r = errno;
      else
        WARNc_S("no data to write into: \"%s\"", filename );

    if(r && openicc_debug > 1)
    {
      switch (errno)
      {
        case EACCES:       WARNc_S("Permission denied: %s", filename); break;
        case EIO:          WARNc_S("EIO : %s", filename); break;
        case ENAMETOOLONG: WARNc_S("ENAMETOOLONG : %s", filename); break;
        case ENOENT:       WARNc_S("A component of the path/file_name does not exist, or the file_name is an empty string: \"%s\"", filename); break;
        case ENOTDIR:      WARNc_S("ENOTDIR : %s", filename); break;
#ifdef HAVE_POSIX
        case ELOOP:        WARNc_S("Too many symbolic links encountered while traversing the path: %s", filename); break;
        case EOVERFLOW:    WARNc_S("EOVERFLOW : %s", filename); break;
#endif
        default:           WARNc_S("%s : %s", strerror(errno), filename);break;
      }
    }

    if (fp) fclose (fp);
  }

  if(path) free( path );

  return written_n;
}
