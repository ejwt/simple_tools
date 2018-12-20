/*
* Copyleft (c) 2018-20xx  Fu Xing (Andy)
* Author: Fu Xing
*
* File name: general_defines.h
* Abstract: Some general defines
*
* Current version: 0.1
* Last Modified: 2018-12-20
*/


#define  MAX(x, y)  ( ((x)>(y)) ? (x) : (y) )
#define  MIN(x, y)  ( ((x)<(y)) ? (x) : (y) )

#define  MAX3(x, y, z)  MAX(MAX(x, y), z)
#define  MIN3(x, y, z)  MIN(MIN(x, y), z)

#define  ABS(x)  ( ( (x) >= 0 ) ? (x) : (-(x)) )





