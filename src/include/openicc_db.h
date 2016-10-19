/*  @file openicc_db.h
 *
 *  libOpenICC - OpenICC Colour Management Configuration
 *
 *  @par Copyright:
 *            2015-2016 (C) Kai-Uwe Behrmann
 *
 *  @brief    OpenICC Colour Management configuration helpers
 *  @author   Kai-Uwe Behrmann <ku.b@gmx.de>
 *  @par License:
 *            MIT <http://www.opensource.org/licenses/mit-license.php>
 *  @since    2015/08/26
 */

#ifndef __OPENICC_DB_H__
#define __OPENICC_DB_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/** \addtogroup OpenICC_config

 *  @{
 */

typedef struct openiccDB_s openiccDB_s;
openiccDB_s * openiccDB_NewFrom      ( const char        * top_key_name,
                                       openiccSCOPE_e      scope );
void     openiccDB_Release           ( openiccDB_s      ** db );
char *   openiccDB_GetString         ( openiccDB_s       * db,
                                       const char        * key_name );
int      openiccDB_GetStrings        ( openiccDB_s       * db,
                                       const char       ** key_names,
                                       char            *** values,
                                       openiccAlloc_f      allocFunc );
char **  openiccDB_GetKeyNames       ( openiccDB_s       * db,
                                       const char        * key_name,
                                       int               * n );
char **  openiccDB_GetKeyNamesOneLevel(openiccDB_s       * db,
                                       const char        * key_name,
                                       int               * n );
  
/* DB key wrappers */
int      openiccDBSetString_         ( const char        * keyName,
                                       openiccSCOPE_e      scope,
                                       const char        * value,
                                       const char        * comment );
char*    openiccDBSearchEmptyKeyname_( const char        * keyParentName,
                                       openiccSCOPE_e      scope );
int      openiccDBEraseKey_          ( const char        * key_name,
                                       openiccSCOPE_e      scope );

/** 
 *  @} *//*OpenICC_config
 */

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */


#endif /* __OPENICC_DB_H__ */
