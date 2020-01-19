#include "malloc.h"
#include "osObjects.h"
#include "common.h"

//////////////////////////////////////////////////////////////////////////////////
//modify 1st: apleilx @ 2017/11/25
//All rights reserved
//////////////////////////////////////////////////////////////////////////////////

/*mem1内存参数设定.mem1完全处于内部SRAM里面.*/
#define MEM_BLOCK_SIZE (4UL * 1024UL)                        //内存块大小
#define MEM_MAX_SIZE (8UL * 1024UL * 1024UL)                 //最大管理内存
#define MEM_ALLOC_TABLE_SIZE (MEM_MAX_SIZE / MEM_BLOCK_SIZE) //内存表大小

/*内存池(32字节对齐)*/
__align(4) uint8_t mem_base[MEM_MAX_SIZE];
/*内存管理表,指示已分配的连续内存块个数*/
uint32_t mem_table[MEM_ALLOC_TABLE_SIZE];

/*内存管理参数*/
#define mem_table_size MEM_ALLOC_TABLE_SIZE /*内存表大小*/
#define mem_block_size MEM_BLOCK_SIZE       /*内存分块大小*/
#define mem_size MEM_MAX_SIZE               /*内存总大小*/

static uint8_t my_mem_man_rdy = 0; /*内存管理是否就绪*/

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
*复制内存
*des:目的地址
*src:源地址
n:需要复制的内存长度(字节为单位)
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
*设置内存
*s:内存首地址
*c :要设置的值
*count:需要设置的内存大小(字节为单位)
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
*内存管理初始化
*memx:所属内存块
* Return(s)   : none
*********************************************************************************************************
*/
void my_mem_init(void)
{
    mymemset(mem_table, 0, mem_table_size * 4); //内存状态表数据清零
    my_mem_man_rdy = 1;                              //内存管理初始化OK
}

/*
*********************************************************************************************************
*                                             my_mem_perused()
//获取内存使用率
//memx:所属内存块
//返回值:使用率(扩大了10倍,0~1000,代表0.0%~100.0%)
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
//内存分配(内部调用)
//memx:所属内存块
//size:要分配的内存大小(字节)
//返回值:0XFFFFFFFF,代表错误;其他,内存偏移地址
*********************************************************************************************************
*/
uint32_t my_mem_malloc(uint32_t size)
{
    signed long offset = 0;
    uint32_t nmemb;     //需要的内存块数
    uint32_t cmemb = 0; //连续空内存块数
    uint32_t i;
    if (!my_mem_man_rdy)
        my_mem_init(); //未初始化,先执行初始化
    if (size == 0)
        return 0XFFFFFFFF;         //不需要分配
    nmemb = size / mem_block_size; //获取需要分配的连续内存块数
    if (size % mem_block_size)
        nmemb++;
    for (offset = mem_table_size - 1; offset >= 0; offset--) //搜索整个内存控制区
    {
        if (!mem_table[offset])
            cmemb++; //连续空内存块数增加
        else
            cmemb = 0;      //连续内存块清零
        if (cmemb == nmemb) //找到了连续nmemb个空内存块
        {
            for (i = 0; i < nmemb; i++) //标注内存块非空
            {
                mem_table[offset + i] = nmemb;
            }
            return (offset * mem_block_size); //返回偏移地址
        }
    }
    return 0XFFFFFFFF; //未找到符合分配条件的内存块
}

/*
*********************************************************************************************************
*                                             my_mem_free()
//释放内存(内部调用)
//memx:所属内存块
//offset:内存地址偏移
//返回值:0,释放成功;1,释放失败;
*********************************************************************************************************
*/
uint8_t my_mem_free(uint32_t offset)
{
    int i;
    if (!my_mem_man_rdy) //未初始化,先执行初始化
    {
        my_mem_init();
        return 1; //未初始化
    }

    if (offset < mem_size) //偏移在内存池内.
    {
        int index = offset / mem_block_size; //偏移所在内存块号码
        int nmemb = mem_table[index];   //内存块数量
        for (i = 0; i < nmemb; i++)          //内存块清零
        {
            mem_table[index + i] = 0;
        }
        return 0;
    }
    else
        return 2; //偏移超区了.
}

/*
*********************************************************************************************************
*                                             myfree()
//释放内存(外部调用)
//memx:所属内存块
//ptr:内存首地址
*********************************************************************************************************
*/
void myfree(void *ptr)
{
    uint32_t offset;

    if(osMutexWait(mid_malloc, 3000))
      return;

    if (ptr != NULL) //地址为0.
    {
        offset = (uint32_t)ptr - (uint32_t)(&mem_base[0]);
        my_mem_free(offset); //释放内存
    }

    osMutexRelease(mid_malloc);
}

/*
*********************************************************************************************************
*                                             mymalloc()
//分配内存(外部调用)
//memx:所属内存块
//size:内存大小(字节)
//返回值:分配到的内存首地址.
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
