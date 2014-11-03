/*  @file openicc_config.c
 *
 *  libOpenICC - OpenICC Colour Management Configuration
 *
 *  @par Copyright:
 *            2011-2013 (C) Kai-Uwe Behrmann
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
#ifndef HAVE_OY
int level_PROG = 0;
#endif
int openicc_backtrace = 0;

/**
 *  @brief   load configurations from in memory JSON text
 *  
 */
OpeniccConfigs_s * openiccConfigs_FromMem( const char       * data )
{
  OpeniccConfigs_s * configs = NULL;
  if(data && data[0])
  {
    configs = calloc( sizeof(OpeniccConfigs_s), 1 );
    if(!configs)
    {
      fprintf( stderr, "%s:%d ERROR: could not allocate %d bytes\n",
               __FILE__,__LINE__, (int)sizeof(OpeniccConfigs_s));
      return configs;
    }

    configs->json_text = strdup( (char*)data );
    configs->yajl = yajl_tree_parse( data, NULL, 0 );
    if(!configs->yajl)
    {
      char * msg = malloc(1024);
      configs->yajl = yajl_tree_parse( data, msg, 1024 );
      fprintf(stderr, "%s:%d ERROR: %s\n", __FILE__,__LINE__, msg?msg:"");
    }
  }

  return configs;
}

/**
 *  @brief   release the data base object
 */
