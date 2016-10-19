/*  @file openicc_config.c
 *
 *  libOpenICC - OpenICC Colour Management Configuration
 *
 *  @par Copyright:
 *            2011-2016 (C) Kai-Uwe Behrmann
 *
 *  @brief    OpenICC Colour Management configuration helpers
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


/**
 *  @brief   load configurations from in memory JSON text
 *  @memberof openiccConfig_s
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
 *  @memberof openiccConfig_s
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
 *  @memberof openiccConfig_s
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
 *  @memberof openiccConfig_s
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
 *  @memberof openiccConfig_s
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
 *  @memberof openiccConfig_s
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
 *  @memberof openiccConfig_s
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

/**
 *  @brief    get a filterd list of key names
 *  @memberof openiccConfig_s
 *
 *  @param[in]     config              a data base entry object
 *  @param[in]     key_name            top key name to filter for
 *  @param[in]     alloc               user allocation function
 *  @param[out]    n                   number of found keys
 *  @param[out]    key_names           found keys
 *  return                             0 - success, >=1 - error, <0 - issue
 */
int                openiccConfig_GetKeyNames (
                                       openiccConfig_s   * config,
                                       const char        * key_name,
                                       openiccAlloc_f      alloc,
                                       char             ** key_names,
                                       int               * n )
{
  char ** keys = NULL;
  int error = 0;
  return error;
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
 *  supported.
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

