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
    // Calculate a mask with the first n bits set to 1
    int mask = ~0<<((sizeof(int)*8-n));

    // Use bitwise AND to check if the first n bits are set
    int maskedValue = num & mask;
    printf("%d\n", maskedValue);
    return maskedValue ;
}


int binaryToDecimal(int binaryNum) {
    int decimalNum = 0, base = 1, remainder;

    while (binaryNum > 0) {
        remainder = binaryNum % 10;
        decimalNum += remainder * base;
        binaryNum /= 10;
        base *= 2;
    }

    return decimalNum;
}

int hash_function(int id, int depth) {
  int bin_value= checkNBits(id, depth);
  return binaryToDecimal(bin_value);

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
    fd->opened_files[fd->num_of_files].oraia_petalouda = (Bucket_info*)malloc(sizeof(Bucket_info) * 2);
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
            fd->opened_files[fd->num_of_files].oraia_petalouda[j].local_depth = fd->opened_files[i].oraia_petalouda[j].local_depth;
          
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
        fd->opened_files[fd->num_of_files].oraia_petalouda[j].local_depth = 1;

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
  int  index = hash_function(record.id, 8);
  //printf("index= %d\n", index);

  return HT_OK;
}

HT_ErrorCode HT_PrintAllEntries(int indexDesc, int *id) {
  //insert code here
  return HT_OK;
}
