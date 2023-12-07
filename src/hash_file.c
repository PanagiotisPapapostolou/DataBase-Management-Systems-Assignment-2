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

// char* intToBinary(int num, int depth) {
//     int max_bits = depth;

//     if (num == 0) {
//         char* binaryStr = (char*)malloc(2 * sizeof(char));
//         strcpy(binaryStr, "0");
//         return binaryStr;
//     }

//     int binary[32]; // Assuming 32-bit integers
//     int index = 0;

//     while (num > 0) {
//         binary[index++] = num % 2;
//         num = num / 2;
//     }

//     char* binaryStr = (char*)malloc((index + 1) * sizeof(char));
//     int strIndex = 0;

//     for (int i = index - 1; i >= 0; i--) {
//         binaryStr[strIndex++] = binary[i] + '0';
//     }

//     binaryStr[strIndex] = '\0';

//     if (strlen(binaryStr) < max_bits) {
//       char* remaining = (char*)malloc(sizeof(char)*(max_bits - strlen(binaryStr)));
//       for (int i = 0; i < max_bits - strlen(binaryStr); i++)
//         remaining[i] = '0';
      
//       strcat(remaining, binaryStr);
//       return remaining;
//     }

//     return binaryStr;
// }

int checkNBits(int num, int n) {
    int numBits = sizeof(num) ;
    
    // Ensure that n is within a valid range
    if (n <= 0 || n > numBits) {
        printf("Invalid value of n. It should be between 1 and %d\n", numBits);
        return 0;
    }
    // Use bitwise AND and right shift to get the most significant bits
    unsigned int result = num >> (numBits - n);
    return result;
}



int hash_function(int id, int depth) {
  return checkNBits(id, depth);;

}



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
    strcpy(fd->opened_files[fd->num_of_files].filename, filename);
    fd->opened_files[fd->num_of_files].num_of_buckets = 2;
    fd->opened_files[fd->num_of_files].bucket_infos = (Bucket_info*)malloc(sizeof(Bucket_info) * 2);
    fd->opened_files[fd->num_of_files].hash_table = (int*)malloc(sizeof(int) * 2);
    fd->opened_files[fd->num_of_files].table_size=2;
    return HT_OK;
  }
  else {
    return HT_ERROR;
  }
  
}

