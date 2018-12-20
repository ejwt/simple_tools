/*
* Copyleft (c) 2016-2019  Fu Xing (Andy)
* Author: Fu Xing
*
* File name: dark_scene_detect.c
* Abstract: The program detects dark scenes in a YUV file.
*
* Usage: dark_scene_detect <-i input_filename> [-f file_format] <-w width> <-h height>
          <-fps frames_per_second> <-t APL_detect_threshold> [-d duration_in_seconds]
*
* Current version: 1.1
* Last Modified: 2018-12-14
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "wav_header.h"

static int8_t  input_filename[384];
static int8_t  APL_filename[384];
static int8_t  result_filename[384];
static int8_t  wav_filename[384];
static int8_t  temp_str0[256], temp_str1[256];

/* 2^32 - 1 = 4,294,967,295
   8-bit luma data: 255*1920*1088 = 532,684,800 < (2^32 - 1)
   8-bit luma data: 255*3840*2160 = 2,115,072,000 < (2^32 - 1)
  10-bit luma data: 255.75*1920*1088 = 534,251,520 < (2^32 - 1)
  10-bit luma data: 255.75*3840*2160 = 2,121,292,800 < (2^32 - 1)
*/
#define  MAX_WIDTH   3840
#define  MAX_HEIGHT  2160


static void show_available_format_string(void)
{
  printf("  The following YUV file formats have been implemented.\n");
  printf("    yuv420p - 4:2:0, 8 bpc, planar YUV file (default)\n");
  printf("    yuv422p - 4:2:2, 8 bpc, planar YUV file\n");
  printf("    yuv444p - 4:4:4, 8 bpc, planar YUV file\n");
}


static void usage(char *program)
{
  printf("The program detects dark scenes in a YUV file.\n");
  printf("\nUsage: %s <-i input_filename> [-f file_format] <-w width> <-h height> <-fps frames_per_second> <-t APL_detect_threshold> [-d duration_in_seconds]\n", program);
  printf("\nExample: %s -i movie.yuv -f yuv420p -w 1280 -h 720 -fps 29.97 -t 20 -d 2.5\n", program);

  printf("\ninput_filename: filename of the YUV file, with or without the full path\n");
  printf("file_format:\n");
  show_available_format_string();
  printf("width: picture width in luma pixels (integer)\n");
  printf("height: picture height in luma lines (integer)\n");
  printf("frames_per_second: frame rate in frames per second (float)\n");
  printf("APL_detect_threshold: When APL is lower than or equal to this threshold, it is considered a dark scene. (float, 0.0 ~ 255.75)\n");
  printf("duration_in_seconds: Only when the dark scene duration is greater than this many seconds, the dark scenes are logged in *_result.txt file (float)\n");
  printf("  default = 0.0, which means even dark scene as short as 1 frame is loggged.\n");
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

  while ( (in_filename[i] != '.') && (i > 0) )
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

  out_filename[k] = '\0';  /* terminate the out_filename[] string */
  strcat(out_filename, append_name);
}


/* modify a character string, changes all letters to lower case */
static void MakeLower(char *p)
{
  for (; *p; p++)
  {
    if ( (*p >= 'A') && (*p <= 'Z') )
    {
      *p += 0x20;
    }
  }
}


