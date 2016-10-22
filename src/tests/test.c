/** @file test.c
 *
 *  libOpenICC - OpenICC Colour Management Configuration
 *
 *  Copyright (C) 2011-2016  Kai-Uwe Behrmann
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
void  myDeAllocFunc(void* data) { delete data[]; }
#else
void* myAllocFunc(size_t size) { return calloc(size,1); }
void  myDeAllocFunc(void* data) { free(data); }
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

oiTESTRESULT_e testI18N()
{
  const char * lang = 0;
  oiTESTRESULT_e result = oiTESTRESULT_UNKNOWN;

  fprintf(stdout, "\n" );

  setlocale(LC_ALL,"");
  openiccInit();

  lang = setlocale(LC_ALL, NULL);
  if(lang && (strcmp(lang, "C") != 0))
  { PRINT_SUB( oiTESTRESULT_SUCCESS, 
    "setlocale() initialised good %s            ", lang );
  } else
  { PRINT_SUB( oiTESTRESULT_XFAIL, 
    "setlocale() initialised failed %s          ", lang );
  }

  return result;
}

#if 0
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
#endif


oiTESTRESULT_e testPaths()
{
  oiTESTRESULT_e result = oiTESTRESULT_UNKNOWN;

  fprintf(stdout, "\n" );

  const char * type_names[] = {
    "openiccPATH_NONE", "openiccPATH_ICC", "openiccPATH_POLICY", "openiccPATH_MODULE", "openiccPATH_SCRIPT", "openiccPATH_CACHE"
  };
  openiccPATH_TYPE_e types[] = {
    openiccPATH_NONE, openiccPATH_ICC, openiccPATH_POLICY, openiccPATH_MODULE, openiccPATH_SCRIPT, openiccPATH_CACHE
  };
  const char * scope_names[] = {
    "openiccSCOPE_USER_SYS", "openiccSCOPE_USER", "openiccSCOPE_SYSTEM", "openiccSCOPE_OPENICC", "openiccSCOPE_MACHINE"
  };
  openiccSCOPE_e scopes[] = {
    openiccSCOPE_USER_SYS, openiccSCOPE_USER, openiccSCOPE_SYSTEM, (openiccSCOPE_e)openiccSCOPE_OPENICC, (openiccSCOPE_e)openiccSCOPE_MACHINE
  };
  int i,j;

  for(i = 1; i <= 5; ++i)
  for(j = 1; j <= 4; ++j)
  {
  char * text = openiccGetInstallPath( types[i], scopes[j], malloc );
  if(text)
  { PRINT_SUB( oiTESTRESULT_SUCCESS,
    "openiccGetInstallPath( %s, %s ): %s", type_names[i],scope_names[j],
                                                openiccNoEmptyString_m_(text) );
    free(text);
  } else
  { PRINT_SUB( oiTESTRESULT_XFAIL,
    "openiccGetInstallPath( %s, %s ): %s", type_names[i],scope_names[j],
                                                openiccNoEmptyString_m_(text) );
  }
  }


  return result;
}

const char * oiGetConfigFileName()
{
  const char * file_name = "../../../OpenICC_device_config_DB.json";
  FILE * fp = fopen( file_name, "r" );

  if(!fp)
  {
    file_name = "../OpenICC_device_config_DB.json";
    fp = fopen( file_name, "r" );
    if(!fp)
    {
      file_name = "OpenICC_device_config_DB.json";
      fp = fopen( file_name, "r" );
      if(!fp)
      {
        file_name = "../../../config/OpenICC_device_config_DB.json";
        fp = fopen( file_name, "r" );
        if(!fp)
          file_name = NULL;
      }
    }
  }

  if(fp)
  {
    fclose(fp);
    fprintf(zout, "\tfile_name: %s\n", file_name );
  }

  return file_name;
}


oiTESTRESULT_e testIO ()
{
  oiTESTRESULT_e result = oiTESTRESULT_UNKNOWN;

  fprintf(stdout, "\n" );

  int error = 0;

  char * t1, *t2, *t3;
  const char * file_name = "/usr/share/color/icc/OpenICC/sRGB.icc";
  size_t size = 0;
  FILE * fp;

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

  if(openiccIsDirFull_("/usr/share/color/icc"))
  { PRINT_SUB( oiTESTRESULT_SUCCESS,
    "openiccIsDirFull_() %s", "/usr/share/color/icc" );
  } else
  { PRINT_SUB( oiTESTRESULT_FAIL,
    "openiccIsDirFull_() %s               ", "/usr/share/color/icc" );
  }

  if(openiccIsDirFull_(t2))
  { PRINT_SUB( oiTESTRESULT_FAIL,
    "openiccIsDirFull_() ! %s", t2 );
  } else
  { PRINT_SUB( oiTESTRESULT_SUCCESS,
    "openiccIsDirFull_() ! %s               ", t2?t2:"" );
  }

  if(t3) free(t3);

  t3 = openiccPathGetParent_(OPENICC_DEVICE_PATH);
  fprintf(zout, "name: %s\n", OPENICC_DEVICE_PATH );
  error = !t3 || (strlen(t3) >= strlen(OPENICC_DEVICE_PATH));
  if(!error)
  { PRINT_SUB( oiTESTRESULT_SUCCESS,
    "openiccPathGetParent_() %s        ", t3 );
  } else
  { PRINT_SUB( oiTESTRESULT_FAIL,
    "openiccPathGetParent_() %s               ", t3?t3:"" );
  }

  if(t1) free(t1);
  if(t2) free(t2);
  if(t3) free(t3);

  file_name = oiGetConfigFileName();
  t1 = openiccOpenFile( file_name, &size );
  if(t1)
  { PRINT_SUB( oiTESTRESULT_SUCCESS,
    "openiccOpenFile() &size %u                       ", (unsigned)size );
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
    "openiccWriteFile() size %u                         ", (unsigned)size );
  } else
  { PRINT_SUB( oiTESTRESULT_FAIL,
    "openiccWriteFile() %s                ", file_name );
  }

  fp = fopen( file_name, "r" );
  t1 = openiccReadFileSToMem( fp, &size );
  if(t1)
  { PRINT_SUB( oiTESTRESULT_SUCCESS,
    "openiccReadFileSToMem() &size %u                 ", (unsigned)size );
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
  openiccConfig_s * config = NULL;

  openiccMessageFuncSet( openiccMessageFunc );
  error = openiccMessage_p( openiccMSG_WARN, config, "test message %s", OPENICC_VERSION_NAME );

  if( !error )
  { PRINT_SUB( oiTESTRESULT_SUCCESS,
    "openiccMessage_p()...                              " );
  } else
  { PRINT_SUB( oiTESTRESULT_FAIL,
    "openiccMessage_p()...                              " );
  }


  openiccStringAddPrintf( &t, OPENICC_BASE_PATH "%s", "/behaviour");
  if( t && strlen(t) > strlen(OPENICC_BASE_PATH) )
  { PRINT_SUB( oiTESTRESULT_SUCCESS,
    "openiccStringAddPrintf() %s", t );
  } else
  { PRINT_SUB( oiTESTRESULT_FAIL,
    "openiccStringAddPrintf() ...                      " );
  }

  STRING_ADD( t, "/rendering_intent" );
  if( t && strlen(t) > 40 )
  { PRINT_SUB( oiTESTRESULT_SUCCESS,
    "openiccStringAdd_() %s", t );
  } else
  { PRINT_SUB( oiTESTRESULT_FAIL,
    "openiccStringAdd_() ...                            " );
  }
  if(t) free(t);

  return result;
}

oiTESTRESULT_e testConfig()
{
  oiTESTRESULT_e result = oiTESTRESULT_UNKNOWN;
  const char * top_key_name = OPENICC_DEVICE_PATH;
  openiccConfig_s * config;
  const char * file_name;
  char * text;
  size_t size = 0;
  int key_names_n = 0, values_n = 0,i,error = 0;
  char ** key_names, ** values;
  const char * base_key = "org/freedesktop/openicc/device/camera/[1]";
  oyjl_val v = NULL;
  char * json = NULL;
  int level = 0;

  fprintf(stdout, "\n" );

  file_name = oiGetConfigFileName();

  /* read JSON input file */
  text = openiccOpenFile( file_name, &size );

  /* parse json and write back */
  v = oyjl_tree_parse( text, NULL, 0 );
  oyjl_tree_to_json( v, &level, &json );
  oyjl_tree_free( v ); v = NULL;
  if(text) free(text);

  /* parse JSON */
  config = openiccConfig_FromMem( json );
  if(json) free(json);
  openiccConfig_SetInfo ( config, file_name );

  if(config)
  { PRINT_SUB( oiTESTRESULT_SUCCESS, 
    "config created                                 " );
  } else
  { PRINT_SUB( oiTESTRESULT_FAIL, 
    "config created                                 " );
  }

  /* Get key names below some point in the JSON tree. */
  error = openiccConfig_GetKeyNames( config, base_key, 0,
                                     myAllocFunc, &key_names, &key_names_n );
  if(key_names_n)
  { PRINT_SUB( oiTESTRESULT_SUCCESS,
    "openiccConfig_GetKeyNames()                  %d", key_names_n );
  } else
  { PRINT_SUB( oiTESTRESULT_FAIL,
    "openiccConfig_GetKeyNames()                    " );
  }

  /* Get values for the above key names. */
  error = openiccConfig_GetStrings( config, (const char **)key_names,
                                    myAllocFunc, &values, &values_n );
  i = 0;
  while(values && values[i]) ++i;
  if(key_names_n == values_n && values_n == i)
  { PRINT_SUB( oiTESTRESULT_SUCCESS,
    "openiccConfig_GetStrings()             %d==%d==%d", key_names_n,values_n,i );
  } else
  { PRINT_SUB( oiTESTRESULT_FAIL,
    "openiccConfig_GetStrings()             %d==%d==%d", key_names_n,values_n,i );
  }

  for(i = 0; i < key_names_n; ++i)
  {
    /* Get a single value from the config object by conviniently contructing
     * the key name.
     */
    const char * t = NULL;
    openiccConfig_GetStringf( config, &t, "%s/[%d]", base_key, i );
    if(!t)
    { PRINT_SUB( oiTESTRESULT_FAIL,
    "openiccConfig_GetString()                      " );
    }
    fprintf(zout, "\t%s:\t\"%s\"\n", key_names[i]?key_names[i]:"????", t?t:"????" );
    myDeAllocFunc( key_names[i] );
    if(values && values[i]) myDeAllocFunc(values[i]);
  }

  if( key_names ) myDeAllocFunc(key_names); key_names = NULL;
  if( values ) myDeAllocFunc(values); values = NULL;


  /* get all key names */
  error = openiccConfig_GetKeyNames( config, "org", 0,
                                     myAllocFunc, &key_names, &i );
  if(key_names_n < i )
  { PRINT_SUB( oiTESTRESULT_SUCCESS,
    "openiccConfig_GetKeyNames(many levels)      %d<%d", key_names_n,i );
  } else
  { PRINT_SUB( oiTESTRESULT_FAIL,
    "openiccConfig_GetKeyNames(many levels)      %d<%d", key_names_n,i );
  }
  i = 0;
  while(key_names && key_names[i]) myDeAllocFunc( key_names[i++] );
  if( key_names ) myDeAllocFunc(key_names); key_names = NULL;


  /* get one key name */
  error = openiccConfig_GetKeyNames( config, "org", 1,
                                     myAllocFunc, &key_names, &i );
  if(i == 1)
  { PRINT_SUB( oiTESTRESULT_SUCCESS,
    "openiccConfig_GetKeyNames(\"org\",one level) 1 == %d", i );
  } else
  { PRINT_SUB( oiTESTRESULT_FAIL, 
    "openiccConfig_GetKeyNames(\"org\",one level) 1 == %d", i );
  }
  i = 0;
  fprintf(zout, "\t%s\n", key_names[i]?key_names[i]:"????" );
  while(key_names && key_names[i]) myDeAllocFunc( key_names[i++] );
  if( key_names ) myDeAllocFunc(key_names); key_names = NULL;



  openiccConfig_Release( &config );

  return result;
}