HT_ErrorCode HT_OpenIndex(const char *fileName, int *indexDesc){
  if(fd->num_of_files+1<MAX_OPEN_FILES) {\
    int flag=1;
    if(strcmp(fd->opened_files[fd->num_of_files].filename, fileName)!=0){
      
      for(int i=0; i<fd->num_of_files;i++) {
        if(strcmp(fd->opened_files[i].filename, fileName)==0) {
          fd->opened_files[fd->num_of_files].global_depth=fd->opened_files[i].global_depth;
          fd->opened_files[fd->num_of_files].capacity=BF_BLOCK_SIZE/sizeof(Record);
          strcpy(fd->opened_files[fd->num_of_files].filename,fileName);
          fd->opened_files[fd->num_of_files].file_desc=fd->opened_files[i].file_desc;
          *indexDesc=fd->num_of_files;
          
          for (int j = 0; j < fd->opened_files[i].num_of_buckets; j++)
            fd->opened_files[fd->num_of_files].bucket_infos[j].local_depth = fd->opened_files[i].bucket_infos[j].local_depth;
          
          for (int j = 0; j < fd->opened_files[i].table_size; j++)
            fd->opened_files[fd->num_of_files].hash_table[j] = fd->opened_files[i].hash_table[j];
          
          fd->num_of_files++;
          flag=0;

          break;
        }
      }
    }
    if(flag){
      int file_desc;
      CALL_BF(BF_OpenFile(fileName, &file_desc));
      for (int j = 0; j < 2; j++) {
        fd->opened_files[fd->num_of_files].bucket_infos[j].local_depth = 1;

        BF_Block* new_block;
        BF_Block_Init(&new_block);
        CALL_BF(BF_AllocateBlock(file_desc, new_block));
        CALL_BF(BF_UnpinBlock(new_block));
      }
      fd->opened_files[fd->num_of_files].hash_table[0] =0;
      fd->opened_files[fd->num_of_files].hash_table[1] =1;
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
  int  index = hash_function(record.id, fd->opened_files[indexDesc].global_depth);
  if(fd->opened_files[indexDesc].bucket_infos[fd->opened_files[indexDesc].hash_table[index]].num_of_records==fd->opened_files[indexDesc].capacity) {
    if(fd->opened_files[indexDesc].bucket_infos[fd->opened_files[indexDesc].hash_table[index]].local_depth < fd->opened_files[indexDesc].global_depth ) {
      fd->opened_files[indexDesc].num_of_buckets++;
      fd->opened_files[indexDesc].bucket_infos=(Bucket_info*)realloc(fd->opened_files[indexDesc].bucket_infos, (fd->opened_files[indexDesc].num_of_buckets)*sizeof(Bucket_info));
      fd->opened_files[indexDesc].bucket_infos[fd->opened_files[indexDesc].num_of_buckets-1].num_of_records=0;
      fd->opened_files[indexDesc].bucket_infos[fd->opened_files[indexDesc].num_of_buckets-1].local_depth=fd->opened_files[indexDesc].global_depth;
      int flag=0;
      BF_Block* new_block, *cur_block;
      BF_Block_Init(&new_block);
      BF_Block_Init(&cur_block);
      CALL_BF(BF_AllocateBlock(fd->opened_files[indexDesc].file_desc, new_block));

      CALL_BF(BF_GetBlock(fd->opened_files[indexDesc].file_desc, fd->opened_files[indexDesc].hash_table[index], cur_block));

      for (int i = 0; i < fd->opened_files[indexDesc].table_size; i++) {
        if (fd->opened_files[indexDesc].hash_table[i] == fd->opened_files[indexDesc].hash_table[index]) {
            void* cur_data = BF_Block_GetData(cur_block);
            Record* records = cur_data;
            for(int j=0;j<fd->opened_files[indexDesc].bucket_infos[fd->opened_files[indexDesc].hash_table[index]].num_of_records;j++) {

              int count=0;
              int new_index=hash_function(records[j].id, fd->opened_files[indexDesc].global_depth);
              if (new_index != index) {
                fd->opened_files[indexDesc].hash_table[new_index]=fd->opened_files[indexDesc].num_of_buckets;
                fd->opened_files[indexDesc].bucket_infos[fd->opened_files[indexDesc].hash_table[index]].num_of_records--;
                
                void* new_data = BF_Block_GetData(new_block);
                Record* rec=new_data; 
                rec[fd->opened_files[indexDesc].bucket_infos[fd->opened_files[indexDesc].num_of_buckets-1].num_of_records]=records[j];

                fd->opened_files[indexDesc].bucket_infos[fd->opened_files[indexDesc].num_of_buckets-1].num_of_records++;
              }
              else{
                records[count]=records[j];
                count++;
              }
            }
            
        }
      }
    }

  }
  else {
      
      BF_Block* block;
      BF_Block_Init(&block);
      CALL_BF(BF_GetBlock(fd->opened_files[indexDesc].file_desc, fd->opened_files[indexDesc].hash_table[index], block));
      void* data=BF_Block_GetData(block);
      Record* rec=data;
      rec[fd->opened_files[indexDesc].bucket_infos[fd->opened_files[indexDesc].hash_table[index]].num_of_records]=record;
      fd->opened_files[indexDesc].bucket_infos[fd->opened_files[indexDesc].hash_table[index]].num_of_records++;
      BF_Block_SetDirty(block);
      CALL_BF(BF_UnpinBlock(block));

  }
  return HT_OK;
}

HT_ErrorCode HT_PrintAllEntries(int indexDesc, int *id) {
  //insert code here
  return HT_OK;
}
