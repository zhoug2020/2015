/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Memory_Pool_Debug_T.h                                                                  *
*       CREATE DATE     : 2007-11-21                                                                                 *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]�ڴ�ص��԰汾�Ķ���![CN]                                                              *
*********************************************************************************************************************/

#ifndef UPF_MEMORY_POOL_DEBUG_T_H
#define UPF_MEMORY_POOL_DEBUG_T_H

#include "UPF_IMemory_Pool.h"
#include "UPF_IMemory_Pool_Info.h"
#include "UPF_Memory_Pool_Base_T.h"        /* for UPF_Scope_Guard, Null Lock */

#include <windows.h>                       /* for CreateThread, 
                                                  CreateEvent,
                                                  WaitForSingleObject. */

#include <time.h>                          /* for time_t, 
                                                  strftime, 
                                                  time, 
                                                  localtime. */

#include <stdio.h>

class UPF_IMemory_Pool_Debug_Print;

// type desc
const char * const DEBUG_MEMORY_POOL_TYPE = "Debug";


/** 
 * @class UPF_Memory_Pool_Debug_T
 * 
 * @brief �ڴ�ص��԰汾, ��������һ���ڴ�صİ�װ�汾����ʵ����allocate_debug,
 * ��ÿ�ε���allocate_debug�����ڴ�ʱ, ��������20�ֽڵ��ڴ�, ���ڱ����ѷ�����ڴ�
 * ���ڵ��ļ���, �к�, ��С(��ʹ�ù̶���С���ڴ��ʱ, ���뱣֤Ϊԭ����С����
 * 20�ֽ�), Ȼ����������Ľڵ���ӵ�һ�����ڱ����ѷ���ڵ��˫�������У�
 * ���ͷ��ڴ�ʱ, ��˫���������Ƴ��ýڵ�. �ڳ�����ֹʱ���Ϳ��Ը�������������鿴
 * �Ƿ����ڴ�й¶.
 * 
 *  - Lock          �����߳�ͬ����.
 * 
 */
