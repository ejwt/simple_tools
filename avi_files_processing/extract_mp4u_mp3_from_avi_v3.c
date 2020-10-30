/*
* Copyleft (c) 2007-2010  Fu Xing (Andy)
* Author: Fu Xing (based on Zhou Yang's work)
*
* File name: extract_mp4u_mp3_from_avi_v3.c
* Abstract: The program is used to extract mp4u (xvid video) and mp3 (audio) files from an avi file (AVI-2 format).
*      It only supports avi files that multiplexes only one audio stream and only one video stream.
*
* Usage: Run the program, and follow the on-screen instructions.
*
* Current version: 3.0
* Last Modified: 2010-07-12
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define RIFF      (('R')|('I'<<8)|('F'<<16)|('F'<<24))
#define AVI       (('A')|('V'<<8)|('I'<<16)|(' '<<24))
#define LIST      (('L')|('I'<<8)|('S'<<16)|('T'<<24))
#define MOVI     (('m')|('o'<<8)|('v'<<16)|('i'<<24))
#define REC       (('r')|('e'<<8)|('c'<<16)|(' '<<24))
#define DB         (('d')|('b'<<8))
#define DC         (('d')|('c'<<8))
#define WB        (('w')|('b'<<8))
#define IX          (('i')|('x'<<8))
#define JUNK     (('J')|('U'<<8)|('N'<<16)|('K'<<24))
#define IDX1     (('i')|('d'<<8)|('x'<<16)|('1'<<24))

long   avi_file_size;


long Search_FCC(FILE *file, int fcc, char list_flag);



void main()
{
	char   avi_filename[256], video_filename[256], audio_filename[256];
	FILE   *avi_file, *video_file, *audio_file;
	unsigned char   *mp4u_header = "MP4U";

	int     code, list_type;    /* 4 byte temporary readback buffer for judgement, including FOURCC, chunkID, and etc. */
	long   movi_list_pos;  /* the next byte immediately follows 'movi' */
	int      audio_size, video_size, junk_size, index_size, list_size;
	unsigned char   *video_buf, *audio_buf;
	int      video_en = 0;
	int      audio_en = 0;
	int      choice_id = 0;
	int      dont_ask_more = 0;
	int      i;     /* loop counter for debugging */
	int	   audio_buf_size = 0x100000;   /* 1MB */
	int      video_buf_size = 0x1000000;    /* 16MB */

	printf("The program is used to extract mp4u (xvid video) and mp3 (audio) files from an avi file (AVI-2 format).\n");
	printf("It only supports avi files that multiplexes only one audio stream and only one video stream.\n");
	
	do{
		printf("\nThe name of the source avi file:\n");
		gets(avi_filename);
		avi_file = fopen(avi_filename, "rb");
	}while(avi_file == NULL);

/*=============  source AVI file verification  ============================*/
	fseek(avi_file, 0, SEEK_END);
	avi_file_size = ftell(avi_file);
	fseek(avi_file, 0, SEEK_SET);

	fread(&code, 4, 1, avi_file);    /* The first 4 bytes should be "RIFF" */
	if(code != RIFF)
	{
		do{
			printf("\nWARNING! This file is NOT an RIFF file, do you wish to continue?\n");
			printf("0: No. Terminate the program, please.\n");
			printf("1: Yes. Continue anyway.\n");
			scanf("%d", &choice_id);
		}while( (choice_id != 0) && (choice_id != 1));

		if(choice_id == 0)
			goto  END_PROG;
	}
 
 	fread(&code, 4, 1, avi_file);    /* Check whether ('RIFF' chunk size) matches (total AVI file size -8)  */
	if( code != (avi_file_size-8) )
	{
		do{
			printf("\nWARNING! (RIFF chunk size) does NOT match (total AVI file size -8), do you wish to continue?\n");
			printf("0: No. Terminate the program, please.\n");
			printf("1: Yes. Continue anyway.\n");
			scanf("%d", &choice_id);
		}while( (choice_id != 0) && (choice_id != 1));

		if(choice_id == 0)
			goto  END_PROG;
	}
	
/*=============  addtional info input  ============================*/
	do{
		printf("\nDo you want to save the video (*.mp4u) file? [1: yes, 0: no]");
		scanf("%d", &video_en);
	}while( (video_en!=0) && (video_en!=1) );
	
	do{
		printf("\nDo you want to save the audio (*.mp3) file? [1: yes, 0: no]");
		scanf("%d", &audio_en);
	}while( (audio_en!=0) && (audio_en!=1) );

	if ( (video_en == 0) && (audio_en == 0) )
		goto END_PROG;	

