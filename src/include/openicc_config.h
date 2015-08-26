/*  @file openicc_config.h
 *
 *  libOpenICC - OpenICC Colour Management Configuration
 *
 *  @par Copyright:
 *            2011-2015 (C) Kai-Uwe Behrmann
 *
 *  @brief    OpenICC Colour Management configuration helpers
 *  @internal
 *  @author   Kai-Uwe Behrmann <ku.b@gmx.de>
 *  @par License:
 *            MIT <http://www.opensource.org/licenses/mit-license.php>
 *  @since    2011/06/27
 */

#ifndef __OPENICC_CONFIG_H__
#define __OPENICC_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/** \addtogroup OpenICC_config OpenICC Color Management Configuration API's

 *  @{
 */

#define OPENICC_DB_PREFIX "color/settings"
#define OPENICC_DB "openicc.json"

#define OPENICC_BASE_PATH "org/freedesktop/openicc"
#define OPENICC_DEVICE_PATH OPENICC_BASE_PATH "/device"
#define OPENICC_DEVICE_MONITOR "monitor"
#define OPENICC_DEVICE_SCANNER "scanner"
#define OPENICC_DEVICE_PRINTER "printer"
#define OPENICC_DEVICE_CAMERA  "camera"
#define OPENICC_DEVICE_JSON_HEADER \
  "{\n" \
  "  \"org\": {\n" \
  "    \"freedesktop\": {\n" \
  "      \"openicc\": {\n" \
  "        \"device\": {\n" \
  "          \"%s\": [{\n"
#define OPENICC_DEVICE_JSON_FOOTER \
  "          ]\n" \
  "        }\n" \
  "      }\n" \
  "    }\n" \
  "  }\n" \
  "}\n"

typedef void * (*OpeniccConfigAlloc_f)(size_t              size );

typedef struct OpeniccConfigs_s OpeniccConfigs_s;

/** @brief customisable messages */
extern int openicc_debug;
extern int openicc_backtrace;

typedef enum {
  openiccMSG_ERROR = 300,              /**< @brief fatal user messages */
  openiccMSG_WARN,                     /**< @brief log messages */
  openiccMSG_DBG,                      /**< @brief developer messages */
} openiccMSG_e;

typedef int  (*openiccMessage_f)     ( openiccMSG_e        error_code,
                                       OpeniccConfigs_s  * context_object,
                                       const char        * format,
                                       ... );
int            openiccMessageFuncSet ( openiccMessage_f    message_func );

OpeniccConfigs_s * openiccConfigs_FromMem (
                                       const char        * data );
void               openiccConfigs_Release (
                                       OpeniccConfigs_s ** configs );

int                openiccConfigs_Count (
                                       OpeniccConfigs_s  * configs,
                                       const char       ** device_classes );
void               openiccConfigs_SetInfo (
                                       OpeniccConfigs_s  * configs,
                                       const char        * debug_info );
const char *       openiccConfigs_DeviceGet (
                                       OpeniccConfigs_s  * configs,
                                       const char       ** device_classes,
                                       int                 pos,
                                       char            *** keys,
                                       char            *** values,
                                       OpeniccConfigAlloc_f alloc );
#define OPENICC_CONFIGS_SKIP_HEADER 0x01
#define OPENICC_CONFIGS_SKIP_FOOTER 0x02
const char *       openiccConfigs_DeviceGetJSON (
                                       OpeniccConfigs_s  * configs,
                                       const char       ** device_classes,
                                       int                 pos,
                                       int                 flags,
                                       const char        * device_class,
                                       char             ** json,
                                       OpeniccConfigAlloc_f alloc );
char *             openiccConfigs_DeviceClassGet (
                                       OpeniccConfigs_s  * config,
                                       OpeniccConfigAlloc_f alloc );
int                openiccConfigs_Search (
                                       OpeniccConfigs_s  * config,
                                       const char       ** keys,
                                       const char       ** values,
                                       int              ** hits,
                                       OpeniccConfigAlloc_f alloc );
const char** const openiccConfigs_GetClasses (
                                       const char       ** device_classes,
                                       int               * count );

/** 
 *  @} *//*OpenICC_config
 */

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */


#endif /* __OPENICC_CONFIG_H__ */