static int8_t get_ID_from_format_string(int8_t *format_string, uint8_t *ID)
{
  int8_t  invalid_id = 1;

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


/* print time in hh:mm:ss.ms format */
static void print_hhmmss_position(uint8_t *str, uint32_t frame_No, double frame_rate)
{
  double s = ((double)frame_No)/frame_rate;
  uint32_t hh = ((uint32_t)s)/3600;  /* hours */
  uint32_t mm = ((uint32_t)s)/60 - 60*hh;  /* minutes */
  s = s - hh*3600.0 - mm*60.0;  /* seconds */
  sprintf(str, "[frame %u] %02u:%02u:%06.3f", frame_No, hh, mm, s);
}

int main(int argc, char *argv[])
{
  int32_t  i;  /* argument index */
  uint8_t  *frame_buffer;  /* for storing one frame of luma data */
  uint8_t  *u8_buffer;     /* for storing one byte of luma data */
  uint8_t   format_id = 0;  /* YUV format ID; 0=yuv420p, 1=yuv422p, 2=yuv444p, ... */
  uint32_t  frame_No = 0;  /* frame number, counts from 0 */

  uint32_t  u32_Y_sum = 0;   /* the sum of luma component (Y) in a frame;
                                8-bit luma data: 255*3840*2160 = 2,115,072,000 < ( 2^32 - 1)
                                10-bit luma data: 255.75*3840*2160 = 2,121,292,800 < ( 2^32 - 1)
                                which can fit into uint32_t */
                             /* To support higher resolutions in the future, we must use uint64_t */
  double  apl = 0.0;  /* Average Picture Level; 0.0 ~ 255.75; for more than 8-bit samples, (n-8) bits are treated as fraction */
  uint32_t  u32_width = 0;    /* width in luma pixels */
  uint32_t  u32_height = 0;   /* height in luma lines */
  double  frame_rate = 0.0;
  double  APL_threshold = 0.0;
  double    duration_threshold_s = 0.0;    /* in seconds */
  uint32_t  duration_threshold_frame = 0;  /* in frames */
  uint32_t  dark_scene_frames = 0;  /* in frames */
  uint8_t is_dark_scene = 0;  /* dark scene flag */
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

  FILE    *fp_input;      /* input YUV file */
  FILE    *fp_out_APL;    /* APL log file *_APL.csv */
  FILE    *fp_out_result; /* final result file *_result.txt, listing the detected dark scenes */
  FILE    *fp_out_wav;    /* *_visualization.wav file for data visualization; the length of the wave file is 2 seconds longer than the YUV file */

/*   ==========   Command line parsing   =============   */
  if ( (argc != 15) && (argc != 13) && (argc != 11) )
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
        exit(-2);
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
    else if( (strcmp("-fps", argv[i]) == 0) && (i < argc - 1) )
    {
      i++;
      frame_rate = atof(argv[i]);
    }
    else if( (strcmp("-t", argv[i]) == 0) && (i < argc - 1) )
    {
      i++;
      APL_threshold = atof(argv[i]);
    }
    else if( (strcmp("-d", argv[i]) == 0) && (i < argc - 1) )
    {
      i++;
      duration_threshold_s = atof(argv[i]);
      duration_threshold_frame = (uint32_t)(duration_threshold_s * frame_rate + 0.5);
    }
    else
    {
      printf("\nUnsupported argument %s\n", argv[i]);
      usage(argv[0]);
      exit(-3);
    }
  }

  if (u32_width*u32_height > MAX_WIDTH*MAX_HEIGHT)
  {
    printf("\nERROR! Resolution %ux%u is too high, current program does NOT support it.\n", u32_width, u32_height);
    exit(-4);
  }

/*==============  Forming output filename ==========  */
  make_filenames(input_filename, APL_filename, "_APL.csv");
  make_filenames(input_filename, result_filename, "_result.txt");
  make_filenames(input_filename, wav_filename, "_visualization.wav");

/*=============== Open the files ============ */
  if ( NULL == (fp_input= fopen(input_filename, "rb")) )
  {
    printf("Error! Can't open file %s\n", input_filename);
    exit(-5);
  }

  if ( NULL == (fp_out_APL = fopen(APL_filename, "w")) )
  {
    printf("Error! Can't create file %s\n", APL_filename);
    fclose(fp_input);
    exit(-6);
  }

  if ( NULL == (fp_out_result = fopen(result_filename, "w")) )
  {
    printf("Error! Can't create file %s\n", result_filename);
    fclose(fp_input);
    fclose(fp_out_APL);
    exit(-7);
  }

  if ( NULL == (fp_out_wav = fopen(wav_filename, "w")) )
  {
    printf("Error! Can't create file %s\n", wav_filename);
    fclose(fp_input);
    fclose(fp_out_APL);
    fclose(fp_out_result);
    exit(-8);
  }

/*=============== Allocate the frame buffer ============ */
  if ( NULL == (frame_buffer = (uint8_t *)malloc(u32_width*u32_height)) )  // 8 bpc, luma only
  {
    printf("Error! Frame buffer allocation failed!\n");
    fclose(fp_input);
    fclose(fp_out_APL);
    fclose(fp_out_result);
    fclose(fp_out_wav);
    exit(-9);
  }

  // print *_APL.csv header line
  fprintf(fp_out_APL, "frame_No, APL\n");

/*=============== calculates the sum of luma component (Y) ============ */
  while ( !feof(fp_input) )
  {
    fread(frame_buffer, 1, u32_width*u32_height, fp_input);
    if (feof(fp_input))
    {
      goto END_OF_FILE;
    }

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
    if (format_id == 0)  /* YCbCr 4:2:0 */
    {
      fread(frame_buffer, 1, (u32_width*u32_height)>>1, fp_input);
    }
    else if (format_id == 1)  /* YCbCr 4:2:2 */
    {
      fread(frame_buffer, 1, u32_width*u32_height, fp_input);
    }
    else if (format_id == 2)  /* YCbCr 4:4:4 */
    {
      fread(frame_buffer, 1, u32_width*u32_height, fp_input);
      fread(frame_buffer, 1, u32_width*u32_height, fp_input);
    }
    else
    {
      printf("Internal error!\n");
      goto CLEAN_UP;
    }

    if (feof(fp_input))
    {
      goto END_OF_FILE;
    }

    /* Calculate APL and update *_APL.txt */
    apl = ((double)u32_Y_sum) / ((double)(u32_width*u32_height));
    fprintf(fp_out_APL, "%u, %.2f\n", frame_No, apl);

    /* Detect dark scene and update *_result.txt */
    if ( apl <= APL_threshold )
    {
      dark_scene_frames++;
    }

    if ( (apl <= APL_threshold) && (is_dark_scene == 0) )  /* Change from non-dark scene to dark scene */
    {
      is_dark_scene = 1;
      memset(temp_str0, 0x00, sizeof(temp_str0));
      memset(temp_str1, 0x00, sizeof(temp_str1));
      sprintf(temp_str0, "Dark scene starts at ");
      print_hhmmss_position(temp_str1, frame_No, frame_rate);
      strcat(temp_str0, temp_str1);
    }

    if ( (apl > APL_threshold) && (is_dark_scene == 1) )  /* Change from dark scene to non-dark scene */
    {
END_OF_FILE:
      is_dark_scene = 0;
      memset(temp_str1, 0x00, sizeof(temp_str1));
      sprintf(temp_str1, ", ends at ");
      strcat(temp_str0, temp_str1);

      memset(temp_str1, 0x00, sizeof(temp_str1));
      print_hhmmss_position(temp_str1, frame_No-1, frame_rate);
      strcat(temp_str0, temp_str1);
      strcat(temp_str0, "\n");

      if (dark_scene_frames > duration_threshold_frame)
      {
        fprintf(fp_out_result, temp_str0);
      }

      dark_scene_frames = 0;
    }

    frame_No++;
  }

CLEAN_UP:
  fclose(fp_input);
  fclose(fp_out_APL);
  fclose(fp_out_result);
  fclose(fp_out_wav);

  if (frame_buffer != NULL)
  {
    free(frame_buffer);
    frame_buffer = NULL;
  }

  return  0;
}

