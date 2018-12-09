/*
* Copyleft (c) 2016-2019  Fu Xing (Andy)
* Author: Fu Xing
*
* File name: dark_scene_detect.c
* Abstract: The program detects dark scenes in a YUV file.
*
* Usage: dark_scene_detect <-i input_filename> [-f file_format] <-w width> <-h height>
          <-t APL_detect_threshold> <-fps frames_per_second>
*
* Current version: 1.0
* Last Modified: 2018-12-6
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

static uint8_t  input_filename[384];
static uint8_t  APL_filename[384];
static uint8_t  result_filename[384];

#define  MAX_WIDTH   1920
#define  MAX_HEIGHT  1088


static void usage(char *program)
{
  printf("\nThe program detects dark scenes in a YUV file.\n");
  printf("\nUsage: %s <-i input_filename> [-f file_format] <-w width> <-h height> <-t APL_detect_threshold> <-fps frames_per_second>\n", program);
  printf("\nExample: dark_scene_detect -i movie.yuv -f yuv420p -w 1280 -h 720 -t 5 -fps 29.97\n", program);
}


static void make_filenames(char *in_filename, char *out_filename, char *append_name)
{
  int  i, j, k;  /* counter */

  i = 0;
  while (in_filename[i] != '\0')
  {
    i++;
  }
  j = i; /* record the position of the end of filename */

  while ( (in_filename[i] != '.') && (i>0) )
  {
    i--;
  }

  if (i == 0)  /* There's no '.' in the in_filename. */
  {
    i = j; /* Now i points to the end of filename (the NULL character) */
  }

  for (k=0; k<i; k++)
  {
    out_filename[k] = in_filename[k];
  }

  out_filename[k] = '\0';
  strcat(out_filename, append_name);
}


/* modify a character string, changes all letters to lower case */
static void MakeLower(unsigned char *p)
{
  for(; *p; p++)
  {
    if (*p >= 'A' && *p <= 'Z')
    {
      *p += 0x20;
    }
  }
}


static char get_ID_from_format_string(char *format_string, char *ID)
{
  char  invalid_id = 1;

  MakeLower(format_string);  

  if (strcmp("yuv420p", format_string) == 0)
  {
    *ID = 0;
    invalid_id = 0;
  }
  else if (strcmp("yuv422p", format_string) == 0)
  {
    *ID = 1;
    invalid_id = 0;
  }
  else if (strcmp("yuv444p", format_string) == 0)
  {
    *ID = 2;
    invalid_id = 0;
  }

  return  invalid_id;
}

static void show_available_format_string(void)
{
  printf("\n  The following YUV file format has been implemented.\n");
  printf("    yuv420p - 4:2:0, 8 bpc, planar YUV file\n");
  printf("    yuv422p - 4:2:2, 8 bpc, planar YUV file\n");
  printf("    yuv444p - 4:4:4, 8 bpc, planar YUV file\n");
}


static void print_hhmmss_position(FILE *fp, uint32_t frame_No, double frame_rate)
{
  double s = (double)frame_No/frame_rate;
  uint32_t hh = ((uint32_t)s)/3600;  /* hours */
  uint32_t mm = ((uint32_t)s)/60 - 60*hh;  /* minutes */
  s = s - hh*3600.0 - mm*60.0;  /* seconds */
  fprintf(fp, "[frame %u] %02u:%02u:%06.3f", frame_No, hh, mm, s);
}

