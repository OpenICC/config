/** @file test.c
 *
 *  Oyranos is an open source Color Management System
 *
 *  Copyright (C) 2004-2019  Kai-Uwe Behrmann
 *
 *  @brief    Oyranos test suite
 *  @internal
 *  @author   Kai-Uwe Behrmann <ku.b@gmx.de>
 *  @par License:\n
 *  new BSD <http://www.opensource.org/licenses/BSD-3-Clause>
 *  @since    2008/12/04
 */

#define TESTS_RUN \
  TEST_RUN( testVersion, "Version matching", 1 ); \
  TEST_RUN( testI18N, "Internationalisation", 1 ); \
  TEST_RUN( testString, "String handling", 1 ); \
  TEST_RUN( testArgs, "Options handling", 1 ); \
  TEST_RUN( testTree, "Tree handling", 1 ); \
  TEST_RUN( testIO, "File handling", 1 );

void oyjlLibRelease();
#define OYJL_TEST_MAIN_SETUP  printf("\n    OyjlCore Test Program\n");
#define OYJL_TEST_MAIN_FINISH printf("\n    OyjlCore Test Program finished\n\n"); oyjlLibRelease();
#define OYJL_TEST_NAME "test-core"
#include "oyjl_test_main.h"
#include "oyjl.h"
#include "oyjl_version.h"
#ifdef OYJL_HAVE_LOCALE_H
#include <locale.h>
#endif
#include "oyjl_i18n.h"


/* --- actual tests --- */

oyjlTESTRESULT_e testVersion()
{
  oyjlTESTRESULT_e result = oyjlTESTRESULT_UNKNOWN;

  fprintf(stdout, "\n" );
  fprintf(zout, "compiled version:     %d\n", OYJL_VERSION );
  fprintf(zout, " runtime version:     %d\n", oyjlVersion(0) );

  if(OYJL_VERSION == oyjlVersion(0))
    result = oyjlTESTRESULT_SUCCESS;
  else
    result = oyjlTESTRESULT_FAIL;

  return result;
}

extern oyjlMessage_f oyjlMessage_p;
oyjlTESTRESULT_e testI18N()
{
  const char * clang;
  oyjlTESTRESULT_e result = oyjlTESTRESULT_UNKNOWN;

  fprintf(stdout, "\n" );

  clang = setlocale(LC_ALL, NULL);
  if((clang && (strcmp(clang, "C") == 0)) || !clang)
  { PRINT_SUB( oyjlTESTRESULT_SUCCESS, 
    "Language uninitialised good \"C\"              " );
  } else
  { PRINT_SUB( oyjlTESTRESULT_FAIL, 
    "Language uninitialised failed                 " );
  }

  clang = getenv("LANG");
  if(!clang || strcmp(clang,"C") == 0)
  {
    char * tmp = oyjlStringCopy("LANG=de_DE.UTF-8", 0);
    putenv(tmp);
    clang = getenv("LANG");
    if(!clang || strcmp(clang,"de_DE.UTF-8") != 0)
      fprintf( stderr, "Could not modify LANG environment variable. Test will not be useful.\n" );
  }

  setlocale(LC_ALL,"");

  if(clang && (strstr(clang, "de_DE") != 0))
  { PRINT_SUB( oyjlTESTRESULT_SUCCESS, 
    "Language initialised good %s            ", clang?clang:"---" );
  } else
  { PRINT_SUB( oyjlTESTRESULT_XFAIL, 
    "Language initialised failed %s          ", clang?clang:"---" );
  }

  setlocale(LC_ALL,"de_DE.UTF8");
  int use_gettext = 0;
#ifdef OYJL_USE_GETTEXT
  use_gettext = 1;
#endif
  int debug = 0;
  oyjlInitLanguageDebug( "Oyjl", "OYJL_DEBUG", &debug, use_gettext, "OYJL_LOCALEDIR", OYJL_LOCALEDIR, OYJL_DOMAIN, oyjlMessage_p );

  const char * lang = setlocale(LC_ALL, NULL);
  if(lang && (strcmp(lang, "C") != 0))
  { PRINT_SUB( oyjlTESTRESULT_SUCCESS, 
    "setlocale() initialised good %s            ", lang );
  } else
  { PRINT_SUB( oyjlTESTRESULT_XFAIL, 
    "setlocale() initialised failed %s          ", lang );
  }

  const char * text = _("Example");
  if(strcmp(text,"Beispiel") == 0)
  { PRINT_SUB( oyjlTESTRESULT_SUCCESS, 
    "dgettext() good \"%s\"                      ", text );
  } else
  { PRINT_SUB( oyjlTESTRESULT_XFAIL, 
    "dgettext() failed \"%s\"                    ", text );
  }

  setlocale(LC_ALL,"");


  return result;
}

#define TEST_DOMAIN "org/freedesktop/openicc/tests"
#define TEST_DOMAIN2 "org2/freedesktop2/openicc2/tests2"
#define TEST_KEY "/test_key"

static void replaceCb(const char * text OYJL_UNUSED, const char * start, const char * end, const char * search, const char ** replace, void * data)
{
  if(start < end)
  {
    const char * word = start;
    int * test = (int*) data;
    while(word && (word = strstr(word+1,"nd")) != NULL && word < end)
      ++test[0];
    word = start;
    while(word && (word = strstr(word+1,"or")) != NULL && word < end)
      --test[0];
    if(test[0] < 0) test[0] = 0;
    word = start;

    if( test[0] )
      *replace = search;
    else
      *replace = "\\/";
  }
}