void               openiccConfigs_Release (
                                       OpeniccConfigs_s ** configs )
{
  OpeniccConfigs_s * c = 0;
  if(configs)
  {
    c = *configs;
    if(c)
    {
      if(c->json_text)
        free(c->json_text);
      else
        fprintf( stderr, "%s:%d ERROR: expected OpeniccConfigs_s::json_text\n",
                  __FILE__,__LINE__ );
      if(c->yajl)
        yajl_tree_free(c->yajl);
      else
        fprintf( stderr, "%s:%d ERROR: expected OpeniccConfigs_s::yajl\n",
                 __FILE__,__LINE__ );
      if(c->dbg_text)
        free(c->dbg_text);
      else
        fprintf( stderr, "%s:%d ERROR: expected OpeniccConfigs_s::dbg_text\n",
                  __FILE__,__LINE__ );
    }
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
const char** const openiccConfigs_GetClasses (
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
 *  @param[in]     configs             the data base object
 *  @param[in]     device_classes      the device class filter
 *  @return                            count of matching device configurations
 */
int                openiccConfigs_Count (
                                       OpeniccConfigs_s  * configs,
                                       const char       ** device_classes )
{
  int n = 0;

  if(configs)
  {
    const char * base_path[] = {"org","freedesktop","openicc","device",0};
    yajl_val base = yajl_tree_get( configs->yajl, base_path, yajl_t_object );
    if(base)
    {
      yajl_val dev_class;
      {
        int i = 0, device_classes_n = 0;

        device_classes = openiccConfigs_GetClasses( device_classes,
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
      fprintf( stderr, "%s:%d ERROR: could not find " OPENICC_BASE_PATH " %s\n",
               __FILE__,__LINE__, configs->dbg_text ? configs->dbg_text : "" );
  }

  return n;
}

/**
 *  @brief   get keys and their values
 *
 *  @param[in]     configs             the data base object
 *  @param[in]     device_classes      the device class filter
 *  @param[in]     pos                 the device position
 *  @param[out]    keys                a zero terminated list of device keys
 *  @param[out]    values              a zero terminated list of device values
 *  @param[in]     alloc               user allocation function
 */
const char *       openiccConfigs_DeviceGet (
                                       OpeniccConfigs_s  * configs,
                                       const char       ** device_classes,
                                       int                 pos,
                                       char            *** keys,
                                       char            *** values,
                                       OpeniccConfigAlloc_f alloc )
{
  int n = 0;
  const char * actual_device_class = 0;

  if(configs)
  {
    const char * base_path[] = {"org","freedesktop","openicc","device",0};
    yajl_val base = yajl_tree_get( configs->yajl, (const char**)base_path,
                                   yajl_t_object );
    if(base)
    {
      yajl_val dev_class;
      {
        int i = 0, device_classes_n = 0;

        device_classes = openiccConfigs_GetClasses( device_classes,
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
                               StringAdd_( &t, "[" );
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
                                   StringAdd_( &t, "," );
                                   StringAdd_( &t, "\"" );
                                   StringAdd_( &t, tmp2 );
                                   StringAdd_( &t, "\"" );
                                   tmp = t;
                                 }
                               }
                               StringAdd_( &t, "]" );
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
      fprintf( stderr, "%s:%d ERROR: could not find " OPENICC_BASE_PATH " %s\n",
               __FILE__,__LINE__, configs->dbg_text ? configs->dbg_text : "" );
  }

  return actual_device_class;
}

/**
 *  @brief   add a string for debugging and error messages
 */
void               openiccConfigs_SetInfo (
                                       OpeniccConfigs_s  * configs,
                                       const char        * debug_info )
{
  if(configs && debug_info)
    configs->dbg_text = strdup( (char*)debug_info );
}

/**
 *  @brief   obtain a JSON string
 * 
 *  @param[in]     configs             a data base object
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
const char *       openiccConfigs_DeviceGetJSON (
                                       OpeniccConfigs_s  * configs,
                                       const char       ** device_classes,
                                       int                 pos,
                                       int                 flags,
                                       const char        * device_class,
                                       char             ** json,
                                       OpeniccConfigAlloc_f alloc )
{
  char            ** keys = 0;
  char            ** values = 0;
  int j, n = 0;
  char * txt = 0;

  const char * d = openiccConfigs_DeviceGet( configs, device_classes, pos,
                                               &keys, &values, malloc );

  if(alloc)
    txt = alloc(4096);
  else
    txt = calloc( sizeof(char), 4096 );

  if(txt)
    txt[0] = '\000';
  else
  {
    fprintf( stderr, "%s:%d ERROR: could not allocate 4096 bytes\n",
             __FILE__,__LINE__ );
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
    sprintf( &txt[strlen(txt)], OPENICC_DEVICE_JSON_FOOTER);
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
char *             openiccConfigs_DeviceClassGet (
                                       OpeniccConfigs_s  * config,
                                       OpeniccConfigAlloc_f alloc )
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
      fprintf( stderr, "%s:%d ERROR: could not find " OPENICC_BASE_PATH " %s\n",
               __FILE__,__LINE__, config->dbg_text ? config->dbg_text : "" );
  }

  return device_class;
}

int                openiccConfigs_Search (
                                       OpeniccConfigs_s  * config,
                                       const char       ** search_keys,
                                       const char       ** search_values,
                                       int              ** hits,
                                       OpeniccConfigAlloc_f alloc )
{
  char            ** keys = 0;
  char            ** values = 0;
  int i,j,k, n = 0, devices_n, keys_n = 0;
  int hits_n = 0;

  while(keys[keys_n++]) ;

  devices_n = openiccConfigs_Count(config, NULL);

  for(i = 0; i < devices_n; ++i)
  {
    const char * d = openiccConfigs_DeviceGet( config, NULL, i,
                                               &keys, &values, malloc );

    if(i)
      fprintf( stderr,"\n");

    n = 0; if(keys) while(keys[n]) ++n;
    fprintf( stderr, "[%d] device class:\"%s\" with %d keys/values pairs\n", i, d, n);
    for( j = 0; j < n; ++j )
    {
      for(k = 0; k < keys_n; ++k)
      {
        fprintf(stderr, "%s:\"%s\"\n", keys[j], values[j]);
        free(keys[j]);
        free(values[j]);
      }
    }
    free(keys); free(values);
  }

  return hits_n;
}

char*              StringAppend_     ( const char        * text,
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
 *  @version Oyranos: 0.1.10
 *  @since   2009/02/07 (Oyranos: 0.1.10)
 *  @date    2009/02/07
 */
int          openiccStringAddPrintf_ ( char             ** string,
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
    fprintf(stderr,
     "openicc_config.c openiccStringAddPrintf_() Could not allocate 256 byte of memory.\n");
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


  text_copy = StringAppend_(*string, text);

  if(string && *string)
    free(*string);

  *string = text_copy;

  free(text);

  return 0;
}


void               StringAdd_        ( char             ** text,
                                       const char        * append )
{
  char * text_copy = NULL;

  text_copy = StringAppend_(*text, append);

  if(text && *text)
    free(*text);

  *text = text_copy;

  return;
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
 *  @version Oyranos: 0.2.1
 *  @since   2008/04/03 (Oyranos: 0.2.1)
 *  @date    2011/01/15
 */
int                openiccMessageFormat (
                                       char             ** message_text,
                                       int                 code,
                                       OpeniccConfigs_s  * context_object,
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
  OpeniccConfigs_s * c = (OpeniccConfigs_s*) context_object;

  if(code == openiccMSG_DBG && !openicc_debug)
    return 0;

  if(c)
  {
    type_name = "OpeniccConfigs_s";
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
    openiccStringAddPrintf_( &t,
                        " %03f: ", DBG_UHR_);
    openiccStringAddPrintf_( &t,
                        "%s%s%s%s ", type_name,
             id_text ? "=\"" : "", id_text ? id_text : "", id_text ? "\"" : "");
  }

  STRING_ADD( t, string );

  if(openicc_backtrace)
  {
#   define TMP_FILE "/tmp/oyranos_gdb_temp." OPENICC_VERSION_NAME "txt"
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
      fprintf( stderr, "could not open " TMP_FILE "\n" );
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
 *  @param         context_object      a OpeniccConfigs_s is expected
 *  @param         format              the text format string for following args
 *  @param         ...                 the variable args fitting to format
 *  @return                            0 - success; 1 - error
 *
 *  @version Oyranos: 0.3.0
 *  @since   2008/04/03 (Oyranos: 0.1.8)
 *  @date    2009/07/20
 */
int  openiccMessageFunc              ( openiccMSG_e        code,
                                       OpeniccConfigs_s  * context_object,
                                       const char        * format,
                                       ... )
{
  char * text = 0, * msg = 0;
  int error = 0;
  va_list list;
  size_t sz = 0;
  int len = 0;
  OpeniccConfigs_s * c = context_object;


  va_start( list, format);
  len = vsnprintf( text, sz, format, list);
  va_end  ( list );

  {
    text = calloc( sizeof(char), len+2 );
    if(!text)
    {
      fprintf(stderr,
      "openicc_config.c openiccMessageFunc() Could not allocate 256 byte of memory.\n");
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
 *  @version Oyranos: 0.1.8
 *  @date    2008/04/03
 *  @since   2008/04/03 (Oyranos: 0.1.8)
 */
int            openiccMessageFuncSet ( openiccMessage_f    message_func )
{
  if(message_func)
    openiccMessage_p = message_func;
  return 0;
}


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
          fprintf(stderr, "Error: fread %lu but should read %lu\n",
                  (long unsigned int) s, (long unsigned int)size);
      } else
      {
        fprintf(stderr, "Error: Could not open file - \"%s\"\n", file_name);
      }
    }

  if(size_ptr)
    *size_ptr = size;

  return text;
}

