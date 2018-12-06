/*
* Copyleft (c) 2016-2019  Fu Xing (Andy)
* Author: Fu Xing (Andy)
*
* File name: TS_Sync_Check.c
* Abstract: This program checks the 0x47 sync byte in TS files.
*
* Current version: 0.1
* Last Modified: 2018-11-19
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <errno.h>


static void Check_TS(char *in_filename, FILE *log_file, unsigned int *file_error_count, unsigned int *sync_error_count, unsigned char *buffer)
{
  FILE  *fp_in;
  int   return_value;

/*========== open the input file ====================*/
  if ( NULL == (fp_in = fopen(in_filename, "rb")) )
  {
    printf("\nError! Can't open input file %s\n", in_filename);
    fprintf(log_file, "Error! Can't open input file %s\n", in_filename);
    (*file_error_count)++;
    return;
  }

/*========== Kernel processing  ====================*/
  while ( !feof(fp_in) )
  {
    return_value = fread(buffer, 1, 188, fp_in);
    if (*buffer != 0x47)  // sync byte error
    {
      printf("\n%s: Sync byte error!\n", in_filename);
      fprintf(log_file, "%s: Sync byte error!\n", in_filename);
      (*sync_error_count)++;
      goto clean_up_A;
    }

    if (return_value != 188)  // end of file reached
    {
      break;
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
  struct _finddata_t  ffblk;  /* for searching *.ts files */
  intptr_t   search_handle;   /* for searching *.ts files */
  errno_t    err;             /* for searching *.ts files */
  int    return_value = 0;    /* for searching *.ts files */
  unsigned char   *buffer;    /* 188-byte buffer */

  unsigned int files_count_in = 0;  /* the number of input files */
  unsigned int file_error_count = 0;
  unsigned int sync_error_count = 0;
  unsigned int previous_error_count = 0;
  unsigned int previous_skip_count = 0;

  if ( NULL == (log_file = fopen("TS_Sync_Check_log.txt", "w")) )
  {
    printf("\nERROR! Can't create TS_Sync_Check_log.txt.\n");
    return  -1;
  }

/*========== buffer allocation  ====================*/
  if ( NULL == (buffer = (unsigned char *)malloc(188)) )
  {
    printf("\n ERROR! Insufficient memory in allocating 188-byte buffer!\n");
    fprintf(log_file, "ERROR! Insufficient memory in allocating 188-byte buffer!\n");
    goto clean_up_end;
  }

/*   ==========   Process the first TS file   =============   */
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
  search_handle = _findfirst("*.ts", &ffblk);  /* search for the first TS file */
  if (search_handle == -1)  // error occurred
  {
    _get_errno(&err);
    if (err == EINVAL)
    {
      printf("\nDuring the first searching for *.ts files, the operating system returned an unexpected error.\n");
      printf("Is the path too long?\n");
    }
    else if (err == ENOENT)
    {
      printf("\nNo .ts files in current directory!\n");
    }
    else if (err == ENOMEM)
    {
      printf("\nInsufficient memory during the first searching for *.ts files!\n");
    }
    else
    {
      printf("\nInternal error during the first searching for *.ts files!\n");
    }

    goto CLEAN_UP;
  }

  strcpy(in_filename, ffblk.name);
  Check_TS(in_filename, log_file, &file_error_count, &sync_error_count, buffer);
  previous_error_count = file_error_count;
  previous_skip_count = sync_error_count;
  files_count_in++;

/*   ==========  START OF MAIN PROCESSING LOOP (Process other TS files)  =============   */
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
        printf("\nDuring subsequent searching for *.ts files, the operating system returned an unexpected error.\n");
      }
      else if (err == ENOMEM)
      {
        printf("\nInsufficient memory during subsequent searching for *.ts files!\n");
        printf("Is the path too long?\n");
      }

      break;
    }

    strcpy(in_filename, ffblk.name);
    Check_TS(in_filename, log_file, &file_error_count, &sync_error_count, buffer);

    // new errors during processing this TS file
    if ( (file_error_count != previous_error_count) || (sync_error_count != previous_skip_count) )
    {
      previous_error_count = file_error_count;
      previous_skip_count = sync_error_count;
      printf("\n========================================\n\n");
      fprintf(log_file, "========================================\n\n");
    }
    files_count_in++;
  } while(1);
/*   ==========  END OF MAIN PROCESSING LOOP (Process other TS files) =============   */

CLEAN_UP:
  _findclose(search_handle);

  if (buffer != NULL)
  {
    free(buffer);
    buffer = NULL;
  }

/*   ==========   Summarize the task and end the program   =============   */
  printf("\n =====   ALL DONE!   =====\n");
  printf("\nProcessed %u file(s).", files_count_in);
  printf("\n%u file operation error(s), %u file(s) have/has sync error!\n", file_error_count, sync_error_count);

  fprintf(log_file, "\nProcessed %u file(s).", files_count_in);
  fprintf(log_file, "\n%u file operation error(s), %u file(s) have/has sync error!\n", file_error_count, sync_error_count);

clean_up_end:
  fclose(log_file);
  log_file = NULL;

  return 0;
}