oyjlTESTRESULT_e testString ()
{
  oyjlTESTRESULT_e result = oyjlTESTRESULT_UNKNOWN;

  fprintf(stdout, "\n" );

  int i;

  const char * test;
  

  test = TEST_DOMAIN "/display.oydi/display_name";
  fprintf(zout, "test \"%s\" %d\n", test, (int)strlen(test));
  char * test_out = oyjlStringCopy(test,malloc);
  int n = oyjlStringReplace( &test_out, TEST_DOMAIN, TEST_DOMAIN2, 0,0 );
  if(verbose)
    fprintf(zout, "test %s \"%s\"\n", test, test_out);
  if( strstr(test_out, TEST_DOMAIN2 ) != NULL &&
      strlen(test_out) == 59 )
  { PRINT_SUB( oyjlTESTRESULT_SUCCESS,
    "oyjlStringReplace(start) %d 59 == %d                  ", n, (int)strlen(test_out) );
  } else
  { PRINT_SUB( oyjlTESTRESULT_FAIL,
    "oyjlStringReplace(start) %d 59 == %d                  ", n, (int)strlen(test_out) );
  }
  myDeAllocFunc(test_out);

  test_out = oyjlStringCopy(test,malloc);
  n = oyjlStringReplace( &test_out, "display.", "foo.", 0,0 );
  if(verbose)
    fprintf(zout, "test %s \"%s\"\n", test, test_out);
  if( strstr(test_out, "foo" ) != NULL &&
      strlen(test_out) == 51 &&
      n == 1 )
  { PRINT_SUB( oyjlTESTRESULT_SUCCESS,
    "oyjlStringReplace(middle) 51 == %d                   ", (int)strlen(test_out) );
  } else
  { PRINT_SUB( oyjlTESTRESULT_FAIL,
    "oyjlStringReplace(middle) 51 == %d                   ", (int)strlen(test_out) );
  }
  myDeAllocFunc(test_out);

  test_out = oyjlStringCopy(test,malloc);
  n = oyjlStringReplace( &test_out, "display_name", "bar", 0,0 );
  if(verbose)
    fprintf(zout, "test %s \"%s\"\n", test, test_out);
  if( strstr(test_out, "bar" ) != NULL &&
      strstr(test_out, "barbar" ) == NULL &&
      strlen(test_out) == 46 &&
      n == 1 )
  { PRINT_SUB( oyjlTESTRESULT_SUCCESS,
    "oyjlStringReplace(end)    46 == %d                   ", (int)strlen(test_out) );
  } else
  { PRINT_SUB( oyjlTESTRESULT_FAIL,
    "oyjlStringReplace(end)    46 == %d                   ", (int)strlen(test_out) );
  }
  if(verbose)
    fprintf( stderr, "%s \"display_name\"->\"bar\" %s\n", test, test_out );
  myDeAllocFunc(test_out);

  test_out = oyjlStringCopy("abc]",malloc);
  n = oyjlStringReplace( &test_out, "]", " ] ", 0,0 );
  if(verbose)
    fprintf(zout, "test %s \"%s\"\n", test, test_out);
  if( strstr(test_out, " ] " ) != NULL &&
      strstr(test_out, "c]" ) == NULL &&
      strlen(test_out) == 6 &&
      n == 1 )
  { PRINT_SUB( oyjlTESTRESULT_SUCCESS,
    "oyjlStringReplace(end)      6 == %d                   ", (int)strlen(test_out) );
  } else
  { PRINT_SUB( oyjlTESTRESULT_FAIL,
    "oyjlStringReplace(end)      6 == %d                   ", (int)strlen(test_out) );
  }
  if(verbose)
    fprintf( stderr, "\"%s\" \"]\"->\" ] \" \"%s\"\n", "abc]", test_out );
  myDeAllocFunc(test_out);

  char * compare;
  compare = test_out = oyjlStringCopy(test,malloc);
  n = oyjlStringReplace( &test_out, "not_inside", "bar", 0,0 );
  if(verbose)
    fprintf(zout, "test %s \"%s\"\n", test, test_out);
  if( compare && compare == test_out &&
      n == 0 &&
      strcmp(compare,test) == 0 )
  { PRINT_SUB( oyjlTESTRESULT_SUCCESS,
    "oyjlStringReplace(none)                              " );
  } else
  { PRINT_SUB( oyjlTESTRESULT_FAIL,
    "oyjlStringReplace(none)                              " );
  }
  myDeAllocFunc(test_out);

  test_out = oyjlStringCopy(test,malloc);
  n = oyjlStringReplace( &test_out, "display", "moni", 0,0 );
  if(verbose)
    fprintf(zout, "test %s \"%s\"\n", test, test_out);
  if( strlen(test) > strlen(test_out) &&
      strlen(test_out) == 49 &&
      n == 2 )
  { PRINT_SUB( oyjlTESTRESULT_SUCCESS,
    "oyjlStringReplace(shorter) 49 == %d                  ", (int)strlen(test_out) );
  } else
  { PRINT_SUB( oyjlTESTRESULT_FAIL,
    "oyjlStringReplace(shorter) 49 == %d                  ", (int)strlen(test_out) );
  }
  myDeAllocFunc(test_out);

  test_out = oyjlStringCopy(test,malloc);
  n = oyjlStringReplace( &test_out, "display", "monitorXYZ", 0,0 );
  if(verbose)
    fprintf(zout, "test %s \"%s\"\n", test, test_out);
  if( strlen(test) < strlen(test_out) &&
      strlen(test_out) == 61 &&
      n == 2 )
  { PRINT_SUB( oyjlTESTRESULT_SUCCESS,
    "oyjlStringReplace(longer) 61 == %d                   ", (int)strlen(test_out) );
  } else
  { PRINT_SUB( oyjlTESTRESULT_FAIL,
    "oyjlStringReplace(longer) 61 == %d                   ", (int)strlen(test_out) );
  }
  myDeAllocFunc(test_out);

  int list_n = 0;
  int *index = NULL;
  const char * text = "org/domain/eins.lib,org/domain/zwei.txt;org/domain/drei.lib?net/welt/vier.lib:net/welt/vier.txt$/net/welt/fuenf;/net/welt/fuenf";
  char ** list = oyjlStringSplit2( text, ";:,?$&§", &list_n, &index, myAllocFunc );

  if( list_n == 7 &&
      (index && text[index[0]] == ',') &&
      (index && text[index[1]] == ';') &&
      (index && text[index[2]] == '?') &&
      (index && text[index[3]] == ':') &&
      (index && text[index[4]] == '$') &&
      (index && text[index[5]] == ';')
    )
  { PRINT_SUB( oyjlTESTRESULT_SUCCESS,
    "oyjlStringSplit2()                                   " );
  } else
  { PRINT_SUB( oyjlTESTRESULT_FAIL,
    "oyjlStringSplit2()                                   " );
  }
  if(oy_test_last_result == oyjlTESTRESULT_FAIL || verbose)
  {
    fprintf( zout, " text: \"%s\"\n", text );
    for(i = 0; i < list_n; ++i)
      fprintf(zout, " list[%d] \"%s\" index in text of delimiter: %d  delimiter:\'%c\'\n", i, list[i], index ? index[i] : -1, index && index[i] ? text[index[i]] : '0' );
  }
  oyjlStringListRelease( &list, list_n, myDeAllocFunc );
  if(index) { myDeAllocFunc( index ); index = NULL; }

  list_n = 0;
  list = oyjlStringSplit( "org/domain/eins.lib;org/domain/zwei.txt;org/domain/drei.lib;net/welt/vier.lib;net/welt/vier.txt;/net/welt/fuenf;/net/welt/fuenf", ';', &list_n, myAllocFunc );

  oyjlStringListFreeDoubles( list, &list_n, myDeAllocFunc );
  if( list_n == 6 )
  { PRINT_SUB( oyjlTESTRESULT_SUCCESS,
    "oyjlStringListFreeDoubles()                          " );
  } else
  { PRINT_SUB( oyjlTESTRESULT_FAIL,
    "oyjlStringListFreeDoubles()                          " );
    for(i = 0; i < list_n; ++i)
      fprintf(zout, " list[%d] \"%s\"\n", i, list[i] );
  }
  oyjlStringListRelease( &list, list_n, myDeAllocFunc );

  list_n = 0;
  list = oyjlStringSplit( "a a b c\td\ne", 0, &list_n, myAllocFunc );
  if(list_n == 6)
  { PRINT_SUB( oyjlTESTRESULT_SUCCESS,
    "oyjlStringSplit(0)                                   " );
  } else
  { PRINT_SUB( oyjlTESTRESULT_FAIL,
    "oyjlStringSplit(0)                                   " );
  }
  if(oy_test_last_result == oyjlTESTRESULT_FAIL || verbose)
  {
    for(i = 0; i < list_n; ++i)
      fprintf(zout, " list[%d] \"%s\"\n", i, list[i] );
  }
  oyjlStringListRelease( &list, list_n, myDeAllocFunc );

  double d = 0.0;
  int error = oyjlStringToDouble( "0.2", &d );
  if( !error &&
      d == 0.2)
  { PRINT_SUB( oyjlTESTRESULT_SUCCESS,
    "oyjlStringToDouble(\"0.2\") = %g                      ", d );
  } else
  { PRINT_SUB( oyjlTESTRESULT_FAIL,
    "oyjlStringToDouble(\"0.2\") = %g  error = %d            ", d, error );
  }

  double * doubles = NULL;
  int count = 0;
  error = oyjlStringsToDoubles( "0.2 1 3.5", " ", &count, malloc, &doubles );
  if( !error &&
      doubles[0] == 0.2 &&
      doubles[1] == 1.0 &&
      doubles[2] == 3.5 &&
      count == 3)
  { PRINT_SUB( oyjlTESTRESULT_SUCCESS,
    "oyjlStringsToDoubles(\"0.2 1 3.5\") error = %d          ", error );
  } else
  { PRINT_SUB( oyjlTESTRESULT_FAIL,
    "oyjlStringsToDoubles(\"0.2 1 3.5\") error = %d          ", error );
  }
  if(doubles) { free(doubles); } doubles = NULL;

  error = oyjlStringsToDoubles( "0.2", " ", &count, malloc, &doubles );
  if( !error &&
      doubles[0] == 0.2 &&
      count == 1)
  { PRINT_SUB( oyjlTESTRESULT_SUCCESS,
    "oyjlStringsToDoubles(\"0.2\") error = %d                ", error );
  } else
  { PRINT_SUB( oyjlTESTRESULT_FAIL,
    "oyjlStringsToDoubles(\"0.2\") error = %d                ", error );
  }
  if(doubles) { free(doubles); } doubles = NULL;

  error = oyjlStringsToDoubles( "0.2;1;3.5", ";", &count, malloc, &doubles );
  if( !error &&
      doubles[0] == 0.2 &&
      doubles[1] == 1.0 &&
      doubles[2] == 3.5 &&
      count == 3)
  { PRINT_SUB( oyjlTESTRESULT_SUCCESS,
    "oyjlStringsToDoubles(\"0.2;1;3.5\") error = %d          ", error );
  } else
  { PRINT_SUB( oyjlTESTRESULT_FAIL,
    "oyjlStringsToDoubles(\"0.2;1;3.5\") error = %d          ", error );
  }
  if(doubles) { free(doubles); } doubles = NULL;

  error = oyjlStringsToDoubles( "0.2 ; 1; 3.5 cm", ";", &count, malloc, &doubles );
  if( doubles[0] == 0.2 &&
      doubles[1] == 1.0 &&
      doubles[2] == 3.5 &&
      count == 3)
  { PRINT_SUB( oyjlTESTRESULT_SUCCESS,
    "oyjlStringsToDoubles(\"0.2 ; 1; 3.5 cm\") error = %d   ", error );
  } else
  { PRINT_SUB( oyjlTESTRESULT_FAIL,
    "oyjlStringsToDoubles(\"0.2 ; 1; 3.5 cm\") error = %d   ", error );
  }
  if(doubles) { free(doubles); } doubles = NULL;

  error = oyjlStringsToDoubles( "0.2,1,3.5", ",", &count, malloc, &doubles );
  if( !error &&
      doubles[0] == 0.2 &&
      doubles[1] == 1.0 &&
      doubles[2] == 3.5 &&
      count == 3)
  { PRINT_SUB( oyjlTESTRESULT_SUCCESS,
    "oyjlStringsToDoubles(\"0.2,1,3.5\") error = %d          ", error );
  } else
  { PRINT_SUB( oyjlTESTRESULT_FAIL,
    "oyjlStringsToDoubles(\"0.2,1,3.5\") error = %d          ", error );
  }
  if(doubles) { free(doubles); } doubles = NULL;

  error = oyjlStringsToDoubles( "2\t1.2\t 27", 0, &count, malloc, &doubles );
  if( error == 0 &&
      doubles[0] == 2 &&
      doubles[1] == 1.2 &&
      doubles[2] == 27 &&
      count == 3)
  { PRINT_SUB( oyjlTESTRESULT_SUCCESS,
    "oyjlStringsToDoubles(\"2\\t1.2\\t 27\") error = %d ", error );
  } else
  { PRINT_SUB( oyjlTESTRESULT_FAIL,
    "oyjlStringsToDoubles(\"2\\t1.2\\t 27\") error = %d ", error );
  }
  if(verbose)
  {
    for(i = 0; i < count; ++i)
      fprintf( zout, "%d:%f ", i, doubles[i] );
    fprintf( zout, "\n" );
  }
  if(doubles) { free(doubles); } doubles = NULL;

  error = oyjlStringsToDoubles( "\n \t2\t 1.2\t 27\t\n", 0, &count, malloc, &doubles );
  if( error == 0 &&
      doubles[0] == 2 &&
      doubles[1] == 1.2 &&
      doubles[2] == 27 &&
      count == 3)
  { PRINT_SUB( oyjlTESTRESULT_SUCCESS,
    "oyjlStringsToDoubles(\"\\n \\t2\\t 1.2\\t 27\\t\\n\") error = %d ", error );
  } else
  { PRINT_SUB( oyjlTESTRESULT_FAIL,
    "oyjlStringsToDoubles(\"\\n \\t2\\t 1.2\\t 27\\t\\n\") error = %d ", error );
  }
  if(verbose)
  {
    for(i = 0; i < count; ++i)
      fprintf( zout, "%d:%f ", i, doubles[i] );
    fprintf( zout, "\n" );
  }
  if(doubles) { free(doubles); } doubles = NULL;

  error = oyjlStringsToDoubles( "x=0.2,one dot five,", ",", &count, malloc, &doubles );
  if( error > 0 &&
      count == 3)
  { PRINT_SUB( oyjlTESTRESULT_SUCCESS,
    "oyjlStringsToDoubles(\"x=0.2:,one dot five,\") error = %d ", error );
  } else
  { PRINT_SUB( oyjlTESTRESULT_FAIL,
    "oyjlStringsToDoubles(\"x=0.2:,one dot five,\") error = %d ", error );
  }
  if(doubles) { free(doubles); } doubles = NULL;

  double clck = oyjlClock();
  char * t = NULL;
  n = 10000;
  for(i = 0; i < n; ++i)
    oyjlStringAdd( &t, 0,0, "/%s/%s", "more", "and" );
  clck = oyjlClock() - clck;
  fprintf( zout, "oyjlStringAdd()\t%dx9  %d               \t\%s\n", n, (int)strlen(t),
                 oyjlProfilingToString(n,clck/(double)CLOCKS_PER_SEC,"ops"));
  if(t) { free(t); t = NULL; }

  clck = oyjlClock();
  for(i = 0; i < n; ++i)
    oyjlStringAddN( &t, "/more/and", 9, malloc,free );
  clck = oyjlClock() - clck;
  fprintf( zout, "oyjlStringAddN()\t%dx9  %d       \t\%s\n", n, (int)strlen(t),
                 oyjlProfilingToString(n,clck/(double)CLOCKS_PER_SEC,"ops"));

  n = 1000;
  clck = oyjlClock();
  int len OYJL_UNUSED;
  for(i = 0; i < n; ++i)
    len = strlen( t );
  clck = oyjlClock() - clck;
  fprintf( zout, "strlen()\t%dx  %d             \t\%s\n", n, (int)strlen(t),
                 oyjlProfilingToString(n,clck/(double)CLOCKS_PER_SEC,"ops"));

  if(t) { free(t); t = NULL; }

  n = 1000000;
  t = calloc(10*n,sizeof(char));
  clck = oyjlClock();
  for(i = 0; i < n; ++i)
    memcpy( &t[9*i], "/more/and", 9 );
  clck = oyjlClock() - clck;
  fprintf( zout, "memcpy()\t%dx9  %d           \t\%s\n", n, (int)strlen(t),
                 oyjlProfilingToString(n,clck/(double)CLOCKS_PER_SEC,"ops"));
  if(t) { free(t); t = NULL; }

  oyjl_str string = oyjlStrNew(10, 0,0);
  clck = oyjlClock();
  n = 1000000;
  for(i = 0; i < n; ++i)
    oyjlStrAppendN( string, "/more/and", 9 );
  clck = oyjlClock() - clck;
  fprintf( zout, "oyjlStrAppendN()\t%dx9  %d    \t\%s\n", n, (int)strlen(oyjlStr(string)),
                 oyjlProfilingToString(n,clck/(double)CLOCKS_PER_SEC,"ops"));
  oyjlStrRelease( &string );

  int inside = 0;
  string = oyjlStrNew(10, 0,0);
  for(i = 0; i < 10; ++i)
    oyjlStrAppendN( string, "/more/and", 9 );
  oyjlStrReplace( string, "/", "\\/", replaceCb, &inside );
  const char * tmp = oyjlStr(string);
  if(strstr(tmp, "/more\\/and/more"))
  { PRINT_SUB( oyjlTESTRESULT_SUCCESS,
    "oyjlStrReplace(callback,user_data)                   " );
  } else
  { PRINT_SUB( oyjlTESTRESULT_FAIL,
    "oyjlStrReplace(callback,user_data)                   " );
  }
  oyjlStrRelease( &string );


  return result;
}

