/*
* Copyleft  2018-20xx  Fu Xing (Andy)
* Author: Fu Xing
*
* Project name: rand_copy_files
* Abstract: The program generates a script to copy 2 files randomly
*           for video quality review.
*
* Usage: interactive, follow the displayed instructions.
*
* Current version: 0.1
* Last Modified: 2018-12-26
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

static char src_filename[2][256];
static char dst_path[256];

static char get_ext_name(char *in_filename, char *out_ext_name)
{
  int  i, j, k;  /* counter */
  char valid = 1;

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
    valid = 0;  /* invalid */
  }

  for (k=0; i<=j; k++, i++)
  {
    out_ext_name[k] = in_filename[i];
  }

  return valid;
}


int main()
{
  char  ext_A[32], ext_B[32];
  int32_t  count, i;
  FILE *fp_bat;
  long  current_time;

  printf("The program generates a script to copy 2 files randomly for video quality review.\n");

again_A:
  printf("\nfilename A:\n");
  scanf("%s", src_filename[0]);
  if (0 == get_ext_name(src_filename[0], ext_A))
  {
    printf("\nYou forgot to input the extension name! Try again.\n");
    goto again_A;
  }

again_B:
  printf("\nfilename B:\n");
  scanf("%s", src_filename[1]);
  if (0 == get_ext_name(src_filename[1], ext_B))
  {
    printf("\nYou forgot to input the extension name! Try again.\n");
    goto again_B;
  }

  if (strcmp(src_filename[0], src_filename[1]) == 0)
  {
    printf("\nThe two files have the same name! Such generated bat file is useless! Try again.\n");
    goto again_A;
  }

  if (strcmp(ext_A, ext_B) != 0)
  {
    printf("\nThe extension names of the two files are different! Try again.\n");
    goto again_A;
  }

again_C:
  printf("\nTotal ouput file count:\n");
  scanf("%d", &count);
  if (count <= 0)
  {
    printf("\nAre you kidding? Count should be positive.\n");
    goto again_C;
  }
  if (count > 20)
  {
    printf("\nAre you sure you have time and patience to review all the video?\n");
    printf("Count should be 1~20.\n");
    goto again_C;
  }

again_P:
  printf("\nOutput path:\n");
  scanf("%s", dst_path);
  for (i=0; dst_path[i] != '\0'; i++)
  {
    if ( (dst_path[i] == '/') || (dst_path[i] == '*') || (dst_path[i] == '?') ||
         (dst_path[i] == '\"') || (dst_path[i] == '<') || (dst_path[i] == '>') || (dst_path[i] == '|') )
    {
      printf("\nThere's at least one illegal character in the path. Try again.\n");
      goto  again_P;
    }
  }

  if (dst_path[i-1] != '\\')
  {
    strcat(dst_path, "\\");
  }

  if (NULL == (fp_bat = fopen("rand_copy.bat", "w")))
  {
    printf("\nCan't create file rand_copy.bat\n");
  }

  current_time = (long)time(NULL);
  printf("current_time = %ld\n", current_time);

  srand(current_time);
  srand(rand());

  for (i=0; i<count; i++)
  {
    fprintf(fp_bat, "copy %s %s%02d%s\n", src_filename[rand()&1], dst_path, i, ext_A);
  }

  printf("rand_copy.bat has been generated. Do NOT peek into its content!\n");

  return  0;
}

