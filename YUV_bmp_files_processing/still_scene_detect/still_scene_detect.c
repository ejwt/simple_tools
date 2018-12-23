/*
* Copyleft (c) 2018-20xx  Fu Xing (Andy)
* Author: Fu Xing
*
* File name: still_scene_detect.c
* Abstract: The program detects still scenes in a YUV file.
*
* Usage: still_scene_detect <-i input_filename> [-f file_format] <-w width> <-h height>
          <-fps frames_per_second> <-t activity_threshold> [-d duration_in_seconds]
*
* Current version: 0.1
* Last Modified: 2018-12-19
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>  /* for pow() */

#include "general_defines.h"
#include "wav_header.h"

static int8_t  input_filename[384];
static int8_t  activity_filename[384];
static int8_t  result_filename[384];
static int8_t  wav_filename[384];
static int8_t  temp_str0[128], temp_str1[128];

/* 2^32 - 1 = 4,294,967,295
   8-bit luma data: 255*1920*1088 = 532,684,800 < (2^32 - 1)
   8-bit luma data: 255*3840*2160 = 2,115,072,000 < (2^32 - 1)
  10-bit luma data: 255.75*1920*1088 = 534,251,520 < (2^32 - 1)
  10-bit luma data: 255.75*3840*2160 = 2,121,292,800 < (2^32 - 1)
  Frame activity weight: Y-6/8, Cb-1/8, Cr-1/8.
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
  printf("The program detects still scenes in a YUV file.\n");
  printf("\nUsage: %s <-i input_filename> [-f file_format] <-w width> <-h height> <-fps frames_per_second> <-t activity_threshold> [-d duration_in_seconds]\n", program);
  printf("\nExample: %s -i movie.yuv -f yuv420p -w 1280 -h 720 -fps 29.97 -t 19.2 -d 2.5\n", program);

  printf("\ninput_filename: filename of the YUV file, with or without the full path\n");
  printf("file_format:\n");
  show_available_format_string();
  printf("width: picture width in luma pixels (integer)\n");
  printf("height: picture height in luma lines (integer)\n");
  printf("frames_per_second: frame rate in frames per second (float)\n");
  printf("activity_threshold: When frame activity is lower than or equal to this threshold, it is considered a still scene. (float, 0.0 ~ 255.75)\n");
  printf("duration_in_seconds: Only when the still scene duration is greater than this many seconds, the still scenes are logged in *_result.txt file (float)\n");
  printf("  default = 0.0, which means even still scene as short as 1 frame is loggged.\n");
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


/* calculate the activity and update *_activity.csv */
static double calculate_activity(FILE *fp_log, uint8_t *buffer_A, uint8_t *buffer_B, uint32_t width, uint32_t height, uint8_t format_id,
                                   double *Y_activity, double *Cb_activity, double *Cr_activity)
{
  uint8_t  *pA, *pB;
  double  frame_activity = 0.0;   /* the average activity of the whole frame; 0.0 ~ 255.75; frame activity weight: Y-6/8, Cb-1/8, Cr-1/8 */

  uint32_t  u32_Y_act = 0;   /* the accumulated activity of luma component (Y) in a frame */
  uint32_t  u32_Cb_act = 0;  /* the accumulated activity of Cb component in a frame */
  uint32_t  u32_Cr_act = 0;  /* the accumulated activity of Cr component in a frame */

  uint32_t  i = 0;    /* counter */
  uint32_t  chroma_size = 0;    /* the number of bytes of ONE chroma component (8 bpc) */

  // luma
  pA = buffer_A;
  pB = buffer_B;
  for (i=0; i<width*height; i++)
  {
    u32_Y_act += ABS(*pA - *pB);
    pA++;
    pB++;
  }

  // chroma
  if (format_id == 0)  /* YCbCr 4:2:0 */
  {
    chroma_size = width * height / 4;
  }
  else if (format_id == 1) /* YCbCr 4:2:2 */
  {
    chroma_size = width * height / 2;
  }
  else if (format_id == 2) /* YCbCr 4:4:4 */
  {
    chroma_size = width * height;
  }
  else
  {
    printf("Internal ERROR 2\n");
    exit(-10);
  }

  for (i=0; i<chroma_size; i++)
  {
    u32_Cb_act += ABS(*pA - *pB);
    pA++;
    pB++;
  }

  for (i=0; i<chroma_size; i++)
  {
    u32_Cr_act += ABS(*pA - *pB);
    pA++;
    pB++;
  }

  *Y_activity = ((double)u32_Y_act)/((double)(width*height));
  *Cb_activity = ((double)u32_Cb_act)/((double)chroma_size);
  *Cr_activity = ((double)u32_Cr_act)/((double)chroma_size);
  frame_activity = *Y_activity*0.75 + *Cb_activity/8.0 + *Cr_activity/8.0;

  // update *_activity.csv
  fprintf(fp_log, "%.2f, %.2f, %.2f, %.2f\n", *Y_activity, *Cb_activity, *Cr_activity, frame_activity);

  return  frame_activity;
}