oyjlTESTRESULT_e testArgs()
{
  oyjlTESTRESULT_e result = oyjlTESTRESULT_UNKNOWN;

  fprintf(stdout, "\n" );
  setlocale(LC_ALL,"en_GB.UTF8");

  int output = 0;
  const char * file = NULL;
  int file_count = 0;
  int show_status = 0;
  int help = 0;
  int verbose_ = 0;
  int state = 0;
  int argc = 1;
  const char * argv[] = {"test","-v","--input","file-name.json", "-z"};

  /* handle options */
  /* Select from *version*, *manufacturer*, *copyright*, *license*, *url*,
   * *support*, *download*, *sources*, *oyjl_modules_author* and
   * *documentation* what you see fit. Add new ones as needed. */
  oyjlUiHeaderSection_s sections[] = {
    /* type, nick,            label, name,                  description  */
    {"oihs", "version",       NULL,  "1.0",                 NULL},
    {"oihs", "documentation", NULL,  "",                    _("The example tool demontrates the usage of the libOyjl API's.")},
    {"oihs", "date",          NULL,  "2018-10-10T12:00:00", _("October 10, 2018")},
    {"",0,0,0,0}};

  /* declare some option choices */
  oyjlOptionChoice_s i_choices[] = {{"oyjl.json", _("oyjl.json"), _("oyjl.json"), ""},
                                    {"oyjl2.json", _("oyjl2.json"), _("oyjl2.json"), ""},
                                    {"","","",""}};
  oyjlOptionChoice_s o_choices[] = {{"0", _("Print All"), _("Print All"), ""},
                                    {"1", _("Print Camera"), _("Print Camera JSON"), ""},
                                    {"2", _("Print None"), _("Print None"), ""},
                                    {"","","",""}};

  /* declare options - the core information; use previously declared choices */
  oyjlOption_s oarray[] = {
  /* type,   flags, o,   option,    key,  name,         description,         help, value_name,    value_type,               values,                                                          variable_type, output variable */
    {"oiwi", 0,     "#", "",        NULL, _("status"),  _("Show Status"),    NULL, NULL,          oyjlOPTIONTYPE_NONE, {}, oyjlINT, {.i = &show_status} },
    {"oiwi", OYJL_OPTION_FLAG_EDITABLE,"@","",NULL,_("input"),_("Set Input"),NULL, _("FILENAME"), oyjlOPTIONTYPE_CHOICE, {}, oyjlINT, {.i = &file_count} },
    {"oiwi", 0,     "i", "input",   NULL, _("input"),   _("Set Input"),      NULL, _("FILENAME"), oyjlOPTIONTYPE_CHOICE, {.choices.list = (oyjlOptionChoice_s*) oyjlStringAppendN( NULL, (const char*)i_choices, sizeof(i_choices), malloc )}, oyjlSTRING, {.s = &file} },
    {"oiwi", 0,     "o", "output",  NULL, _("output"),  _("Control Output"), NULL, "0|1|2",       oyjlOPTIONTYPE_CHOICE, {.choices.list = (oyjlOptionChoice_s*) oyjlStringAppendN( NULL, (const char*)o_choices, sizeof(o_choices), malloc )}, oyjlINT, {.i = &output} },
    {"oiwi", 0,     "h", "help",    NULL, _("help"),    _("Help"),           NULL, NULL,          oyjlOPTIONTYPE_NONE, {}, oyjlINT, {.i = &help} },
    {"oiwi", 0,     "v", "verbose", NULL, _("verbose"), _("verbose"),        NULL, NULL,          oyjlOPTIONTYPE_NONE, {}, oyjlINT, {.i = &verbose_} },
    {"",0,0,0,0,0,0,0, NULL, oyjlOPTIONTYPE_END, {},0,{}}
  };

  /* declare option groups, for better syntax checking and UI groups */
  oyjlOptionGroup_s groups_no_args[] = {
  /* type,   flags, name,      description,          help, mandatory, optional, detail */
    {"oiwg", 0,     _("Mode1"),_("Simple mode"),     NULL, "#",       "o,v",    "o" }, /* accepted even if none of the mandatory options is set */
    {"oiwg", OYJL_OPTION_FLAG_EDITABLE,_("Mode2"),_("Any arg mode"),NULL,"@","o,v","@,o"},/* accepted if anonymous arguments are set */
    {"oiwg", 0,     _("Mode3"),_("Actual mode"),     NULL, "i",       "o,v",    "i,o" },/* parsed and checked with -i option */
    {"oiwg", 0,     _("Misc"), _("General options"), NULL, "",        "",       "v,h" },/* just show in documentation */
    {"",0,0,0,0,0,0,0}
  };

  /* tell about the tool */
  oyjlUi_s * ui = oyjlUi_Create( argc, argv, /* argc+argv are required for parsing the command line options */
                                       "oiCR", "oyjl-config-read", _("Short example tool using libOyjl"), "logo",
                                       sections, oarray, groups_no_args, NULL );
  /* done with options handling */

  if(verbose)
    oyjlOptions_PrintHelp( ui->opts, ui, 1, "%s v%s - %s", argv[0],
                            "1.0", "Test Tool for testing" );
  if(ui)
  { PRINT_SUB( oyjlTESTRESULT_SUCCESS, 
    "ui created - no args                           " );
  } else
  { PRINT_SUB( oyjlTESTRESULT_FAIL, 
    "ui created - no args                           " );
  }
  char * text = oyjlUi_ToJson( ui, 0 );
  if(text)
    OYJL_TEST_WRITE_RESULT( text, strlen(text), "UiCreateNoArgs", "txt" )
  if(verbose)
    fprintf( zout, "%s\n", text );
  if(text) {free(text);} text = NULL;
  oyjlUi_Release( &ui);


  const char * argv_anonymous[] = {"test","-v","file-name.json","file-name2.json"};
  int argc_anonymous = 4;
  ui = oyjlUi_Create( argc_anonymous, argv_anonymous, /* argc+argv are required for parsing the command line options */
                                       "oiCR", "oyjl-config-read", _("Short example tool using libOyjl"), "logo",
                                       sections, oarray, groups_no_args, NULL );
  if(ui)
  { PRINT_SUB( oyjlTESTRESULT_SUCCESS, 
    "ui created - anonymous args                    " );
  } else
  { PRINT_SUB( oyjlTESTRESULT_FAIL, 
    "ui created - anonymous args                    " );
  }
  int count = 0, i;
  /* detect all anonymous arguments */
  char ** results = oyjlOptions_ResultsToList( ui?ui->opts:NULL, "@", &count );
  if(count == 2 &&
     file_count == count &&
     strcmp(argv_anonymous[2],results[0]) == 0 &&
     strcmp(argv_anonymous[3],results[1]) == 0)
  { PRINT_SUB( oyjlTESTRESULT_SUCCESS, 
    "anonymous args correctly detected  %d          ", count );
  } else
  { PRINT_SUB( oyjlTESTRESULT_FAIL, 
    "anonymous args correctly detected  %d          ", count );
  }
  if(verbose)
  for(i = 0; i < count; ++i)
    fprintf( zout, "%s\n", results[i] );
  oyjlUi_Release( &ui);
  oyjlStringListRelease( &results, count, 0 );

  /* declare option groups, for better syntax checking and UI groups */
  oyjlOptionGroup_s groups[] = {
  /* type,   flags, name,      description,          help, mandatory, optional, detail */
    {"oiwg", 0,     _("Mode"), _("Actual mode"),     NULL, "i",       "o,v",    "i,o" },
    {"oiwg", 0,     _("Misc"), _("General options"), NULL, "h",       "",       "h,v" },
    {"",0,0,0,0,0,0,0}
  };

  argc = 2;
  ui = oyjlUi_Create( argc, argv, /* argc+argv are required for parsing the command line options */
                                       "oiCR", "oyjl-config-read", _("Short example tool using libOyjl"), "logo",
                                       sections, oarray, groups, NULL );

  if(ui)
  { PRINT_SUB( oyjlTESTRESULT_SUCCESS, 
    "ui created - correct args                      " );
  } else
  { PRINT_SUB( oyjlTESTRESULT_FAIL, 
    "ui created - correct args                      " );
  }
  oyjlUi_Release( &ui);

  argc = 3;
  ui = oyjlUi_Create( argc, argv, /* argc+argv are required for parsing the command line options */
                                       "oiCR", "oyjl-config-read", _("Short example tool using libOyjl"), "logo",
                                       sections, oarray, groups, &state );
  if(!ui)
  { PRINT_SUB( oyjlTESTRESULT_SUCCESS, 
    "ui not created - missing arg %d                " , state >> oyjlUI_STATE_OPTION );
  } else
  { PRINT_SUB( oyjlTESTRESULT_FAIL, 
    "ui not created - missing arg                   " );
  }
  oyjlUi_Release( &ui);

  argc = 4;
  ui = oyjlUi_Create( argc, argv, /* argc+argv are required for parsing the command line options */
                                       "oiCR", "oyjl-config-read", _("Short example tool using libOyjl"), "logo",
                                       sections, oarray, groups, NULL );
  if(ui && strcmp(file,"file-name.json") == 0)
  { PRINT_SUB( oyjlTESTRESULT_SUCCESS, 
    "ui created - parse string                      " );
  } else
  { PRINT_SUB( oyjlTESTRESULT_FAIL, 
    "ui created - parse string                      " );
  }
  oyjlUi_Release( &ui);

  argc = 5;
  ui = oyjlUi_Create( argc, argv, /* argc+argv are required for parsing the command line options */
                                       "oiCR", "oyjl-config-read", _("Short example tool using libOyjl"), "logo",
                                       sections, oarray, groups, &state );
  if(!ui)
  { PRINT_SUB( oyjlTESTRESULT_SUCCESS, 
    "ui not created - wrong arg  %d                 ", state >> oyjlUI_STATE_OPTION );
  } else
  { PRINT_SUB( oyjlTESTRESULT_FAIL, 
    "ui not created - wrong arg                     " );
  }
  oyjlUi_Release( &ui);


  argc = 4;
  ui = oyjlUi_Create( argc, argv, /* argc+argv are required for parsing the command line options */
                                       "oyjl-config-read", "Oyjl Config Reader", _("Short example tool using libOyjl"), "logo",
                                       sections, oarray, groups, NULL );
  text = oyjlUi_ToJson( ui, 0 );
  if(text && strlen(text) == 3348)
  { PRINT_SUB( oyjlTESTRESULT_SUCCESS, 
    "oyjlUi_ToJson() %lu                           ", strlen(text) );
  } else
  { PRINT_SUB( oyjlTESTRESULT_FAIL, 
    "oyjlUi_ToJson() 3348 == %lu                   ", strlen(text) );
  }
  OYJL_TEST_WRITE_RESULT( text, strlen(text), "oyjlUi_ToJson", "txt" )
  if(verbose && text)
    fprintf( zout, "%s\n", text );
  if(text) {free(text);} text = NULL;

  text = oyjlUi_ToMan( ui, 0 );
  if(text && strlen(text) == 692)
  { PRINT_SUB( oyjlTESTRESULT_SUCCESS, 
    "oyjlUi_ToMan() %lu                            ", strlen(text) );
  } else
  { PRINT_SUB( oyjlTESTRESULT_FAIL, 
    "oyjlUi_ToMan() 662 == %lu                     ", strlen(text) );
  }
  OYJL_TEST_WRITE_RESULT( text, strlen(text), "oyjlUi_ToMan", "txt" )
  if(verbose)
    fprintf( zout, "%s\n", text );
  if(text) {free(text);} text = NULL;

  text = oyjlOptions_ResultsToJson( ui->opts );
  if(text && strlen(text) == 39)
  { PRINT_SUB( oyjlTESTRESULT_SUCCESS, 
    "oyjlOptions_ResultsToJson() %lu                 ", strlen(text) );
  } else
  { PRINT_SUB( oyjlTESTRESULT_FAIL, 
    "oyjlOptions_ResultsToJson() %lu                 ", strlen(text) );
  }
  if(verbose)
    fprintf( zout, "%s\n", text );
  if(text) {free(text);} text = NULL;
  setlocale(LC_ALL,"");

  text = oyjlOptions_ResultsToText( ui->opts );
  if(text && strlen(text) == 21)
  { PRINT_SUB( oyjlTESTRESULT_SUCCESS, 
    "oyjlOptions_ResultsToText() %lu                 ", strlen(text) );
  } else
  { PRINT_SUB( oyjlTESTRESULT_FAIL, 
    "oyjlOptions_ResultsToText() %lu                 ", strlen(text) );
  }
  if(verbose)
    fprintf( zout, "%s\n", text );
  if(text) {free(text);} text = NULL;

  if(verbose)
  {
    fprintf(stdout, "Help text -> stderr:\n" );
    oyjlOptions_PrintHelp( ui->opts, ui, 1, "%s v%s - %s", argv[0],
                            "1.0", "Test Tool for testing" );
  }

  text = oyjlUi_ExportToJson( ui, 0 );
  if(text && strlen(text) == 6180)
  { PRINT_SUB( oyjlTESTRESULT_SUCCESS, 
    "oyjlUi_ExportToJson()       %lu                 ", strlen(text) );
  } else
  { PRINT_SUB( oyjlTESTRESULT_FAIL, 
    "oyjlUi_ExportToJson()       6180 == %lu         ", strlen(text) );
  }
  if(verbose && text)
    fprintf( zout, "%s\n", text );
  if(text) { free(text); text = NULL; }

  oyjlUi_Release( &ui);
  char * wrong = "test";
  fprintf(stdout, "oyjlUi_Release(&\"test\") - should give a warning message:\n" );
  oyjlUi_Release( (oyjlUi_s **)&wrong);

  free(oarray[2].values.choices.list);
  free(oarray[3].values.choices.list);

  return result;
}


