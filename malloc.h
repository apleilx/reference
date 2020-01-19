#ifndef __MALLOC_H
#define __MALLOC_H
#include "mytype.h" 

//////////////////////////////////////////////////////////////////////////////////	 
//All rights reserved 
////////////////////////////////////////////////////////////////////////////////// 	 


#ifndef NULL
#define NULL 0
#endif


void mymemset(void *s, uint8_t c, uint32_t count); //�����ڴ�
void mymemcpy(void *des, void *src, uint32_t n); //�����ڴ�     
void my_mem_init(void); //�ڴ�����ʼ������(��/�ڲ�����)
uint32_t my_mem_malloc( uint32_t size); //�ڴ����(�ڲ�����)
uint8_t my_mem_free( uint32_t offset); //�ڴ��ͷ�(�ڲ�����)
uint16_t my_mem_perused(void); //����ڴ�ʹ����(��/�ڲ�����) 
////////////////////////////////////////////////////////////////////////////////
//�û����ú���
void myfree( void *ptr); //�ڴ��ͷ�(�ⲿ����)
void *mymalloc( uint32_t size); //�ڴ����(�ⲿ����)

#endif













