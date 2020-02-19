#include "malloc.h"
#include "osObjects.h"
#include "common.h"

//////////////////////////////////////////////////////////////////////////////////
//modify 1st: apleilx @ 2020/2/19
//All rights reserved
//////////////////////////////////////////////////////////////////////////////////

/*mem1内存参数设定.mem1完全处于内部SRAM里面.*/
#define mem_block_size (1UL * 1024UL)                        //内存块大小
#define mem_size (8UL * 1024UL * 1024UL)                     //最大管理内存
#define mem_table_size (mem_size / mem_block_size)           //内存表大小

/*内存池(32字节对齐)*/
__align(32) u8 mem_base[mem_size];

/*内存管理表*/
u16 mem_table[mem_table_size], *mem_table_base;

/*内存管理是否就绪*/
static u8 my_mem_man_rdy = 0; 

#ifndef NULL
#define NULL 0
#endif

void mymemset(void *s, u8 c, u32 count); //设置内存
void mymemcpy(void *des, void *src, u32 n); //复制内存     
void my_mem_init(void); //内存管理初始化函数(外/内部调用)
u32 my_mem_malloc( u32 size); //内存分配(内部调用)
u8 my_mem_free( u32 offset); //内存释放(内部调用)
u16 my_mem_perused(void); //获得内存使用率(外/内部调用) 


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
*设置内存
*s:内存首地址
*c :要设置的值
*count:需要设置的内存大小(字节为单位)
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
*内存管理初始化
*memx:所属内存块
* Return(s)   : none
*********************************************************************************************************
*/
void my_mem_init(void)
{
    //mem_table_base 使用无cache区
    mem_table_base = &mem_table[0];
    mem_table_base = (u16*)((u32)mem_table_base | 0x80000000);
    
    //初始化管理表
    mymemset(mem_table_base, 0, mem_table_size * 4); //内存状态表数据清零
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
//内存分配(内部调用)
//memx:所属内存块
//size:要分配的内存大小(字节)
//返回值:0XFFFFFFFF,代表错误;其他,内存偏移地址
*********************************************************************************************************
*/
u32 my_mem_malloc(u32 size)
{
    signed long offset = 0;
    u32 nmemb;     //需要的内存块数
    u32 cmemb = 0; //连续空内存块数
    u32 i;
    
    //未初始化,先执行初始化
    if (!my_mem_man_rdy)
    {
        my_mem_init(); 
    }
    
    //不需要分配
    if (size == 0)
    {
        return 0XFFFFFFFF;         
    }
    
    //获取需要分配的连续内存块数
    nmemb = size / mem_block_size; 
    if (size % mem_block_size)
    {
        nmemb++;
    }
    
    //搜索整个内存控制区
    for (offset = mem_table_size - 1; offset >= 0; offset--) 
    {
        if (!mem_table_base[offset])
            cmemb++; //连续空内存块数增加
        else
            cmemb = 0;      //连续内存块清零
        
        //找到了连续nmemb个空内存块
        if (cmemb == nmemb) 
        {
            for (i = 0; i < nmemb; i++) //标注内存块非空
            {
                mem_table_base[offset + i] = nmemb;
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
u8 my_mem_free(u32 offset)
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
        int nmemb = mem_table_base[index];   //内存块数量
        
        for (i = 0; i < nmemb; i++)          //内存块清零
        {
            mem_table_base[index + i] = 0;
        }
        return 0;
    }
    else
    {
        return 2; //偏移超区了.
    }
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
    u32 offset;

    if(osMutexWait(mid_malloc, 3000))
      return;

    if (ptr != NULL) //地址为0.
    {
        offset = (u32)ptr - (u32)(&mem_base[0]);
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
