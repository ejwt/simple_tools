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
#include <time.h>

#define  VERSION  0.1
#define  DATE     "2020-12-5"

#define  TYPE_ADD              0
#define  TYPE_SUB              1
#define  TYPE_FILL_ADDEND      2
#define  TYPE_FILL_MINUEND     3
#define  TYPE_FILL_SUBTRAHEND  4
#define  TYPE_ADD_ADD          5
#define  TYPE_SUB_SUB          6
#define  TYPE_ADD_SUB_MIX      7

#define  TOTAL_TYPES  8



#define  ABS(x)  (((x)>0) ? (x) : (-(x)))

float quiz_add(int32_t addend_min, int32_t addend_max, int32_t sum_min, int32_t sum_max,
               int32_t *err_flag)
{
	int32_t addend_0, addend_1, sum, user_input;

	srand((uint32_t)time(NULL));
generate_again:
	addend_0 = rand() % (addend_max - addend_min) + addend_min + rand() % 2;
	addend_1 = rand() % (addend_max - addend_min) + addend_min + rand() % 2;
	sum = addend_0 + addend_1;

	if ((sum < sum_min) || (sum > sum_max))
		goto generate_again;

	printf("%d + %d = ", addend_0, addend_1);
  scanf("%d", &user_input);

  if (sum == user_input)
  {
    *err_flag = 0;
    printf("正确\n");
  }
  else
  {
  	*err_flag = 1;
  	printf("错误\n");
  	printf("正确的结果是 %d\n", sum);
  }

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
  int32_t what_to_do_next;
  uint32_t  i = 0;  // counter
  time_t  current_time;
  int32_t err_flag;
  quiz_type my_quiz[TOTAL_TYPES];

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

  if (working_mode == 0)  // 指定各题型的题量
  {
  	// 加法
    printf("加法，几道题？  ");
    scanf("%u", &my_quiz[TYPE_ADD].count);

  set_add:
    printf("被加数/加数的最小值 = ");
    scanf("%d", &my_quiz[TYPE_ADD].problem_min);

    printf("被加数/加数的最大值 = ");
    scanf("%d", &my_quiz[TYPE_ADD].problem_max);
  set_sum:
    printf("和的最小值 = ");
    scanf("%d", &my_quiz[TYPE_ADD].result_min);

    printf("和的最大值 = ");
    scanf("%d", &my_quiz[TYPE_ADD].result_max);

    if (my_quiz[TYPE_ADD].problem_min > my_quiz[TYPE_ADD].problem_max)
    {
      printf("被加数/加数的最小值 比 最大值 还大。无效！请重新输入。\n");
      goto set_add;
    }

    if (my_quiz[TYPE_ADD].result_min > my_quiz[TYPE_ADD].result_max)
    {
      printf("和的最小值 比 最大值 还大。无效！请重新输入。\n");
      goto set_sum;
    }
  }
  else
  {
    printf("此功能尚未实现，请等待付兴更新软件。\n");
    goto next_step;
  }

  quiz_again_with_same_cfg:
    // 加法
    for (i=0; i<my_quiz[TYPE_ADD].count; i++)
    {
    	quiz_add(my_quiz[TYPE_ADD].problem_min, my_quiz[TYPE_ADD].problem_max,
    	         my_quiz[TYPE_ADD].result_min, my_quiz[TYPE_ADD].result_max,
               &err_flag);
    }





next_step:
  printf("练习结束，请选择\n");
  printf("0: 退出\n");
  printf("1: 用同样的配置再练一遍\n");
  printf("2: 用不同的配置再练一遍\n");
  scanf("%d", &what_to_do_next);
  if ((what_to_do_next != 0) && (what_to_do_next != 1) && (what_to_do_next != 2))
  {
    printf("无效输入！请重试。\n");
    goto next_step;
  }

  if (what_to_do_next == 0)  // 退出
  {

  }
  else if (what_to_do_next == 1)  // 用同样的配置再练一遍
  {
    goto quiz_again_with_same_cfg;
  }
  else  // 用不同的配置再练一遍
  {
    goto select_mode;
  }

  return 0;
}

