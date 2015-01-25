/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Memory_Pool_Info_Manager_Impl_T.cpp                                                    *
*       CREATE DATE     : 2007-11-20                                                                                 *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : CN]UPF_IMemory_Pool_Info_Manager�ӿڵ�ʵ��ģ����![CN]                                      *
*********************************************************************************************************************/
#include <stdio.h>               /* for atexit */
#include <string.h>              /* for strcmp */
#include <assert.h>              /* for assert */

#include "UPF_Memory_Pool_Info_Manager_Impl_T.h"
#include "UPF_Thread_Sync.h"
#include "UPF_Memory_Pool_Components.h"
#include "UPF_Memory_Pool_Util.h"

// Static member definitions.
template < class Lock, class PrintPolicy >
Lock UPF_Memory_Pool_Info_Manager_Impl< Lock, PrintPolicy >::sm_instance_lock;

/** 
 * һ����������������һ��UPF_IMemory_Pool_Info_Manager��ʵ��.
 * 
 *  
 * @return UPF_IMemory_Pool_Info_Manager&
 */
template < class Lock, class PrintPolicy >
UPF_IMemory_Pool_Info_Manager * 
UPF_Memory_Pool_Info_Manager_Impl< Lock, PrintPolicy >::instance( void )
{
    static UPF_Memory_Pool_Info_Manager_Impl< Lock, PrintPolicy > * 
        sl_memory_pool_info_manager = 0;

    if ( 0 == sl_memory_pool_info_manager )
    {
        UPF_Guard< Lock > guard( sm_instance_lock );

        if ( 0 == sl_memory_pool_info_manager )
        {
            sl_memory_pool_info_manager =
                 new UPF_Memory_Pool_Info_Manager_Impl< Lock, PrintPolicy >;
    
            atexit( destroy );
        }
    }

    return sl_memory_pool_info_manager;
}

/**
 *  �����UPF_Memory_Pool_Info_Manager_Impl������ռ�õ��ڴ�.
 */
template < class Lock, class PrintPolicy >
void
UPF_Memory_Pool_Info_Manager_Impl< Lock, PrintPolicy >::destroy( void )
{
    delete instance();
}

/** 
 * ���캯��, ��ʼ���ڲ���״̬.
 * 
 */
template < class Lock, class PrintPolicy >
UPF_Memory_Pool_Info_Manager_Impl< Lock, PrintPolicy >::
UPF_Memory_Pool_Info_Manager_Impl( void )
{
    m_memory_pool_info_head = 0;
}

/** 
 * �����������ͷ��ڲ����������Դ.
 * 
 */
template < class Lock, class PrintPolicy >
UPF_Memory_Pool_Info_Manager_Impl< Lock, PrintPolicy >::
~UPF_Memory_Pool_Info_Manager_Impl( void )
{
    UPF_Guard< Lock > guard( this->m_lock );

    // destroy memory pool info item list.
    UPF_Memory_Pool_Info_Item * next_pool_info_item = 0;

    while ( m_memory_pool_info_head )
    {
        next_pool_info_item = m_memory_pool_info_head->m_next_item;

        delete m_memory_pool_info_head;

        m_memory_pool_info_head = next_pool_info_item;
    }
}

/**
 * ���UPF_IMemory_Pool_Infoʵ�����ڴ�ط�����Ϣ��������.
 *
 * @param memory_pool_info   [in]  ʵ��UPF_IMemory_Pool_Info�ӿڵ�ʵ��
 */
template < class Lock, class PrintPolicy >
void
UPF_Memory_Pool_Info_Manager_Impl< Lock, PrintPolicy >::add_memory_pool_info(
    UPF_IMemory_Pool_Info* memory_pool_info )
{
    if ( memory_pool_info != 0 )
    {
        UPF_Guard< Lock > guard( this->m_lock );

        m_memory_pool_info_head = new UPF_Memory_Pool_Info_Item(
            memory_pool_info, m_memory_pool_info_head );
    }
}

/**
 * ���ڴ�ط�����Ϣ���������Ƴ�UPF_IMemory_Pool_Infoʵ��.
 *
 * @param memory_pool_info   [in] ʵ��UPF_IMemory_Pool_Info�ӿڵ�ʵ��
 * 
 */