/*=============  filenames construction ============================*/
	for(i=0; (video_filename[i] = audio_filename[i] = avi_filename[i]) != '.'; i++) ;

	i++;
	video_filename[i] = audio_filename[i] = 'm';
	
	i++;
	video_filename[i] = audio_filename[i] = 'p';

	video_filename[++i] = '4';
	video_filename[++i] = 'u';
	video_filename[++i] = '\0';

	i-=2;
	audio_filename[i++] = '3';
	audio_filename[i] = '\0';


/*=============  files creation and buffer allocation ============================*/
	if(video_en)
		if( NULL == (video_file = fopen(video_filename, "wb")) )
		{
			printf("Can't create the output files. Check whether the media is read-only and the available disk space!\n");
			exit(0);
		}

	if(audio_en)
		if( NULL == (audio_file = fopen(audio_filename, "wb")) )
		{
			printf("Can't create the output files. Check whether the media is read-only and the available disk space!\n");
			exit(0);
		}

	if( NULL == (audio_buf = (unsigned char *)malloc(audio_buf_size*sizeof(char)))  )
	{
		printf("error in audio memory allocation!\n");
		exit(0);
	}

	if( NULL == (video_buf = (unsigned char *)malloc(video_buf_size*sizeof(char)))  )
	{
		printf("error in video memory allocation!\n");
		exit(0);
	}


	/* Write the four characters code 'MP4U' for the mp4u file */
	if(video_en)
		fwrite(mp4u_header, 4, 1, video_file);

	/* Search the position of LIST 'movi' */
	movi_list_pos = Search_FCC(avi_file, MOVI, 1);

	if(movi_list_pos == avi_file_size)
	{
		printf("\n ERROR! No \'movi\' chunk has been found! No video and audio data can be saved.\n");
		goto  END_PROG_A;
	}

	/* scan the 'movi' LIST chunk to extract video and audio data */
	fseek(avi_file, movi_list_pos, SEEK_SET);

	printf("\n Processing...     Please wait patiently.\n");
	i = 0;   /* loop counter for debugging */
	
	do{
		printf("%d\n", ++i);
		fread(&code, 4, 1, avi_file);   /* get (2byte type ID) + (2bytes stream ID) */
			
		if(code == LIST)       /* curent location is a 'LIST' chunk */
		{
			fread(&list_size, 4, 1, avi_file);    /* read 'listSize' field */
			fread(&list_type, 4, 1, avi_file);    /* read 'listType' field */
			
			if(list_type != REC)
				fseek(avi_file, (list_size - 4), SEEK_CUR);   /* skip this LIST */
			else
				fread(&code, 4, 1, avi_file);    /* get chunkID */
		}
		
		else  if( ((code>>16)==DB) || ((code>>16)==DC) )    /* Video chunk */
		{
			fread(&video_size, 4, 1, avi_file);   /* copy chunk size info */
			if(video_en)
				fwrite(&video_size, 4, 1, video_file);
			
			while(video_size >= video_buf_size)     /* copy video chunk data */
			{
				fread(video_buf, 1, video_buf_size, avi_file);
				if(video_en)
					fwrite(video_buf, 1, video_buf_size, video_file);
				video_size -= video_buf_size;
			}

			if(video_size > 0)      /* copy the last block of video chunk data */
			{
				fread(video_buf, 1, video_size, avi_file);
				if(video_en)
					fwrite(video_buf, 1, video_size, video_file);
			}
			
			if(video_size&1)   /* odd number */
				fseek(avi_file, 1, SEEK_CUR);    /* skip the padding byte */
				/* The start of chunk data is word-aligned with respect to the start of the RIFF file. 
				If the chunk size is an odd number of bytes, a padding byte with value zero is written after chunk data. */

			video_size = 0;
		}

		else  if( (code>>16) == WB)             /* Audio chunk */
		{
			fread(&audio_size, 4, 1, avi_file);      /* get audio chunk size info */
			
			while(audio_size >= audio_buf_size)      /* copy audio chunk data */
			{
				fread(audio_buf, 1, audio_buf_size, avi_file);
				if(audio_en)
					fwrite(audio_buf, 1, audio_buf_size, audio_file);
				audio_size -= audio_buf_size;
			}

			if(audio_size > 0)          /* copy the last block of audio chunk data */
			{
				fread(audio_buf, 1, audio_size, avi_file);
				if(audio_en)
					fwrite(audio_buf, 1, audio_size, audio_file);
			}

				if(audio_size&1)   /* odd number */
					fseek(avi_file, 1, SEEK_CUR);    /* skip the padding byte */

				audio_size = 0;
		}

		else if ( ((code>>16) == IX) || ((code&0x0000FFFF) == IX))             /* Index chunk */
		{
			fread(&index_size, 4, 1, avi_file);   /* get index size info */
			fseek(avi_file, index_size, SEEK_CUR);    /* skip the index chunk */
		}

		else if ( code == JUNK )             /* Junk (padding) chunk */
		{
			fread(&junk_size, 4, 1, avi_file);   /* get junk size info */
			fseek(avi_file, junk_size, SEEK_CUR);    /* skip the junk chunk */
		}

		else if (code == IDX1)             /* Index at the end of the avi file*/
			goto  END_PROG_A;

		else         /* a ChunkID can't be handled by this program */
		{
			if(dont_ask_more == 0)
			{
				do{
					printf("\nWARNING! Encounter a ChunkID can't be handled by this program at adsress 0x%08X. \
Its content is 0x%08X. Do you wish to continue?\n", (ftell(avi_file)-4), code);
					printf("0: No. Terminate the program, please.\n");
					printf("1: Yes. Continue anyway.\n");
					printf("2: Yes. Continue anyway, and don't ask anymore! Continue regarless of error!!\n");
					scanf("%d", &choice_id);
				}while( (choice_id != 0) && (choice_id != 1) && (choice_id != 2));

				if(choice_id == 0)
					goto  END_PROG_A;

				if(choice_id == 2)
					dont_ask_more = 1;
			}
		}

	}while ( ftell(avi_file) < avi_file_size);

