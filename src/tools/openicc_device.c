/*  @file openicc_device.c
 *
 *  libOpenICC - OpenICC Colour Management Configuration
 *
 *  @par Copyright:
 *            2011-2018 (C) Kai-Uwe Behrmann
 *
 *  @brief    OpenICC Colour Management configuration helpers
 *  @internal
 *  @author   Kai-Uwe Behrmann <ku.b@gmx.de>
 *  @par License:
 *            MIT <http://www.opensource.org/licenses/mit-license.php>
 *  @since    2011/10/20
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "openicc_conf.h"
#include "openicc_version.h"
#include "openicc_config.h"
#include "openicc_core.h"
#include "openicc_macros.h"
#include "openicc_config_internal.h"
#include "xdg_bds.h"

typedef enum {
	ucmm_ok = 0,
	ucmm_resource,				/* Resource failure (e.g. out of memory) */
	ucmm_invalid_profile,		/* Profile is not a valid display ICC profile */
	ucmm_no_profile,			/* There is no associated profile */
	ucmm_no_home,				/* There is no HOME environment variable defined */
	ucmm_no_edid_or_display,	/* There is no edid or display name */
	ucmm_profile_copy,			/* There was an error copying the profile */
	ucmm_open_config,			/* There was an error opening the config file */
	ucmm_access_config,			/* There was an error accessing the config information */
	ucmm_set_config,			/* There was an error setting the config file */
	ucmm_save_config,			/* There was an error saving the config file */
	ucmm_monitor_not_found,		/* The EDID or display wasn't matched */
	ucmm_delete_key,			/* Delete_key failed */
	ucmm_delete_profile,		/* Delete_key failed */
} ucmm_error;

/* Install scope */
typedef enum {
	ucmm_user,
	ucmm_local_system
} ucmm_scope;


