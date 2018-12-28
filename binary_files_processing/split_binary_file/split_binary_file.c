/*
* Copyleft  2005-20xx  Fu Xing (Andy)
* Author: Fu Xing
*
* File name: split_binary_file.c
*
* Abstract: This program splits a binary file. There are two modes.
* Mode 0: Split the file evenly and generate several small file blocks. A batch file join.bat is also generated
*         to join the file blocks together.
* Mode 1: Specify the decimal start address and end address, the data between them are saved into a new file.
*
* Current version: 5.2
* Last Modified: 2018-12-27
*
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>


#define MAX_NUM_DST_BLOCK    10000

#define INITIAL_BUFFER_SIZE  (256 * 1024 * 1024)
#define FINAL_BUFFER_SIZE    (64)
#define SCALE_FACTOR    (4)  /* in each iteration, the buffer_size shifted right this many bits */
// 2^SCALE_FACTOR must be smaller than FINAL_BUFFER_SIZE.

int main()
{
  int8_t    fname_src[384]; // source filename
  FILE     *fp_src;
  uint64_t  i = 0; // counter; 2^64 - 1 (byte) = 16,777,216 (TiB)
  int32_t   split_mode; // split mode, 0 or 1
  uint8_t   byte_buffer = 0;
  uint8_t   *dynamic_buffer;
  uint32_t  buffer_size; // buffer size in bytes, for large amount of data copy

  //********* mode selection & source file operation *********
  printf("This program splits a binary file. There are two modes.\n\nMode 0: Split the file evenly \
and generate several small file blocks. A batch file join.bat is also generated to join the file blocks together.\n\n\
Mode 1: Specify the decimal start address and end address, the data between them are saved into a new file\n");

input_again_mode:
  printf("\nPlease select MODE. mode 0: Split the file evenly; mode 1: Specify the decimal \
start address and end address. : ");
  scanf("%d", &split_mode);
  if ( (split_mode != 0) && (split_mode != 1) )
  {
    printf("\nError! You can only input 0 or 1 for selecting the mode.\n");
    goto input_again_mode;
  }

input_again_filename:
  printf("\nInput the source file name: ");
  scanf("%s", fname_src);
  if (NULL == (fp_src = fopen(fname_src, "rb")))
  {
    printf("Error! Can't open file %s\n", fname_src);
    goto input_again_filename;
  }

  //************************** mode 0 *************************
  if (split_mode == 0)
  {
    uint64_t  dst_block_size = 1024; // destination block size in bytes
    uint32_t  num_dst_block = 1; // the number of destination blocks, NOT including the possible extra block; the actual number is (num_dst_block + extra_dst_block_flag)
    int8_t   *fname_bat = "join.bat"; // filename of the batch file
    FILE     *fp_bat, *fp_dst_block;
    int8_t    fname_dst_block_0[256]; // filename of destination blocks, before block number
    int8_t    fname_dst_block_2[256]; // filename of destination blocks, after block number
    int8_t    fname_dst_block[384]; // filename of destination blocks
    uint32_t  extra_dst_block_flag = 0;
    uint32_t  b_continue = 0;
    uint64_t  j, k; // loop counter

input_again_dst_size:
    printf("\nInput the size (in bytes) of the destination block: ");
    scanf("%llu", &dst_block_size);
    if (dst_block_size < 1)
    {
      printf("\nError! dst_block_size must be positive. Please input again.\n");
      goto input_again_dst_size;
    }

input_again_num_dstB:
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
      {
        goto input_again_num_dstB;
      }
    }

input_again_last_blk:
    printf("\nDoes the last smaller block exist? (0: no, 1: yes) : ");
    scanf("%u", &extra_dst_block_flag);
    if ( (extra_dst_block_flag != 0) && (extra_dst_block_flag != 1) )
    {
      printf("\nError! You can only input 0 or 1 here.\n");
      goto input_again_last_blk;
    }

    //================== generate join.bat ==================
    printf("\nInput the destination block name (the part that BEFORE the block number) :\n");
    scanf("%s", fname_dst_block_0);

    printf("\nInput the destination block name (the part that AFTER the block number) :\n");
    scanf("%s", fname_dst_block_2);

    if ( NULL == (fp_bat = fopen(fname_bat, "w")) )
    {
      printf("Error!Can't creat file %s \n", fname_bat);
      goto Exit_prog;
    }

    if (b_continue)  // The number of destination blocks is more than MAX_NUM_DST_BLOCK.
    {
      fprintf(fp_bat, "copy %s%010llu%s joined_%s\ncopy /B joined_%s", fname_dst_block_0, i, fname_dst_block_2, fname_src, fname_src);

      for (i=1; i<(num_dst_block+extra_dst_block_flag-1); i++)
      {
        fprintf(fp_bat, "+%s%010llu%s", fname_dst_block_0, i, fname_dst_block_2);

        if (i%10 == 0)
        {
          fprintf(fp_bat, "\ncopy /B joined_%s", fname_src);
        }
      }

      fprintf(fp_bat, "+%s%010llu%s\n", fname_dst_block_0, i, fname_dst_block_2);  /* last filename */
    }
    else
    {
      fprintf(fp_bat, "copy %s%04llu%s joined_%s\ncopy /B joined_%s", fname_dst_block_0, i, fname_dst_block_2, fname_src, fname_src);

      for (i=1; i<(num_dst_block+extra_dst_block_flag-1); i++)
      {
        fprintf(fp_bat, "+%s%04llu%s", fname_dst_block_0, i, fname_dst_block_2);

        if (i%10 == 0)
        {
          fprintf(fp_bat, "\ncopy /B joined_%s", fname_src);
        }
      }

      fprintf(fp_bat, "+%s%04llu%s\n", fname_dst_block_0, i, fname_dst_block_2);  /* last filename */
    }

    fclose(fp_bat);

    // =========================== create data blocks ===========================
    for (i=0; i<num_dst_block; i++)
    {
      memset(fname_dst_block, 0x00, sizeof(fname_dst_block));
      if (b_continue)  // The number of destination blocks is more than MAX_NUM_DST_BLOCK.
      {
        sprintf(fname_dst_block, "%s%010llu%s", fname_dst_block_0, i, fname_dst_block_2);
      }
      else
      {
        sprintf(fname_dst_block, "%s%04llu%s", fname_dst_block_0, i, fname_dst_block_2);
      }

      if (NULL == (fp_dst_block = fopen(fname_dst_block, "wb"))) //create destination block
      {
        printf("\nError! Can't creat file %s", fname_dst_block);
        goto Exit_prog;
      }

      j = 0;  /* how many bytes of data have been copied for this data block */

      for (buffer_size=INITIAL_BUFFER_SIZE; buffer_size >= FINAL_BUFFER_SIZE; buffer_size >>= SCALE_FACTOR)
      {
        if ( NULL == (dynamic_buffer = (uint8_t *)malloc(buffer_size)) )
        {
          continue;  // buffer allocation failed; let's try smaller buffers.
        }
        else
        {
          for (k=0; k<(dst_block_size-j)/buffer_size; k++)
          {
            printf("buffer_size = %u, j = %llu, k = %llu\n", buffer_size, j, k);  // debug

            if ( buffer_size != fread(dynamic_buffer, 1, buffer_size, fp_src) )
            {
              if ( feof(fp_src) )
              {
                printf("\nError! Unexpected end of file");
              }
              else if (ferror(fp_src))
              {
                printf("\nError occurred");
              }
              printf(" when reading %s for creating %s\n", fname_src, fname_dst_block);
              printf("buffer_size = %u, j = %llu, k = %llu\n", buffer_size, j, k);
              fclose(fp_dst_block);
              goto Exit_prog;
            }

            if ( buffer_size != fwrite(dynamic_buffer, 1, buffer_size, fp_dst_block) )
            {
              printf("\nError occurred when writing %s\n", fname_dst_block);
              printf("buffer_size = %u, j = %llu, k = %llu\n", buffer_size, j, k);
              fclose(fp_dst_block);
              goto Exit_prog;
            }
          }
          j += buffer_size * k;
          free(dynamic_buffer);
        }
      }

      for (; j<dst_block_size; j++) // copy remaining data
      {
        if ( feof(fp_src) )
        {
          printf("\nError! Unexpected end of file %s", fname_src);
          fclose(fp_dst_block);
          goto Exit_prog;
        }

        if ( 1 != fread(&byte_buffer, 1, 1, fp_src) )
        {
          if ( feof(fp_src) )
          {
            printf("\nError! Unexpected end of file");
          }
          else if (ferror(fp_src))
          {
            printf("\nError occurred");
          }
          printf(" when reading %s for creating %s\n", fname_src, fname_dst_block);
          printf("%llu bytes are NOT copied to %s\n", dst_block_size - j, fname_dst_block);
          fclose(fp_dst_block);
          goto Exit_prog;
        }

        if ( 1 != fwrite(&byte_buffer, 1, 1, fp_dst_block) )
        {
          printf("\nError occurred when writing %s\n", fname_dst_block);
          printf("%llu bytes are NOT copied to %s\n", dst_block_size - j, fname_dst_block);
          fclose(fp_dst_block);
          goto Exit_prog;
        }
      }

      fclose(fp_dst_block);
    }

    if (extra_dst_block_flag) // copy last block (smaller than others)
    {
      if (b_continue)  // The number of destination blocks is more than MAX_NUM_DST_BLOCK.
      {
        sprintf(fname_dst_block, "%s%010lu%s", fname_dst_block_0, num_dst_block, fname_dst_block_2);
      }
      else
      {
        sprintf(fname_dst_block, "%s%04lu%s", fname_dst_block_0, num_dst_block, fname_dst_block_2);
      }

      if ( NULL == (fp_dst_block = fopen(fname_dst_block, "wb")) ) // create destination block
      {
        printf("\nError! Can't creat file %s", fname_dst_block);
        goto Exit_prog;
      }

      if ( 1 != fread(&byte_buffer, 1, 1, fp_src) )
      {
        if ( feof(fp_src) )
        {
          printf("\nError! Unexpected end of file");
        }
        else if (ferror(fp_src))
        {
          printf("\nError occurred");
        }
        printf(" when reading %s for creating %s (the last smaller block)\n", fname_src, fname_dst_block);
        fclose(fp_dst_block);
        goto Exit_prog;
      }

      while( !feof(fp_src) )   // copy data
      {
        if ( 1 != fwrite(&byte_buffer, 1, 1, fp_dst_block) )
        {
          printf("\nError occurred when writing %s (the last smaller block)\n", fname_dst_block);
          fclose(fp_dst_block);
          goto Exit_prog;
        }

        if ( 1 != fread(&byte_buffer, 1, 1, fp_src) )
        {
          if (ferror(fp_src))
          {
            printf("\nError occurred");
          }
          printf(" when reading %s for creating %s (the last smaller block)\n", fname_src, fname_dst_block);
          fclose(fp_dst_block);
          goto Exit_prog;
        }
      }

      fclose(fp_dst_block);
    }
  }


  //************************** mode 1 ************************
  else
  {
    int8_t   fname_dst[384]; // filename
    uint64_t  startAddress = 0;
    uint64_t  endAddress   = 0;
    FILE    *fp_dst;

    printf("\nInput the destination filename: ");
    scanf("%s", fname_dst);
    if ( NULL == (fp_dst = fopen(fname_dst, "wb")) )
    {
      printf("Error! Can't create file %s\n", fname_dst);
      goto Exit_prog;
    }

    printf("\nInput decimal start address: ");
    scanf("%llu", &startAddress);

input_again6:
    printf("\nInput decimal end address: ");
    scanf("%llu", &endAddress);
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

  printf("\n done!");

Exit_prog:
  fclose(fp_src);

  printf("\nPress ENTER to quit.\n");
  getchar();
  getchar();

  return  0;
}

