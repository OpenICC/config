/** @file test.c
 *
 *  libOpenICC - OpenICC Colour Management Configuration
 *
 *  Copyright (C) 2011-2015  Kai-Uwe Behrmann
 *
 *  @brief    OpenICC test suite
 *  @internal
 *  @author   Kai-Uwe Behrmann <ku.b@gmx.de>
 *  @par License:\n
 *            MIT <http://www.opensource.org/licenses/mit-license.php>
 *  @since    2011/06/27
 */

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "openicc_config.h"
#include "openicc_version.h"

/* C++ includes and definitions */
#ifdef __cplusplus
#include <fstream>
#include <iostream>
#define USE_NEW
#endif

#ifdef USE_NEW
void* myAllocFunc(size_t size) { return new char [size]; }
#else
void* myAllocFunc(size_t size) { return calloc(size,1); }
#endif

#include <math.h>


/* --- general test routines --- */

typedef enum {
  oiTESTRESULT_SYSERROR,
  oiTESTRESULT_FAIL,
  oiTESTRESULT_XFAIL,
  oiTESTRESULT_SUCCESS,
  oiTESTRESULT_UNKNOWN
} oiTESTRESULT_e;


const char * oiTestResultToString    ( oiTESTRESULT_e      error )
{
  const char * text = "";
  switch(error)
  {
    case oiTESTRESULT_SYSERROR:text = "SYSERROR"; break;
    case oiTESTRESULT_FAIL:    text = "FAIL"; break;
    case oiTESTRESULT_XFAIL:   text = "XFAIL"; break;
    case oiTESTRESULT_SUCCESS: text = "SUCCESS"; break;
    case oiTESTRESULT_UNKNOWN: text = "UNKNOWN"; break;
    default:                   text = "Huuch, what's that?"; break;
  }
  return text;
}

const char  *  oiIntToString         ( int                 integer )
{
  static char texts[3][255];
  static int a = 0;
  int i;

  if(a >= 3) a = 0;

  for(i = 0; i < 8-log10(integer); ++i)
    sprintf( &texts[a][i], " " );

  sprintf( &texts[a][i], "%d", integer );

  return texts[a++];
}

const char  *  oiProfilingToString   ( int                 integer,
                                       double              duration,
                                       const char        * term )
{
  static char texts[3][255];
  static int a = 0;
  int i, len;

  if(a >= 3) a = 0;

  if(integer/duration >= 1000000.0)
    sprintf( &texts[a][0], "%.02f M%s/s", integer/duration/1000000.0, term );
  else
    sprintf( &texts[a][0], "%.00f %s/s", integer/duration, term );

  len = strlen(&texts[a][0]);

  for(i = 0; i < 16-len; ++i)
    sprintf( &texts[a][i], " " );

  if(integer/duration >= 1000000.0)
    sprintf( &texts[a][i], "%.02f M%s/s", integer/duration/1000000.0, term );
  else
    sprintf( &texts[a][i], "%.00f %s/s", integer/duration, term );

  return texts[a++];
}

FILE * zout;

int oi_test_sub_count = 0;
#define PRINT_SUB( result_, ... ) { \
  if(result_ < result) \
    result = result_; \
  fprintf(stdout, ## __VA_ARGS__ ); \
  fprintf(stdout, " ..\t%s", oiTestResultToString(result_)); \
  if(result_ <= oiTESTRESULT_FAIL) \
    fprintf(stdout, " !!! ERROR !!!" ); \
  fprintf(stdout, "\n" ); \
  ++oi_test_sub_count; \
}


/* --- actual tests --- */

oiTESTRESULT_e testVersion()
{
  oiTESTRESULT_e result = oiTESTRESULT_UNKNOWN;

  fprintf(stdout, "\n" );
  fprintf(zout, "compiled version:     %d\n", OPENICC_VERSION );
  fprintf(zout, " runtime version:     %d\n", openiccVersion() );

  if(OPENICC_VERSION == openiccVersion())
    result = oiTESTRESULT_SUCCESS;
  else
    result = oiTESTRESULT_FAIL;

  return result;
}



