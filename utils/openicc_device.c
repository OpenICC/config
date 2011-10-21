/*  @file openicc_device.c
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
 *  @since    2011/10/20
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "openicc_version.h"
#include "openicc_config.h"
#include "openicc_macros.h"


void printfHelp(int argc, char ** argv)
{
  fprintf( stderr, "\n");
  fprintf( stderr, "%s %s\n",   argv[0],
                                _("is a color management data base tool"));
  fprintf( stderr, "  v%s\n",
                  OPENICC_VERSION_NAME );
  fprintf( stderr, "\n");
  fprintf( stderr, "%s\n",                 _("Usage"));
  fprintf( stderr, "  %s\n",               _("List devices:"));
  fprintf( stderr, "      %s -l [-v] [-f FILE_NAME] [--long]\n",        argv[0]);
  fprintf( stderr, "                        [-p NUMBER|--pos NUMBER] [-j]\n");
  fprintf( stderr, "        -f FILE_NAME    %s\n", _("specify DB file"));
  fprintf( stderr, "        --long          %s\n", _("listing all key/values pairs"));
  fprintf( stderr, "        -p NUMBER | --pos NUMBER\n"
                   "                        %s\n", _("select device by position"));
  fprintf( stderr, "        -j | --json     %s\n", _("dump raw JSON"));
  fprintf( stderr, "\n");
  fprintf( stderr, "  %s\n",               _("Print a help text:"));
  fprintf( stderr, "      %s -h\n",        argv[0]);
  fprintf( stderr, "\n");
  fprintf( stderr, "  %s\n",               _("General options:"));
  fprintf( stderr, "        -v              %s\n", _("verbose"));
  fprintf( stderr, "\n");
  fprintf( stderr, "\n");
}


int main(int argc, char ** argv)
{
  int count = 0;

  int verbose = 0;
  int result = 0;
  int list_devices = 0,
      list_long = 0;
  int error;
  FILE * fp = 0;
  const char * home,
             * file_name = NULL,
             * devices_filter = NULL;
  int list_pos = -1;
  int dump_json = 0;
  char * t = NULL;

#ifdef USE_GETTEXT
  setlocale(LC_ALL,"");
#endif

  if(argc >= 2)
  {
    int pos = 1, i;
    char *wrong_arg = 0;
    while(pos < argc)
    {
      switch(argv[pos][0])
      {
        case '-':
            for(i = 1; pos < argc && i < strlen(argv[pos]); ++i)
            switch (argv[pos][i])
            {
              case 'f': OY_PARSE_STRING_ARG(file_name); break;
              case 'j': dump_json = 1; break;
              case 'l': list_devices = 1; break;
              case 'p': OY_PARSE_INT_ARG(list_pos); break;
              case 'v': ++verbose; ++openicc_debug; break;
              case 'h':
              case '-':
                        if(i == 1)
                        {
                             if(OY_IS_ARG("verbose"))
                        { ++openicc_debug; ++verbose; i=100; break; }
                        else if(OY_IS_ARG("json"))
                        { dump_json = 1; i=100; break; }
                        else if(OY_IS_ARG("long"))
                        { list_long = 1; i=100; break; }
                        else if(OY_IS_ARG("pos"))
                        { OY_PARSE_INT_ARG2( list_pos, "pos" ); break; }
                        }
              default:
                        printfHelp(argc, argv);
                        exit (0);
                        break;
            }
            break;
        default:
                        printfHelp(argc, argv);
                        exit (0);
                        break;
      }
      if( wrong_arg )
      {
       WARN( 0, "%s %s", "wrong argument to option:", wrong_arg);
       printfHelp(argc, argv);
       exit(1);
      }
      ++pos;
    }
  } else
  {
                        printfHelp(argc, argv);
                        exit (0);
  }


  if(file_name)
    fp = fopen( file_name, "r" );
  else
  {
    home = getenv("HOME");
    if(!home)
    {
      WARN( 0, "%s", _("unable to obtain home directory name"));
      exit(1);
    }

    openiccStringAddPrintf_( &t, "%s%s%s", home,
                             (home[strlen(home)] == '/')?"":"/",
                             OPENICC_USER_DEVICE_DB_NAME );
    fp = fopen( t, "r" );
    if(t)
      free(t);
  }

  if(!fp)
  {
    DBG( 0, "%s at \"%s\"", _("unable to open data base"), t);
    exit(0);
  }

  if(list_devices)
  {
    OpeniccConfigs_s * configs, * config;
    char * text = 0;
    size_t size = 0;
    char            ** keys = 0;
    char            ** values = 0;
    int i,j, n = 0, devices_n, flags;
    char * json, * device_class;

    /* read JSON input file */
    fseek(fp,0L,SEEK_END);
    size = ftell (fp);
    rewind(fp);
    if(size)
    {
      text = malloc(size+1);
      if(text)
        fread(text, sizeof(char), size, fp);
      text[size] = '\000';
    }
 

    /* parse JSON */
    configs = openiccConfigs_FromMem( text );
    openiccConfigs_SetInfo ( configs, file_name );
    devices_n = openiccConfigs_Count(configs, NULL);
    DBG( configs, "Found %d devices.", devices_n );

  
    if(dump_json)
    {
      int pos = -1;

      devices_n = openiccConfigs_Count(configs, devices_filter);
      count = devices_n;
      if(list_pos != -1)
        count = list_pos + 1;
      fprintf(stderr, "Found %d devices.\n", devices_n );
      for(i = 0; i < devices_n; ++i)
      {
        if(list_pos != -1 && i != list_pos)
          continue;
        ++pos;
        flags = 0;
        if(pos != 0) /* not the first */
          flags |= OPENICC_CONFIGS_SKIP_HEADER;
        if(i != count - 1) /* not the last */
          flags |= OPENICC_CONFIGS_SKIP_FOOTER;
        /* end the current JSON array field and open the next one */
        if(pos > 0 && pos < count) 
          printf("            },\n            {\n");

        json = openiccConfigs_DeviceGetJSON( configs, devices_filter, i,
                                             flags, malloc );

        printf( "%s", json );
        free(json);
      }
    } else
    {
      /* print all found key/value pairs */
      for(i = 0; i < devices_n; ++i)
      {
        const char * d;
        char * manufacturer = 0,
             * model = 0,
             * prefix = 0;

        if(list_pos != -1 && i != list_pos)
          continue;

        d = openiccConfigs_DeviceGet( configs, NULL, i,
                                                   &keys, &values, malloc );

        if(i && list_long)
          fprintf( stderr,"\n");

        n = 0; if(keys) while(keys[n]) ++n;
        if(verbose)
        fprintf( stderr, "[%d] device class:\"%s\" with %d keys/values pairs\n",
                 i, d, n );
        for( j = 0; j < n; ++j )
        {
          if(!list_long)
          {
            const char * check_key = keys[j];
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
          fprintf(stdout, "%s\t%s\n", manufacturer, model);
      }
    }


    openiccConfigs_Release( &configs );

  }

  return result;
}