template < class Lock, class PrintPolicy >
void 
UPF_Memory_Pool_Info_Manager_Impl< Lock, PrintPolicy >::remove_memory_pool_info( 
    UPF_IMemory_Pool_Info* memory_pool_info )
{
    UPF_Guard< Lock > guard( this->m_lock );

    UPF_Memory_Pool_Info_Item * previous_item = 0;
    UPF_Memory_Pool_Info_Item * current_item  = 0;

    for ( current_item = m_memory_pool_info_head; 
          current_item != 0;
          previous_item = current_item,
          current_item = current_item->m_next_item )
    {
        if ( current_item->m_memory_pool_info == memory_pool_info )
        {
            break;
        }
    }

    if ( current_item != 0 )
    {
        if ( previous_item != 0 )
        {
            previous_item->m_next_item = current_item->m_next_item;
        }
        else
        {
            m_memory_pool_info_head = current_item->m_next_item;
        }

        delete current_item;
    }

}

/**
 * �����ڴ��������ȡ��UPF_IMemory_Pool_Infoʵ��.
 *
 * @param memory_pool_name      [in]   �ڴ������.
 *
 * @retval UPF_IMemory_Pool_Infoʵ��   ���ҳɹ�
 * @retval 0                           ����ʧ��
 */
template < class Lock, class PrintPolicy >
UPF_IMemory_Pool_Info* 
UPF_Memory_Pool_Info_Manager_Impl< Lock, PrintPolicy >::get_memory_pool_info(
    const char* memory_pool_name )
{
    UPF_Guard< Lock > guard( this->m_lock );

    UPF_IMemory_Pool_Info* result = 0;

    for ( UPF_Memory_Pool_Info_Item * current_item = m_memory_pool_info_head;
          current_item != 0;
          current_item = current_item->m_next_item )
    {
        if ( strcmp( memory_pool_name,  
                     current_item->m_memory_pool_info->get_memory_pool_name() ) == 0 )
        {
            result = current_item->m_memory_pool_info;
            break;
        }
    }

    return result;
}

/** 
 * �����ڴ���ַ������UPF_IMemory_Poolʵ��.
 * 
 * @param ptr                   [in] �ڴ���ַ.
 * @param excluded_memory_pool  [in] �ڲ���UPF_IMemory_Poolʵ��ʱ�������
 *                                   ���ڴ��ʵ��.
 * 
 * @retval һ������ptrָ����ڴ��UPF_IMemory_Poolʵ�� ���ҳɹ�.
 * @retval 0                                                ����ʧ��.
 */
template < class Lock, class PrintPolicy >
UPF_IMemory_Pool * 
UPF_Memory_Pool_Info_Manager_Impl< Lock, PrintPolicy >::
get_memory_pool_with_memory_addr( void *             ptr,
                                  UPF_IMemory_Pool * excluded_memory_pool )
{
    UPF_Guard< Lock > guard( this->m_lock );

    UPF_IMemory_Pool* result = 0;

    for ( UPF_Memory_Pool_Info_Item * current_item = m_memory_pool_info_head;
          current_item != 0;
          current_item = current_item->m_next_item )
    {
        assert( current_item->m_memory_pool_info != 0 );

        UPF_IMemory_Pool * memory_pool = 
                current_item->m_memory_pool_info->get_memory_pool();

        assert( memory_pool != 0 );

        if ( memory_pool != excluded_memory_pool &&
             memory_pool->is_memory_block_exist( ptr ) )
        {
            result = memory_pool;
            break;
        }
    }

    return result;
}

/**
 * ��ӡ������������ڴ�ط�����Ϣ.
 *
 */
