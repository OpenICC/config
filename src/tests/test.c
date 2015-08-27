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
#include "openicc_config_internal.h"

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

#include <locale.h>

#if 0
oiTESTRESULT_e testI18N()
{
  const char * lang = 0;
  oiTESTRESULT_e result = oiTESTRESULT_UNKNOWN;

  fprintf(stdout, "\n" );

  oiI18Nreset();

  lang = oiLanguage();
  if((lang && (strcmp(lang, "C") == 0)) || !lang)
  { PRINT_SUB( oiTESTRESULT_SUCCESS, 
    "oiLanguage() uninitialised good %s                ", lang );
  } else
  { PRINT_SUB( oiTESTRESULT_FAIL, 
    "oiLanguage() uninitialised failed                 " );
  }

  setlocale(LC_ALL,"");
  oiI18Nreset();

  lang = oiLanguage();
  if(lang && (strcmp(lang, "C") != 0))
  { PRINT_SUB( oiTESTRESULT_SUCCESS, 
    "oiLanguage() initialised good %s                  ", lang );
  } else
  { PRINT_SUB( oiTESTRESULT_XFAIL, 
    "oiLanguage() initialised failed %s                ", lang );
  }

  return result;
}

#define TEST_DOMAIN "sw/Oyranos/Tests"
#define TEST_KEY "/test_key"

