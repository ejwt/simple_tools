/*
* Copyleft (c) 2016-20xx  Fu Xing (Andy)
* Author: Fu Xing
*
* File name: dark_scene_detect.c
* Abstract: The program detects dark scenes in a YUV file.
*
* Usage: dark_scene_detect <-i input_filename> [-f file_format] <-w width> <-h height>
          <-fps frames_per_second> <-t APL_detect_threshold> [-d duration_in_seconds]
*
* Current version: 1.1
* Last Modified: 2018-12-21
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>  /* for pow() */

#include "general_defines.h"
#include "wav_header.h"

static int8_t  input_filename[384];
static int8_t  APL_filename[384];
static int8_t  result_filename[384];
static int8_t  wav_filename[384];
static int8_t  temp_str0[128], temp_str1[128];

/* 2^32 - 1 = 4,294,967,295
   8-bit luma data: 255*1920*1088 = 532,684,800 < (2^32 - 1)
   8-bit luma data: 255*3840*2160 = 2,115,072,000 < (2^32 - 1)
  10-bit luma data: 255.75*1920*1088 = 534,251,520 < (2^32 - 1)
  10-bit luma data: 255.75*3840*2160 = 2,121,292,800 < (2^32 - 1)
*/
#define  MAX_WIDTH   3840
#define  MAX_HEIGHT  2160

#define  WAV_SAMPLE_RATE  8000
#define  WAV_BIT_DEPTH    16

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
  printf("\nExample: %s -i movie.yuv -f yuv420p -w 1280 -h 720 -fps 29.97 -t 19.2 -d 2.5\n", program);

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
  int32_t   i;  /* counter */
  uint8_t  *frame_buffer;  /* for storing one frame of luma data */
  uint8_t  *u8_buffer;     /* for storing one byte of luma data (the moving pointer) */
  uint8_t  *wav_buffer;     /* for storing 1 second wav data */
  uint32_t  audio_length = 0;  /* wav data length, in bytes */
  uint8_t   format_id = 0;  /* YUV format ID: 0=yuv420p, 1=yuv422p, 2=yuv444p, ... */
  uint32_t  frame_No = 0;   /* YUV frame number, counts from 0 */
  uint32_t  sample_No = 0;  /* audio sample number, counts from 0 */
  uint32_t  samples_to_fill;   /* the number of audio samples to fill into the 1 second audio buffer */

  uint32_t  u32_Y_sum = 0;   /* the sum of luma component (Y) in a frame;
                                8-bit luma data: 255*3840*2160 = 2,115,072,000 < ( 2^32 - 1)
                                10-bit luma data: 255.75*3840*2160 = 2,121,292,800 < ( 2^32 - 1)
                                which can fit into uint32_t */
                             /* To support higher resolutions in the future, we must use uint64_t */
  double    apl = 0.0;  /* Average Picture Level; 0.0 ~ 255.75; for more than 8-bit luma samples, the least significant (n-8) bits are treated as the fractional part */
  uint32_t  u32_width = 0;    /* width in luma pixels */
  uint32_t  u32_height = 0;   /* height in luma lines */
  double    frame_rate = 0.0;
  double    APL_threshold = 0.0;  /* When APL is lower than or equal to this threshold, it is considered a dark scene. (0.0 ~ 255.75) */
  double    duration_threshold_s = 0.0;    /* in seconds */
  uint32_t  duration_threshold_frame = 0;  /* in frames */
  uint32_t  dark_scene_frames = 0;  /* detected consecutive dark scene frames */
  uint8_t   is_dark_scene = 0;  /* dark scene flag */
  uint32_t  offset = 0;    /* offset in bytes relative to the start of a YUV frame */

  FILE    *fp_input;      /* input YUV file */
  FILE    *fp_out_APL;    /* APL log file *_APL.csv */
  FILE    *fp_out_result; /* final result file *_result.txt, listing the detected dark scenes */
  FILE    *fp_out_wav;    /* *_visualization.wav file for APL data visualization */
  struct wav  wav_header;
  double  scale_factor;  /* scale factor for visualizing the APL data */