int main(int argc, char *argv[])
{
  int32_t   i;  /* argument index */
  uint8_t  *frame_buffer;   /* for storing 2 frames of YCbCr data */
  uint8_t  *wav_buffer;     /* for storing 1 second wav data */
  uint32_t  audio_length = 0;  /* wav data length, in bytes */
  uint8_t  *buffer_base[2];    /* pointing to the base addresses of the frames to be compared (to calculate activity) */
  uint8_t   format_id = 0;  /* YUV format ID; 0=yuv420p, 1=yuv422p, 2=yuv444p, ... */
  uint32_t  frame_No = 0;   /* frame number, counts from 0 */
  uint32_t  sample_No = 0;  /* audio sample number, counts from 0 */
  uint32_t  samples_to_fill;   /* the number of audio samples to fill into the 1 second audio buffer */

  double  Y_activity = 0.0;   /* the average activity of luma component (Y) in a frame; 0.0 ~ 255.75 */
  double  Cb_activity = 0.0;  /* the average activity of Cb component in a frame; 0.0 ~ 255.75 */
  double  Cr_activity = 0.0;  /* the average activity of Cr component in a frame; 0.0 ~ 255.75 */
  double  frame_activity = 0.0;   /* the average activity of the whole frame; 0.0 ~ 255.75; frame activity weight: Y-6/8, Cb-1/8, Cr-1/8 */

  uint32_t  u32_width = 0;    /* width in luma pixels */
  uint32_t  u32_height = 0;   /* height in luma lines */
  uint32_t  buffer_size = 0;  /* buffer size in bytes */
  double    frame_rate = 0.0;
  double    activity_threshold = 0.0;
  double    duration_threshold_s = 0.0;    /* in seconds */
  uint32_t  duration_threshold_frame = 0;  /* in frames */
  uint32_t  still_scene_frames = 0;        /* detected consecutive still scene frames */
  uint8_t   is_still_scene = 0;  /* still scene flag */

  FILE    *fp_input;         /* input YUV file */
  FILE    *fp_out_activity;  /* activity log file *_activity.csv */
  FILE    *fp_out_result;    /* final result file *_result.txt, listing the detected dark scenes */
  FILE    *fp_out_wav;       /* *_visualization.wav file for APL data visualization */
  struct wav  wav_header;
  double  scale_factor;  /* scale factor for visualizing the APL data */

/*   ==========   Command line parsing   =============   */
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
      activity_threshold = atof(argv[i]);
      if ( (activity_threshold < 0.0) || (activity_threshold > 255.75) )
      {
        printf("activity_threshold is illegal. It shoule be 0.0 ~ 255.75.\n");
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

/*==============  Forming output filenames ==========  */
  make_filenames(input_filename, activity_filename, "_activity.csv");
  make_filenames(input_filename, result_filename, "_result.txt");
  make_filenames(input_filename, wav_filename, "_visualization.wav");

/*=============== Open the files ============ */
  if ( NULL == (fp_input = fopen(input_filename, "rb")) )
  {
    printf("Error! Can't open file %s\n", input_filename);
    exit(-5);
  }

  if ( NULL == (fp_out_activity = fopen(activity_filename, "w")) )
  {
    printf("Error! Can't create file %s\n", activity_filename);
    fclose(fp_input);
    exit(-6);
  }

  if ( NULL == (fp_out_result = fopen(result_filename, "w")) )
  {
    printf("Error! Can't create file %s\n", result_filename);
    fclose(fp_input);
    fclose(fp_out_activity);
    exit(-7);
  }

  if ( NULL == (fp_out_wav = fopen(wav_filename, "wb")) )
  {
    printf("Error! Can't create file %s\n", wav_filename);
    fclose(fp_input);
    fclose(fp_out_activity);
    fclose(fp_out_result);
    exit(-8);
  }

/*=============== Allocate the frame buffer: 8 bpc, luma + chroma, 2 frames ============ */
  if (format_id == 0)  /* YCbCr 4:2:0 */
  {
    buffer_size = u32_width * u32_height * 3;  /* 1.5*2 */
  }
  else if (format_id == 1) /* YCbCr 4:2:2 */
  {
    buffer_size = u32_width * u32_height * 4;  /* 2*2 */
  }
  else if (format_id == 2) /* YCbCr 4:4:4 */
  {
    buffer_size = u32_width * u32_height * 6;  /* 3*2 */
  }
  else
  {
    printf("Internal ERROR 1\n");
    fclose(fp_input);
    fclose(fp_out_activity);
    fclose(fp_out_result);
    fclose(fp_out_wav);
    exit(-8);
  }

  if ( NULL == (frame_buffer = (uint8_t *)malloc(buffer_size)) )
  {
    printf("Error! YUV frame buffer allocation failed!\n");
    fclose(fp_input);
    fclose(fp_out_activity);
    fclose(fp_out_result);
    fclose(fp_out_wav);
    exit(-9);
  }

  buffer_base[0] = frame_buffer;
  buffer_base[1] = frame_buffer + buffer_size/2;

/* =============== prepare the wav file header ============ */
	wav_header.ChunkID = 0x46464952;           /* letters "RIFF" in ASCII form */
	wav_header.ChunkSize = 36 + audio_length;  /* to be updated after writing the audio sample data */
	wav_header.Format = 0x45564157;            /* letters "WAVE" in ASCII form */

	wav_header.Subchunk1ID = 0x20746d66;       /* letters "fmt " in ASCII form */
	wav_header.Subchunk1Size = 16;             /* Fixed to 16 in this "still_scene_detect" project!! */
	wav_header.AudioFormat = 1;                /* Fixed to 1 in this "still_scene_detect" project!! */
	wav_header.NumChannels = 2;                /* Fixed to 2 in this "still_scene_detect" project!! */
	wav_header.SampleRate = WAV_SAMPLE_RATE;
	wav_header.ByteRate = WAV_SAMPLE_RATE * wav_header.NumChannels * WAV_BIT_DEPTH / 8;
	wav_header.BlockAlign = wav_header.NumChannels * WAV_BIT_DEPTH / 8;
	wav_header.BitsPerSample = WAV_BIT_DEPTH;

	wav_header.Subchunk2ID = 0x61746164;      /* letters "data" in ASCII form */
	wav_header.Subchunk2Size = audio_length;  /* to be updated after writing the audio sample data */

/*=============== Allocate 1 second wav buffer (wav_header.NumChannels-channel audio) ============ */
  if ( NULL == (wav_buffer = (uint8_t *)malloc(wav_header.NumChannels*WAV_SAMPLE_RATE*WAV_BIT_DEPTH/8)) )
  {
    printf("Error! WAV buffer allocation failed!\n");
    fclose(fp_input);
    fclose(fp_out_activity);
    fclose(fp_out_result);
    fclose(fp_out_wav);
    free(frame_buffer);
    exit(-10);
  }

  // print *_activity.csv header line
  fprintf(fp_out_activity, "frame_No, Y_activity, Cb_activity, Cr_activity, frame_activity\n");

  // write the wav file header
  fwrite(&wav_header, 44, 1, fp_out_wav);

  scale_factor = (pow(2.0, WAV_BIT_DEPTH) - 1.0) / 256.0 - 0.02; /* 0.02 is the margin */

/*=============== calculates the activities ============ */
  while ( !feof(fp_input) )
  {
    fread(buffer_base[frame_No&1], 1, buffer_size/2, fp_input);
    if (feof(fp_input))
    {
      goto END_OF_FILE;
    }

    /* Calculate the activities and update *_activity.txt */
    if (frame_No > 0)
    {
      fprintf(fp_out_activity, "%u, ", frame_No);
      frame_activity = calculate_activity(fp_out_activity, buffer_base[0], buffer_base[1], u32_width, u32_height, format_id,
                                          &Y_activity, &Cb_activity, &Cr_activity);

      /* =============== Update *_visualization.wav =============== */
      samples_to_fill = (uint32_t)(((double)(WAV_SAMPLE_RATE * (frame_No+1)))/frame_rate + 0.5) - sample_No;

      #if (WAV_BIT_DEPTH == 8)
      {
        // treat 8-bit wav data as unsigned.
        uint8_t  *p_sample = wav_buffer;
        uint8_t  temp1 = (uint8_t)(Y_activity * scale_factor + 0.5);
        uint8_t  temp2 = (uint8_t)(frame_activity * scale_factor + 0.5);

        for (i=0; i<(int32_t)samples_to_fill; i++)
        {
          *p_sample = temp1;
          p_sample++;
          *p_sample = temp2;
          p_sample++;
        }
      }
      #elif (WAV_BIT_DEPTH == 16)
      {
        int16_t  *p_sample = (int16_t *)wav_buffer;
        int16_t  temp1 = (int16_t)((Y_activity - 128.0) * scale_factor + 0.5);
        int16_t  temp2 = (int16_t)((frame_activity - 128.0) * scale_factor + 0.5);

        for (i=0; i<(int32_t)samples_to_fill; i++)
        {
          *p_sample = temp1;
          p_sample++;
          *p_sample = temp2;
          p_sample++;
        }
      }
      #elif (WAV_BIT_DEPTH == 32)
      {
        int32_t  *p_sample = (int32_t *)wav_buffer;
        int32_t  temp1 = (int32_t)((Y_activity - 128.0) * scale_factor + 0.5);
        int32_t  temp2 = (int32_t)((frame_activity - 128.0) * scale_factor + 0.5);

        for (i=0; i<(int32_t)samples_to_fill; i++)
        {
          *p_sample = temp1;
          p_sample++;
          *p_sample = temp2;
          p_sample++;
        }
      }
      #elif (WAV_BIT_DEPTH == 24)
      {
        int8_t  *p_sample = (int8_t *)wav_buffer;
        int32_t  temp1 = (int32_t)((Y_activity - 128.0) * scale_factor + 0.5);
        int32_t  temp2 = (int32_t)((frame_activity - 128.0) * scale_factor + 0.5);
        for (i=0; i<(int32_t)samples_to_fill; i++)
        {
          *p_sample = temp1 & 0xFF;  /* assuming little endian */
          p_sample++;
          *p_sample = (temp1 >> 8) & 0xFF;
          p_sample++;
          *p_sample = (temp1 >> 16) & 0xFF;
          p_sample++;

          *p_sample = temp2 & 0xFF;  /* assuming little endian */
          p_sample++;
          *p_sample = (temp2 >> 8) & 0xFF;
          p_sample++;
          *p_sample = (temp2 >> 16) & 0xFF;
          p_sample++;
        }
      }
      #else
      {
        #error "NOT implemented!";
      }
      #endif

      fwrite(wav_buffer, 1, wav_header.NumChannels * samples_to_fill * WAV_BIT_DEPTH / 8, fp_out_wav);
      sample_No += samples_to_fill;

      /* =============== Detect still scene and update *_result.txt =============== */
      if ( frame_activity <= activity_threshold )
      {
        still_scene_frames++;
      }

      if ( (frame_activity <= activity_threshold) && (is_still_scene == 0) )  /* Change from non-still scene to still scene */
      {
        is_still_scene = 1;
        memset(temp_str0, 0x00, sizeof(temp_str0));
        memset(temp_str1, 0x00, sizeof(temp_str1));
        sprintf(temp_str0, "Still scene starts at ");
        print_hhmmss_position(temp_str1, frame_No, frame_rate);
        strcat(temp_str0, temp_str1);
      }

      if ( (frame_activity > activity_threshold) && (is_still_scene == 1) )  /* Change from still scene to non-still scene */
      {
  END_OF_FILE:
        is_still_scene = 0;
        memset(temp_str1, 0x00, sizeof(temp_str1));
        sprintf(temp_str1, ", ends at ");
        strcat(temp_str0, temp_str1);

        memset(temp_str1, 0x00, sizeof(temp_str1));
        print_hhmmss_position(temp_str1, frame_No-1, frame_rate);
        strcat(temp_str0, temp_str1);
        strcat(temp_str0, "\n");

        if (still_scene_frames > duration_threshold_frame)
        {
          fprintf(fp_out_result, temp_str0);
        }

        still_scene_frames = 0;
      }
    }

    frame_No++;
  }

/*=============== Update wav file header ============ */
  audio_length = wav_header.NumChannels * sample_No * WAV_BIT_DEPTH / 8;

	wav_header.ChunkSize = 36 + audio_length;
	wav_header.Subchunk2Size = audio_length;

  fseek(fp_out_wav, 0, SEEK_SET);
  fwrite(&wav_header, 44, 1, fp_out_wav);

  fclose(fp_input);
  fclose(fp_out_activity);
  fclose(fp_out_result);
  fclose(fp_out_wav);

  if (frame_buffer != NULL)
  {
    free(frame_buffer);
    frame_buffer = NULL;
  }

  if (wav_buffer != NULL)
  {
    free(wav_buffer);
    wav_buffer = NULL;
  }

  return  0;
}

