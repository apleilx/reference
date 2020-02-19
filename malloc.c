#include "malloc.h"
#include "osObjects.h"
#include "common.h"

//////////////////////////////////////////////////////////////////////////////////
//modify 1st: apleilx @ 2020/2/19
//All rights reserved
//////////////////////////////////////////////////////////////////////////////////

/*mem1�ڴ�����趨.mem1��ȫ�����ڲ�SRAM����.*/
#define mem_block_size (1UL * 1024UL)                        //�ڴ���С
#define mem_size (8UL * 1024UL * 1024UL)                     //�������ڴ�
#define mem_table_size (mem_size / mem_block_size)           //�ڴ���С

/*�ڴ��(32�ֽڶ���)*/
__align(32) u8 mem_base[mem_size];

/*�ڴ�����*/
u16 mem_table[mem_table_size], *mem_table_base;

/*�ڴ�����Ƿ����*/
static u8 my_mem_man_rdy = 0; 

#ifndef NULL
#define NULL 0
#endif

void mymemset(void *s, u8 c, u32 count); //�����ڴ�
void mymemcpy(void *des, void *src, u32 n); //�����ڴ�     
void my_mem_init(void); //�ڴ�����ʼ������(��/�ڲ�����)
u32 my_mem_malloc( u32 size); //�ڴ����(�ڲ�����)
u8 my_mem_free( u32 offset); //�ڴ��ͷ�(�ڲ�����)
u16 my_mem_perused(void); //����ڴ�ʹ����(��/�ڲ�����) 


/*
*********************************************************************************************************
*                                             osMutexWait()
* Return(s)   : 0:OK     1:ERR
*********************************************************************************************************
*/
/*
static __inline uint8_t osMutexWait(rt_mutex_t mid, uint32_t timeout)
{
    rt_err_t rt_err;
    
    rt_err = rt_mutex_take(mid, timeout);

    if (RT_EOK == rt_err)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}
*/
/*
*********************************************************************************************************
*                                             osMutexRelease()
* Return(s)   : none
*********************************************************************************************************
*/
/*
static __inline void osMutexRelease(rt_mutex_t mid)
{

    rt_mutex_release(mid);
}
*/

/*
*********************************************************************************************************
*                                             mymemcpy()
*�����ڴ�
*des:Ŀ�ĵ�ַ
*src:Դ��ַ
n:��Ҫ���Ƶ��ڴ泤��(�ֽ�Ϊ��λ)
* Return(s)   : none
*********************************************************************************************************
*/
void mymemcpy(void *des, void *src, u32 n)
{
    u8 *xdes = des;
    u8 *xsrc = src;
    while (n--)
        *xdes++ = *xsrc++;
}

/*
*********************************************************************************************************
*                                             mymemset()
*�����ڴ�
*s:�ڴ��׵�ַ
*c :Ҫ���õ�ֵ
*count:��Ҫ���õ��ڴ��С(�ֽ�Ϊ��λ)
* Return(s)   : none
*********************************************************************************************************
*/
void mymemset(void *s, u8 c, u32 count)
{
    u8 *xs = s;
    while (count--)
        *xs++ = c;
}

/*
*********************************************************************************************************
*                                             my_mem_init()
*�ڴ�����ʼ��
*memx:�����ڴ��
* Return(s)   : none
*********************************************************************************************************
*/
void my_mem_init(void)
{
    //mem_table_base ʹ����cache��
    mem_table_base = &mem_table[0];
    mem_table_base = (u16*)((u32)mem_table_base | 0x80000000);
    
    //��ʼ�������
    mymemset(mem_table_base, 0, mem_table_size * 4); //�ڴ�״̬����������
    my_mem_man_rdy = 1;                              //�ڴ�����ʼ��OK
}

/*
*********************************************************************************************************
*                                             my_mem_perused()
//��ȡ�ڴ�ʹ����
//memx:�����ڴ��
//����ֵ:ʹ����(������10��,0~1000,����0.0%~100.0%)
*********************************************************************************************************
*/
u16 my_mem_perused(void)
{
    u32 used = 0;
    u32 i;
    for (i = 0; i < mem_table_size; i++)
    {
        if (mem_table_base[i])
            used++;
    }
    return (used * 1000) / (mem_table_size);
}

