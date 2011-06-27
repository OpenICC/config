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
  int error = 0;
  char            ** keys = 0;
  char            ** values = 0;
  int i,j, n = 0, devices_n;
  char * json, * device_class;

  if(argc == 1)
  {
    fprintf( stderr, "Usage: %s openicc.json\n\n", argv[0] );
    return 0;
  }

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
 

  configs = openiccConfigs_FromMem( text );
  openiccConfigs_SetInfo ( configs, file_name );
  devices_n = openiccConfigs_Count(configs, NULL);
  fprintf(stderr, "Found %d devices.\n", devices_n );

  

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

  json = openiccConfigs_DeviceGetJSON ( configs, NULL, 0, 0, malloc );
  config = openiccConfigs_FromMem( json );
  device_class = openiccConfigs_DeviceClassGet( config, malloc );
  fprintf( stderr, "\ndevice class: \"%s\"\n", device_class);
  printf( "%s\n", json );

  openiccConfigs_Release( &configs );

  return 0;
}

