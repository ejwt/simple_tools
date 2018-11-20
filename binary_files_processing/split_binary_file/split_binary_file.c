/*
* Copyleft  2010-2016  Fu Xing (Andy)
* Author: Fu Xing (Andy)
*
* File name: split_binary_file.c
*
* Abstract: The program splits a binary file. There are two modes. Mode 0: Split the file evenly
* and generate several small file blocks. A batch file join.bat is also generated to join the file blocks
* together. Mode 1: Specify the decimal start address and end address,
* the data between them are saved into a new file.
*
* Current version: 2.1
* Date: 2010-08-30
*
*/
#include <stdio.h>
#include <stdlib.h>

typedef unsigned int INT32U;
typedef int INT32S;
typedef unsigned char INT8U;
typedef char INT8S;

#define MAX_NUM_DST_BLOCK    3000

void main()
{
  //********* variable declaration *********
  INT8S   fname_src[250]; //filename
  FILE    *fp_src;
  INT32U  i; //loop counter
  INT32U  split_mode; //mode, 0 or 1
  INT32U  src_length; //number of bytes in the file to be splitted
  INT8U   byte_buffer = 0;


  //********* mode selection & source file operation *********
  printf("\nThe program splits a binary file. There are two modes. Mode 0: Split the file evenly \
and generate several small file blocks. A batch file join.bat is also generated to join the file blocks together. \
Mode 1: Specify the decimal start address and end address, the data between them are saved into a new file.\n");

input_again1:
  printf("\nPlease select MODE. mode 0: Split the file evenly; mode 1: Specify the decimal \
start address and end address. : ");
  scanf("%u", &split_mode);
  if ( (split_mode != 0) && (split_mode != 1) )
  {
    printf("\n\7Error! You can only input 0 or 1 for selecting the mode.\n");
    goto input_again1;
  }

input_again5:
  printf("\nInput the source file name: ");
  scanf("%s", fname_src);
  if (NULL == (fp_src = fopen(fname_src, "rb")))
  {
    printf("\7Error!Can't open file %s \n", fname_src);
    goto input_again5;
  }

  fseek(fp_src, 0, SEEK_END);
  src_length = ftell(fp_src);
  fseek(fp_src, 0, SEEK_SET);
  printf("\nThe source file is %u bytes.\n", src_length);


  //********* mode 0 *********
  if (split_mode == 0)
  {
    INT32U  dst_block_size = 1024; //destination file size(single block)
    INT32U  num_dst_block = 1; //the number of destination block, the actual number maybe num_dst_block+1
    INT32U  last_block_size = 0; //size of the last destination block(smaller than others,
            //when src_length can't be divided exactly by dst_block_size
    INT8S  *fname_bat = "join.bat"; //filename of the batch file
    INT8S  *fname_tmp = "dst_block_name.tmp"; //filename of the temporary file that stores the filenames of destination blocks
    FILE   *fp_tmp, *fp_bat, *fp_dst_block;
    INT8S   fname_dst_block_0[220]; //filename of destination block, before block number
    INT8S   fname_dst_block_2[25]; //filename of destination block, after block number
    INT8S   fname_dst_block_all[250]; //filename of destination block
    INT32U  extra_dst_block_flag = 0;
    INT32U  j; //loop counter

input_again4:
    printf("\nInput the size(in bytes) of the destination block: ");
    scanf("%u", &dst_block_size);
    if ((dst_block_size<1) || (dst_block_size>src_length))
    {
      printf("\n\7Error! dst_block_size must belong to [1, %u]. Please input again.\n", src_length);
      goto input_again4;
    }

    num_dst_block = src_length / dst_block_size;
    if (num_dst_block > MAX_NUM_DST_BLOCK)
    {
      printf("\n\7Error! The number of destination block is over %u, please input a smaller block size.\n", MAX_NUM_DST_BLOCK);
      goto input_again4;
    }

    if ((last_block_size = src_length % dst_block_size) != 0)
    {
      printf("\nThe last destination block is %u bytes.(Smaller than others.)", last_block_size);
      extra_dst_block_flag = 1; //total number of destination block is num_dst_block+1
    }
    printf("\nThe file will be splitted into %u blocks. Double click join.bat can join them together.", (num_dst_block+extra_dst_block_flag));

    //***** generate dst_block_name.tmp and join.bat *****
    printf("\nInput the destination block name(the part that before block number) : ");
    scanf("%s", fname_dst_block_0);

    printf("\nInput the destination block name(the part that after block number) : ");
    scanf("%s", fname_dst_block_2);

    if (NULL == (fp_tmp = fopen(fname_tmp, "w+")))
    {
      printf("\7Error!Can't creat file %s \n", fname_tmp);
    goto Exit_prog;
    }

    if (NULL == (fp_bat = fopen(fname_bat, "w")))
    {
      printf("\7Error!Can't creat file %s \n", fname_bat);
    goto Exit_prog;
    }

    fprintf(fp_bat, "copy /B ");

    for (i=0; i<(num_dst_block+extra_dst_block_flag-1); i++)
    {
      fprintf(fp_tmp, "%s%04u%s\n", fname_dst_block_0, i, fname_dst_block_2);
      fprintf(fp_bat, "%s%04u%s+", fname_dst_block_0, i, fname_dst_block_2);
    }

    fprintf(fp_tmp, "%s%04u%s\n", fname_dst_block_0, i, fname_dst_block_2);  /* last filename */
    fprintf(fp_bat, "%s%04u%s", fname_dst_block_0, i, fname_dst_block_2);

    fprintf(fp_bat, " Joined_%s\n", fname_src);
    fclose(fp_bat);

    printf("\nBegin procssing, please wait patiently......\n");

    fseek(fp_tmp, 0, SEEK_SET);

    for (i=0; i<num_dst_block; i++)
    {
      fscanf(fp_tmp, "%s", fname_dst_block_all); //read filename from temporary file

      if (NULL == (fp_dst_block = fopen(fname_dst_block_all, "wb"))) //create destination block
      {
        printf("\n\7Error! Can't creat file %s", fname_dst_block_all);
        goto Exit_prog;
      }

      for (j=0; j<dst_block_size; j++) //copy data
      {
        fread(&byte_buffer, 1, 1, fp_src);
        fwrite(&byte_buffer, 1, 1, fp_dst_block);
      }

      fclose(fp_dst_block);
    }

    if (extra_dst_block_flag == 1) //copy last block(smaller than others)
    {
      fscanf(fp_tmp, "%s", fname_dst_block_all); //read filename from temporary file

      if (NULL == (fp_dst_block = fopen(fname_dst_block_all, "wb"))) //create destination block
      {
        printf("\n\7Error! Can't creat file %s", fname_dst_block_all);
        goto Exit_prog;
      }

      for (j=0; j<last_block_size; j++) //copy data
      {
        fread(&byte_buffer, 1, 1, fp_src);
        fwrite(&byte_buffer, 1, 1, fp_dst_block);
      }

      fclose(fp_dst_block);
    }

    fclose(fp_tmp);

    if (-1 == remove("dst_block_name.tmp") )
    {
      printf("\n Can't delete temporary file dst_block_name.tmp. You need to delete it manually.\n");
    }

  }


  //********* mode 1 *********
  else
  {
    INT8S   fname_dst[96]; //filename
    INT32U  startAddress = 0;
    INT32U  endAddress = 0;
    FILE  *fp_dst;

    printf("\nInput the destination file name: ");
    scanf("%s", fname_dst);
    if (NULL == (fp_dst = fopen(fname_dst, "wb")))
    {
      printf("\7Error!Can't create file %s \n", fname_dst);
      goto Exit_prog;
    }

input_again2:
    printf("\nInput decimal start address: ");
    scanf("%u", &startAddress);
    if ( (startAddress<0) || (startAddress>(src_length-1)) )
    {
      printf("\7\nError! startAddress must belong to [0, %u]. Please input again.\n", src_length-1);
      goto input_again2;
    }

input_again3:
    printf("\nInput decimal end address: ");
    scanf("%u", &endAddress);
    if ( (endAddress<startAddress) || (endAddress>(src_length-1)) )
    {
      printf("\7\nError! endAddress must belong to [%u, %u]. Please input again.\n", startAddress, src_length-1);
      goto input_again3;
    }

    printf("\nBegin procssing, please wait patiently......\n");

    fseek(fp_src, startAddress, SEEK_SET);
    for (i=0; i<(endAddress-startAddress+1); i++)
    {
      fread(&byte_buffer, 1, 1, fp_src);
      fwrite(&byte_buffer, 1, 1, fp_dst);
    }
    fclose(fp_dst);
  }

  fclose(fp_src);
  printf("\n Succeed !");

Exit_prog:
  printf("\nPress ENTER to quit.\n");
  getchar();
  getchar();
}

