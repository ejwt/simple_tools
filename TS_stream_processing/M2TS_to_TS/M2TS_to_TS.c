/*
* Copyleft (c) 2016-2019  Fu Xing (Andy)
* Author: Fu Xing (Andy)
*
* File name: M2TS_to_TS.c
* Abstract: This program converts *.m2ts and *.mts to *.ts files.
*
* Current version: 0.2
* Last Modified: 2018-11-19
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <errno.h>

/* filenames construction */
static void make_filenames(char *in_filename, char *out_filename)
{
  char *p, *q;
  int  i = 0;
  p = in_filename;

  while ( *p != '\0' )
    p++;

  while ( (*p != '.') && (p > in_filename) )
    p--;    /* p points to the last "."; or the beginning of the input file name, if there's no "." in the input filename */

  if (p == in_filename)   /* There's no "." in input filename */
  {
    strcpy(out_filename, in_filename);
  }
  else    /* There's at least one "." in original filename */
  {
    q = p;  /* Record the position of the last (right-most) "." using pointer q */
    p = in_filename;
    while (p < q)
      out_filename[i++] = *p ++;    /* only keep the portion before the last "." */

    out_filename[i] = '\0';     /* form a null terminated ASCII string. */
  }

  strcat(out_filename, ".ts");
}


static void M2TS_to_TS(char *in_filename, FILE *log_file, unsigned int *error_count, unsigned int *skip_count, unsigned char *buffer)
{
  char   out_filename[320]; /* filenames */
  FILE   *fp_in, *fp_out;
  int    return_value1, return_value2;
  long   initial_offset = 4;
  char   remove_output_file_flag = 0;

/*========== open the input file ====================*/
  if ( NULL == (fp_in = fopen(in_filename, "rb")) )
  {
    printf("\nError! Can't open input file %s\n", in_filename);
    fprintf(log_file, "Error! Can't open input file %s\n", in_filename);
    (*error_count)++;
    return;
  }

/*========== detect input file format ====================*/
  // For most *.m2ts and *.mts files, the 1st sync byte is at offset = 4,
  // for some *.m2ts and *.mts files, the 1st sync byte is at offset = 0.
  fread(buffer, 1, 5, fp_in);

  if ( (buffer[0] != 0x47) && (buffer[4] == 0x47) )  // the 1st sync byte is at offset = 4
  {
    initial_offset = 4;
  }
  else if ( (buffer[0] == 0x47) && (buffer[4] != 0x47) )  // the 1st sync byte is at offset = 0
  {
    initial_offset = 0;
    printf("\n%s: Warning! Special file: The 1st sync byte is at offset = 0.\n", in_filename);
    fprintf(log_file, "%s: Warning! Special file: The 1st sync byte is at offset = 0.\n", in_filename);
  }
  else
  {
    printf("\n Unsupported input file format! SKIP %s\n", in_filename);
    fprintf(log_file, "Unsupported input file format! SKIP %s\n", in_filename);
    (*skip_count)++;
    goto clean_up_A;
  }

  fseek(fp_in, initial_offset, SEEK_SET);

/*========== open the output file  ====================*/
  make_filenames(in_filename, out_filename);

  if ( NULL == (fp_out = fopen(out_filename, "wb")) )
  {
    printf("\n%s: Error! Can't create ouput file %s\n", in_filename, out_filename);
    fprintf(log_file, "\n%s: Error! Can't create ouput file %s\n", in_filename, out_filename);
    (*error_count)++;
    goto clean_up_A;
  }

/*========== Kernel processing  ====================*/
  while ( !feof(fp_in) )
  {
    return_value1 = fread(buffer, 1, 192, fp_in);
    if (*buffer != 0x47)  // sync byte error
    {
      printf("\nSync byte error. SKIP %s!\n", in_filename);
      fprintf(log_file, "Sync byte error. SKIP %s!\n", in_filename);
      (*skip_count)++;
      remove_output_file_flag = 1;
      goto clean_up_C;
    }

    return_value2 = fwrite(buffer, 1, 188, fp_out);
    if (return_value2 != 188)
    {
      printf("\n%s: ERROR writing ouput file %s\n", in_filename, out_filename);
      fprintf(log_file, "\n%s: ERROR writing ouput file %s\n", in_filename, out_filename);
      (*error_count)++;
      remove_output_file_flag = 1;
      goto clean_up_C;
    }

    if (return_value1 != 192)  // end of file reached
    {
      break;
    }
  }

clean_up_C:
  if (fp_out != NULL)
  {
    fclose(fp_out);
    if (remove_output_file_flag)
    {
      if (-1 == remove(out_filename) )
      {
        printf("\n Can't delete output file %s. You need to delete it manually.\n", out_filename);
        fprintf(log_file, "Can't delete output file %s. You need to delete it manually.\n", out_filename);
      }
      else
      {
        printf("\n Output file %s has been deleted!\n", out_filename);
        fprintf(log_file, "Output file %s has been deleted!\n", out_filename);
      }
    }
  }

clean_up_A:
  if (fp_in != NULL)
    fclose(fp_in);
}