/*
*********************************************************************************************************
*                                             my_mem_malloc()
//�ڴ����(�ڲ�����)
//memx:�����ڴ��
//size:Ҫ������ڴ��С(�ֽ�)
//����ֵ:0XFFFFFFFF,�������;����,�ڴ�ƫ�Ƶ�ַ
*********************************************************************************************************
*/
u32 my_mem_malloc(u32 size)
{
    signed long offset = 0;
    u32 nmemb;     //��Ҫ���ڴ����
    u32 cmemb = 0; //�������ڴ����
    u32 i;
    
    //δ��ʼ��,��ִ�г�ʼ��
    if (!my_mem_man_rdy)
    {
        my_mem_init(); 
    }
    
    //����Ҫ����
    if (size == 0)
    {
        return 0XFFFFFFFF;         
    }
    
    //��ȡ��Ҫ����������ڴ����
    nmemb = size / mem_block_size; 
    if (size % mem_block_size)
    {
        nmemb++;
    }
    
    //���������ڴ������
    for (offset = mem_table_size - 1; offset >= 0; offset--) 
    {
        if (!mem_table_base[offset])
            cmemb++; //�������ڴ��������
        else
            cmemb = 0;      //�����ڴ������
        
        //�ҵ�������nmemb�����ڴ��
        if (cmemb == nmemb) 
        {
            for (i = 0; i < nmemb; i++) //��ע�ڴ��ǿ�
            {
                mem_table_base[offset + i] = nmemb;
            }
            return (offset * mem_block_size); //����ƫ�Ƶ�ַ
        }
    }
    
    return 0XFFFFFFFF; //δ�ҵ����Ϸ����������ڴ��
}

/*
*********************************************************************************************************
*                                             my_mem_free()
//�ͷ��ڴ�(�ڲ�����)
//memx:�����ڴ��
//offset:�ڴ��ַƫ��
//����ֵ:0,�ͷųɹ�;1,�ͷ�ʧ��;
*********************************************************************************************************
*/
u8 my_mem_free(u32 offset)
{
    int i;
    if (!my_mem_man_rdy) //δ��ʼ��,��ִ�г�ʼ��
    {
        my_mem_init();
        return 1; //δ��ʼ��
    }

    if (offset < mem_size) //ƫ�����ڴ����.
    {
        int index = offset / mem_block_size; //ƫ�������ڴ�����
        int nmemb = mem_table_base[index];   //�ڴ������
        
        for (i = 0; i < nmemb; i++)          //�ڴ������
        {
            mem_table_base[index + i] = 0;
        }
        return 0;
    }
    else
    {
        return 2; //ƫ�Ƴ�����.
    }
}

/*
*********************************************************************************************************
*                                             myfree()
//�ͷ��ڴ�(�ⲿ����)
//memx:�����ڴ��
//ptr:�ڴ��׵�ַ
*********************************************************************************************************
*/
void myfree(void *ptr)
{
    u32 offset;

    if(osMutexWait(mid_malloc, 3000))
      return;

    if (ptr != NULL) //��ַΪ0.
    {
        offset = (u32)ptr - (u32)(&mem_base[0]);
        my_mem_free(offset); //�ͷ��ڴ�
    }
    
    osMutexRelease(mid_malloc);
}

/*
*********************************************************************************************************
*                                             mymalloc()
//�����ڴ�(�ⲿ����)
//memx:�����ڴ��
//size:�ڴ��С(�ֽ�)
//����ֵ:���䵽���ڴ��׵�ַ.
*********************************************************************************************************
*/
void *mymalloc(u32 size)
{
    u32 offset;
    void *pDes;

    if(osMutexWait(mid_malloc, 3000))
      return NULL;

    offset = my_mem_malloc(size);
    if (offset == 0XFFFFFFFF)
        pDes = NULL;
    else
        pDes = (void *)((u32)(&mem_base[0]) + offset);

    osMutexRelease(mid_malloc);

    return pDes;
}