oiTESTRESULT_e testDeviceJSON ()
{
  oiTESTRESULT_e result = oiTESTRESULT_UNKNOWN;

  openiccConfig_s * config, * config2;
  const char * file_name;
  char * text = 0;
  char            ** keys = 0;
  char            ** values = 0;
  int i,j, n = 0, devices_n, flags;
  char * json, * full_json = NULL, * device_class;
  const char * devices_filter[] = {OPENICC_DEVICE_CAMERA,NULL},
             * old_device_class = NULL,
             * d = NULL;
  size_t size = 0;

  const char * non_json = "{\"org\":{\"free{\"openicc\")))";

  fprintf(stdout, "\n" );

  config = openiccConfig_FromMem( non_json );
  if( !config )
  { PRINT_SUB( oiTESTRESULT_SUCCESS,
    "openiccConfig_FromMem(\"%s\") ", non_json );
  } else
  { PRINT_SUB( oiTESTRESULT_XFAIL,
    "openiccConfig_FromMem(\"%s\") ", non_json );
  }
  openiccConfig_Release( &config );

  file_name = oiGetConfigFileName();
  /* read JSON input file */
  text = openiccOpenFile( file_name, &size );

  /* parse JSON */
  config = openiccConfig_FromMem( text );
  if(text) free(text);
  openiccConfig_SetInfo ( config, file_name );
  devices_n = openiccConfig_DevicesCount(config, NULL);
  fprintf( zout, "Found %d devices.\n", devices_n );
  if( devices_n )
  { PRINT_SUB( oiTESTRESULT_SUCCESS,
    "openiccConfig_FromMem(\"%s\") %d ", file_name, devices_n );
  } else
  { PRINT_SUB( oiTESTRESULT_XFAIL,
    "openiccConfig_FromMem()...                        " );
  }


  
  /* print all found key/value pairs */
  for(i = 0; i < devices_n; ++i)
  {
    const char * d = openiccConfig_DeviceGet( config, NULL, i,
                                               &keys, &values, malloc );

    if(i && openicc_debug)
      fprintf( zout,"\n");

    n = 0; if(keys) while(keys[n]) ++n;
    fprintf( zout, "[%d] device class:\"%s\" with %d key/value pairs\n", i, d, n);
    for( j = 0; j < n; ++j )
    {
      if(openicc_debug)
      fprintf(zout, "%s:\"%s\"\n", keys[j], values[j]);
      free(keys[j]);
      free(values[j]);
    }
    free(keys); free(values);
  }
  fprintf(zout, "\n" );

  /* get a single JSON device */
  i = 2; /* select the second one, we start counting from zero */
  d = openiccConfig_DeviceGetJSON ( config, NULL, i, 0,
                                     old_device_class, &json, malloc );
  config2 = openiccConfig_FromMem( json );
  openiccConfig_SetInfo ( config2, file_name );
  device_class = openiccConfig_DeviceClassGet( config2, malloc );
  if( strcmp(device_class,"camera") == 0 )
  { PRINT_SUB( oiTESTRESULT_SUCCESS,
    "openiccConfig_DeviceClassGet([%d]) %s      ", i, device_class );
  } else
  { PRINT_SUB( oiTESTRESULT_XFAIL,
    "openiccConfig_DeviceClassGet()...                 " );
  }
  if(json) free(json);
  if(device_class) free(device_class);
  openiccConfig_Release( &config2 );


  /* we want a single device class DB for lets say cameras */
  devices_n = openiccConfig_DevicesCount(config, devices_filter);
  if( devices_n == 2 )
  { PRINT_SUB( oiTESTRESULT_SUCCESS,
    "openiccConfig_DevicesCount(%s) %d          ", OPENICC_DEVICE_CAMERA, devices_n );
  } else
  { PRINT_SUB( oiTESTRESULT_XFAIL,
    "openiccConfig_DevicesCount()...                 " );
  }
  old_device_class = NULL;
  for(i = 0; i < devices_n; ++i)
  {
    flags = 0;
    if(i != 0) /* not the first */
      flags |= OPENICC_CONFIGS_SKIP_HEADER;
    if(i != devices_n - 1) /* not the last */
      flags |= OPENICC_CONFIGS_SKIP_FOOTER;

    d = openiccConfig_DeviceGetJSON( config, devices_filter, i, flags,
                                     old_device_class, &json, malloc );
    old_device_class = d;
    STRING_ADD( full_json, json );
    free(json);
  }
  openiccConfig_Release( &config );


  config = openiccConfig_FromMem( full_json );
  openiccConfig_SetInfo ( config, "full_json" );
  if(full_json) free(full_json);
  devices_n = openiccConfig_DevicesCount(config, NULL);
  if( devices_n == 2 )
  { PRINT_SUB( oiTESTRESULT_SUCCESS,
    "openiccConfig_DeviceGetJSON()                     " );
  } else
  { PRINT_SUB( oiTESTRESULT_XFAIL,
    "openiccConfig_DeviceGetJSON()                     " );
  }
  openiccConfig_Release( &config );


  return result;
}


