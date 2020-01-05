/* sys tem init
 *
 */

#ifndef __SYSTEM_INIT_H__
#define __SYSTEM_INIT_H__

#ifdef __cplusplus
extern "C" {
#endif

#if defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
#define __CLANG_ARM
#endif

/* Compiler Related Definitions */
#if defined(__CC_ARM) || defined(__CLANG_ARM)           /* ARM Compiler */
    #include <stdarg.h>
    #define OBJ_SECTION(x)                  __attribute__((section(x)))
    #define OBJ_UNUSED                   __attribute__((unused))
    #define OBJ_USED                     __attribute__((used))
    #define OBJ_ALIGN(n)                    __attribute__((aligned(n)))

    #define OBJ_WEAK                     __attribute__((weak))
    #define OBJ_INLINE                   static __inline

#elif defined (__IAR_SYSTEMS_ICC__)     /* for IAR Compiler */
    #include <stdarg.h>
    #define OBJ_SECTION(x)                  @ x
    #define OBJ_UNUSED
    #define OBJ_USED                     __root
    #define PRAGMA(x)                   _Pragma(#x)
    #define OBJ_ALIGN(n)                    PRAGMA(data_alignment=n)
    #define OBJ_WEAK                     __weak
    #define OBJ_INLINE                   static inline

#elif defined (__GNUC__)                /* GNU GCC Compiler */
    #ifdef RT_USING_NEWLIB
        #include <stdarg.h>
    #else
        /* the version of GNU GCC must be greater than 4.x */
        typedef __builtin_va_list       __gnuc_va_list;
        typedef __gnuc_va_list          va_list;
        #define va_start(v,l)           __builtin_va_start(v,l)
        #define va_end(v)               __builtin_va_end(v)
        #define va_arg(v,l)             __builtin_va_arg(v,l)
    #endif

    #define OBJ_SECTION(x)                  __attribute__((section(x)))
    #define OBJ_UNUSED                   __attribute__((unused))
    #define OBJ_USED                     __attribute__((used))
    #define OBJ_ALIGN(n)                    __attribute__((aligned(n)))
    #define OBJ_WEAK                     __attribute__((weak))
    #define OBJ_INLINE                   static __inline
    
#else
    #error not supported tool chain
#endif

/* initialization export */

typedef int (*sys_init_fn)(char pos);


#define OBJ_INIT_EXPORT(fn, level)                                                       \
    OBJ_USED const sys_init_fn __sys_init_##fn OBJ_SECTION(".rti_fn." level) = fn




/* board init routines will be called in board_init() function */
#define OBJ_INIT_BOARD_EXPORT(fn)           OBJ_INIT_EXPORT(fn, "1")

/* pre/device/component/env/app init routines will be called in init_thread */
/* components pre-initialization (pure software initilization) */
#define OBJ_INIT_PREV_EXPORT(fn)            OBJ_INIT_EXPORT(fn, "2")
/* device initialization */
#define OBJ_INIT_DEVICE_EXPORT(fn)          OBJ_INIT_EXPORT(fn, "3")
/* components initialization (dfs, lwip, ...) */
#define OBJ_INIT_COMPONENT_EXPORT(fn)       OBJ_INIT_EXPORT(fn, "4")
/* environment initialization (mount disk, ...) */
#define OBJ_INIT_ENV_EXPORT(fn)             OBJ_INIT_EXPORT(fn, "5")
/* appliation initialization (rtgui application etc ...) */
#define OBJ_INIT_APP_EXPORT(fn)             OBJ_INIT_EXPORT(fn, "6")

    
void sys_hal_init(void);
void sys_app_init(void);

#ifdef __cplusplus
}
#endif

#endif
