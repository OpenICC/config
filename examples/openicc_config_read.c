/**
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "openicc_config.h"

int main(int argc, char ** argv)
{
  OpeniccConfigs_s * configs, * config;
  const char * file_name = argc > 1 ? argv[1] : "test.json";
  FILE * fp = fopen(file_name,"rb");
  char * text = 0;
  size_t size = 0;
  char            ** keys = 0;
  char            ** values = 0;
  int i,j, n = 0, devices_n, flags;
  char * json, * device_class;
  const char * devices_filter[] = {OPENICC_DEVICE_CAMERA,NULL};

  /* show help text */
  if(argc == 1)
  {
    fprintf( stderr, "Usage: %s openicc.json\n\n", argv[0] );
    return 0;
  }

  /* read JSON input file */
  if(fp)
  { 
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
  }
 

  /* parse JSON */
  configs = openiccConfigs_FromMem( text );
  openiccConfigs_SetInfo ( configs, file_name );
  devices_n = openiccConfigs_Count(configs, NULL);
  fprintf(stderr, "Found %d devices.\n", devices_n );

  
  /* print all found key/value pairs */
  for(i = 0; i < devices_n; ++i)
  {
    const char * d = openiccConfigs_DeviceGet( configs, NULL, i,
                                               &keys, &values, malloc );

    if(i)
      fprintf( stderr,"\n");

    n = 0; if(keys) while(keys[n]) ++n;
    fprintf( stderr, "[%d] device class:\"%s\" with %d keys/values pairs\n", i, d, n);
    for( j = 0; j < n; ++j )
    {
      fprintf(stderr, "%s:\"%s\"\n", keys[j], values[j]);
      free(keys[j]);
      free(values[j]);
    }
    free(keys); free(values);
  }

  /* get a single JSON device */
  i = 1; /* select the second one, we start counting from zero */
  json = openiccConfigs_DeviceGetJSON ( configs, NULL, i, 0, malloc );
  config = openiccConfigs_FromMem( json );
  device_class = openiccConfigs_DeviceClassGet( config, malloc );
  fprintf( stderr, "\ndevice class[%d]: \"%s\"\n", i, device_class);
  printf( "%s\n", json );
  free(json);


  /* we want a single device class DB for lets say cameras */
  devices_n = openiccConfigs_Count(configs, devices_filter);
  fprintf(stderr, "Found %d %s devices.\n", devices_n, devices_filter[0] );
  for(i = 0; i < devices_n; ++i)
  {
    flags = 0;
    if(i != 0) /* not the first */
      flags |= OPENICC_CONFIGS_SKIP_HEADER;
    if(i != devices_n - 1) /* not the last */
      flags |= OPENICC_CONFIGS_SKIP_FOOTER;
    else /* end the current JSON array field and open the next one */
      printf("            },\n            {\n");

    json = openiccConfigs_DeviceGetJSON( configs, devices_filter, i,
                                         flags, malloc );

    printf( "%s\n", json );
    free(json);
  }

  openiccConfigs_Release( &configs );

  return 0;
}