#include "xdg_bds.h"
oiTESTRESULT_e testXDG()
{
  oiTESTRESULT_e result = oiTESTRESULT_UNKNOWN;
  const char * config_file = OPENICC_DB_PREFIX OPENICC_SLASH OPENICC_DB;
  int i;
  /* Locate the directories where the config file is, */
  /* and where we should copy the profile to. */
  int npaths;
  xdg_error er;
  char **paths;
  openiccSCOPE_e scope = openiccSCOPE_USER;

  fprintf(stdout, "\n" );

  if ((npaths = xdg_bds(&er, &paths, xdg_conf, xdg_write, 
                        (scope == openiccSCOPE_SYSTEM) ? xdg_local : xdg_user,
                        config_file)) == 0)
  {
    ERRc_S("%s %d", _("Could not find config"), scope );
    return 1;
  }

  if(openicc_debug)
    fprintf( zout, "%s\n", _("Paths:") );
  for(i=0; i < npaths; ++i)
    if(openicc_debug)
      fprintf( zout, "%s\n", paths[i]);

  xdg_free(paths, npaths);

  if(npaths == 1)
  { PRINT_SUB( oiTESTRESULT_SUCCESS, 
    "xdg_bds openiccSCOPE_USER found %d             ", npaths );
  } else
  { PRINT_SUB( oiTESTRESULT_FAIL, 
    "xdg_bds openiccSCOPE_USER found %d             ", npaths );
  }

  scope = openiccSCOPE_SYSTEM;
  if ((npaths = xdg_bds(&er, &paths, xdg_conf, xdg_write, 
                        (scope == openiccSCOPE_SYSTEM) ? xdg_local : xdg_user,
                        config_file)) == 0)
  {
    ERRc_S("%s %d", _("Could not find config"), scope );
  }

  if(openicc_debug)
    fprintf( zout, "%s\n", _("Paths:") );
  for(i=0; i < npaths; ++i)
    if(openicc_debug)
      fprintf( zout, "%s\n", paths[i]);

  xdg_free(paths, npaths);

  if(npaths == 1)
  { PRINT_SUB( oiTESTRESULT_SUCCESS, 
    "xdg_bds openiccSCOPE_SYSTEM found %d           ", npaths );
  } else
  { PRINT_SUB( oiTESTRESULT_FAIL, 
    "xdg_bds openiccSCOPE_SYSTEM found %d           ", npaths );
  }

  return result;
}

#include "openicc_db.h"
oiTESTRESULT_e testODB()
{
  oiTESTRESULT_e result = oiTESTRESULT_UNKNOWN;
  openiccSCOPE_e scope = openiccSCOPE_USER_SYS;
  const char * top_key_name = OPENICC_DEVICE_PATH;
  openiccDB_s * db = openiccDB_NewFrom( top_key_name, scope );

  fprintf(stdout, "\n" );

  if(db)
  { PRINT_SUB( oiTESTRESULT_SUCCESS, 
    "db created                                     " );
  } else
  { PRINT_SUB( oiTESTRESULT_FAIL, 
    "db created                                     " );
  }

  openiccDB_Release( &db );

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
  TEST_RUN( testI18N, "i18n" );
  TEST_RUN( testIO, "file i/o" );
  TEST_RUN( testStringRun, "String handling" );
  TEST_RUN( testConfig, "JSON handling" );
  TEST_RUN( testDeviceJSON, "Device JSON handling" );
  TEST_RUN( testPaths, "Paths" );
  TEST_RUN( testXDG, "XDG" );
  TEST_RUN( testODB, "ODB" );
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