oiTESTRESULT_e testDB()
{
  int error = 0;
  char * value = 0,
       * start = 0;
  oiTESTRESULT_e result = oiTESTRESULT_UNKNOWN;

  fprintf(stdout, "\n" );

  error = oiSetPersistentString( TEST_DOMAIN TEST_KEY, oiSCOPE_USER,
                                 "NULLTestValue", "NULLTestComment" );
  if(error)
  {
    PRINT_SUB( oiTESTRESULT_FAIL, 
    "oiSetPersistentString(%s)", TEST_DOMAIN TEST_KEY );
  } else
  {
    PRINT_SUB( oiTESTRESULT_SUCCESS,
    "oiSetPersistentString(%s)", TEST_DOMAIN TEST_KEY );
  }

  start = oiGetPersistentString(TEST_DOMAIN TEST_KEY, 0, oiSCOPE_USER_SYS, 0);
  if(start && start[0])
  {
    PRINT_SUB( oiTESTRESULT_SUCCESS, 
    "oiGetPersistentString(%s)", TEST_DOMAIN TEST_KEY );
  } else
  {
    PRINT_SUB( oiTESTRESULT_XFAIL,
    "oiGetPersistentString(%s)", TEST_DOMAIN TEST_KEY );
  }

  printf ("start is %s\n", oiNoEmptyString_m_(start));
  if(!start)
  {
    oiExportStart_(EXPORT_CHECK_NO);
    oiExportEnd_();
    error = oiSetPersistentString(TEST_DOMAIN TEST_KEY, oiSCOPE_USER,
                                 "NULLTestValue", "NULLTestComment" );
    start = oiGetPersistentString(TEST_DOMAIN TEST_KEY, 0, oiSCOPE_USER_SYS, 0);
    printf ("start is %s\n", start);
    
    PRINT_SUB( start?oiTESTRESULT_SUCCESS:oiTESTRESULT_XFAIL,
    "DB not initialised? try oiExportStart_(EXPORT_CHECK_NO)" );
  }
  if(!start)
  {
    oiExportStart_(EXPORT_SETTING);
    oiExportEnd_();
    error = oiSetPersistentString(TEST_DOMAIN TEST_KEY, oiSCOPE_USER,
                                 "NULLTestValue", "NULLTestComment" );
    start = oiGetPersistentString(TEST_DOMAIN TEST_KEY, 0, oiSCOPE_USER_SYS, 0);
    PRINT_SUB( start?oiTESTRESULT_SUCCESS:oiTESTRESULT_XFAIL, 
    "DB not initialised? try oiExportStart_(EXPORT_SETTING)" );
  }
  if(start)
    fprintf(zout, "start key value: %s\n", start );
  else
    fprintf(zout, "could not initialise\n" );

  error = oiSetPersistentString(TEST_DOMAIN TEST_KEY, oiSCOPE_USER,
                                 "myTestValue", "myTestComment" );
  value = oiGetPersistentString(TEST_DOMAIN TEST_KEY, 0, oiSCOPE_USER_SYS, 0);
  if(value)
    fprintf(zout, "result key value: %s\n", value );

  if(error)
  {
    PRINT_SUB( oiTESTRESULT_SYSERROR, 
    "DB error: %d", error );
  } else
  /* we want "start" to be different from "value" */
  if(start && value && strcmp(start,value) == 0)
  {
    PRINT_SUB( oiTESTRESULT_FAIL, 
    "DB (start!=value) failed: %s|%s", start, value );
  } else
  if(!value)
  {
    if(!value)
      PRINT_SUB( oiTESTRESULT_FAIL, 
      "DB (value) failed" );
    if(!start)
      PRINT_SUB( oiTESTRESULT_FAIL, 
      "DB (init) failed" );
  } else
  if(value)
  {
    if(strcmp(value,"myTestValue") == 0)
    {
      PRINT_SUB( oiTESTRESULT_SUCCESS, 
      "DB (value): %s", value );
    } else
    {
      PRINT_SUB( oiTESTRESULT_FAIL, 
      "DB (value) wrong: %s", value );
    }
  } else
    result = oiTESTRESULT_SUCCESS;
  oiFree_m_( start );
  oiFree_m_( value );

  error = oiDBEraseKey_( TEST_DOMAIN TEST_KEY, oiSCOPE_USER );
  if(error)
  {
    PRINT_SUB( oiTESTRESULT_FAIL, 
    "oiDBEraseKey_(%s)", TEST_DOMAIN TEST_KEY );
  } else
  {
    PRINT_SUB( oiTESTRESULT_SUCCESS,
    "oiDBEraseKey_(%s)", TEST_DOMAIN TEST_KEY );
  }
  oiDB_s * db = oiDB_newFrom( TEST_DOMAIN, oiSCOPE_USER_SYS, oiAllocateFunc_ );
  value = oiDB_getString(db, TEST_DOMAIN TEST_KEY);
  oiDB_release( &db );
  if(value && strlen(value))
  {
    PRINT_SUB( oiTESTRESULT_FAIL, 
    "DB key not erased                  " );
  } else
  {
    PRINT_SUB( oiTESTRESULT_SUCCESS,
    "DB key erased                      " );
  }
  oiFree_m_(value);


  error = oiSetPersistentString( OY_STD "/device" TEST_KEY "/#0/key-01", oiSCOPE_USER,
                                 "SomeValue", "SomeComment" );
  error = oiSetPersistentString( OY_STD "/device" TEST_KEY "/#0/key-02", oiSCOPE_USER,
                                 "SomeValue", "SomeComment" );
  error = oiSetPersistentString( OY_STD "/device" TEST_KEY "/#1/key-01", oiSCOPE_USER,
                                 "SomeValue", "SomeComment" );
  error = oiSetPersistentString( OY_STD "/device" TEST_KEY "/#1/key-02", oiSCOPE_USER,
                                 "SomeValue", "SomeComment" );
  value = oiDBSearchEmptyKeyname_(OY_STD "/device" TEST_KEY, oiSCOPE_USER);
  if(value && strcmp( "user/" OY_STD "/device" TEST_KEY "/#2",value) == 0 )
  {
    PRINT_SUB( oiTESTRESULT_SUCCESS, 
    "oiDBSearchEmptyKeyname_()=%s", value );
  } else
  {
    PRINT_SUB( oiTESTRESULT_FAIL,
    "oiDBSearchEmptyKeyname_(%s)", OY_STD "/device" TEST_KEY );
  }
  if(value)
    oiFree_m_( value );

  error = oiDBEraseKey_( OY_STD "/device" TEST_KEY, oiSCOPE_USER );
  value = oiDBSearchEmptyKeyname_(OY_STD "/device" TEST_KEY, oiSCOPE_USER);
  if(value && strcmp( "user/" OY_STD "/device" TEST_KEY "/#0",value) == 0 )
  {
    PRINT_SUB( oiTESTRESULT_SUCCESS, 
    "oiDBSearchEmptyKeyname_()=%s", value );
  } else
  {
    PRINT_SUB( oiTESTRESULT_FAIL,
    "oiDBSearchEmptyKeyname_(%s)", OY_STD "/device" TEST_KEY );
  }
  oiFree_m_( value );

  error = oiSetPersistentString( TEST_DOMAIN "/device" TEST_KEY "/#0/key-01", oiSCOPE_USER,
                                 "SomeValue", "SomeComment" );
  error = oiSetPersistentString( TEST_DOMAIN "/device" TEST_KEY "/#0/key-02", oiSCOPE_USER,
                                 "SomeValue", "SomeComment" );
  error = oiSetPersistentString( TEST_DOMAIN "/device" TEST_KEY "/#1/key-01", oiSCOPE_USER,
                                 "SomeValue", "SomeComment" );
  error = oiSetPersistentString( TEST_DOMAIN "/device" TEST_KEY "/#1/key-02", oiSCOPE_USER,
                                 "SomeValue", "SomeComment" );
  value = oiDBSearchEmptyKeyname_(TEST_DOMAIN "/device" TEST_KEY, oiSCOPE_USER);
  if(value && strcmp( "user/" TEST_DOMAIN "/device" TEST_KEY "/#2",value) == 0 )
  {
    PRINT_SUB( oiTESTRESULT_SUCCESS, 
    "oiDBSearchEmptyKeyname_()=%s", value );
  } else
  {
    PRINT_SUB( oiTESTRESULT_FAIL,
    "oiDBSearchEmptyKeyname_()=%s", value );
  }
  oiFree_m_( value );

  error = oiDBEraseKey_( TEST_DOMAIN "/device" TEST_KEY, oiSCOPE_USER );
  value = oiDBSearchEmptyKeyname_(TEST_DOMAIN "/device" TEST_KEY, oiSCOPE_USER);
  if(value && strcmp( "user/" TEST_DOMAIN "/device" TEST_KEY "/#0",value) == 0 )
  {
    PRINT_SUB( oiTESTRESULT_SUCCESS, 
    "oiDBSearchEmptyKeyname_()=%s", value );
  } else
  {
    PRINT_SUB( oiTESTRESULT_FAIL,
    "oiDBSearchEmptyKeyname_()=%s", TEST_DOMAIN "/device" TEST_KEY );
  }

  char * key = 0;
  oiStringAddPrintf( &key, oiAllocateFunc_, oiDeAllocateFunc_,
                     "%s/array_key", value );
  error = oiSetPersistentString( key, oiSCOPE_USER_SYS,
                                 "ArrayValue", "ArrayComment" );
  oiFree_m_( value );
  value = oiGetPersistentString(strchr(key,'/')+1, 0, oiSCOPE_USER_SYS, 0);
  if(value && strcmp(value, "ArrayValue") == 0)
  {
    PRINT_SUB( oiTESTRESULT_SUCCESS, 
    "oiSetPersistentString(%s, oiSCOPE_USER_SYS)", key );
  } else
  {
    PRINT_SUB( oiTESTRESULT_FAIL,
    "oiSetPersistentString(%s, oiSCOPE_USER_SYS)", key );
  }
  oiFree_m_( key );
  oiFree_m_( value );

  return result;
}