int main()
{
  char   in_filename[320];
  FILE   *log_file;
  struct _finddata_t  ffblk;  /* for searching *.m2ts and *.mts files */
  intptr_t   search_handle;   /* for searching *.m2ts and *.mts files */
  errno_t    err;             /* for searching *.m2ts and *.mts files */
  int    return_value = 0;    /* for searching *.m2ts and *.mts files */
  unsigned char *buffer;      /* 192-byte buffer */

  unsigned int files_count_in = 0;  /* the number of input files */
  unsigned int error_count = 0;
  unsigned int skip_count = 0;
  unsigned int previous_error_count = 0;
  unsigned int previous_skip_count = 0;

  if ( NULL == (log_file = fopen("log.txt", "w")) )
  {
    printf("\nERROR! Can't create log.txt.\n");
    return  -1;
  }

/*========== buffer allocation  ====================*/
  if ( NULL == (buffer = (unsigned char *)malloc(192)) )
  {
    printf("\nERROR! Insufficient memory in allocating 192-byte buffer!\n");
    fprintf(log_file, "ERROR! Insufficient memory in allocating 192-byte buffer!\n");
    goto clean_up_end;
  }

/*   ==========   Process the first M2TS file   =============   */
  /*
    intptr_t _findfirst(
       const char *filespec,
       struct _finddata_t *fileinfo
    );

    Provide information about the first instance of a file name that matches the file
    specified in the filespec argument.

    [ Parameters ]
      filespec
        Target file specification (can include wildcard characters).

      fileinfo
        File information buffer.

    [ Return Value ]
      If successful, _findfirst returns a unique search handle identifying the file or
      group of files that match the filespec specification, which can be used in a
      subsequent call to _findnext or to _findclose. Otherwise, _findfirst returns -1 and
      sets errno to one of the following values.

      EINVAL
        Invalid parameter: filespec or fileinfo was NULL. Or, the operating system returned an unexpected error.

      ENOENT
        File specification that could not be matched.

      ENOMEM
        Insufficient memory.

      EINVAL
        Invalid file name specification or the file name given was larger than MAX_PATH.
  */
  search_handle = _findfirst("*.m2ts", &ffblk);  /* search for the first M2TS file */
  if (search_handle == -1)  // error occurred
  {
    _get_errno(&err);
    if (err == EINVAL)
    {
      printf("\nDuring the first searching for *.m2ts files, the operating system returned an unexpected error.\n");
      printf("Is the path too long?\n");
    }
    else if (err == ENOENT)
    {
      printf("\nNo .m2ts files in current directory!\n");
    }
    else if (err == ENOMEM)
    {
      printf("\nInsufficient memory during the first searching for *.m2ts files!\n");
    }
    else
    {
      printf("\nInternal error during the first searching for *.m2ts files!\n");
    }

    goto MTS_FILES;
  }

  strcpy(in_filename, ffblk.name);
  M2TS_to_TS(in_filename, log_file, &error_count, &skip_count, buffer);
  previous_error_count = error_count;
  previous_skip_count = skip_count;
  files_count_in++;

/*   ==========  START OF MAIN PROCESSING LOOP (Process other M2TS files)  =============   */
  /*
    int _findnext(
       intptr_t handle,
       struct _finddata_t *fileinfo
    );

    Find the next name, if any, that matches the filespec argument in a previous call to _findfirst,
    and then alter the fileinfo structure contents accordingly.

    [ Parameters ]
      handle
        Search handle returned by a previous call to _findfirst.

      fileinfo
        File information buffer.

    [ Return Value ]
      If successful, returns 0. Otherwise, returns -1 and sets errno to a value indicating
      the nature of the failure. Possible error codes are shown in the following table.

      EINVAL
        Invalid parameter: fileinfo was NULL. Or, the operating system returned an unexpected error.

      ENOENT
        No more matching files could be found.

      ENOMEM
        Not enough memory or the file name's length exceeded MAX_PATH.
  */
  do
  {
    return_value = _findnext(search_handle, &ffblk);
    if (return_value == -1)  // error occurred
    {
      _get_errno(&err);
      if (err == EINVAL)
      {
        printf("\nDuring subsequent searching for *.m2ts files, the operating system returned an unexpected error.\n");
      }
      else if (err == ENOMEM)
      {
        printf("\nInsufficient memory during subsequent searching for *.m2ts files!\n");
        printf("Is the path too long?\n");
      }

      break;
    }

    strcpy(in_filename, ffblk.name);
    M2TS_to_TS(in_filename, log_file, &error_count, &skip_count, buffer);

    // new errors during processing this M2TS file
    if ( (error_count != previous_error_count) || (skip_count != previous_skip_count) )
    {
      previous_error_count = error_count;
      previous_skip_count = skip_count;
      printf("\n========================================\n\n");
      fprintf(log_file, "========================================\n\n");
    }
    files_count_in++;
  } while(1);
/*   ==========  END OF MAIN PROCESSING LOOP (Process other M2TS files) =============   */

MTS_FILES:
  _findclose(search_handle);

/*   ==========   Process the first MTS file   =============   */
  search_handle = _findfirst("*.mts", &ffblk);  /* search for the first MTS file */
  if (search_handle == -1)  // error occurred
  {
    _get_errno(&err);
    if (err == EINVAL)
    {
      printf("\nDuring the first searching for *.mts files, the operating system returned an unexpected error.\n");
      printf("Is the path too long?\n");
    }
    else if (err == ENOENT)
    {
      printf("\nNo .mts files in current directory!\n");
    }
    else if (err == ENOMEM)
    {
      printf("\nInsufficient memory during the first searching for *.mts files!\n");
    }
    else
    {
      printf("\nInternal error during the first searching for *.mts files!\n");
    }

    goto CLEAN_UP;
  }

  strcpy(in_filename, ffblk.name);
  M2TS_to_TS(in_filename, log_file, &error_count, &skip_count, buffer);
  previous_error_count = error_count;
  previous_skip_count = skip_count;
  files_count_in++;

/*   ==========  START OF MAIN PROCESSING LOOP (Process other MTS files)  =============   */
  do
  {
    return_value = _findnext(search_handle, &ffblk);
    if (return_value == -1)  // error occurred
    {
      _get_errno(&err);
      if (err == EINVAL)
      {
        printf("\nDuring subsequent searching for *.mts files, the operating system returned an unexpected error.\n");
      }
      else if (err == ENOMEM)
      {
        printf("\nInsufficient memory during subsequent searching for *.mts files!\n");
        printf("Is the path too long?\n");
      }

      break;
    }

    strcpy(in_filename, ffblk.name);
    M2TS_to_TS(in_filename, log_file, &error_count, &skip_count, buffer);

    // new errors during processing this MTS file
    if ( (error_count != previous_error_count) || (skip_count != previous_skip_count) )
    {
      previous_error_count = error_count;
      previous_skip_count = skip_count;
      printf("\n========================================\n\n");
      fprintf(log_file, "========================================\n\n");
    }
    files_count_in++;
  } while(1);
/*   ==========  END OF MAIN PROCESSING LOOP (Process other MTS files) =============   */

CLEAN_UP:
  _findclose(search_handle);


/*   ==========   Summarize the task and end the program   =============   */
  printf("\n =====   ALL DONE!   =====\n");
  printf("\nProcessed %u file(s).", files_count_in);
  printf("\n%u ERROR(s), %u file(s) are skipped.\n", error_count, skip_count);

  fprintf(log_file, "\nProcessed %u file(s).", files_count_in);
  fprintf(log_file, "\n%u ERROR(s), %u file(s) are skipped.\n", error_count, skip_count);

  if (buffer != NULL)
  {
    free(buffer);
    buffer = NULL;
  }

clean_up_end:
  fclose(log_file);
  log_file = NULL;

  return 0;
}

