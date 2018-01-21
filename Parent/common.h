#ifndef _COMMON_H_
#define _COMMON_H_

#include <in430.h>
#include <io430x20x2.h>

typedef char                  s8_t;
typedef unsigned char 	      u8_t;
typedef int                   s16_t;
typedef unsigned int          u16_t;
typedef long                  s32_t;
typedef unsigned long         u32_t;
typedef void                  *pointer;
typedef char                  bool;

#define TRUE                  (1)
#define FALSE                 (0)
#define NULL_PTR              ((void *)0)
#define ON                    (TRUE)
#define OFF                   (FALSE)

typedef enum
{
  RET_SUCCESS                     = 0x00,
  RET_FAIL                        = 0x01,
  
  RET_MAX
} RetCode_t;

#endif