template < class Lock >
class UPF_Memory_Pool_Debug_T : public UPF_IMemory_Pool,
                                public UPF_IMemory_Pool_Info
{
public:
     /// ���캯��, ���ڳ�ʼ���ڲ�״̬    
    UPF_Memory_Pool_Debug_T( UPF_IMemory_Pool * memory_pool );

    /// �����������ͷ��ڲ���Դ.
    ~UPF_Memory_Pool_Debug_T( void );

public:
    /************************ Implements UPF_IMemory_Pool ********************/

    /// ���ڴ���з����ڴ�.
    virtual void* allocate( size_t size );

    /// allocate�ĵ��԰汾.
    virtual void* allocate_debug( size_t      size, 
                                  const char* file_name, 
                                  int         line_no );

    /// ���ڴ�������·����ڴ�.
    virtual void * reallocate( void * ptr,
                               size_t new_size );

    /// reallocate�ĵ��԰汾.
    virtual void * reallocate_debug( void *       ptr,
                                     size_t       new_size,
                                     const char * file_name,
                                     int          line_no );
    /// �ͷ��ڴ�.
    virtual void deallocate( void* ptr, size_t size );

    /// �ж��ڴ���Ƿ�������ڴ����.
    virtual bool is_memory_block_exist( void * ptr );

    /** 
     * ��ȡָ��UPF_IMemory_Pool_Infoʵ����ָ��.
     * 
     * 
     * @return ָ��UPF_IMemory_Pool_Infoʵ����ָ��.
     */
    UPF_IMemory_Pool_Info * get_memory_pool_info( void )
    {
        return static_cast< UPF_IMemory_Pool_Info * >( this );
    }

    /******************* Implements UPF_IMemory_Pool_Info ********************/

    /** 
     * ����UPF_IMemory_Pool�ӿ�ָ��.
     * 
     * @return ʵ��UPF_IMemory_Pool�ӿڵ�ָ��.
     */
    virtual UPF_IMemory_Pool * get_memory_pool( void )
    {
        return static_cast< UPF_IMemory_Pool * >( this );
    }

    /**
     * ��ȡ�ڴ�ص����ͣ���ǰ֧�����֣��̶���С�ģ��ɱ��С��.
     * 
     * @return �ڴ�ص�����.
     */
    virtual const char * get_memory_pool_name( void ) const
    {
        return m_memory_pool->get_memory_pool_info()->get_memory_pool_name();
    }

    /**
     * ��ȡ�ڴ�ص����ͣ���ǰ֧�����֣��̶���С�ģ��ɱ��С��.
     * 
     * @return �ڴ�ص�����.
     */
    virtual const char * get_memory_pool_type( void ) const
    {
        return DEBUG_MEMORY_POOL_TYPE;
    }

    /** 
     * ����ڴ����������ڴ��С, ���ڴ�ش�ϵͳ������������ڴ�Ĵ�С.
     *  
     * @return �ڴ����������ڴ��С
     */
    virtual size_t get_managed_memory_size( void ) const
    {
        return m_memory_pool->get_memory_pool_info()->get_managed_memory_size();
    }

    /** 
     * ����ڴ���е�ǰ�ѷ����Client�˵��ڴ��С, ���ֵ�ܱ�ʾ��ĳһʱ��Client��
     * ��ʹ�õ��ڴ��С.
     * 
     * @return  �����Client�˵��ڴ��С. 
     */
    virtual size_t get_allocated_memory_size( void ) const
    {
        return m_memory_pool->get_memory_pool_info()->get_allocated_memory_size();
    }


    /** 
     *  ��ȡ�����Client�˵��ڴ��С�ķ�ֵ, ���ֵ�ܱ�ʾ��ĳһʱ��Client��ʹ�õ�
     *  �ڴ��С�����ֵ.
     * 
     * @return �����Client�˵��ڴ��С�ķ�ֵ
     */
    virtual size_t get_allocated_memory_peak( void ) const
    {
        return m_memory_pool->get_memory_pool_info()->get_allocated_memory_peak();
    }

    /** 
     * ��ȡ�ڴ�����˷ѵ��ڴ��С, ��Ϊ�ڴ���е�ÿ���ڵ��С���ǰ���һ����������
     * �����, ����ÿ�η���������˷ѵ��ڴ�, ʹ������ӿں������Ի�ȡĳһʱ���ڴ��
     * �����˷ѵ��ڴ��С.
     * 
     * @return �ڴ�����˷ѵ��ڴ��С
     */
    virtual size_t get_wasted_memory_size( void ) const
    {
        return m_memory_pool->get_memory_pool_info()->get_wasted_memory_size();
    }

    /**
     * ��ȡ�ڴ���������ֵ.
     */
    virtual size_t get_allocated_memory_limit( void ) const
    {
        return m_memory_pool->get_memory_pool_info()->get_allocated_memory_limit();
    }

    virtual toplevel_allocator_t  get_allocator_type( void ) const
    {
        return m_memory_pool->get_memory_pool_info()->get_allocator_type();
    }


public:
    /// ��ӡ��������ڴ��ĸ�Ҫ��Ϣ, �����ڴ�Ĵ�С, filename, lineno.
    void print_allocated_memory_block_summary( FILE * fp );

public:
    /// �ڴ�ص�����Ϣ�ڵ�ṹ
    struct UPF_Memory_Pool_Debug_Node
    {
        const char *                 file_name;
        int                          line_no;        
        time_t                       allocate_time;
        UPF_Memory_Pool_Debug_Node * previous_node;
        UPF_Memory_Pool_Debug_Node * next_node;
        size_t                       size;
    };

public:
    /// �ڴ�ص�����Ϣ�ڵ�ṹ�Ĵ�С
    enum { SIZE_OF_DEBUG_NODE = ( sizeof( UPF_Memory_Pool_Debug_Node ) ) };

private:
    /// DUMP �ڴ������
    enum UPF_Dump_Memory_Type
    {
        DUMP_DETAIL  = 1,   /**< dupm�ڴ����ϸ��Ϣ.                 */
        DUMP_SUMMARY = 2,   /**< dump�ڴ���Ҫ��Ϣ.                 */
        DUMP_ALL     = 3    /**< ��dump�ڴ����ϸ��Ϣ��dump��Ҫ��Ϣ. */
    };

    enum UPF_Memory_Pool_Debug_Print_Type
    {
        DEBUG_PRINT_OUTPUTDEBUGSTRING = 1, /**< ����OutputDebugString���.       */
        DEBUG_PRINT_CONSOLE = 2,           /**< ���������̨.       */
        DEBUG_PRINT_FILE    = 3,           /**< ������ļ�.         */
        DEBUG_PRINT_ALL     = 4            /**< ���������̨���ļ�. */
    };

private:

    /// ����ڴ�ص�����Ϣ�ڵ㵽�ڲ���˫��������
    UPF_Memory_Pool_Debug_Node *  add_node( const char * file_name,
                                            int          line_no,
                                            time_t       allocate_time,
                                            size_t       size,
                                            UPF_Memory_Pool_Debug_Node * new_node,
                                            UPF_Memory_Pool_Debug_Node * head_node );

    /// ���ڲ���˫����������ȥ�ýڵ�
    void remove_node( UPF_Memory_Pool_Debug_Node * node );

private:

#ifdef UPF_OS_IS_WINNT
    /// dump memory �̺߳���
    static DWORD WINAPI dump_memory_thread_proc( LPVOID lpParameter );

    /// ����һ��dump memory �߳�
    void start_dump_memory_thread( void );

    /// ֹͣһ��dump memory �߳�
    void stop_dump_memory_thread( void );

    /// Dump �Ѿ�������ڴ��
    void dump_allocated_memory_block( void );

    /// Dump�Ѿ�������ڴ�����ϸ��Ϣ, �����ѷ�����ڴ�������.
    void dump_allocated_memory_block_detail( const char * memory_pool_name );

    /// Dump�Ѿ�������ڴ��ĸ�Ҫ��Ϣ
    void dump_allocated_memory_block_summary( const char * memory_pool_name );

    /// Dump start_memory_ptr��end_memory_ptr֮����ڴ������
    void dump_memory( unsigned char * start_memory_ptr,
                      unsigned char * end_memory_ptr );

    /// ����ָʾdump memory �߳��Ƿ���ֹ
    bool is_dump_memory_thread_stopped( void ) const
    {
        return m_is_dump_memory_thread_stopped;
    }

    /// ����dump memory �¼�����.
    HANDLE & dump_memory_event( void ) 
    {
         return m_dump_memory_event;
    }

    /// �ӻ��������ж�ȡdump typeֵ.
    void read_memory_dump_type_from_env( void );

    /// �ӻ��������ж�ȡdebug print typeֵ, ��������Ӧ��debug printʵ��.
    void create_debug_print_from_env( void );

    /// ȡ��������Ϣ
    static char * get_profile_string( const char * entry_name );

#endif /* UPF_OS_IS_WINNT */


private:
    /// ���ʵ��UPF_IMemory_Pool�ӿڵ�ʵ��, ������������
    /// UPF_Memory_Pool_Debug������
    UPF_IMemory_Pool * m_memory_pool;

    /// ���ڱ����ѷ���ڵ��˫��ѭ�������ͷָ��.
    UPF_Memory_Pool_Debug_Node * m_head_debug_node;

    /// �߳�ͬ����
    Lock m_lock;

#ifdef UPF_OS_IS_WINNT

    /// dump memory �߳̾��
    HANDLE       m_dump_memory_thread_handle;

    /// dump memory �߳��Ƿ���ֹ�ı�־
    bool         m_is_dump_memory_thread_stopped;

    /// dump memory �߳���������Ӧ֪ͨ���¼�����.
    HANDLE       m_dump_memory_event;

    /// DUMP�ڴ������
    UPF_Dump_Memory_Type m_dump_memory_type;

    /// ���Debug��Ϣ�Ľӿ�
    UPF_IMemory_Pool_Debug_Print * m_debug_print;

#endif /* UPF_OS_IS_WINNT */

    /// �ܹ������ȥ���ڴ��С
    size_t       m_total_allocated_size;

    /// �ܹ������ȥ���ڴ�����
    size_t       m_total_allocated_count;
};

#if defined (UPF_TEMPLATES_REQUIRE_SOURCE)
#include "UPF_Memory_Pool_Debug_T.cpp"
#endif /* UPF_TEMPLATES_REQUIRE_SOURCE */

#endif /* UPF_MEMORY_POOL_DEBUG_T_H */