static int test_number = 0;
#define TEST_RUN( prog, text ) { \
  if(argc > argpos) { \
      for(i = argpos; i < argc; ++i) \
        if(strstr(text, argv[i]) != 0 || \
           atoi(argv[i]) == test_number ) \
          oiTestRun( prog, text, test_number ); \
  } else if(list) \
    printf( "[%d] %s\n", test_number, text); \
  else \
    oiTestRun( prog, text, test_number ); \
  ++test_number; \
}

int results[oiTESTRESULT_UNKNOWN+1];
char * tests_failed[64];
char * tests_xfailed[64];

oiTESTRESULT_e oiTestRun             ( oiTESTRESULT_e    (*test)(void),
                                       const char        * test_name,
                                       int                 number )
{
  oiTESTRESULT_e error = oiTESTRESULT_UNKNOWN;

  fprintf( stdout, "\n________________________________________________________________\n" );
  fprintf(stdout, "Test[%d]: %s ... ", test_number, test_name );

  error = test();

  fprintf(stdout, "\t%s", oiTestResultToString(error));

  if(error == oiTESTRESULT_FAIL)
    tests_failed[results[error]] = (char*)test_name;
  if(error == oiTESTRESULT_XFAIL)
    tests_xfailed[results[error]] = (char*)test_name;
  results[error] += 1;

  /* print */
  if(error <= oiTESTRESULT_FAIL)
    fprintf(stdout, " !!! ERROR !!!" );
  fprintf(stdout, "\n" );

  return error;
}

/*  main */
int main(int argc, char** argv)
{
  int i, error = 0,
      argpos = 1,
      list = 0;

  zout = stdout;  /* printed inbetween results */

  if(getenv("OY_DEBUG"))
  {
    int value = atoi(getenv("OY_DEBUG"));
    if(value > 0)
      openicc_debug += value;
  }

  /* init */
  for(i = 0; i <= oiTESTRESULT_UNKNOWN; ++i)
    results[i] = 0;

  i = 1; while(i < argc) if( strcmp(argv[i++],"-l") == 0 )
  { ++argpos;
    zout = stderr;
    list = 1;
  }

  i = 1; while(i < argc) if( strcmp(argv[i++],"--silent") == 0 )
  { ++argpos;
    zout = stderr;
  }

  fprintf( zout, "\nOpenICC Tests v" OPENICC_VERSION_NAME
           "\n\n" );

  /* do tests */

  TEST_RUN( testVersion, "Version matching" );
  /*TEST_RUN( testElektra, "Elektra" );
  TEST_RUN( testStringRun, "String handling" );
  TEST_RUN( testPaths, "Paths" );*/

  /* give a summary */
  if(!list)
  {

    fprintf( stdout, "\n################################################################\n" );
    fprintf( stdout, "#                                                              #\n" );
    fprintf( stdout, "#                     Results                                  #\n" );
    fprintf( stdout, "    Total of Sub Tests:         %d\n", oi_test_sub_count );
    for(i = 0; i <= oiTESTRESULT_UNKNOWN; ++i)
      fprintf( stdout, "    Tests with status %s:\t%d\n",
                       oiTestResultToString( (oiTESTRESULT_e)i ), results[i] );

    error = (results[oiTESTRESULT_FAIL] ||
             results[oiTESTRESULT_SYSERROR] ||
             results[oiTESTRESULT_UNKNOWN]
            );

    for(i = 0; i < results[oiTESTRESULT_XFAIL]; ++i)
      fprintf( stdout, "    %s: \"%s\"\n",
               oiTestResultToString( oiTESTRESULT_XFAIL), tests_xfailed[i] );
    for(i = 0; i < results[oiTESTRESULT_FAIL]; ++i)
      fprintf( stdout, "    %s: \"%s\"\n",
               oiTestResultToString( oiTESTRESULT_FAIL), tests_failed[i] );

    if(error)
      fprintf( stdout, "    Tests FAILED\n" );
    else
      fprintf( stdout, "    Tests SUCCEEDED\n" );

    fprintf( stdout, "\n    Hint: the '-l' option will list all test names\n" );

  }

  return error;
}

