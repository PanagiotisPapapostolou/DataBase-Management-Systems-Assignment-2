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

char* intToBinary(int num, int depth) {
    int max_bits = depth;

    if (num == 0) {
        char* binaryStr = (char*)malloc(2 * sizeof(char));
        strcpy(binaryStr, "0");
        return binaryStr;
    }

    int binary[32]; // Assuming 32-bit integers
    int index = 0;

    while (num > 0) {
        binary[index++] = num % 2;
        num = num / 2;
    }

    char* binaryStr = (char*)malloc((index + 1) * sizeof(char));
    int strIndex = 0;

    for (int i = index - 1; i >= 0; i--) {
        binaryStr[strIndex++] = binary[i] + '0';
    }

    binaryStr[strIndex] = '\0';

    if (strlen(binaryStr) < max_bits) {
      char* remaining = (char*)malloc(sizeof(char)*(max_bits - strlen(binaryStr)));
      for (int i = 0; i < max_bits - strlen(binaryStr); i++)
        remaining[i] = '0';
      
      strcat(remaining, binaryStr);
      return remaining;
    }

    return binaryStr;
}

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
    fd->opened_files[fd->num_of_files].hash_table = (hash_table_type*)malloc(sizeof(hash_table_type) * 2);
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
          
          for (int j = 0; j < fd->opened_files[i].table_size; j++) {
            fd->opened_files[fd->num_of_files].hash_table[j].block_id = fd->opened_files[i].hash_table[j].block_id;
            fd->opened_files[fd->num_of_files].hash_table[j].binary_id = fd->opened_files[i].hash_table[j].binary_id;
          }
          
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
      fd->opened_files[fd->num_of_files].hash_table[0].block_id = 0;
      fd->opened_files[fd->num_of_files].hash_table[0].binary_id = intToBinary(0, fd->opened_files[fd->num_of_files].global_depth);
      fd->opened_files[fd->num_of_files].hash_table[1].block_id = 1;
      fd->opened_files[fd->num_of_files].hash_table[1].binary_id = intToBinary(1, fd->opened_files[fd->num_of_files].global_depth);
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
  int file_desc=fd->opened_files[indexDesc].file_desc;
<<<<<<< HEAD
  CALL_BF(BF_Close(file_desc));
=======
  CALL_BF(BF_CloseFile(file_desc));
>>>>>>> 73cfb461fbbcdda49dd529b81f10bcc9729fc4a0
  free(&fd->opened_files[indexDesc]);
  
  return HT_OK;
}

