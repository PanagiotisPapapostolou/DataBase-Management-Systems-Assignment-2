#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "hash_file.h"
#define MAX_OPEN_FILES 20

#define CALL_BF(call)       \
{                           \
  BF_ErrorCode code = call; \
  if (code != BF_OK) {         \
    BF_PrintError(code);    \
    return HT_ERROR;        \
  }                         \
}

File_details* fd;



HT_ErrorCode HT_Init() {
  fd=(File_details*)malloc(sizeof(File_details));
  fd->opened_files=(HT_info*)malloc(MAX_OPEN_FILES*sizeof(HT_info));
  fd->num_of_files=0;
  return HT_OK;
}

HT_ErrorCode HT_CreateIndex(const char *filename, int depth) {
  
  
  CALL_BF(BF_CreateFile(filename));
  if(fd->num_of_files+1<MAX_OPEN_FILES) {
    fd->opened_files[fd->num_of_files].global_depth=depth;
    fd->opened_files[fd->num_of_files].capacity=BF_BLOCK_SIZE/sizeof(Record);
    printf("hi\n");
    printf("%d\n", fd->opened_files[fd->num_of_files].global_depth);
    strcpy(fd->opened_files[fd->num_of_files].filename,filename);
    fd->num_of_files++;
    return HT_OK;
  }
  else {
    return HT_ERROR;
  }
  
}

HT_ErrorCode HT_OpenIndex(const char *fileName, int *indexDesc){
  if(fd->num_of_files+1<MAX_OPEN_FILES) {
    int flag=1;
    for(int i=0; i<fd->num_of_files;i++) {
      if(strcmp(fd->opened_files[i].filename, fileName)==0) {
        fd->opened_files[fd->num_of_files].global_depth=fd->opened_files[i].global_depth;
        fd->opened_files[fd->num_of_files].capacity=BF_BLOCK_SIZE/sizeof(Record);
        strcpy(fd->opened_files[fd->num_of_files].filename,fileName);
        fd->opened_files[fd->num_of_files].file_desc=fd->opened_files[i].file_desc;
        *indexDesc=fd->num_of_files;
        fd->num_of_files++;
        flag=0; 
        break;
      }
    }
    if(flag){
      int file_desc;
      CALL_BF(BF_OpenFile(fileName, &file_desc));
      fd->opened_files[fd->num_of_files].file_desc=file_desc;
      *indexDesc=fd->num_of_files;
      fd->num_of_files++;
    }
    return HT_OK;
  }
  else {
    return HT_ERROR;
  }
    
}

HT_ErrorCode HT_CloseFile(int indexDesc) {
  //insert code here
  return HT_OK;
}

HT_ErrorCode HT_InsertEntry(int indexDesc, Record record) {
  //insert code here
  return HT_OK;
}

HT_ErrorCode HT_PrintAllEntries(int indexDesc, int *id) {
  //insert code here
  return HT_OK;
}
