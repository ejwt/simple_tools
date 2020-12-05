/*
* Copyleft  2020-2021  Fu Xing (Andy)
* Author: Fu Xing
*
* File name: integer_calculation_quiz.c
* Abstract: The program is for my child.
*           It creates an integer calculation quiz.
*
* Usage: integer_calculation_quiz.exe (no arguments)
*
* Current version: 0.1
* Last Modified: 2020-12-5
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define  VERSION  0.1
#define  DATE     "2020-12-5"

#define  ABS(x)  (((x)>0) ? (x) : (-(x)))

#ifdef  WIN32
typedef __int64   	    INT64;
#else
typedef long long int  	INT64;
#endif

float quiz_add(int32_t addend_min, int32_t addend_max, int32_t sum_min, int32_t sum_max,
                   int32_t *err_flag)
{

  return 1.0;
}

typedef struct { 
  uint32_t type_id;
  uint32_t count;
  int32_t problem_min;
  int32_t problem_max;
  int32_t result_min;
  int32_t result_max;
}  quiz_type;

int main(void)
{
  int32_t working_mode;

  printf("===== 整数计算练习 =====\n");
  printf("版本：%.1f, 更新日期：%s\n\n", VERSION, DATE);

select_mode:
  printf("请选择模式\n");
  printf("0: 指定各题型的题量\n");
  printf("1: 指定总题量和题型，各题型的题量随机产生\n");

  scanf("%d", &working_mode);
  if ((working_mode != 0) && (working_mode != 1))
  {
    printf("无效输入！请重试。\n");
    goto select_mode;
  }



  return 0;
}






