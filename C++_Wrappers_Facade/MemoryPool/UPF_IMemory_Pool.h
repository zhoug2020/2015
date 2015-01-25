/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_IMemory_Pool.h                                                                         *
*       CREATE DATE     : 2007-11-12                                                                                 *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]内存池接口的定义![CN]                                                                  *
*********************************************************************************************************************/
#ifndef UPF_IMEMORY_POOL_H
#define UPF_IMEMORY_POOL_H

#include <stddef.h>        /* for size_t */

class UPF_IMemory_Pool_Info;

/** 
 * @class UPF_IMemory_Pool 
 *  
 * @brief UPF_IMemory_Pool为内存池的接口，其他的内存池实现类需要实现这个接口所定义的方法. 
 */
class UPF_IMemory_Pool
{
public:
    /** 
     * 从内存池中分配内存.
     * 
     * @param size    [in] 需要分配的内存的大小, 如果size为0, 那么默认分配1字节的内存.
     * 
     * @retval 一个指向所申请的内存的指针    申请内存成功. 
     * @retval 0                             申请内存失败. 
     */
    virtual void * allocate( size_t size ) = 0;

    /** 
     * allocate的调试版本, 主要用于跟踪当前分配内存的位置，可以用于
     * 内存泄露的检查.
     * 
     * @param size          [in] 需要分配的内存的大小, 如果size为0, \n
     *                           那么默认分配1字节的内存.
     * @param file_name     [in] 调用allocate_debug的源文件名
     * @param line_no       [in] 调用allocate_debug的行号
     * 
     * @retval 一个指向所申请的内存的指针    申请内存成功. 
     * @retval 0                             申请内存失败. 
     */
    virtual void * allocate_debug( size_t       size, 
                                   const char * file_name,
                                   int          line_no ) = 0;

    /** 
     * 从内存池中重新分配内存, 并保留原有的内容，当分配内存时，如果在原来的位置
     * 还有足够的内存可以分配, 则进行就地分配.
     * 
     * 有如下三种情况:
     *      -# 如果ptr为0, 则调用allocate进行分配内存.
     *      -# 如果ptr不为0, new_size为0, 则调用deallocate释放内存.
     *      -# 如果ptr不为0, new_size不为0, 则进行重新分配内存操作, 如果在原\n
     *         位置有可用的内存，则进行就地分配，否则重新分配一块内存, 并把原\n
     *         始内存的内容拷贝到新申请的内存中, 并释放原来的内存.
     * 
     * @note 如果新分配内存失败, 原有内存并不释放, 原来的指针仍然有效.
     * 
     * 
     * @param ptr           [in] 调用allocate或reallocate所申请的内存.
     * @param new_size      [in] 新的内存大小
     * 
     * @retval 一个指向所申请的内存的指针    重新申请内存成功. 
     * @retval 0                             重新申请内存失败, ptr所指向的内存\n
     *                                       并不释放, ptr仍然有效.
     * 
     */
    virtual void * reallocate( void * ptr,
                               size_t new_size ) = 0;

    /** 
     * reallocate的调试版本, 主要用于跟踪当前分配内存的位置，可以用于
     * 内存泄露的检查.
     * 
     * @param ptr           [in] 调用allocate或reallocate所申请的内存.
     * @param new_size      [in] 新的内存大小
     * @param file_name     [in] 调用reallocate_debug的源文件名
     * @param line_no       [in] 调用reallocate_debug的行号
     * 
     * @retval 一个指向所申请的内存的指针    重新申请内存成功. 
     * @retval 0                             重新申请内存失败.
     * 
     */
    virtual void * reallocate_debug( void *       ptr,
                                     size_t       new_size,
                                     const char * file_name,
                                     int          line_no ) = 0;

    /** 
     * 释放内存.
     * 
     * @param ptr     [in] 调用allocate所申请的内存的指针.
     * @param size    [in] ptr所指向的内存的大小.
     */
    virtual void deallocate( void * ptr, size_t size ) = 0;

    /** 
     * 判断内存块是否存在于内存池中.
     * 
     * @param ptr     [in] 一个指向内存块的地址.
     * 
     * @retval true  内存块存在于该内存池中.
     * @retval false 内存块不存在于该内存池中.
     */
    virtual bool is_memory_block_exist( void * ptr ) = 0;

    /** 
     * 获取指向UPF_IMemory_Pool_Info实例的指针.
     *  
     * @return 指向UPF_IMemory_Pool_Info实例的指针, 当前实现确保返回值肯定不为0
     */
    virtual UPF_IMemory_Pool_Info * get_memory_pool_info( void ) = 0;

public:
    virtual ~UPF_IMemory_Pool() { }
        
};

#endif /* UPF_IMEMORY_POOL_H */

