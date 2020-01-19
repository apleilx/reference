#include "malloc.h"
#include "osObjects.h"
#include "common.h"

//////////////////////////////////////////////////////////////////////////////////
//modify 1st: apleilx @ 2017/11/25
//All rights reserved
//////////////////////////////////////////////////////////////////////////////////

/*mem1�ڴ�����趨.mem1��ȫ�����ڲ�SRAM����.*/
#define MEM_BLOCK_SIZE (4UL * 1024UL)                        //�ڴ���С
#define MEM_MAX_SIZE (8UL * 1024UL * 1024UL)                 //�������ڴ�
#define MEM_ALLOC_TABLE_SIZE (MEM_MAX_SIZE / MEM_BLOCK_SIZE) //�ڴ���С

/*�ڴ��(32�ֽڶ���)*/
__align(4) uint8_t mem_base[MEM_MAX_SIZE];
/*�ڴ�����,ָʾ�ѷ���������ڴ�����*/
uint32_t mem_table[MEM_ALLOC_TABLE_SIZE];

/*�ڴ�������*/
#define mem_table_size MEM_ALLOC_TABLE_SIZE /*�ڴ���С*/
#define mem_block_size MEM_BLOCK_SIZE       /*�ڴ�ֿ��С*/
#define mem_size MEM_MAX_SIZE               /*�ڴ��ܴ�С*/

static uint8_t my_mem_man_rdy = 0; /*�ڴ�����Ƿ����*/

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
void mymemcpy(void *des, void *src, uint32_t n)
{
    uint8_t *xdes = des;
    uint8_t *xsrc = src;
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
void mymemset(void *s, uint8_t c, uint32_t count)
{
    uint8_t *xs = s;
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
    mymemset(mem_table, 0, mem_table_size * 4); //�ڴ�״̬����������
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
uint16_t my_mem_perused(void)
{
    uint32_t used = 0;
    uint32_t i;
    for (i = 0; i < mem_table_size; i++)
    {
        if (mem_table[i])
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
uint32_t my_mem_malloc(uint32_t size)
{
    signed long offset = 0;
    uint32_t nmemb;     //��Ҫ���ڴ����
    uint32_t cmemb = 0; //�������ڴ����
    uint32_t i;
    if (!my_mem_man_rdy)
        my_mem_init(); //δ��ʼ��,��ִ�г�ʼ��
    if (size == 0)
        return 0XFFFFFFFF;         //����Ҫ����
    nmemb = size / mem_block_size; //��ȡ��Ҫ����������ڴ����
    if (size % mem_block_size)
        nmemb++;
    for (offset = mem_table_size - 1; offset >= 0; offset--) //���������ڴ������
    {
        if (!mem_table[offset])
            cmemb++; //�������ڴ��������
        else
            cmemb = 0;      //�����ڴ������
        if (cmemb == nmemb) //�ҵ�������nmemb�����ڴ��
        {
            for (i = 0; i < nmemb; i++) //��ע�ڴ��ǿ�
            {
                mem_table[offset + i] = nmemb;
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
uint8_t my_mem_free(uint32_t offset)
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
        int nmemb = mem_table[index];   //�ڴ������
        for (i = 0; i < nmemb; i++)          //�ڴ������
        {
            mem_table[index + i] = 0;
        }
        return 0;
    }
    else
        return 2; //ƫ�Ƴ�����.
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
    uint32_t offset;

    if(osMutexWait(mid_malloc, 3000))
      return;

    if (ptr != NULL) //��ַΪ0.
    {
        offset = (uint32_t)ptr - (uint32_t)(&mem_base[0]);
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
void *mymalloc(uint32_t size)
{
    uint32_t offset;
    void *pDes;

    if(osMutexWait(mid_malloc, 3000))
      return NULL;

    offset = my_mem_malloc(size);
    if (offset == 0XFFFFFFFF)
        pDes = NULL;
    else
        pDes = (void *)((uint32_t)(&mem_base[0]) + offset);

    osMutexRelease(mid_malloc);

    return pDes;
}