oiTESTRESULT_e testPaths()
{
  oiTESTRESULT_e result = oiTESTRESULT_UNKNOWN;

  fprintf(stdout, "\n" );

  const char * type_names[] = {
    "oiPATH_NONE", "oiPATH_ICC", "oiPATH_POLICY", "oiPATH_MODULE", "oiPATH_SCRIPT", "oiPATH_CACHE"
  };
  oiPATH_TYPE_e types[] = {
    oiPATH_NONE, oiPATH_ICC, oiPATH_POLICY, oiPATH_MODULE, oiPATH_SCRIPT, oiPATH_CACHE
  };
  const char * scope_names[] = {
    "oiSCOPE_USER_SYS", "oiSCOPE_USER", "oiSCOPE_SYSTEM", "oiSCOPE_OPENICC", "oiSCOPE_MACHINE"
  };
  oiSCOPE_e scopes[] = {
    oiSCOPE_USER_SYS, oiSCOPE_USER, oiSCOPE_SYSTEM, (oiSCOPE_e)oiSCOPE_OPENICC, (oiSCOPE_e)oiSCOPE_MACHINE
  };

  for(int i = 1; i <= 5; ++i)
  for(int j = 1; j <= 4; ++j)
  {
  char * text = oiGetInstallPath( types[i], scopes[j], oiAllocateFunc_ );
  if(text)
  { PRINT_SUB( oiTESTRESULT_SUCCESS,
    "oiGetInstallPath( %s, %s ): %s", type_names[i],scope_names[j],
                                                oiNoEmptyString_m_(text) );
  } else
  { PRINT_SUB( oiTESTRESULT_XFAIL,
    "oiGetInstallPath( %s, %s ): %s", type_names[i],scope_names[j],
                                                oiNoEmptyString_m_(text) );
  }
  }


  return result;
}
#endif


