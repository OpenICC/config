#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <yajl/yajl_tree.h>

#include "openicc_config.h"

struct OpeniccConfigs_s {
  char     * json_text;
  yajl_val   yajl;
  char     * dbg_text;
};

void               StringAdd_        ( char             ** text,
                                       const char        * append );

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

/**
 *  @brief   get default device class
 */
const char **      openiccConfigs_GetClasses (
                                       const char       ** device_classes,
                                       int               * count )
{
        int device_classes_n = 0;

        if(device_classes)
          while(device_classes[device_classes_n++]) ;
        else
        {
          static const char * dev_cl[] = {
                OPENICC_DEVICE_MONITOR,
                OPENICC_DEVICE_SCANNER,
                OPENICC_DEVICE_PRINTER ,
                OPENICC_DEVICE_CAMERA , 0 };
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
                      int len = 0;
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
                      len = strlen(tmp);
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
 *  @param[in]     alloc               user allocation function
 *  @return                            the resulting JSON string allocated by
 *                                     alloc
 */
char *             openiccConfigs_DeviceGetJSON (
                                       OpeniccConfigs_s  * configs,
                                       const char       ** device_classes,
                                       int                 pos,
                                       int                 flags,
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

  if(!(flags & OPENICC_CONFIGS_SKIP_FOOTER))
    sprintf( &txt[strlen(txt)], OPENICC_DEVICE_JSON_FOOTER);

  return txt;
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
  int *hits_, hits_n = 0;

  while(keys[keys_n++]) ;

  devices_n = openiccConfigs_Count(config, NULL);
  hits_ = calloc( sizeof(int), devices_n + 1 );

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

