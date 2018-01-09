/*  @file openicc_json.c
 *
 *  libOpenICC - JSON helper tool
 *
 *  @par Copyright:
 *            2018 (C) Kai-Uwe Behrmann
 *
 *  @brief    OpenICC Colour Management configuration helpers
 *  @internal
 *  @author   Kai-Uwe Behrmann <ku.b@gmx.de>
 *  @par License:
 *            MIT <http://www.opensource.org/licenses/mit-license.php>
 *  @since    2018/01/06
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "openicc_conf.h"
#include "openicc_version.h"
#include "openicc_config.h"
#include "openicc_macros.h"
#include "openicc_config_internal.h"

void printfHelp(int argc OI_UNUSED, char ** argv)
{
  fprintf( stderr, "\n");
  fprintf( stderr, "%s %s\n",   argv[0],
                                _("is a JSON helper tool"));
  fprintf( stderr, "  v%s\n",
                  OPENICC_VERSION_NAME );
  fprintf( stderr, "\n");
  fprintf( stderr, "%s\n",                 _("Usage"));
  fprintf( stderr, "  %s\n",               _("Convert JSON to gettext ready C strings:"));
  fprintf( stderr, "      %s -e [-v] -i FILE_NAME -o FILE_NAME -f '_(%%s)\\n' -k name,description,help\n",        argv[0]);
  fprintf( stderr, "        -f              %s\n", _("output format string"));
  fprintf( stderr, "\n");
  fprintf( stderr, "  %s\n",               _("Add gettext translated keys to JSON:"));
  fprintf( stderr, "      %s -a [-v] -i FILE_NAME -o FILE_NAME -k name,description,help -d TEXTDOMAIN -p LOCALEDIR -l de,es -r root-key\n",        argv[0]);
  fprintf( stderr, "        -d TEXTDOMAIN   %s\n", _("text domain of your project"));
  fprintf( stderr, "        -l locales      %s\n", _("locales in a comma separated list"));
  fprintf( stderr, "        -p LOCALEDIR    %s\n", _("locale directory containing the your-locale/LC_MESSAGES/your-textdomain.mo gettext translations"));
  fprintf( stderr, "\n");
  fprintf( stderr, "  %s\n",               _("Print a help text:"));
  fprintf( stderr, "      %s -h\n",        argv[0]);
  fprintf( stderr, "\n");
  fprintf( stderr, "  %s\n",               _("General options:"));
  fprintf( stderr, "        -i FILE_NAME    %s\n", _("specify JSON file"));
  fprintf( stderr, "        -o              %s\n", _("output"));
  fprintf( stderr, "        -k STRING_LIST  %s\n", _("to be used key names in a comma separated list"));
  fprintf( stderr, "        -v              %s\n", _("verbose"));
  fprintf( stderr, "\n");
  fprintf( stderr, "\n");
}


int main(int argc, char ** argv)
{
  int verbose = 0;
  int error = 0;
  int add = 0;
  int extract = 0;
  int size = 0;
  const char * output = NULL,
             * file_name = NULL,
             * format = NULL,
             * key_list = NULL,
             * lang_list = NULL,
             * localedir = NULL,
             * ctextdomain = "OpenICC";
  char * json = NULL;
  oyjl_val root = NULL,v;
  char * text = NULL;

#ifdef USE_GETTEXT
  setlocale(LC_ALL,"");
#endif
  openiccInit();

  if(argc >= 2)
  {
    int pos = 1;
    unsigned i;
    char *wrong_arg = 0;
    while(pos < argc)
    {
      switch(argv[pos][0])
      {
        case '-':
            for(i = 1; pos < argc && i < strlen(argv[pos]); ++i)
            switch (argv[pos][i])
            {
              case 'a': add = 1; break;
              case 'd': OY_PARSE_STRING_ARG(ctextdomain); break;
              case 'e': extract = 1; break;
              case 'f': OY_PARSE_STRING_ARG(format); break;
              case 'i': OY_PARSE_STRING_ARG(file_name); break;
              case 'k': OY_PARSE_STRING_ARG(key_list); break;
              case 'l': OY_PARSE_STRING_ARG(lang_list); break;
              case 'o': OY_PARSE_STRING_ARG(output); break;
              case 'p': OY_PARSE_STRING_ARG(localedir); break;
              case 'v': ++verbose; ++*openicc_debug; break;
              case 'h':
              case '-':
                        if(i == 1)
                        {
                             if(OY_IS_ARG("verbose"))
                        { ++*openicc_debug; ++verbose; i=100; break; }
                        } OI_FALLTHROUGH
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

  if(verbose)
    fprintf(stderr, "i18n test:\t\"%s\" %s\n", _("Usage"), textdomain(NULL) );

  if(file_name)
    json = openiccOpenFile( file_name, &size );

  if(json)
  {
    char * text = malloc(256);

    text[0] = 0;

    /* parse json ... */
    root = oyjl_tree_parse( json, text, 256 );
    if(text[0])
    {
      fprintf( stderr, "ERROR: %s\n%s\n", file_name, text );
    }
  }

  if(root)
  {
    char ** paths = NULL;
    int count = 0, i;

    oyjl_tree_to_paths( root, 1000000, NULL, 0, &paths );
    if(verbose)
      fprintf(stderr, "processed:\t\"%s\"\n", file_name);
    while(paths && paths[count]) ++count;

    if(extract)
    {
      int n = 0;
      char ** list = oyjl_string_split( key_list, ',', &n, malloc );
      if(verbose)
        fprintf(stderr, "use:\t%d keys\n", n);

      for(i = 0; i < count; ++i)
      {
        char * path = paths[i];
        int j;
        for(j = 0; j < n; ++j)
        {
          char * key = list[j];
          if(oyjl_path_match(path, key, OYJL_PATH_MATCH_LAST_ITEMS ))
          {
            const char * t = NULL;
            v = oyjl_tree_get_value( root, 0, path );
            if(v)
              t = OYJL_GET_STRING(v);
            if(verbose)
              fprintf(stderr, "found:\t%d key: %s value: \"%s\"\n", i, path, t?t:"----" );
            if(t)
            {
              if(format)
                oyjl_string_add( &text, malloc, free, format, t );
              else
                oyjl_string_add( &text, malloc, free, "// %s:%s\n{ const char * t = _(\"%s\"); }\n\n",
                                 file_name, path, t );
            }
          }
        }
      }

    } else if(add)
    {
      int ln = 0, n = 0;
      char ** langs = oyjl_string_split( lang_list, ',', &ln, malloc );
      char * var = NULL;
      const char * openicc_domain_path = OPENICC_LOCALEDIR;
      char ** list = oyjl_string_split( key_list, ',', &n, malloc );
      char * dir;

      if(verbose)
        fprintf(stderr, "use:\t%d langs - %s\n", ln, dgettext( ctextdomain, "Rendering Intent" ));
      if(verbose)
        fprintf(stderr, "use:\t%d keys\n", n);

      if(localedir)
      {
        if(!openiccIsDirFull_ (localedir))
        {
          fprintf(stderr, "ERROR: Can not find absolute path:\t%s\n", localedir);
          exit(1);
        }
        openicc_domain_path = localedir;
      }
      if(!openicc_domain_path && getenv("OI_LOCALEDIR") && strlen(getenv("OI_LOCALEDIR")))
        openicc_domain_path = strdup(getenv("OI_LOCALEDIR"));

      var = textdomain( ctextdomain );
      dir = bindtextdomain( ctextdomain, openicc_domain_path );

      if(*openicc_debug)
        fprintf(stdout, "%s = bindtextdomain() to \"%s\"\ntextdomain: %s == %s\n", dir, openicc_domain_path, ctextdomain, var );
      var = NULL;

      STRING_ADD( var, "NLSPATH=");
      STRING_ADD( var, openicc_domain_path);
      putenv(var); /* Solaris */

      for(i = 0; i < ln; ++i)
      {
        char * lang = langs[i];
        int j;

        setlocale( LC_MESSAGES, lang );
        if(*openicc_debug)
          fprintf(stdout, "setlocale() %s\n", setlocale( LC_MESSAGES, NULL ));

        for(j = 0; j < count; ++j)
        {
          char * path = paths[j];

          int k;
          for(k = 0; k < n; ++k)
          {
            char * key = list[k];

            if(oyjl_path_match(path, key, OYJL_PATH_MATCH_LAST_ITEMS ))
            {
              const char * t = NULL,
                         * tr = NULL;
              v = oyjl_tree_get_value( root, 0, path );
              if(v)
                t = OYJL_GET_STRING(v);
              if(t)
                tr = dgettext( ctextdomain, t );
              if(verbose)
                fprintf(stderr, "found:\t key: %s value: \"%s\"\n", path, tr?tr:"----" );
              if(t != tr)
              {
                char * new_path = NULL;
                oyjl_string_add( &new_path, malloc, free, "%s.%s", path, lang );
                v = oyjl_tree_get_value( root, OYJL_CREATE_NEW, new_path );
                oyjl_value_set_string( v, tr );
                oyjl_string_add( &text, malloc, free, "%s\n", tr );
                free(new_path);
              }
            }
          }
        }
      }

      if(verbose)
        fprintf(stderr, "found i18n:\n%s", text );
      if(text) free(text);
      text = NULL;
      i = 0;
      oyjl_tree_to_json( root, &i, &text );

    } else
      for(i = 0; i < count; ++i)
        fprintf(stdout,"%s\n", paths[i]);

    oyjl_string_list_release( &paths, count, free );

    if(text)
    {
      if(!output || strcmp(output,"-") == 0)
        fputs( text, stdout );
      else
        openiccWriteFile( output, text, strlen(text) );
    }
  }

  return error;
}