void oyjlTreeToJson2 (oyjl_val v, int * level, char ** json);

oyjlTESTRESULT_e testTree ()
{
  oyjlTESTRESULT_e result = oyjlTESTRESULT_UNKNOWN;

  fprintf(stdout, "\n" );

  int i;

  oyjl_val value = 0;
  oyjl_val root = 0;

  value = oyjlTreeGetValue( NULL, OYJL_CREATE_NEW, "not/existing" );
  if(!value)
  { PRINT_SUB( oyjlTESTRESULT_SUCCESS,
    "oyjlTreeGetValue( NULL, flags=OYJL_CREATE_NEW, xpath ) == NULL" );
  } else
  { PRINT_SUB( oyjlTESTRESULT_FAIL,
    "oyjlTreeGetValue( NULL, flags=OYJL_CREATE_NEW, xpath ) == NULL" );
  }

  root = oyjlTreeNew( NULL );
  if(root)
  { PRINT_SUB( oyjlTESTRESULT_SUCCESS,
    "oyjlTreeNew( NULL )                     " );
  } else
  { PRINT_SUB( oyjlTESTRESULT_FAIL,
    "oyjlTreeNew( NULL )                     " );
  }
  oyjlTreeFree( root );

  root = oyjlTreeNew( "new/tree/key" );
  char * rjson = NULL; i = 0;
  oyjlTreeToJson( root, &i, &rjson ); i = 0;
  size_t len = strlen(rjson);
  if(root)
  { PRINT_SUB( oyjlTESTRESULT_SUCCESS,
    "oyjlTreeNew( \"new/tree/key\" )       %d", (int)len );
  } else
  { PRINT_SUB( oyjlTESTRESULT_FAIL,
    "oyjlTreeNew( \"new/tree/key\" )       %d", (int)len );
  }
  if(verbose)
    fprintf( zout, "%s\n", rjson );
  myDeAllocFunc( rjson ); rjson = NULL;

  value = oyjlTreeGetValue( root, 0, "new/[0]" );
  int count = oyjlValueCount( value );
  if(count == 1)
  { PRINT_SUB( oyjlTESTRESULT_SUCCESS,
    "oyjlTreeGetValue(root, 0, \"new/[0]\") oyjlValueCount() %i", count );
  } else
  { PRINT_SUB( oyjlTESTRESULT_FAIL,
    "oyjlTreeGetValue(root, 0, \"new/[0]\") oyjlValueCount() %i", count );
  }

  value = oyjlTreeGetValue( root, 0, "new/[]" );
  count = oyjlValueCount( value );
  if(count == 1)
  { PRINT_SUB( oyjlTESTRESULT_SUCCESS,
    "oyjlTreeGetValue(root, 0, \"new/[]\")  oyjlValueCount() %i", count );
  } else
  { PRINT_SUB( oyjlTESTRESULT_FAIL,
    "oyjlTreeGetValue(root, 0, \"new/[]\")  oyjlValueCount() %i", count );
  }

  value = oyjlTreeGetValue( root, OYJL_CREATE_NEW, "new/[]" );
  count = oyjlValueCount( value );
  if(count == 1)
  { PRINT_SUB( oyjlTESTRESULT_SUCCESS,
    "oyjlTreeGetValue(root, OYJL_CREATE_NEW, \"new/[]\") oyjlValueCount() %i", count );
  } else
  { PRINT_SUB( oyjlTESTRESULT_FAIL,
    "oyjlTreeGetValue(root, OYJL_CREATE_NEW, \"new/[]\") oyjlValueCount() %i", count );
  }


  value = oyjlTreeGetValue( root, 0, "new/tree/key" );
#define VALUE "value-\"one\""
  oyjlValueSetString( value, VALUE );
  oyjlTreeToJson( root, &i, &rjson ); i = 0;
  if(len < strlen(rjson))
  { PRINT_SUB( oyjlTESTRESULT_SUCCESS,
    "oyjlValueSetString( %s )         %d", VALUE, (int)strlen(rjson) );
  } else
  { PRINT_SUB( oyjlTESTRESULT_FAIL,
    "oyjlValueSetString( %s )         %d", VALUE, (int)strlen(rjson) );
  }
  if(verbose)
    fprintf( zout, "%s\n", rjson );
  len = strlen(rjson);
  myDeAllocFunc( rjson ); rjson = NULL;

  value = oyjlTreeGetValue( root, 0, "new/tree/key" );
  char * v = oyjlValueText(value, myAllocFunc);
  if(v && strlen(v) == strlen(VALUE))
  { PRINT_SUB( oyjlTESTRESULT_SUCCESS,
    "oyjlValueText( \"new/tree/key\" ) = %s", v );
  } else
  { PRINT_SUB( oyjlTESTRESULT_FAIL,
    "oyjlValueText( \"new/tree/key\" ) = %s", VALUE );
  }
  myDeAllocFunc(v);

  oyjlTreeClearValue( root,"new/tree/key" );
  oyjlTreeToJson( root, &i, &rjson ); i = 0;
  if(!rjson || strcmp(rjson, "null") == 0)
  { PRINT_SUB( oyjlTESTRESULT_SUCCESS,
    "oyjlTreeClearValue( \"new/tree/key\" ) " );
  } else
  { PRINT_SUB( oyjlTESTRESULT_FAIL,
    "oyjlTreeClearValue( \"new/tree/key\" ) " );
  }
  oyjlTreeFree( root );

  double clck = oyjlClock();
  root = oyjlTreeNew("");
  int n = 2000;
  for(i = 0; i < n; ++i)
    oyjlTreeSetStringF( root, OYJL_CREATE_NEW, "value", "data/key-%d", i );
  clck = oyjlClock() - clck;
  fprintf( zout, "oyjTreeSetStringF()\t%dx              \t\%s\n", n,
                 oyjlProfilingToString(n,clck/(double)CLOCKS_PER_SEC,"node"));
  i = 0;

  clck = oyjlClock();
  oyjlTreeToJson2( root, &i, &rjson ); i = 0;
  clck = oyjlClock() - clck;
  fprintf( zout, "oyjTreeToJson2()       \t1x %d            \t\%s\n", (int)strlen(rjson),
                 oyjlProfilingToString(1,clck/(double)CLOCKS_PER_SEC,"dump"));
  myDeAllocFunc( rjson ); rjson = NULL;

  clck = oyjlClock();
  oyjlTreeToJson( root, &i, &rjson ); i = 0;
  clck = oyjlClock() - clck;
  fprintf( zout, "oyjTreeToJson()        \t1x %d            \t\%s\n", (int)strlen(rjson),
                 oyjlProfilingToString(1,clck/(double)CLOCKS_PER_SEC,"dump"));
  myDeAllocFunc( rjson ); rjson = NULL;
  oyjlTreeFree( root );

  return result;
}

oyjlTESTRESULT_e testIO ()
{
  oyjlTESTRESULT_e result = oyjlTESTRESULT_UNKNOWN;

  fprintf(stdout, "\n" );

  int len = 48;
  int size = oyjlWriteFile( "test.txt", "test", 5 );
  char info[len];
  int r = oyjlIsFile( "test.txt", "r", info, len );
  int size2 = oyjlWriteFile( "test2.txt", "test2", 6 );
  char info2[len];
  int r2 = oyjlIsFile( "test2.txt", "r", info2, len );

  if(r && r2 && size == 5 && size2 == 6)
  { PRINT_SUB( oyjlTESTRESULT_SUCCESS,
    "oyjlWriteFile()                         " );
  } else
  { PRINT_SUB( oyjlTESTRESULT_FAIL,
    "oyjlWriteFile()                         " );
  }

  if(strlen(info) && strlen(info2))
  { PRINT_SUB( oyjlTESTRESULT_SUCCESS,
    "oyjlIsFile()                            " );
  } else
  { PRINT_SUB( oyjlTESTRESULT_FAIL,
    "oyjlIsFile()                            " );
  }

  if( verbose )
    fprintf( zout, "info: %s %s\n", info, info2 );

  return result;
}

/* --- end actual tests --- */