HT_ErrorCode HT_InsertEntry(int indexDesc, Record record) {
  int  index = hash_function(record.id, fd->opened_files[indexDesc].global_depth);
  if(fd->opened_files[indexDesc].bucket_infos[fd->opened_files[indexDesc].hash_table[index].block_id].num_of_records==fd->opened_files[indexDesc].capacity) {
    if(fd->opened_files[indexDesc].bucket_infos[fd->opened_files[indexDesc].hash_table[index].block_id].local_depth < fd->opened_files[indexDesc].global_depth ) {
      printf("Hello1\n");
      fd->opened_files[indexDesc].num_of_buckets++;
      fd->opened_files[indexDesc].bucket_infos=(Bucket_info*)realloc(fd->opened_files[indexDesc].bucket_infos, (fd->opened_files[indexDesc].num_of_buckets)*sizeof(Bucket_info));
      fd->opened_files[indexDesc].bucket_infos[fd->opened_files[indexDesc].num_of_buckets-1].num_of_records=0;
      fd->opened_files[indexDesc].bucket_infos[fd->opened_files[indexDesc].num_of_buckets-1].local_depth=fd->opened_files[indexDesc].global_depth;
      int flag=0;
      BF_Block* new_block, *cur_block;
      BF_Block_Init(&new_block);
      BF_Block_Init(&cur_block);
      CALL_BF(BF_AllocateBlock(fd->opened_files[indexDesc].file_desc, new_block));

      CALL_BF(BF_GetBlock(fd->opened_files[indexDesc].file_desc, fd->opened_files[indexDesc].hash_table[index].block_id, cur_block));
      int count=0;
      for (int i = 0; i < fd->opened_files[indexDesc].table_size; i++) {
        if (fd->opened_files[indexDesc].hash_table[i].block_id == fd->opened_files[indexDesc].hash_table[index].block_id) {
            void* cur_data = BF_Block_GetData(cur_block);
            
            Record* records = cur_data;
            for(int j=0;j<fd->opened_files[indexDesc].bucket_infos[fd->opened_files[indexDesc].hash_table[index].block_id].num_of_records;j++) {

              
              int new_index=hash_function(records[j].id, fd->opened_files[indexDesc].global_depth);
              if (new_index != index) {
                fd->opened_files[indexDesc].hash_table[new_index].block_id=fd->opened_files[indexDesc].num_of_buckets;
                fd->opened_files[indexDesc].bucket_infos[fd->opened_files[indexDesc].hash_table[index].block_id].num_of_records--;
                fd->opened_files[indexDesc].bucket_infos[fd->opened_files[indexDesc].hash_table[index].block_id].local_depth++;
                
                void* new_data = BF_Block_GetData(new_block);
                Record* rec=new_data; 
                rec[fd->opened_files[indexDesc].bucket_infos[fd->opened_files[indexDesc].num_of_buckets-1].num_of_records]=records[j];
                fd->opened_files[indexDesc].hash_table[new_index].block_id = fd->opened_files[indexDesc].num_of_buckets-1;
                fd->opened_files[indexDesc].bucket_infos[fd->opened_files[indexDesc].num_of_buckets-1].num_of_records++;
              }
              else{
                records[count]=records[j];
                count++;
              }
            }
            
        }
      }
      void* onoufrios=BF_Block_GetData(cur_block);
      Record* rec=onoufrios;
      rec[count]=record;
      fd->opened_files[indexDesc].bucket_infos[fd->opened_files[indexDesc].hash_table[index].block_id].num_of_records=count+2;
      BF_Block_SetDirty(cur_block);
      BF_Block_SetDirty(new_block);
      CALL_BF(BF_UnpinBlock(cur_block));
      CALL_BF(BF_UnpinBlock(new_block));
    }
    else {
      hash_table_type* new_hash_table = (hash_table_type*)malloc(sizeof(hash_table_type)*2*fd->opened_files[indexDesc].table_size);
      
      for (int i = 0; i < fd->opened_files[indexDesc].table_size; i++) {
        new_hash_table[i].binary_id = (char*)malloc(sizeof(char) * 50);
        new_hash_table[i].block_id = fd->opened_files[indexDesc].hash_table[i].block_id;
        strcpy(new_hash_table[i].binary_id, fd->opened_files[indexDesc].hash_table[i].binary_id);
      }

      int counter = 0;
      for (int i = fd->opened_files[indexDesc].table_size; i < 2*fd->opened_files[indexDesc].table_size; i++) {
        new_hash_table[i].binary_id = new_hash_table[counter++].binary_id;
      }

      for (int i = 0; i < fd->opened_files[indexDesc].table_size; i++) {
        char* new_binary_id = (char*)malloc(strlen(new_hash_table[i].binary_id)+2);
        strcpy(new_binary_id, "0");
        strcat(new_binary_id, new_hash_table[i].binary_id);
        new_hash_table[i].binary_id = new_binary_id;
      }

      for (int i = fd->opened_files[indexDesc].table_size; i < 2*fd->opened_files[indexDesc].table_size; i++) {
        char* new_binary_id = (char*)malloc(strlen(new_hash_table[i].binary_id)+2);
        strcpy(new_binary_id, "1");
        strcat(new_binary_id, new_hash_table[i].binary_id);
        new_hash_table[i].binary_id = new_binary_id;
      }


      fd->opened_files[indexDesc].global_depth++;
      fd->opened_files[indexDesc].table_size *= 2;

      for (int i = 0; i < fd->opened_files[indexDesc].table_size; i++) {
        //printf("%s\n", new_hash_table[i].binary_id);
        char* cur_binary_id = new_hash_table[i].binary_id;
        char* prev_binary_id = (char*)malloc(sizeof(char) * fd->opened_files[indexDesc].global_depth);

        for (int j = 0; j < fd->opened_files[indexDesc].global_depth-1; j++) {

          prev_binary_id[j] = cur_binary_id[j];
        }


        int block_id;
        for (int j = 0; j < fd->opened_files[indexDesc].table_size/2; j++) {
          printf("%s=%s\n", new_hash_table[j].binary_id, prev_binary_id);
          if (strcmp(fd->opened_files[indexDesc].hash_table[j].binary_id, prev_binary_id) == 0) {
            block_id = fd->opened_files[indexDesc].hash_table[j].block_id;
            printf("Mpike%s\n", prev_binary_id);
            break;
          }
        }

        new_hash_table[i].block_id = block_id;
      }
      //free(fd->opened_files[indexDesc].hash_table);

      fd->opened_files[indexDesc].hash_table = (hash_table_type*)realloc(fd->opened_files[indexDesc].hash_table, sizeof(hash_table_type) * fd->opened_files[indexDesc].table_size);
      fd->opened_files[indexDesc].hash_table = new_hash_table;
      for(int i=0;i<fd->opened_files[indexDesc].table_size;i++){
        //printf("%d----\n", new_hash_table[i].block_id);
      }

      printf("hi\n");
      fd->opened_files[indexDesc].num_of_buckets++;
      fd->opened_files[indexDesc].bucket_infos=(Bucket_info*)realloc(fd->opened_files[indexDesc].bucket_infos, (fd->opened_files[indexDesc].num_of_buckets)*sizeof(Bucket_info));
      fd->opened_files[indexDesc].bucket_infos[fd->opened_files[indexDesc].num_of_buckets-1].num_of_records=0;
      fd->opened_files[indexDesc].bucket_infos[fd->opened_files[indexDesc].num_of_buckets-1].local_depth=fd->opened_files[indexDesc].global_depth;
      int flag=0;
      BF_Block* new_block, *cur_block;
      BF_Block_Init(&new_block);
      BF_Block_Init(&cur_block);
      CALL_BF(BF_AllocateBlock(fd->opened_files[indexDesc].file_desc, new_block));

      CALL_BF(BF_GetBlock(fd->opened_files[indexDesc].file_desc, fd->opened_files[indexDesc].hash_table[index].block_id, cur_block));
      int count=0;
      for (int i = 0; i < fd->opened_files[indexDesc].table_size; i++) {
        if (fd->opened_files[indexDesc].hash_table[i].block_id == fd->opened_files[indexDesc].hash_table[index].block_id) {
            void* cur_data = BF_Block_GetData(cur_block);
            
            Record* records = cur_data;
            for(int j=0;j<fd->opened_files[indexDesc].bucket_infos[fd->opened_files[indexDesc].hash_table[index].block_id].num_of_records;j++) {

              
              int new_index=hash_function(records[j].id, fd->opened_files[indexDesc].global_depth);
              if (new_index != index) {
                fd->opened_files[indexDesc].hash_table[new_index].block_id=fd->opened_files[indexDesc].num_of_buckets;
                fd->opened_files[indexDesc].bucket_infos[fd->opened_files[indexDesc].hash_table[index].block_id].num_of_records--;
                fd->opened_files[indexDesc].bucket_infos[fd->opened_files[indexDesc].hash_table[index].block_id].local_depth++;
                
                void* new_data = BF_Block_GetData(new_block);
                Record* rec=new_data; 
                rec[fd->opened_files[indexDesc].bucket_infos[fd->opened_files[indexDesc].num_of_buckets-1].num_of_records]=records[j];
                fd->opened_files[indexDesc].hash_table[new_index].block_id = fd->opened_files[indexDesc].num_of_buckets-1;
                fd->opened_files[indexDesc].bucket_infos[fd->opened_files[indexDesc].num_of_buckets-1].num_of_records++;
              }
              else{
                records[count]=records[j];
                count++;
              }
            }
            
        }
      }
      void* onoufrios=BF_Block_GetData(cur_block);
      Record* rec=onoufrios;
      rec[count]=record;
      fd->opened_files[indexDesc].bucket_infos[fd->opened_files[indexDesc].hash_table[index].block_id].num_of_records=count+2;
      BF_Block_SetDirty(cur_block);
      BF_Block_SetDirty(new_block);
      CALL_BF(BF_UnpinBlock(cur_block));
      CALL_BF(BF_UnpinBlock(new_block));
    }
  }
  else {
    BF_Block* block;
    BF_Block_Init(&block);
    CALL_BF(BF_GetBlock(fd->opened_files[indexDesc].file_desc, fd->opened_files[indexDesc].hash_table[index].block_id, block));
    void* data=BF_Block_GetData(block);
    Record* rec=data;
    rec[fd->opened_files[indexDesc].bucket_infos[fd->opened_files[indexDesc].hash_table[index].block_id].num_of_records]=record;
    fd->opened_files[indexDesc].bucket_infos[fd->opened_files[indexDesc].hash_table[index].block_id].num_of_records++;
    BF_Block_SetDirty(block);
    CALL_BF(BF_UnpinBlock(block));

  }
  return HT_OK;
}

