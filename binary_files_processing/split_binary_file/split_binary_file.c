/*
* Copyleft  2010-2016  Fu Xing (Andy)
* Author: Fu Xing
*
* File name: split_binary_file.c
*
* Abstract: This program splits a binary file. There are two modes.
* Mode 0: Split the file evenly and generate several small file blocks. A batch file join.bat is also generated
* to join the file blocks together.
* Mode 1: Specify the decimal start address and end address, the data between them are saved into a new file.
*
* Current version: 5.0
* Last Modified: 2015-6-1
*
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef unsigned int UINT32;
typedef unsigned char UINT8;

#define MAX_NUM_DST_BLOCK    9999

int main()
{
  int8_t   fname_src[384]; // source filename
  FILE    *fp_src;
  int64_t  i; // loop counter; 2^63 - 1 (byte) = 8,388,608 (TiB)
  int    split_mode; // mode, 0 or 1
  UINT8  byte_buffer = 0;

  //********* mode selection & source file operation *********
  printf("This program splits a binary file. There are two modes.\nMode 0: Split the file evenly \
and generate several small file blocks. A batch file join.bat is also generated to join the file blocks together.\n\
Mode 1: Specify the decimal start address and end address, the data between them are saved into a new file\n");

input_again0:
  printf("\nPlease select MODE. mode 0: Split the file evenly; mode 1: Specify the decimal \
start address and end address. : ");
  scanf("%d", &split_mode);
  if ( (split_mode != 0) && (split_mode != 1) )
  {
    printf("\nError! You can only input 0 or 1 for selecting the mode.\n");
    goto input_again0;
  }

input_again2:
  printf("\nInput the source file name: ");
  scanf("%s", fname_src);
  if (NULL == (fp_src = fopen(fname_src, "rb")))
  {
    printf("Error! Can't open file %s \n", fname_src);
    goto input_again2;
  }

  //************************** mode 0 *************************
  if (split_mode == 0)
  {
    int64_t  dst_block_size = 1024; // destination block size
    UINT32  num_dst_block = 1; // the number of destination blocks, the actual number is (num_dst_block + extra_dst_block_flag)
    int64_t  last_block_size = 0; // size of the last destination block (smaller than others,
                                  // when src_length is not divisible by dst_block_size)
    int8_t  *fname_bat = "join.bat"; // filename of the batch file
    int8_t  *fname_tmp = "dst_block_name.tmp"; // filename of the temporary file that stores the filenames of destination blocks
    FILE   *fp_tmp, *fp_bat, *fp_dst_block;
    int8_t   fname_dst_block_0[340]; // filename of destination blocks, before block number
    int8_t   fname_dst_block_2[340]; // filename of destination blocks, after block number
    int8_t   fname_dst_block_all[384]; // filename of destination blocks
    UINT32  extra_dst_block_flag = 0;
    UINT32  b_continue = 0;
    int64_t  j; // loop counter

input_again4L0:
    printf("\nInput the size (in bytes) of the destination block: ");
    scanf("%lld", &dst_block_size);
    if ( dst_block_size < 1 )
    {
      printf("\nError! dst_block_size must be positive. Please input again.\n");
      goto input_again4L0;
    }

    printf("\nInput the number of destination blocks (NOT including the possible last smaller block): ");
    scanf("%u", &num_dst_block);

    if (num_dst_block > MAX_NUM_DST_BLOCK)
    {
      printf("\nWARNING! The number of destination blocks is more than %u.\n", MAX_NUM_DST_BLOCK);
      printf("Lots of files will be generated and deleting them will be slow!\n");
      printf("Are you sure you wish to continue?\n");
      printf("1 or any other non-zero value: Yes, 0: No.\n");
      scanf("%d", &b_continue);

      if (b_continue == 0)
        exit(-1);
    }

input_again4L2:
    printf("\nDoes the last smaller block exist? (0: no, 1: yes) : ");
    scanf("%u", &extra_dst_block_flag);
    if ( (extra_dst_block_flag != 0) && (extra_dst_block_flag != 1) )
    {
      printf("\nError! You can only input 0 or 1 here.\n");
      goto input_again4L2;
    }

    //***** generate dst_block_name.tmp and join.bat *****
    printf("\nInput the destination block name (the part that BEFORE the block number) :\n");
    scanf("%s", fname_dst_block_0);

    printf("\nInput the destination block name (the part that AFTER the block number) :\n");
    scanf("%s", fname_dst_block_2);

    if ( NULL == (fp_tmp = fopen(fname_tmp, "w+")) )
    {
      printf("Error!Can't creat file %s \n", fname_tmp);
      goto Exit_prog;
    }

    if ( NULL == (fp_bat = fopen(fname_bat, "w")) )
    {
      printf("Error!Can't creat file %s \n", fname_bat);
        goto Exit_prog;
    }

    fprintf(fp_bat, "copy /B ");

    if (b_continue)  // The number of destination blocks is more than MAX_NUM_DST_BLOCK.
    {
      for (i=0; i<(num_dst_block+extra_dst_block_flag-1); i++)
      {
        fprintf(fp_tmp, "%s%09lld%s\n", fname_dst_block_0, i, fname_dst_block_2);
        fprintf(fp_bat, "%s%09lld%s+", fname_dst_block_0, i, fname_dst_block_2);
      }

      fprintf(fp_tmp, "%s%09lld%s\n", fname_dst_block_0, i, fname_dst_block_2);  /* last filename */
      fprintf(fp_bat, "%s%09lld%s", fname_dst_block_0, i, fname_dst_block_2);
    }
    else
    {
      for (i=0; i<(num_dst_block+extra_dst_block_flag-1); i++)
      {
        fprintf(fp_tmp, "%s%04lld%s\n", fname_dst_block_0, i, fname_dst_block_2);
        fprintf(fp_bat, "%s%04lld%s+", fname_dst_block_0, i, fname_dst_block_2);
      }

      fprintf(fp_tmp, "%s%04lld%s\n", fname_dst_block_0, i, fname_dst_block_2);  /* last filename */
      fprintf(fp_bat, "%s%04lld%s", fname_dst_block_0, i, fname_dst_block_2);
    }


    fprintf(fp_bat, " Joined_%s\n", fname_src);
    fclose(fp_bat);

    fseek(fp_tmp, 0, SEEK_SET);

    for (i=0; i<num_dst_block; i++)
    {
      fscanf(fp_tmp, "%s", fname_dst_block_all); // read filename from temporary file

      if (NULL == (fp_dst_block = fopen(fname_dst_block_all, "wb"))) //create destination block
      {
        printf("\nError! Can't creat file %s", fname_dst_block_all);
        goto Exit_prog;
      }

      for (j=0; j<dst_block_size; j++) // copy data
      {
        if ( feof(fp_src) )
        {
          printf("\nError! End of source data file has been reached.");
          fclose(fp_tmp);
          fclose(fp_dst_block);
          fclose(fp_src);

          if (-1 == remove("dst_block_name.tmp") )
          {
            printf("\n Can't delete temporary file dst_block_name.tmp. You need to delete it manually.\n");
          }
          goto Exit_prog;
        }
        fread(&byte_buffer, 1, 1, fp_src);
        fwrite(&byte_buffer, 1, 1, fp_dst_block);
      }

      fclose(fp_dst_block);
    }

    if (extra_dst_block_flag) // copy last block (smaller than others)
    {
      fscanf(fp_tmp, "%s", fname_dst_block_all); // read filename from temporary file

      if ( NULL == (fp_dst_block = fopen(fname_dst_block_all, "wb")) ) //create destination block
      {
        printf("\nError! Can't creat file %s", fname_dst_block_all);
        goto Exit_prog;
      }

      fread(&byte_buffer, 1, 1, fp_src);

      while( !feof(fp_src) )   // copy data
      {
        fwrite(&byte_buffer, 1, 1, fp_dst_block);
        fread(&byte_buffer, 1, 1, fp_src);
      }

      fclose(fp_dst_block);
    }

    fclose(fp_tmp);

    if (-1 == remove("dst_block_name.tmp") )
    {
      printf("\n Can't delete temporary file dst_block_name.tmp. You need to delete it manually.\n");
    }
  }


  //************************** mode 1 ************************
  else
  {
    int8_t   fname_dst[384]; // filename
    int64_t  startAddress = 0;
    int64_t  endAddress   = 0;
    FILE    *fp_dst;

    printf("\nInput the destination filename: ");
    scanf("%s", fname_dst);
    if ( NULL == (fp_dst = fopen(fname_dst, "wb")) )
    {
      printf("Error! Can't create file %s\n", fname_dst);
      goto Exit_prog;
    }

    printf("\nInput decimal start address: ");
    scanf("%lld", &startAddress);

input_again6:
    printf("\nInput decimal end address: ");
    scanf("%lld", &endAddress);
    if ( endAddress < startAddress )
    {
      printf("\nError! endAddress must be larger than or equal to startAddress.\n");
      goto input_again6;
    }

    for (i=0; i<startAddress; i++)
    {
      fread(&byte_buffer, 1, 1, fp_src);  // skip
    }

    for (i=0; i<(endAddress-startAddress+1); i++)
    {
      fread(&byte_buffer, 1, 1, fp_src);

      if (feof(fp_src))
      {
        printf("ERROR: EOF of input file reached, data copy stopped.\n");
        break;
      }

      fwrite(&byte_buffer, 1, 1, fp_dst);
    }

    fclose(fp_dst);
  }

  fclose(fp_src);
  printf("\n done!");

Exit_prog:
  printf("\nPress ENTER to quit.\n");
  getchar();
  getchar();

  return  0;
}

