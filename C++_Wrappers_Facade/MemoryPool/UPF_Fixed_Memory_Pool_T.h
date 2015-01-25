/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Fixed_Memory_Pool_T.h                                                                  *
*       CREATE DATE     : 2007-11-12                                                                                 *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]�̶������С���ڴ��ʵ��![CN]                                                          *
*********************************************************************************************************************/

#ifndef UPF_FIXED_MEMORY_POOL_T_H
#define UPF_FIXED_MEMORY_POOL_T_H

#include <stddef.h>                     /* for size_t */

#include "UPF_Memory_Pool_Base_T.h"     /* for base class of UPF_Fixed_Memory_Pool */
#include "UPF_Memory_Pool_Define.h"     /* for DEFAULT_OBJECT_SIZE,
                                               DEFAULT_NUM_OBJECTS_PER_BLOCK,
                                               UPF_MEMORY_ALIGN,
                                               MAX_MEMORY_REQ_SIZE */

/** 
 * @class UPF_Fixed_Memory_Pool_T
 * 
 * @brief �̶������С���ڴ��ʵ��
 * 
 * һ�������ڴ�������ɸ��������, ÿ������ͷ����һָ��ָ����һ����
 * һ�������ڴ���β����һָ��ָ����һ�������ڴ��, �ڴ���ŵĲ�������:<br><pre>
 * object 1-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *            |                    ptr of object 2                            |
 *            |                    unused memory                              |
 *            |                    unused memory                              |
 *            |                    ....                                       |
 *            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * object 2 ->+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *            |                    ptr of object 3                            |
 *            |                    unused memory                              |
 *            |                    unused memory                              |
 *            |                    ....                                       |
 *            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * ...........
 * object n ->+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *            |                    NULL                                       |
 *            |                    unused memory                              |
 *            |                    unused memory                              |
 *            |                    ....                                       |
 *            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *            |                    ptr of block 2                             |
 *            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * ............
 * </pre>
 */
template < class Allocator, class Lock, class RecordPolicy > 
class UPF_Fixed_Memory_Pool_T : public UPF_Memory_Pool_Base< Allocator, 
                                                             Lock,
                                                             RecordPolicy >
{
public:
    /// ���캯���������ڴ�صĳ�ʼ��.
    UPF_Fixed_Memory_Pool_T( const char *          memory_pool_name,
                             size_t                object_size = DEFAULT_OBJECT_SIZE - SIZEOF_ALLOC_SIZE_FIELD,
                             size_t                num_objects_per_block = DEFAULT_NUM_OBJECTS_PER_BLOCK,
                             toplevel_allocator_t  allocator_type = UPF_TOPLEVEL_PERMANENT_ALLOCATOR);

    /// �������������ڹر��ڴ�أ��ͷ���������ڴ�.
    ~UPF_Fixed_Memory_Pool_T( void );

public:
    /// ���ڴ���з����ڴ�.
    virtual void* allocate( size_t size );
    
    /// �ͷ��ڴ�.
    virtual void deallocate( void* ptr, size_t size );

    /// �ӹ̶���С���ڴ�������·����ڴ�.
    virtual void * reallocate( void * ptr, size_t new_size );
   
private:
    enum { BLOCK_OVERHEAD = sizeof( int ) /**< ÿ���ڴ����Ҫ����һ��4�ֽ��ֶΣ�
                                               ����ָ����һ���ڴ��. */
         };

private:
    /// �����ڴ��.
    void * create_memory_block( void );

    /// �����ڴ���ж����next ptr.
    void set_next_object_ptr( void * object_ptr, void * next_object_ptr );

    /// ȡ���ڴ���ж����next ptr
    void * get_next_object_ptr( void * object_ptr );

    /// �����ڴ���next ptr.
    void set_next_block_ptr( void * block_ptr, void * next_block_ptr );

    /// ȡ���ڴ���next ptr
    void * get_next_block_ptr( void * block_ptr );

    /// �ͷ��ڴ���е������ڴ��ռ�õ��ڴ�, �������ڲ���״̬.
    void destroy_memory_blocks( void );

    /// �����ڲ���״̬
    void reset( void );

private:
    /// �����ֽڶ����Ķ����С
    size_t m_align_object_size;

    /// �����ֽڶ������ڴ���С
    size_t m_align_block_size;

    /// ����m_align_object_size, m_align_block_size, BLOCK_OVERHEAD�����
    /// ÿ���ڴ��ʵ�ʴ�ŵĶ������.
    int m_num_objects_per_block;

    /// ָ���һ���ڴ��.
    void *          m_first_block;

    /// ָ���һ�����п�.
    void *          m_free_list_head;

};

#if defined (UPF_TEMPLATES_REQUIRE_SOURCE)
#include "UPF_Fixed_Memory_Pool_T.cpp"
#endif /* UPF_TEMPLATES_REQUIRE_SOURCE */

#endif /* UPF_FIXED_MEMORY_POOL_T_H */