END_PROG_A:
	/*free the allocated memory space and close the opened files*/
	free(audio_buf);
	free(video_buf);
	
	if(audio_en)
		fclose(audio_file);

	if(video_en)
		fclose(video_file);

	printf("\n\n ===  All done!  ===\n\n");

END_PROG:
	fclose(avi_file);
	printf("\nPress ENTER to quit.\n");
	getchar();
	getchar();
}



long Search_FCC(FILE *file, int fcc, char list_flag)
{
	int    code, id_list, size;
	long    position;   /* The position where the payload data in the chunk starts, without ID/size/type field */

	fseek(file, 12, SEEK_SET);

	if(list_flag)    /* (list_flag != 0) means searching for a LIST chunk with the specified FOURCC as the 'listType' */
	{
		do
		{		
			fread(&id_list, 4, 1, file);
			
			if(id_list == LIST)    /* a LIST chunk has been found, which is maybe what we need */
			{
				fread(&size, 4, 1, file);      /* get 'listSize' info */
				fread(&code, 4, 1, file);    /* get 'listType' info */

				if(code == fcc)     /* a LIST chunk with the specified FOURCC as the 'listType' has been found */
				{
					printf("A LIST chunk with the specified FOURCC (0x%8X) as the \'listType\' has been found.", fcc);
					position = ftell(file);       /* get the start position of 'listData' */
					return position;
				}
				else
					fseek(file, size-4, SEEK_CUR);    /* Skip to the next chunk */
			}

			else    /* what encountered is not a LIST chunk, but an ordinary chunk */
			{
				fread(&size, 4, 1, file);	 /* get chunk size info */
				fseek(file, size, SEEK_CUR);  /* Skip to the next chunk */
			}
		}while( ftell(file) < avi_file_size);
	}

	else       /* (list_flag == 0) means searching for an ordinary chunk with the specified FOURCC as the 'chunkID' */
	{
		do
		{
			fread(&id_list, 4, 1, file);

			if(id_list == LIST)    /* a LIST chunk has been found, which is NOT what we need */
			{
				fread(&size, 4, 1, file);	 /* get list size info */
				fseek(file, size, SEEK_CUR);  /* Skip to the next chunk */
			}
			
			else    /* what encountered is not a LIST chunk, but an ordinary chunk, which is maybe what we need */
			{
				if(id_list == fcc)    /* an ordinary chunk with the specified FOURCC as the chunk ID has been found */
				{
					printf("An ordinary chunk with the specified FOURCC (0x%8X) as the \'chunkID\' has been found.", fcc);
					fread(&size, 4, 1, file);	 /* get chunk size info */
					position = ftell(file);    /* get the start position of 'chunkData' */
					return position;
				}
				else
				{
					fread(&size, 4, 1, file);	 /* get chunk size info */
					fseek(file, size, SEEK_CUR);  /* Skip to the next chunk */
				}
			}
		}while( ftell(file) < avi_file_size);
	}

	if( ftell(file) >= (avi_file_size -8))    /* 'movi' has NOT been found */
		return avi_file_size;
}