oiTESTRESULT_e testIO ()
{
  oiTESTRESULT_e result = oiTESTRESULT_UNKNOWN;

  fprintf(stdout, "\n" );

  int error = 0;

  char * t1, *t2, *t3;
  const char * file_name = "/usr/share/color/icc/OpenICC/sRGB.icc";

  t1 = openiccExtractPathFromFileName_( file_name );
  fprintf(zout, "file_name: %s\n", file_name );
  error = !t1 || (strlen(t1) >= strlen(file_name));
  if(!error)
  { PRINT_SUB( oiTESTRESULT_SUCCESS,
    "openiccExtractPathFromFileName_() %s", t1 );
  } else
  { PRINT_SUB( oiTESTRESULT_FAIL,
    "openiccExtractPathFromFileName_()                  " );
  }

  file_name = "../icc/OpenICC/sRGB.icc";
  t2 = openiccExtractPathFromFileName_( file_name );
  fprintf(zout, "file_name: %s\n", file_name );
  error = !t2 || (strlen(t2) >= strlen(file_name));
  if(!error)
  { PRINT_SUB( oiTESTRESULT_SUCCESS,
    "openiccExtractPathFromFileName_() %s", t2 );
  } else
  { PRINT_SUB( oiTESTRESULT_FAIL,
    "openiccExtractPathFromFileName_() %s               ", t2?t2:"" );
  }

  file_name = "~/.local/share/color/icc/OpenICC/sRGB.icc";
  t3 = openiccExtractPathFromFileName_( file_name );
  fprintf(zout, "file_name: %s\n", file_name );
  error = !t3 || (strlen(t3) >= strlen(file_name));
  if(!error)
  { PRINT_SUB( oiTESTRESULT_SUCCESS,
    "openiccExtractPathFromFileName_() %s", t3 );
  } else
  { PRINT_SUB( oiTESTRESULT_FAIL,
    "openiccExtractPathFromFileName_() %s               ", t3?t3:"" );
  }

  if(openiccIsDirFull_(t1))
  { PRINT_SUB( oiTESTRESULT_SUCCESS,
    "openiccIsDirFull_() %s", t1 );
  } else
  { PRINT_SUB( oiTESTRESULT_FAIL,
    "openiccIsDirFull_() %s               ", t1?t1:"" );
  }

  if(openiccIsDirFull_(t2))
  { PRINT_SUB( oiTESTRESULT_FAIL,
    "openiccIsDirFull_() ! %s", t2 );
  } else
  { PRINT_SUB( oiTESTRESULT_SUCCESS,
    "openiccIsDirFull_() ! %s               ", t2?t2:"" );
  }

  if(t3) free(t3);

  t3 = oyPathGetParent_(OPENICC_DEVICE_PATH);
  fprintf(zout, "name: %s\n", OPENICC_DEVICE_PATH );
  error = !t3 || (strlen(t3) >= strlen(OPENICC_DEVICE_PATH));
  if(!error)
  { PRINT_SUB( oiTESTRESULT_SUCCESS,
    "oyPathGetParent_() %s        ", t3 );
  } else
  { PRINT_SUB( oiTESTRESULT_FAIL,
    "oyPathGetParent_() %s               ", t3?t3:"" );
  }

  if(t1) free(t1);
  if(t2) free(t2);
  if(t3) free(t3);

  size_t size = 0;
  file_name = "../../../test.json";
  t1 = openiccOpenFile( file_name, &size );
  if(t1)
  { PRINT_SUB( oiTESTRESULT_SUCCESS,
    "openiccOpenFile() &size %u                       ", size );
  } else
  { PRINT_SUB( oiTESTRESULT_FAIL,
    "openiccOpenFile() %s    ", file_name );
  }
  if(t1) free(t1);


  file_name = "test.txt";
  size = openiccWriteFile( file_name,
                           OPENICC_DEVICE_PATH,
                           strlen(OPENICC_DEVICE_PATH) + 1 );
  if(size)
  { PRINT_SUB( oiTESTRESULT_SUCCESS,
    "openiccWriteFile() size %u                         ", size );
  } else
  { PRINT_SUB( oiTESTRESULT_FAIL,
    "openiccWriteFile() %s                ", file_name );
  }

  FILE * fp = fopen( file_name, "r" );
  t1 = openiccReadFileSToMem( fp, &size );
  if(t1)
  { PRINT_SUB( oiTESTRESULT_SUCCESS,
    "openiccReadFileSToMem() &size %u                 ", size );
  } else
  { PRINT_SUB( oiTESTRESULT_FAIL,
    "openiccReadFileSToMem() %s    ", file_name );
  }
  if(t1) free(t1);
  fclose(fp);

  return result;
}