int main(int argc, char *argv[])
{
  int  i;  /* argument index */
	uint8_t  *frame_buffer;  /* for storing one frame of luma data */
	uint8_t  *u8_buffer;     /* for storing one byte of luma data */
	uint8_t   format_id = 0;  /* YUV format ID; 0=yuv420p, 1=yuv422p, ... */
	uint32_t  frame_No = 0;  /* frame number, counts from 0 */
//	int8_t    APL_append[] = "_APL.txt"; /* add "_APL.txt" to the log filename */
//	int8_t    result_append[] = "_result.txt"; /* add "_result.txt" to the detection result filename */
  
	uint32_t  u32_Y_sum = 0;   /* the sum of luma component (Y) in a frame; 1920*1088*1023 = 2,137,006,080, which can fit into uint32_t */
                             /* To support higher resolutions in the future, we must use uint64_t */
  double  apl;  /* Average Picture Level; 0.0 ~ 255.75 */
	uint32_t  u32_width = 0;    /* width in luma pixels */
	uint32_t  u32_height = 0;   /* height in luma pixels */
  double  APL_threshold = 0.0;
  double  frame_rate = 0.0;
  uint8_t is_dark_scene = 0;
  uint32_t  x = 0;    /* x coordinate */
  uint32_t  y = 0;    /* y coordinate */
/*
  -----------> x
  |
  |
  |
  |
  V
  
  y
*/

	FILE    *fp_input, *fp_out_APL, *fp_out_result;

/*   ==========   Command line parsing   =============   */
	if ( (argc != 13) && (argc != 11) )
	{
		printf("\nThe number of parameters is WRONG!\n");
		usage(argv[0]);
		exit(-1);
	}

	for (i=1; i<argc; i++)
	{
		if ( (strcmp("-i", argv[i]) == 0) && (i < argc - 1) )
		{
			i++;
			strcpy(input_filename, argv[i]);
		}
		else if( (strcmp("-f", argv[i]) == 0) && (i < argc - 1) )
		{
			i++;
			if (0 != get_ID_from_format_string(argv[i], &format_id))
      {
        printf("\nThe YUV file format %s is NOT supported!\n", argv[i]);
        show_available_format_string();
        exit(-1);
      }
		}
		else if( (strcmp("-w", argv[i]) == 0) && (i < argc - 1) )
		{
			i++;
			u32_width = atoi(argv[i]);
		}
		else if( (strcmp("-h", argv[i]) == 0) && (i < argc - 1) )
		{
			i++;
			u32_height = atoi(argv[i]);
		}
		else if( (strcmp("-t", argv[i]) == 0) && (i < argc - 1) )
		{
			i++;
			APL_threshold = atof(argv[i]);
		}
		else if( (strcmp("-fps", argv[i]) == 0) && (i < argc - 1) )
		{
			i++;
			frame_rate = atof(argv[i]);
		}
		else
		{
		  printf("\nUnsupported argument %s\n", argv[i]);
			usage(argv[0]);
			exit(-1);
		}
	}

/*==============  Forming output filename ==========  */
  make_filenames(input_filename, APL_filename, "_APL.csv");
  make_filenames(input_filename, result_filename, "_result.txt");

/*=============== Open the files ============ */
	if ( NULL == (fp_input= fopen(input_filename, "rb")) )
	{
		printf("Error! Can't open file %s\n", input_filename);
		exit(-1);
	}

	if ( NULL == (fp_out_APL = fopen(APL_filename, "w")) )
	{
		printf("Error! Can't create file %s\n", APL_filename);
    fclose(fp_input);
		exit(-1);
	}

	if ( NULL == (fp_out_result = fopen(result_filename, "w")) )
	{
		printf("Error! Can't create file %s\n", result_filename);
    fclose(fp_input);
    fclose(fp_out_APL);
		exit(-1);
	}

/*=============== Allocate the frame buffer ============ */
	if ( NULL == (frame_buffer = malloc(u32_width*u32_height)) )  // 8 bpc, luma only
	{
		printf("Error! Frame buffer allocation failed!\n");
    fclose(fp_input);
    fclose(fp_out_APL);
    fclose(fp_out_result);
		exit(-1);
	}

/*=============== calculates the sum of luma component (Y) ============ */
  while ( !feof(fp_input) )
  {
    fread(frame_buffer, 1, u32_width*u32_height, fp_input);
    u8_buffer = frame_buffer;

    u32_Y_sum = 0;  /* clear this accumulator at the beginning of each frame */

    for (y=0; y<u32_height; y++)
    {
      for (x=0; x<u32_width; x++)
      {
        u32_Y_sum += *u8_buffer;
        u8_buffer++;
      }
    }

    /* skip chroma samples */
    if (format_id == 0)
    {
      fread(frame_buffer, 1, (u32_width*u32_height)>>1, fp_input);
    }
    else if (format_id == 1)
    {
      fread(frame_buffer, 1, u32_width*u32_height, fp_input);
    }
    else if (format_id == 2)
    {
      fread(frame_buffer, 1, (u32_width*u32_height)<<1, fp_input);
    }

    /* Calculate APL and update *_APL.txt */
    apl = ((double)u32_Y_sum) / ((double)(u32_width*u32_height));
    fprintf(fp_out_APL, "%u, %.2f\n", frame_No, apl);

    /* Detect dark scene and update *_result.txt */
    if ( (apl <= APL_threshold) && (is_dark_scene == 0) )  /* Change from non-dark scene to dark scene */
    {
      is_dark_scene = 1;
      fprintf(fp_out_result, "Dark scene starts at ");
      print_hhmmss_position(fp_out_result, frame_No, frame_rate);
    }

    if ( (apl > APL_threshold) && (is_dark_scene == 1) )  /* Change from dark scene to non-dark scene */
    {
      is_dark_scene = 0;
      fprintf(fp_out_result, ", ends at ");
      print_hhmmss_position(fp_out_result, frame_No-1, frame_rate);
      fprintf(fp_out_result, "\n");
    }
    
    frame_No++;
  }
  
  fclose(fp_input);
  fclose(fp_out_APL);
  fclose(fp_out_result);

  if (frame_buffer != NULL)
  {
    free(frame_buffer);
    frame_buffer = NULL;
  }
  
  return  0;
}


