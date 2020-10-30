#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LIST (('L')|('I'<<8)|('S'<<16)|('T'<<24))
#define MOVI (('m')|('o'<<8)|('v'<<16)|('i'<<24))
#define REC (('r')|('e'<<8)|('c'<<16)|(' '<<24))
#define DB (('d')|('b'<<8))
#define DC (('d')|('c'<<8))
#define WB (('w')|('b'<<8))

int Search_FCC(FILE *file, int fcc, char list_flag, int *size);

void main()
{
	char   avi_filename[256], video_filename[256], audio_filename[256];
	FILE   *avi_file, *video_file, *audio_file;
	unsigned char   *mp4u_header = "MP4U";

	int   code, code2;
	int   movi_list_pos;
	int   movi_size;
	int   audio_size, video_size;
	unsigned char   *video_buf, *audio_buf;
	int   video_en = 0;
	int   audio_en = 0;
	int   i;
	int	audio_buf_size = 0x10000;   /* 64KB */
	int    video_buf_size = 0x100000;    /* 1MB */

	
	do{
		printf("The name of the source avi file:\n");
		gets(avi_filename);
		avi_file = fopen(avi_filename, "rb");
	}while(!avi_file);

	do{
		printf("Do you want to save the video (*.mp4u) file? [1: yes, 0: no]");
		scanf("%d", &video_en);
	}while( (video_en!=0) && (video_en!=1) );
	
	do{
		printf("Do you want to save the audio (*.mp3) file? [1: yes, 0: no]");
		scanf("%d", &audio_en);
	}while( (audio_en!=0) && (audio_en!=1) );

	if ( (video_en + audio_en) == 0 )
		goto END_PROG;	

/*=============  filenames construction ============================*/
	for(i=0; (video_filename[i]=audio_filename[i]=avi_filename[i]) != '.'; i++) ;

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


	/*write the four characters code 'MP4U' for the mp4u file*/
	if(video_en)
		fwrite(mp4u_header, 4, 1, video_file);

	/*search the position of LIST movi*/
	movi_list_pos = Search_FCC(avi_file, MOVI, 1, &movi_size);

	/* scan the 'movi' LIST chunk to extract mp4u and mp3 data */
	fseek(avi_file, movi_list_pos, SEEK_SET);

	fread(&code, 4, 1, avi_file);   /* get (2byte type ID) + (2bytes stream ID) */

	printf("\n Processing...     Please wait patiently.\n");
	i = 0;   /* loop counter for debugging */

	while( ((code>>16)==DB) || ((code>>16)==DC) || ((code>>16)==WB) || (code==LIST) )
	{
		printf("Loop %d\n", ++i);
		
		if(code == LIST)
		{
			fseek(avi_file, 4, SEEK_CUR);   /* curent location is "LIST type" */
			fread(&code2, 4, 1, avi_file);
			
			if(code2 != REC)
				break;
			else
				fread(&code, 4, 1, avi_file);    /* get (2byte type ID) + (2bytes stream ID) */
		}
		
		else  if( ((code>>16)==DB) || ((code>>16)==DC) )    /* Video chunk */
		{
			fread(&video_size, 4, 1, avi_file);   /* copy chunk size info */
			if(video_en)
				fwrite(&video_size, 4, 1, video_file);
			
			while(video_size > video_buf_size)     /* copy video chunk data */
			{
				fread(video_buf, 1, video_buf_size, avi_file);
				if(video_en)
					fwrite(video_buf, 1, video_buf_size, video_file);
				video_size -= video_buf_size;
			}			
			fread(video_buf, 1, video_size, avi_file);      /* copy the last block of video chunk data */
			if(video_en)
				fwrite(video_buf, 1, video_size, video_file);
			
			if(video_size&1)   /* odd number */
				fseek(avi_file, 1, SEEK_CUR);    /* skip the padding byte */
				/* The start of chunk data is word-aligned with respect to the start of the RIFF file. 
				If the chunk size is an odd number of bytes, a padding byte with value zero is written after chunk data. */	
		}

		else               /* Audio chunk */
		{
			fread(&audio_size, 4, 1, avi_file);      /* get audio chunk size info */
			
			while(audio_size > audio_buf_size)      /* copy audio chunk data */
			{
				fread(audio_buf, 1, audio_buf_size, avi_file);
				if(audio_en)
					fwrite(audio_buf, 1, audio_buf_size, audio_file);
				audio_size -= audio_buf_size;
			}
			fread(audio_buf, 1, audio_size, avi_file);      /* copy the last block of audio chunk data */
			if(audio_en)
				fwrite(audio_buf, 1, audio_size, audio_file);

			if(audio_size&1)   /* odd number */
				fseek(avi_file, 1, SEEK_CUR);    /* skip the padding byte */
		}
			fread(&code, 4, 1, avi_file);
	}

	/*free the allocated memory space and close the opened files*/
	free(audio_buf);
	free(video_buf);
	
	if(audio_en)
		fclose(audio_file);

	if(video_en)
		fclose(video_file);

END_PROG:
	fclose(avi_file);
	printf("\n\n ===  All done!  ===\n\n");
}

int Search_FCC(FILE *file, int fcc, char list_flag, int *size)
{
	int code, list;
	int position;   /* The position where the payload data in the chunk starts, without ID/size/type field */

	fseek(file, 12, SEEK_SET);

	if(list_flag)    /* list_flag!=0 means searching for a LIST chunk with the specified FOURCC as the list type */
	{
		while(1)
		{		
			fread(&list, 4, 1, file);
			
			if(list == LIST)    /* a LIST chunk has been found, which is maybe what we need */
			{
				fread(size, 4, 1, file);
				fread(&code, 4, 1, file);

				if(code == fcc)     /* a LIST chunk with the specified FOURCC as the list type has been found */
				{
					printf("A LIST chunk with the specified FOURCC (0x%8X) as the list type has been found.", fcc);
					position = ftell(file);
					return position;
				}
				else
					fseek(file, *size-4, SEEK_CUR);    /* Skip to the next chunk */
			}

			else    /* what encountered is not a LIST chunk, but an ordinary chunk */
			{
				fread(size, 4, 1, file);	 /* get chunk size info */
				fseek(file, *size, SEEK_CUR);  /* Skip to the next chunk */
			}
		}
	}

	else       /* list_flag==0 means searching for an ordinary chunk with the specified FOURCC as the chunk ID */
	{
		while(1)
		{
			fread(&list, 4, 1, file);

			if(list == LIST)    /* a LIST chunk has been found, which is NOT what we need */
			{
				fread(size, 4, 1, file);	 /* get list size info */
				fseek(file, *size, SEEK_CUR);  /* Skip to the next chunk */
			}
			
			else    /* what encountered is not a LIST chunk, but an ordinary chunk, which is maybe what we need */
			{
				if(list == fcc)    /* an ordinary chunk with the specified FOURCC as the chunk ID has been found */
				{
					printf("An ordinary chunk with the specified FOURCC (0x%8X) as the chunk ID has been found.", fcc);
					fread(size, 4, 1, file);	 /* get chunk size info */
					position = ftell(file);
					return position;
				}
				else
				{
					fread(size, 4, 1, file);	 /* get chunk size info */
					fseek(file, *size, SEEK_CUR);  /* Skip to the next chunk */
				}
			}
		}
	}
	
}


