/*
 * system init
 */

#include "sys_init.h"


static int sysinit_start(char pos)
{
    return 0;
}
OBJ_INIT_EXPORT(sysinit_start, "0");

static int sysinit_board_start(char pos)
{
    return 0;
}
OBJ_INIT_EXPORT(sysinit_board_start, "0.end");

static int sysinit_board_end(char pos)
{
    return 0;
}
OBJ_INIT_EXPORT(sysinit_board_end, "1.end");

static int sysinit_end(char pos)
{
    return 0;
}
OBJ_INIT_EXPORT(sysinit_end, "6.end");

/**
 * hal init
 */
void sys_hal_init(void)
{
    char pos = 1;
    volatile const sys_init_fn *fn_ptr;

    for (fn_ptr = &__sys_init_sysinit_start; fn_ptr < &__sys_init_sysinit_board_end; fn_ptr++)
    {
        (*fn_ptr)(pos++);
    }
}

/**
 * app init
 */
void sys_app_init(void)
{
    char pos = 1;
    volatile const sys_init_fn *fn_ptr;

    for (fn_ptr = &__sys_init_sysinit_board_end; fn_ptr < &__sys_init_sysinit_end; fn_ptr ++)
    {
        (*fn_ptr)(pos++);
    }

}