oiTESTRESULT_e testStringRun ()
{
  oiTESTRESULT_e result = oiTESTRESULT_UNKNOWN;

  fprintf(stdout, "\n" );

  int error = 0;

  char * t = NULL;
  OpeniccConfigs_s * configs = NULL;

  error = openiccMessage_p( openiccMSG_WARN, configs, "test message %s", OPENICC_VERSION_NAME );

  if( !error )
  { PRINT_SUB( oiTESTRESULT_SUCCESS,
    "openiccMessage_p()...                              " );
  } else
  { PRINT_SUB( oiTESTRESULT_FAIL,
    "openiccMessage_p()...                              " );
  }


  openiccStringAddPrintf_( &t, OPENICC_BASE_PATH "%s", "/behaviour");
  if( t && strlen(t) > strlen(OPENICC_BASE_PATH) )
  { PRINT_SUB( oiTESTRESULT_SUCCESS,
    "openiccStringAddPrintf_() %s", t );
  } else
  { PRINT_SUB( oiTESTRESULT_FAIL,
    "openiccStringAddPrintf_() ...                      " );
  }

  STRING_ADD( t, "/rendering_intent" );
  if( t && strlen(t) > 40 )
  { PRINT_SUB( oiTESTRESULT_SUCCESS,
    "openiccStringAdd_() %s", t );
  } else
  { PRINT_SUB( oiTESTRESULT_FAIL,
    "openiccStringAdd_() ...                            " );
  }

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
  //TEST_RUN( testDB, "DB" );
  TEST_RUN( testIO, "file i/o" );
  TEST_RUN( testStringRun, "String handling" );
  //TEST_RUN( testPaths, "Paths" );

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

