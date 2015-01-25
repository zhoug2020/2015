
/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Common_Global_Memory_Pool_T.h                                                          *
*       CREATE DATE     : 2008-7-18                                                                                  *
*       MODULE          : Memory pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]�ɱ�����С��ȫ���ڴ��![CN]                                                          *
*********************************************************************************************************************/
#ifndef UPF_COMMON_GLOBAL_MEMORY_POOL_T_H
#define UPF_COMMON_GLOBAL_MEMORY_POOL_T_H

#include "UPF_IGlobal_Memory_Pool.h"
#include "UPF_Memory_Pool_Define.h"     /* for MAX_SMALL_BLOCK_SIZE,
                                               MAX_SMALL_BLOCKS_INDEX,
                                               MIN_ALLOC_MEM_SIZE,
                                               MIN_ALLOC_GRANULARITY,
                                               UPF_MEMORY_ALIGN_LACK_ONE,
                                               UPF_MEMORY_ALIGN,
                                               UPF_MEMORY_ALIGN_DEFAULT,
                                               MAX_MEMORY_REQ_SIZE,
                                               MESSAGE_ASSERT */

/** 
 * @class UPF_Common_Global_Memory_Pool_T
 * 
 * @brief �ɱ��ڴ�أ���������ڴ�С��16Kʱ���ɸ��ڴ�ع���, ������16Kʱ������
 * ϵͳ����.
 * 
 * ��ŵĹ��������, �ڲ�����һ��128��Ԫ�ص�����m_free_small_block_list,ÿ��Ԫ��
 * ����̶���С���ڴ��, ����������μ��±�:<br><pre>
 * -------------------------------------------
 * |  ��������  |   ��������    |  ����Χ   |
 * -------------------------------------------
 * |  1-32      |      8        |   1-256     | 
 * |  33-48     |      16       |  257-512    |
 * |  49-64     |      32       |  513-1024   | 
 * |  65-80     |      64       | 1025-2048   |
 * |  81-96     |     128       | 2049-4096   |
 * | 97-112     |     256       | 4097-8192   |
 * | 113-128    |     512       | 8193-16384  |
 * -------------------------------------------
 * </pre>
 * 
 * ��Client��������ڴ�С��16Kʱ, ����������ڴ��С�ҵ���m_free_small_block_list
 * �е���Ӧ������Ȼ���ж��ڸ�λ�����Ƿ��п��е��ڴ�飬����У�������ȥ������
 * ��ϵͳ�����һ������ڴ�飬��СΪ2��ҳ���С(8K), Ȼ���������ڴ���л��ֳ�
 * ��Ӧ�Ŀ飬�ڽ��з���.
 * 
 * ��Client��������ڴ����16kʱ����ֱ�ӽ���ϵͳȥ����.
 * 
 * �ڷ����ڴ�ʱ, �����Client�˵��ڴ�������Ĵ�С���4�ֽڣ��ⲿ���������ڼ�¼
 * �����Client�˵��ڴ��С.
 * 
 * ��Client���ͷ��ڴ�ʱ, ��������ڴ�ָ��, ���ƫ��4���ֽ�, ȡ�����ڴ��Ĵ�С,
 * Ȼ���������ڴ��С�����ж�, ���С��16K, �����m_free_small_block_list
 * �е���Ӧ����, Ȼ��Ѹ��ڴ����ӵ������ڴ��������, �������16K, ����ϵͳȥ
 * �ͷ�.
 *
 * UPF_Common_Global_Memory_Pool_TĬ��ʹ�õ�toplevel allocator type��
 * UPF_TOPLEVEL_PERMANENT_ALLOCATOR
 */
template < class Allocator, class Lock, class RecordPolicy > 
class UPF_Common_Global_Memory_Pool_T : public UPF_IGlobal_Memory_Pool
{
public:
    /// ��ʼ���ڲ�״̬.
    UPF_Common_Global_Memory_Pool_T( void );