HT_ErrorCode HT_PrintAllEntries(int indexDesc, int *id) {
  BF_Block* block;
  BF_Block_Init(&block);
  if (id!=NULL) {
   int index=hash_function(*id, fd->opened_files[indexDesc].global_depth);
   CALL_BF(BF_GetBlock(fd->opened_files[indexDesc].file_desc, fd->opened_files[indexDesc].hash_table[index].block_id, block));
   void *data=BF_Block_GetData(block);
   Record* rec=data;
   for(int i=0;i<fd->opened_files[indexDesc].bucket_infos[fd->opened_files[indexDesc].hash_table[index].block_id].num_of_records;i++){
<<<<<<< HEAD
      if(rec[i].id==id) {
        printf("%s, %s, %s, %d",rec[i].name, rec[i].surname, rec[i].city, rec[i].id);
=======
      if(rec[i].id==*id) {
        printf("%s, %s, %s, %d\n",rec[i].name, rec[i].surname, rec[i].city, rec[i].id);
>>>>>>> 73cfb461fbbcdda49dd529b81f10bcc9729fc4a0
      }
    }
  }
  else{
    for(int i=0; i<fd->opened_files[indexDesc].num_of_buckets; i++) {
      CALL_BF(BF_GetBlock(fd->opened_files[indexDesc].file_desc, 0, block));
      void *data=BF_Block_GetData(block);
      Record* rec=data;
      for(int j=0;j<fd->opened_files[indexDesc].bucket_infos[i].num_of_records;j++){
<<<<<<< HEAD
      if(rec[j].id==id) {
        printf("%s, %s, %s, %d",rec[j].name, rec[j].surname, rec[j].city, rec[j].id);
      }
    }
    }
  }
  CALL_BF(BF_UnpinBlock(block));
  return HT_OK;
  
}



HT_ErrorCode HashStatistics(char* filename){
  int sum=0;
  int min=9;
  int max=-1;
  int index;
  for(int i=0;i<fd->num_of_files;i++) {
    if(strcmp(fd->opened_files[i].filename,filename)==0) {
      index=i;
      break;
    }
  }
  for(int i=0;i<fd->opened_files[index].num_of_buckets;i++) {
    if(fd->opened_files[index].bucket_infos[i].num_of_records<min){
      min=fd->opened_files[index].bucket_infos[i].num_of_records;
    }
    if(fd->opened_files[index].bucket_infos[i].num_of_records>max){
      max=fd->opened_files[index].bucket_infos[i].num_of_records;
    }
    sum+=fd->opened_files[index].bucket_infos[i].num_of_records;

  }
  int avg=sum/fd->opened_files[index].num_of_buckets;
  printf("Number of blocks in file is: %d\n", fd->opened_files[index].num_of_buckets);
  printf("Most recosrds in one bucket are: %d\n", max);
  printf("Least records in one bucket are: %d\n", min);
  printf("Average number of blocks per bucket is: %d\n", avg);
=======
        printf("%s, %s, %s, %d\n",rec[j].name, rec[j].surname, rec[j].city, rec[j].id);
      }
    }
  }
  CALL_BF(BF_UnpinBlock(block));
>>>>>>> 73cfb461fbbcdda49dd529b81f10bcc9729fc4a0
  return HT_OK;
}

HT_ErrorCode HashStatistics(char* filename){
  int sum=0;
  int min=9;
  int max=-1;
  int index;
  for(int i=0;i<fd->num_of_files;i++) {
    if(strcmp(fd->opened_files[i].filename,filename)==0) {
      index=i;
      break;
    }
  }
  for(int i=0;i<fd->opened_files[index].num_of_buckets;i++) {
    if(fd->opened_files[index].bucket_infos[i].num_of_records<min){
      min=fd->opened_files[index].bucket_infos[i].num_of_records;
    }
    if(fd->opened_files[index].bucket_infos[i].num_of_records>max){
      max=fd->opened_files[index].bucket_infos[i].num_of_records;
    }
    sum+=fd->opened_files[index].bucket_infos[i].num_of_records;

  }
  int avg=sum/fd->opened_files[index].num_of_buckets;
  printf("Number of blocks in file is: %d\n", fd->opened_files[index].num_of_buckets);
  printf("Most recosrds in one bucket are: %d\n", max);
  printf("Least records in one bucket are: %d\n", min);
  printf("Average number of blocks per bucket is: %d\n", avg);
  return HT_OK;
}