int main(int argc, char ** argv)
{
  int count = 0;

  int help = 0;
  int verbose = 0;
  int list_devices = 0,
      list_long = 0;
  int error = 0;
  int size = 0;
  char * text = NULL;
  const char * db_file = NULL,
             * file_name = NULL,
             * devices_filter[] = {NULL,NULL};
  int write_db_file = 0;
  int add_device = 0;
  int erase_device = 0;
  int list_pos = -1;
  int dump_json = 0;
  int show_path = 0;
  char *conf_name = NULL;		/* Configuration path to use */
  ucmm_scope scope = ucmm_user;		/* Scope of instalation */
  openiccConfig_s * config = NULL;
  int devices_n = 0, devices_new_n = 0;
  const char * device_classes_[] = {NULL, NULL};
  const char ** device_classes = 0;
  int device_classes_n = 0;

  openiccConfig_s * config_new = NULL;
  char * json_new = NULL;
  const char * d = NULL,
             * old_d = NULL,
             * device_class = NULL;
  int pos = -1;
  char * json;
  int i,j, n = 0,
      flags = OPENICC_CONFIGS_SKIP_HEADER | OPENICC_CONFIGS_SKIP_FOOTER;

#ifdef USE_GETTEXT
  setlocale(LC_ALL,"");
#endif
  openiccInit();

  openiccUi_s * ui = openiccUi_New( argc, argv );
  ui->app_type = "tool";
  oiUiFill( ui, "oiDv", "openicc-device", _("OpenICC devices"), "openicc-logo",
            _("Manipulation of OpenICC color management data base device entries.") );

  openiccOptions_s * options = ui->opts;

  /* declare some option choices */
  openiccOptionChoice_s b_choices[] = {{"DB-file-name.json", _("DB File"), _("File Name of device JSON Data Base"), ""},
                                       {"","","",""}};
  openiccOptionChoice_s c_choices[] = {{"monitor", _("Monitor"), _("Monitor"), ""},
                                       {"printer", _("Printer"), _("Printer"), ""},
                                       {"camera", _("Camera"), _("Camera"), ""},
                                       {"scanner", _("Scanner"), _("Scanner"), ""},
                                       {"","","",""}};
  openiccOptionChoice_s f_choices[] = {{"device-file-name.json", _("Device File"), _("File Name of Device JSON"), ""},
                                       {"","","",""}};


  /* declare options - the core information; use previously declared choices */
  openiccOption_s oarray[] = {
  /* type,   flags, o,  option,          key,  name,             description,         help, value_name,    value_type,        values, variable_type, result */
    {"oiwi", 0,    'a', "add",           NULL, _("add"),         _("Add Device to DB"),NULL,NULL, openiccOPTIONTYPE_NONE,     {}, openiccINT,{.i=&add_device} },
    {"oiwi", 0,    'b', "db-file",       NULL, _("db-file"),     _("DB File Name"),   NULL, _("FILENAME"), openiccOPTIONTYPE_CHOICE, {.choices.list = openiccMemDup(b_choices,sizeof(b_choices))}, openiccSTRING,{.s=&db_file} },
    {"oiwi", 0,    'c', "device-class",  NULL, _("device-class"),_("Device Class"),   NULL, "monitor|printer|camera|scanner", openiccOPTIONTYPE_CHOICE, {.choices.list = openiccMemDup( c_choices, sizeof(c_choices) )}, openiccSTRING, {.s = &device_class} },
    {"oiwi", 0,    'd', "device",        NULL, _("device"),      _("Device position"),NULL, _("NUMBER"), openiccOPTIONTYPE_DOUBLE, {.dbl.tick=1,.dbl.start=0,.dbl.end=10000}, openiccINT,{.i=&list_pos} },
    {"oiwi", 0,    'e', "erase-device",  NULL, _("erase-device"),_("Erase Devices"),  NULL, NULL, openiccOPTIONTYPE_NONE,     {},      openiccINT,   {.i=&erase_device} },
    {"oiwi", 0,    'f', "file-name",     NULL, _("file-name"),   _("File Name"),      NULL, _("FILENAME"), openiccOPTIONTYPE_CHOICE, {.choices.list = openiccMemDup(f_choices,sizeof(f_choices))}, openiccSTRING,{.s=&file_name} },
    {"oiwi", 0,    'h', "help",          NULL, _("help"),        _("Help"),           NULL, NULL, openiccOPTIONTYPE_NONE,     {},      openiccINT,   {.i=&help} },
    {"oiwi", 0,    'j', "dump-json",     NULL, _("dump-json"),   _("Dump JSON"),      NULL, NULL, openiccOPTIONTYPE_NONE,     {},      openiccINT,   {.i=&dump_json} },
    {"oiwi", 0,    'l', "list-devices",  NULL, _("list-devices"),_("List Devices"),   NULL, NULL, openiccOPTIONTYPE_NONE,     {},     openiccINT,    {.i=&list_devices} },
    {"oiwi", 0,    'n', "long",          NULL, _("long"),        _("List all key/values pairs"),  NULL, NULL, openiccOPTIONTYPE_NONE,     {},      openiccINT,   {.i=&list_long} },
    {"oiwi", 0,    'p', "show-path",     NULL, _("show-path"),   _("Show Path"),      NULL, NULL, openiccOPTIONTYPE_NONE,     {},      openiccINT,   {.i=&show_path} },
    {"oiwi", 0,    's', "scope",         NULL, _("scope"),       _("System"),         NULL, NULL, openiccOPTIONTYPE_NONE,     {},      openiccINT,   {.i=(int*)&scope} },
    {"oiwi", 0,    'v', "verbose",       NULL, _("verbose"),     _("verbose"),        NULL, NULL, openiccOPTIONTYPE_NONE,     {},      openiccINT,   {.i=&verbose} },
    {"oiwi", 0,    'w', "write",         NULL, _("write"),       _("Write DB File"),  NULL, NULL, openiccOPTIONTYPE_NONE,     {},      openiccINT,   {.i=&write_db_file} },
    {"",0,0,0,0,0,0,0, NULL, 0,{},0,{}}
  };
  /* copy in */
  options->array = openiccMemDup( oarray, sizeof(oarray) );

  /* declare option groups, for better syntax checking and UI groups */
  openiccOptionGroup_s groups[] = {
  /* type,   flags, name,              description,                          help, mandatory, optional, detail */
    {"oiwg", 0,     _("List Devices"), _("Print the Devices in the DB"),     NULL, "l",       "djnbv",  "ldjn" },
    {"oiwg", 0,     _("Add Device"),   _("Add a Devices to the DB"),         NULL, "af",      "bv",     "af" },
    {"oiwg", 0,     _("Erase Device"), _("Erase a Devices from the DB"),     NULL, "ed",      "bv",     "ed" },
    {"oiwg", 0,     _("Show DB Path"), _("Show Filepath to the DB"),         NULL, "p",       "sv",     "ps" },
    {"oiwg", 0,     _("Misc"),         _("General options"),                 NULL, "",        "",       "bvh" },
    {"",0,0,0,0,0,0,0}
  };
  /* copy in */
  options->groups = openiccMemDup( groups, sizeof(groups));

  openiccOPTIONSTATE_e state = openiccOptions_Parse( options );
  /* ... and report detected errors */
  if(state != openiccOPTION_NONE)
  {
    fputs( _("... try with --help|-h option for usage text. give up"), stderr );
    fputs( "\n", stderr );
    exit(1);
  }
  if(help)
  {
    openiccOptions_PrintHelp( options, ui, verbose, "%s %s %s", argv[0],
                              OPENICC_VERSION_NAME,
                              _("is a color management data base tool"));
    exit(0);
  }

  if(erase_device && list_pos == -1)
  {
    openiccOptions_PrintHelp( options, ui, verbose, "%s %s %s", argv[0],
                              OPENICC_VERSION_NAME,
                              _("is a color management data base tool"));
                        exit (0);
  }

  if(!db_file)
  {
    char *config_file = OPENICC_DB_PREFIX "/" OPENICC_DB;
    /* Locate the directories where the config file is, */
    /* and where we should copy the profile to. */
    {
      int npaths;
      xdg_error er;
      char **paths;

      if ((npaths = xdg_bds(&er, &paths, xdg_conf, xdg_write, 
                            (scope == ucmm_local_system) ? xdg_local : xdg_user,
                            config_file)) == 0)
      {
        return ucmm_open_config;
      }
      if ((conf_name = strdup(paths[0])) == NULL)
      {
        xdg_free(paths, npaths);
        return ucmm_resource;
      }
      if(show_path)
      {
        if(verbose)
          fprintf(stderr, "%s\n", _("Paths:"));
        for(i=0; i < npaths; ++i)
          printf("%s\n", paths[i]);
      }

      xdg_free(paths, npaths);
    }

    db_file = conf_name;
  }

  if(!db_file)
  {
    DBG( 0, "%s at \"%s\"", _("unable to open data base"), db_file);
    exit(0);
  } else
    fprintf( stderr, "using DB: %s\n", db_file);
 
  {
    /* read JSON input file */
    text = openiccOpenFile( db_file, &size );

    /* parse JSON */
    config = openiccConfig_FromMem( text );
    if(text) { free(text); text = NULL; }
    openiccConfig_SetInfo ( config, db_file );

    if(device_class)
    {
      device_classes_[0] = device_class;
      device_classes = device_classes_;
      device_classes_n = 1;
    } else
      device_classes = openiccConfigGetDeviceClasses( device_classes,
                                                  &device_classes_n );

    devices_n = openiccConfig_DevicesCount(config, device_classes);
    DBG( config, "Found %d devices.", devices_n );
  }

  if(add_device)
  {
    if(file_name)
      text = openiccOpenFile( file_name, &size );
    else
      error = openiccReadFileSToMem( stdin, &text, &size );

    /* parse JSON */
    config_new = openiccConfig_FromMem( text );
    if(text) { free(text); text = NULL; }
    openiccConfig_SetInfo ( config_new, file_name );
    devices_new_n = openiccConfig_DevicesCount(config_new, NULL);
    DBG( config_new, "Found %d devices.", devices_new_n );
    if(devices_new_n)
      list_devices = 1;
  }

  if(erase_device && devices_n)
  {
    list_devices = 1;
  }

  if(dump_json && devices_n)
  {
    list_devices = 1;
  }


  if(list_devices)
  {
    char            ** keys = 0;
    char            ** values = 0;

    n = 0;
    d = 0;
    old_d = 0;

    if(dump_json)
    {
      pos = -1;

      n = openiccConfig_DevicesCount(config, NULL);

      for(j = 0; j < device_classes_n; ++j)
      {
        devices_filter[0] = device_classes[j];
        devices_n = openiccConfig_DevicesCount(config, devices_filter);

        for(i = 0; i < devices_n; ++i)
        {
          ++pos;

          if(list_pos != -1 && ((!erase_device && pos != list_pos) ||
                                (erase_device && pos == list_pos)))
            continue;

          d = openiccConfig_DeviceGetJSON( config, devices_filter, i,
                                           flags, old_d, &json, malloc,free );

          if(d)
          {
            if(!old_d)
              oyjlStringAdd( &json_new, 0,0, OPENICC_DEVICE_JSON_HEADER, d );
            STRING_ADD( json_new, json );
            old_d = d;
          }
          if(json) { free(json); json = NULL; }
        }

        count = openiccConfig_DevicesCount(config_new, devices_filter);
        for(i = 0; i < count; ++i)
        {
          ++pos;

          d = openiccConfig_DeviceGetJSON( config_new, devices_filter, i,
                                           flags, old_d, &json, malloc,free );

          if(d)
          {
            if(!old_d)
              oyjlStringAdd( &json_new, 0,0, OPENICC_DEVICE_JSON_HEADER, d );
            STRING_ADD( json_new, json );
            old_d = d;
          }

          if(json) { free(json); json = NULL; }
          if(json_new)
            list_devices = 1;
        }
      }
 
      if(json_new)
      {
        STRING_ADD( json_new, "\n" OPENICC_DEVICE_JSON_FOOTER );
        printf( "%s", json_new );
      }

      if(write_db_file)
      {
        if(device_class)
        {
          WARNc_S("Can not write file %s  with single device class \"%s\"",
                    db_file, device_class );
          exit(1);
        }

        if(json_new)
        {
          int len = strlen(json_new) + 1;
          int s = openiccWriteFile( db_file, json_new, len );
          if(s != len)
            WARNc_S("Could not write file %s  %u!=%u", db_file, s, len );
        } else
          remove(db_file);
      }

    } else
    {
      /* print all found key/value pairs */
      for(i = 0; i < devices_n; ++i)
      {
        char * manufacturer = 0,
             * model = 0,
             * prefix = 0;
        const char * check_key;
        if(list_pos != -1 && ((!erase_device && i != list_pos) ||
                              (erase_device && pos == list_pos)))
          continue;

        d = openiccConfig_DeviceGet( config, device_classes, i,
                                     &keys, &values, malloc,free );

        if(i && list_long)
          fprintf( stderr,"\n");

        n = 0; if(keys) while(keys[n]) ++n;
        if(verbose)
        fprintf( stderr, "[%d] device class:\"%s\" with %d keys/values pairs\n",
                 i, d, n );
        for( j = 0; j < n; ++j )
        {
          check_key = keys[j];
          if(strcmp(check_key, "prefix") == 0)
            prefix = values[j];
        }

        for( j = 0; j < n; ++j )
        {
          if(!list_long)
          {
            check_key = keys[j];
            if(prefix && strlen(prefix) < strlen(check_key) &&
               memcmp(prefix, check_key, strlen(prefix)) == 0)
              check_key += strlen(prefix);
            if(strcmp(check_key, "manufacturer") == 0)
              manufacturer = values[j];
            else if(strcmp(check_key, "model") == 0)
              model = values[j];
            else if(strcmp(check_key, "prefix") == 0)
              prefix = values[j];
            else
              free(values[j]);
          }
          else
          {
            fprintf(stdout, "%s:\"%s\"\n", keys[j], values[j]);
            free(values[j]);
          }
          free(keys[j]);
        }
        free(keys); free(values);
        if(!list_long)
          fprintf(stdout, "%d : \"%s\" - \"%s\"\n", i, manufacturer, model);
      }
    }
  }

  openiccConfig_Release( &config );

  if(conf_name)
    free(conf_name);

  return error;
}