    /// ����ڴ���ڲ�����������ڴ�
    ~UPF_Common_Global_Memory_Pool_T( void );

public:
    /// ���ڴ���з����ڴ�.
    virtual void* allocate( size_t size, void * hint_ptr );
    
    /// �ͷ��ڴ�.
    virtual void deallocate( void* ptr, size_t size, void * hint_ptr );

    /// �ӿɱ��С���ڴ�������·����ڴ�.
    virtual void * reallocate( void * ptr, size_t new_size, void * hint_ptr );

private:
    /// С�ڴ��Ľڵ����� (memory size <= 16K)
    struct Small_Block_Node
    {
        union
        {
            /// �ڴ��Ĵ�С���������ڴ��ID
            size_t id_alloc_size;

            /// ����free״̬ʱ, ����ָ����һ��Small Block.
            Small_Block_Node * next_block_node;

        };
        
        /// �����ڴ�鱻Client��ʹ��ʱ, ����ָ����Client��ʹ�õ��ڴ�.
        char               client_data[1];
    };

    /// ���ڴ��Ľڵ����� (memory size > 16K), Large_Block_Node�����һ���ֶ�
    /// �����Small_Block_Node�ĵ�һ���ֶ�ƥ��, ��Ϊ���ͷ��ڴ�ʱ��������ڴ�
    /// ָ�����ƫ��4���ֽ�, ��ȷ�����ڴ�Ĵ�С.
    struct Large_Block_Node
    {
        /// �ڴ��Ĵ�С���������ڴ��ID
        size_t id_alloc_size;
    };

    /// ��ÿ�η����ڴ�ʱ, ���û�п����ڴ�, ��Ҫ��ϵͳ�����һ����ڴ�, ��СΪ
    /// ����ҳ���С, Memory_Node���ڼ�¼�ѷ����ڴ������.
    struct Memory_Node
    {
        Memory_Node * next_memory_node;
        size_t        memory_size;
    };

private:            
    enum  { 
        /* ÿ�������ȥ���ڴ��ļ�¼��Ϣ�ֶεĴ�С, ͨ��Ϊ4. */
        SIZEOF_ID_ALLOC_SIZE_FIELD = sizeof( size_t ) 
        };

    enum {
        /* Large_Block_Node�Ĵ�С */
        LARGE_BLOCK_NODE_SIZE = sizeof( Large_Block_Node )
    };

    enum {
        /* Memory_Node�Ĵ�С*/
        MEOMRY_NODE_SIZE = UPF_MEMORY_ALIGN_DEFAULT( sizeof(Memory_Node) )
    };
    
private:
    /// ��ϵͳ�����������ڴ�.
    void * get_memory_space( size_t size, RecordPolicy * size_recorder );

    /// ��m_free_small_block_list��ȡ�ÿ��е�small block.
    void * get_avail_memory_from_free_block_list( size_t   size, 
                                                  size_t & bytes_to_get );

    /// �Ѷ�����ڴ���䵽���е��ڴ���б���.
    void place_remainder_memory_on_block_list( size_t bytes_left );
    
    /// ����ڴ���ڲ�����������ڴ�
    void destroy_memory_space( void );

    /// �����ڲ�״̬
    void reset( void );

private:
    /// ���ڴ��free small block, ������μ�UPF_Common_Memory_Pool��˵��.
    Small_Block_Node * m_free_small_block_list[ MAX_SMALL_BLOCKS_INDEX ];

    /// ���ڱ���һ��ָ����ڴ�������ͷָ��.
    Memory_Node * m_first_memory_node;

    /// ָ������ڴ����ʼλ��.
    char *        m_start_avail_memory;

    /// ָ������ڴ����ֹλ��.
    char *        m_end_avail_memory;

    /// �߳���, �����߳�ͬ��.
    mutable Lock  m_lock;
   
};

#if defined (UPF_TEMPLATES_REQUIRE_SOURCE)
#include "UPF_Common_Global_Memory_Pool_T.cpp"
#endif /* UPF_TEMPLATES_REQUIRE_SOURCE */

#endif /* UPF_COMMON_GLOBAL_MEMORY_POOL_T_H */

