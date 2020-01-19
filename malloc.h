#ifndef __MALLOC_H
#define __MALLOC_H
#include "mytype.h" 

//////////////////////////////////////////////////////////////////////////////////	 
//All rights reserved 
////////////////////////////////////////////////////////////////////////////////// 	 


#ifndef NULL
#define NULL 0
#endif


void mymemset(void *s, uint8_t c, uint32_t count); //设置内存
void mymemcpy(void *des, void *src, uint32_t n); //复制内存     
void my_mem_init(void); //内存管理初始化函数(外/内部调用)
uint32_t my_mem_malloc( uint32_t size); //内存分配(内部调用)
uint8_t my_mem_free( uint32_t offset); //内存释放(内部调用)
uint16_t my_mem_perused(void); //获得内存使用率(外/内部调用) 
////////////////////////////////////////////////////////////////////////////////
//用户调用函数
void myfree( void *ptr); //内存释放(外部调用)
void *mymalloc( uint32_t size); //内存分配(外部调用)

#endif













