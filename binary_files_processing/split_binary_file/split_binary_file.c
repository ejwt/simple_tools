/*
* Copyleft  2010-2016  Fu Xing (Andy)
* Author: Fu Xing
*
* File name: split_binary_file.c
*
* Abstract: This program splits a binary file. There are two modes.
* Mode 0: Split the file evenly and generate several small file blocks. A batch file join.bat is also generated
* to join the file blocks together.
* Mode 1: Specify the decimal start address and end address, the data between them are saved into a new file
* (for file >= 2GiB, you input start address and length instead of
* start address and end address, and the address and length are in MiB, NOT bytes).
*
* Current version: 4.0
* Last Modified: 2011-04-22
*
*/
#include <stdio.h>
#include <stdlib.h>

typedef unsigned int INT32U;
typedef int INT32S;
typedef unsigned char INT8U;
typedef char INT8S;

#define MAX_NUM_DST_BLOCK    9999
#define MiB    1048576

void main()
{
	INT8S   fname_src[384]; // source filename
	FILE    *fp_src;
	INT32U  i; // loop counter
	INT32U  split_mode; // mode, 0 or 1
	INT32U  large_en; // enable >= 2GiB file processing or not
	INT32U  src_length; // number of bytes in the file to be splitted (<2GiB case)
	INT8U   byte_buffer = 0;
  INT8U  *MiB_buffer;

  if ( NULL == (MiB_buffer = ((INT8U *)malloc(MiB)) ) ) /* 1 MiB buffer */
  {
  	printf("Error! Inssuficient memory!!\n");
  	goto Exit_prog;
  }

	//********* mode selection & source file operation *********
	printf("This program splits a binary file. There are two modes.\nMode 0: Split the file evenly \
and generate several small file blocks. A batch file join.bat is also generated to join the file blocks together.\n\
Mode 1: Specify the decimal start address and end address, the data between them are saved into a new file \
(for file >= 2GiB, you input start address and length instead of \
start address and end address, and the address and length are in MiB, NOT bytes).\n");

input_again0:
	printf("\nPlease select MODE. mode 0: Split the file evenly; mode 1: Specify the decimal \
start address and end address. : ");
	scanf("%u", &split_mode);
	if ( (split_mode != 0) && (split_mode != 1) )
	{
		printf("\nError! You can only input 0 or 1 for selecting the mode.\n");
		goto input_again0;
	}

	if ( split_mode == 1 )
  {
input_again1:
  	printf("\nEnable large file mode (can process file >= 2GiB) 0: no, 1: yes : ");
  	scanf("%u", &large_en);
  	if ( (large_en != 0) && (large_en != 1) )
  	{
  		printf("\nError! You can only input 0 or 1 here.\n");
  		goto input_again1;
  	}
  }

input_again2:
	printf("\nInput the source file name: ");
	scanf("%s", fname_src);
	if (NULL == (fp_src = fopen(fname_src, "rb")))
	{
		printf("Error! Can't open file %s \n", fname_src);
		goto input_again2;
	}

    if ( !large_en )  /* fseek() can only apply to files < 2GiB */
    {
    	fseek(fp_src, 0, SEEK_END);
    	src_length = ftell(fp_src);
        fseek(fp_src, 0, SEEK_SET);

        if (src_length == 0xFFFFFFFF)
        {
        	printf("\nThe source file is >= 2GiB, force to large file mode!!\n");
            large_en = 1;
            goto here3;
        }
    	printf("\nThe source file is %u bytes.\n", src_length);
    }

here3:
	//************************** mode 0 *************************
	if (split_mode == 0)
	{
		INT32U  dst_block_size = 1024; // destination block size (for source file < 2GiB)
		INT32U  num_dst_block = 1; // the number of destination blocks, the actual number is (num_dst_block + extra_dst_block_flag)
		INT32U  last_block_size = 0; // size of the last destination block (smaller than others,
						// when src_length can't be divided exactly by dst_block_size)
		INT8S  *fname_bat = "join.bat"; // filename of the batch file
		INT8S  *fname_tmp = "dst_block_name.tmp"; // filename of the temporary file that stores the filenames of destination blocks
		FILE   *fp_tmp, *fp_bat, *fp_dst_block;
		INT8S   fname_dst_block_0[340]; // filename of destination blocks, before block number
		INT8S   fname_dst_block_2[340]; // filename of destination blocks, after block number
		INT8S   fname_dst_block_all[384]; // filename of destination blocks
		INT32U  extra_dst_block_flag = 0;
		INT32U  j; // loop counter

input_again4L0:
		printf("\nInput the size (in bytes) of the destination block: ");
		scanf("%u", &dst_block_size);
		if ( dst_block_size < 1 )
		{
			printf("\nError! dst_block_size must be positive. Please input again.\n");
			goto input_again4L0;
		}

input_again4L1:
		printf("\nInput the number of destination blocks (NOT including the possible last smaller block): ");
		scanf("%u", &num_dst_block);

		if (num_dst_block > MAX_NUM_DST_BLOCK)
		{
			printf("\nError! The number of destination blocks is more than %u.\n", MAX_NUM_DST_BLOCK);
			printf("\nPlease modify macro MAX_NUM_DST_BLOCK and rebuild the program if you really need to cut \
the file into more than %u pieces.\n", MAX_NUM_DST_BLOCK);
      goto input_again4L1;
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
		printf("\nInput the destination block name (the part that before block number) :\n");
		scanf("%s", fname_dst_block_0);

		printf("\nInput the destination block name (the part that after block number) :\n");
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

		for (i=0; i<(num_dst_block+extra_dst_block_flag-1); i++)
		{
			fprintf(fp_tmp, "%s%04u%s\n", fname_dst_block_0, i, fname_dst_block_2);
			fprintf(fp_bat, "%s%04u%s+", fname_dst_block_0, i, fname_dst_block_2);
		}

		fprintf(fp_tmp, "%s%04u%s\n", fname_dst_block_0, i, fname_dst_block_2);  /* last filename */
		fprintf(fp_bat, "%s%04u%s", fname_dst_block_0, i, fname_dst_block_2);

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
		INT8S   fname_dst[384]; // filename
		INT32U  startAddress = 0;  // for small file
		INT32U  endAddress = 0;    // for small file
		INT32U  startAddress_MiB = 0;  // for large file
		INT32U  len_MiB = 0;    // for large file
		FILE    *fp_dst;

		printf("\nInput the destination filename: ");
		scanf("%s", fname_dst);
		if ( NULL == (fp_dst = fopen(fname_dst, "wb")) )
		{
			printf("Error! Can't create file %s\n", fname_dst);
			goto Exit_prog;
		}

        if ( !large_en )   /* small file */
        {
input_again5:
    		printf("\nInput decimal start address: ");
    		scanf("%u", &startAddress);
    		if ( startAddress > (src_length-1) )
    		{
    			printf("\nError! startAddress must belong to [0, %u]. Please input again.\n", (src_length-1));
    			goto input_again5;
    		}

input_again6:
    		printf("\nInput decimal end address: ");
    		scanf("%u", &endAddress);
    		if ( (endAddress < startAddress) || (endAddress > (src_length-1)) )
    		{
    			printf("\nError! endAddress must belong to [%u, %u]. Please input again.\n", startAddress, (src_length-1));
    			goto input_again6;
    		}
        }
        else   /* large file */
        {
    		printf("\nInput decimal start address (in MiB): ");
    		scanf("%u", &startAddress_MiB);

    		printf("\nInput the length you need to cut (in MiB): ");
    		scanf("%u", &len_MiB);
        }

        if ( !large_en )   /* small file */
        {

    		fseek(fp_src, startAddress, SEEK_SET);
    		for (i=0; i<(endAddress-startAddress+1); i++)
    		{
    			fread(&byte_buffer, 1, 1, fp_src);
    			fwrite(&byte_buffer, 1, 1, fp_dst);
    		}
        }
        else   /* large file */
        {
    		for (i=0; i<startAddress_MiB; i++)
    		{
                if ( feof(fp_src) )
                {
    				printf("\nError! End of source data file has been reached.");
        			fclose(fp_dst);
                    fclose(fp_src);
    				goto Exit_prog;
                }
    			fread(MiB_buffer, 1, MiB, fp_src);  // skip startAddress_MiB MiB
    		}

    		for (i=0; i<len_MiB; i++)
    		{
                if ( feof(fp_src) )
                {
    				printf("\nError! End of source data file has been reached.");
        			fclose(fp_dst);
                    fclose(fp_src);
    				goto Exit_prog;
                }
    			fread(MiB_buffer, 1, MiB, fp_src);
    			fwrite(MiB_buffer, 1, MiB, fp_dst);
    		}
        }

		fclose(fp_dst);
	}

	fclose(fp_src);
	printf("\n done!");

Exit_prog:
	printf("\nPress ENTER to quit.\n");
	getchar();
	getchar();
}