/* ============= Command line parsing ============= */
  if ( (argc != 15) && (argc != 13) && (argc != 11) )
  {
    printf("\nThe number of parameters is incorrect!\n");
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
      if (u32_width == 0)
      {
        printf("width = 0, illegal.\n");
        exit(-11);
      }
    }
    else if( (strcmp("-h", argv[i]) == 0) && (i < argc - 1) )
    {
      i++;
      u32_height = atoi(argv[i]);
      if (u32_height == 0)
      {
        printf("height = 0, illegal.\n");
        exit(-12);
      }
    }
    else if( (strcmp("-fps", argv[i]) == 0) && (i < argc - 1) )
    {
      i++;
      frame_rate = atof(argv[i]);
      if (frame_rate < 1.0)
      {
        printf("frame_rate lower than 1 fps is not supported.\n");
        printf("To support it, we need to enlarge the wav buffer size (currently 1 second).\n");
        exit(-13);
      }
    }
    else if( (strcmp("-t", argv[i]) == 0) && (i < argc - 1) )
    {
      i++;
      APL_threshold = atof(argv[i]);
      if ( (APL_threshold < 0.0) || (APL_threshold > 255.75) )
      {
        printf("APL_threshold is illegal. It shoule be 0.0 ~ 255.75.\n");
        exit(-14);
      }
    }
    else if( (strcmp("-d", argv[i]) == 0) && (i < argc - 1) )
    {
      i++;
      duration_threshold_s = atof(argv[i]);
      if (duration_threshold_s < 0.0)
      {
        printf("duration_threshold_s is negative, illegal.\n");
        exit(-15);
      }
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

/* =============  Forming output filenames ============= */
  make_filenames(input_filename, APL_filename, "_APL.csv");
  make_filenames(input_filename, result_filename, "_result.txt");
  make_filenames(input_filename, wav_filename, "_visualization.wav");

/* =============== Open the files ============ */
  if ( NULL == (fp_input = fopen(input_filename, "rb")) )
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

  if ( NULL == (fp_out_wav = fopen(wav_filename, "wb")) )
  {
    printf("Error! Can't create file %s\n", wav_filename);
    fclose(fp_input);
    fclose(fp_out_APL);
    fclose(fp_out_result);
    exit(-8);
  }

/* =============== Allocate the buffers ============ */
  if ( NULL == (frame_buffer = (uint8_t *)malloc(u32_width*u32_height)) )  // 8 bpc, luma only
  {
    printf("Error! YUV frame buffer allocation failed!\n");
    fclose(fp_input);
    fclose(fp_out_APL);
    fclose(fp_out_result);
    fclose(fp_out_wav);
    exit(-9);
  }

  if ( NULL == (wav_buffer = (uint8_t *)malloc(WAV_SAMPLE_RATE*WAV_BIT_DEPTH/8)) )
  {
    printf("Error! WAV buffer allocation failed!\n");
    fclose(fp_input);
    fclose(fp_out_APL);
    fclose(fp_out_result);
    fclose(fp_out_wav);
    free(frame_buffer);
    exit(-10);
  }

  // print *_APL.csv header line
  fprintf(fp_out_APL, "frame_No, APL\n");

/* =============== write wav file header ============ */
	wav_header.ChunkID = 0x46464952;           /* letters "RIFF" in ASCII form */
	wav_header.ChunkSize = 36 + audio_length;  /* to be updated after writing the audio sample data */
	wav_header.Format = 0x45564157;            /* letters "WAVE" in ASCII form */

	wav_header.Subchunk1ID = 0x20746d66;       /* letters "fmt " in ASCII form */
	wav_header.Subchunk1Size = 16;             /* Fixed to 16 in this "dark_scene_detect" project!! */
	wav_header.AudioFormat = 1;                /* Fixed to 1 in this "dark_scene_detect" project!! */
	wav_header.NumChannels = 1;                /* Fixed to 1 in this "dark_scene_detect" project!! */
	wav_header.SampleRate = WAV_SAMPLE_RATE;
	wav_header.ByteRate = WAV_SAMPLE_RATE * wav_header.NumChannels * WAV_BIT_DEPTH / 8;
	wav_header.BlockAlign = wav_header.NumChannels * WAV_BIT_DEPTH / 8;
	wav_header.BitsPerSample = WAV_BIT_DEPTH;

	wav_header.Subchunk2ID = 0x61746164;      /* letters "data" in ASCII form */
	wav_header.Subchunk2Size = audio_length;  /* to be updated after writing the audio sample data */

  fwrite(&wav_header, 44, 1, fp_out_wav);

  scale_factor = (pow(2.0, WAV_BIT_DEPTH) - 1.0) / 255.75 - 0.02; /* 0.02 is the margin */

/* =============== dark scene detection core logic ============ */
  while ( !feof(fp_input) )
  {
    fread(frame_buffer, 1, u32_width*u32_height, fp_input);
    if (feof(fp_input))
    {
      goto END_OF_FILE;
    }

    u8_buffer = frame_buffer;
    u32_Y_sum = 0;  /* clear this accumulator at the beginning of each frame */

    for (offset=0; offset<(u32_width*u32_height); offset++)
    {
      u32_Y_sum += *u8_buffer;
      u8_buffer++;
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

    /* =============== Update *_visualization.wav =============== */
    samples_to_fill = (uint32_t)(((double)(WAV_SAMPLE_RATE * (frame_No+1)))/frame_rate + 0.5) - sample_No;

    #if (WAV_BIT_DEPTH == 8)
    {
      memset(wav_buffer, (int8_t)((apl-128.0)*scale_factor+0.5), samples_to_fill);
    }
    #elif (WAV_BIT_DEPTH == 16)
    {
      int16_t  *p_sample = (int16_t *)wav_buffer;
      int16_t  temp = (int16_t)((apl - 128.0) * scale_factor + 0.5);
      for (i=0; i<(int32_t)samples_to_fill; i++)
      {
        *p_sample = temp;
        p_sample++;
      }
    }
    #elif (WAV_BIT_DEPTH == 32)
    {
      int32_t  *p_sample = (int32_t *)wav_buffer;
      int32_t  temp = (int32_t)((apl - 128.0) * scale_factor + 0.5);
      for (i=0; i<(int32_t)samples_to_fill; i++)
      {
        *p_sample = temp;
        p_sample++;
      }
    }
    #elif (WAV_BIT_DEPTH == 24)
    {
      int8_t  *p_sample = (int8_t *)wav_buffer;
      int32_t  temp = (int32_t)((apl - 128.0) * scale_factor + 0.5);
      for (i=0; i<(int32_t)samples_to_fill; i++)
      {
        *p_sample = temp & 0xFF;  /* assuming little endian */
        p_sample++;
        *p_sample = (temp >> 8) & 0xFF;
        p_sample++;
        *p_sample = (temp >> 16) & 0xFF;
        p_sample++;
      }
    }
    #else
    {
      #error "NOT implemented!";
    }
    #endif

    fwrite(wav_buffer, 1, samples_to_fill*WAV_BIT_DEPTH/8, fp_out_wav);
    sample_No += samples_to_fill;

    /* =============== Detect dark scene and update *_result.txt =============== */
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

/*=============== Update wav file header ============ */
  audio_length = sample_No * WAV_BIT_DEPTH / 8;

	wav_header.ChunkSize = 36 + audio_length;
	wav_header.Subchunk2Size = audio_length;

  fseek(fp_out_wav, 0, SEEK_SET);
  fwrite(&wav_header, 44, 1, fp_out_wav);

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