template < class Lock, class PrintPolicy >
void 
UPF_Memory_Pool_Info_Manager_Impl< Lock, PrintPolicy >::print_all_memory_pool_infos( FILE * fp )
{
    UPF_Guard< Lock > guard( this->m_lock );

    // print header.
    UPF_Memory_Pool_Util::print_memory_size_headers( fp );

    // print content
    size_t  total_managed_memory_size    = 0;
    size_t  total_allocated_memory_size  = 0;
    size_t  total_allocated_memory_peak  = 0;
    size_t  total_wasted_memory_size     = 0;
    size_t  total_allocated_memory_limit = 0;

    size_t  current_managed_memory_size    = 0;
    size_t  current_allocated_memory_size  = 0;
    size_t  current_allocated_memory_peak  = 0;
    size_t  current_wasted_memory_size     = 0;
    size_t  current_allocated_memory_limit = 0;

    UPF_Memory_Pool_Info_Item * current_item = 0;


    for ( current_item = m_memory_pool_info_head;
          current_item != 0;
          current_item = current_item->m_next_item )
    {

        UPF_IMemory_Pool_Info * memory_pool_info = current_item->m_memory_pool_info;
       
        current_managed_memory_size    = memory_pool_info->get_managed_memory_size();
        current_allocated_memory_size  = memory_pool_info->get_allocated_memory_size();
        current_allocated_memory_peak  = memory_pool_info->get_allocated_memory_peak();
        current_wasted_memory_size     = memory_pool_info->get_wasted_memory_size();
        current_allocated_memory_limit = memory_pool_info->get_allocated_memory_limit();

        total_managed_memory_size    += current_managed_memory_size;
        total_allocated_memory_size  += current_allocated_memory_size;
        total_allocated_memory_peak  += current_allocated_memory_peak;
        total_wasted_memory_size     += current_wasted_memory_size;
        total_allocated_memory_limit += current_allocated_memory_limit;

        UPF_Memory_Pool_Util::print_memory_size_infos(
            fp,
            memory_pool_info->get_memory_pool_name(),
            current_managed_memory_size,
            current_allocated_memory_size,
            current_allocated_memory_peak,           
            current_wasted_memory_size,
            current_allocated_memory_limit
            );

    }

    // print seperate line.
    UPF_Memory_Pool_Util::print_chars( fp, 120, '=' );
    
    // print total size info
    UPF_Memory_Pool_Util::print_memory_size_infos(
        fp,
        "Total",
        total_managed_memory_size,
        total_allocated_memory_size,
        total_allocated_memory_peak,           
        total_wasted_memory_size,
        total_allocated_memory_limit
        );

    fprintf( fp, "\n\n" );

}


/**
 * ��ӡ������������ڴ�صĸ�Ҫ��Ϣ.
 *
 */
template < class Lock, class PrintPolicy >
void 
UPF_Memory_Pool_Info_Manager_Impl< Lock, PrintPolicy >::
print_all_memory_pool_summary_infos( const char * dir )
{
    UPF_Guard< Lock > guard( this->m_lock );

    char memory_pool_report_path[MAX_PATH];

    FILE * fp = NULL;
         
    bool is_first = true;

    UPF_Memory_Pool_Info_Item * current_item = 0;

    for ( current_item = m_memory_pool_info_head;
          current_item != 0;
          current_item = current_item->m_next_item )
    {
        UPF_IMemory_Pool_Info * memory_pool_info = current_item->m_memory_pool_info;        

        if ( is_first )
        {
            if ( strcmp( memory_pool_info->get_memory_pool_type(),
                         DEBUG_MEMORY_POOL_TYPE ) != 0 )
            {                
                break;
            }

            is_first = false;
        }

        _snprintf( memory_pool_report_path,
                   sizeof( memory_pool_report_path ),
                   "%s\\%s.csv",
                   dir,              
                   memory_pool_info->get_memory_pool_name() );

        fp = fopen( memory_pool_report_path, "wb" );

        if ( NULL != fp )
        {
            UPF_Memory_Pool_Debug * memory_pool_debug =
                static_cast< UPF_Memory_Pool_Debug * >( memory_pool_info );

            memory_pool_debug->print_allocated_memory_block_summary( fp );

            fclose( fp );
        }

    }

}


template < class Lock, class PrintPolicy >
bool 
UPF_Memory_Pool_Info_Manager_Impl< Lock, PrintPolicy >::is_exist_memory_pool(
    toplevel_allocator_t  allocator_type,
     bool                 is_log,
     const char *         custom_msg )
{
    UPF_Guard< Lock > guard( this->m_lock );

    bool is_exist = false;
    bool is_first_print_custom_msg = true;

    for ( UPF_Memory_Pool_Info_Item * current_item = m_memory_pool_info_head;
          current_item != 0;
          current_item = current_item->m_next_item )
    {
        if ( current_item->m_memory_pool_info->get_allocator_type() == allocator_type )
        {
            is_exist = true;

            if ( is_log )
            {
                if ( 0 != custom_msg && is_first_print_custom_msg )
                {
                    UPF_Memory_Pool_Util::report( "%s", custom_msg );

                    is_first_print_custom_msg = false;
                }

                UPF_Memory_Pool_Util::report( "[%d][%s] exist!",
                                              allocator_type,
                                              current_item->m_memory_pool_info->get_memory_pool_name() );
            }
        }
    }


    return ( is_exist );
}