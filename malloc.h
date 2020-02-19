#ifndef __MALLOC_H
#define __MALLOC_H
#include "mytype.h" 

//用户调用函数
void myfree( void *ptr); //内存释放(外部调用)
void *mymalloc( u32 size); //内存分配(外部调用)

#endif













