#define OPENICC_BASE_PATH "org/freedesktop/openicc/device"
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
  "          \"%s\": {\n" \
  "            \"%d\": {\n"
#define OPENICC_DEVICE_JSON_FOOTER \
  "            }\n" \
  "          }\n" \
  "        }\n" \
  "      }\n" \
  "    }\n" \
  "  }\n" \
  "}\n"

typedef void * (*OpeniccConfigAlloc_f)(size_t              size );

typedef struct OpeniccConfigs_s OpeniccConfigs_s;

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
char *             openiccConfigs_DeviceGetJSON (
                                       OpeniccConfigs_s  * configs,
                                       const char       ** device_classes,
                                       int                 pos,
                                       int                 flags,
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

