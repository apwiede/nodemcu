#include "flash_fs.h"
#include "c_string.h"

#include "spiffs.h"

int fs_mode2flag(const char *mode){
  if(c_strlen(mode)==1){
  	if(c_strcmp(mode,"w")==0)
  	  return FS_WRONLY|FS_CREAT|FS_TRUNC;
  	else if(c_strcmp(mode, "r")==0)
  	  return FS_RDONLY;
  	else if(c_strcmp(mode, "a")==0)
  	  return FS_WRONLY|FS_CREAT|FS_APPEND;
#if defined(SPIFFS_USE_ENCRYPT)
  	else if(c_strcmp(mode, "e")==0)
  	  return FS_RDONLY|FS_ENCRYPT;
#endif
  	else
  	  return FS_RDONLY;
  } else if (c_strlen(mode)==2){
  	if(c_strcmp(mode,"r+")==0)
  	  return FS_RDWR;
  	else if(c_strcmp(mode, "w+")==0)
  	  return FS_RDWR|FS_CREAT|FS_TRUNC;
  	else if(c_strcmp(mode, "a+")==0)
  	  return FS_RDWR|FS_CREAT|FS_APPEND;
#if defined(SPIFFS_USE_ENCRYPT)
  	else if(c_strcmp(mode, "e+")==0)
  	  return FS_RDWR|FS_CREAT|FS_ENCRYPT;
#endif
  	else
  	  return FS_RDONLY;
#if defined(SPIFFS_USE_ENCRYPT)
  } else if (c_strlen(mode)==3){
  	if(c_strcmp(mode,"re+")==0)
  	  return FS_RDWR|FS_ENCRYPT;
  	else if(c_strcmp(mode, "we+")==0)
  	  return FS_RDWR|FS_CREAT|FS_TRUNC|FS_ENCRYPT;
  	else if(c_strcmp(mode, "ae+")==0)
  	  return FS_RDWR|FS_CREAT|FS_APPEND|FS_ENCRYPT;
  	else
  	  return FS_RDONLY;
#endif
  } else {
  	return FS_RDONLY;
  }
}
