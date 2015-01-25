/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Memory_Pool_Factory_T.cpp                                                              *
*       CREATE DATE     : 2007-11-12                                                                                 *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]�ڴ�ع�����ʵ��![CN]                                                                  *
*********************************************************************************************************************/

#include <algorithm>  /* for find_if */
#include <string.h>   /* for strcmp */
#include <stdlib.h>   /* for getenv */

#include "UPF_Memory_Pool_Factory_T.h"
#include "UPF_Memory_Pool_Components.h"
#include "UPF_Thread_Sync.h"

// Static data definitions
template < class Lock >
Lock UPF_Memory_Pool_Factory_T< Lock >::sm_lock;

template < class Lock >
Lock UPF_Memory_Pool_Factory_T< Lock >::sm_env_lock;

template < class Lock >
typename UPF_Memory_Pool_Factory_T< Lock >::Memory_Pool_Mgr
UPF_Memory_Pool_Factory_T< Lock >::sm_memory_pool_mgr;

/** 
 * @struct UPF_Memory_Pool_Map_Item_Equal_Functor
 * 
 * @brief һ�������Ĳ���UPF_Memory_Pool�ĺ�������
 * 
 */
struct UPF_Memory_Pool_Map_Item_Equal_Functor
{
    UPF_Memory_Pool_Map_Item_Equal_Functor( const char * memory_pool_name )
    {
        m_memory_pool_name = memory_pool_name;
    }

    bool operator() ( const UPF_Memory_Pool_Map_Item & item )
    {
        return ( strcmp( m_memory_pool_name, item.memory_pool_name ) == 0 );
    }

    const char * m_memory_pool_name;
};


/** 
 * ���ݸ�������Ϣ������Ӧ���ڴ��.
 * 
 * ���is_fixedΪtrue���򴴽��̶������С���ڴ�أ���ʱ����ָ��ÿ������Ĵ�С
 * (object_size), ÿ���ڴ��а����Ķ������(num_object_per_block), �̶������С
 * ���ڴ�ػ������������Ϣ����һ�����뵽ҳ���С(ͨ��Ϊ4k)�ĳ�ʼ�ڴ�飬
 * Ȼ���ڽ������ķ����ڴ����.
 * 
 * ���is_fixedΪfalse, �򴴽��ɱ�����С���ڴ�أ�object_size��
 * num_object_per_block����ʹ��.
 * 
 * ������ڻ�������UPF_MEMORY_POOL_IS_DEBUG, ������Ϊ1, ��ʹ�õ��԰汾���ڴ��.
 * 
 * @param memory_pool_name       [in]  �ڴ������
 * @param is_fixed               [in]  �Ƿ�Ϊ�̶���С���ڴ��, trueΪ�ǹ̶���С
 *                                     ���ڴ��,  falseΪ�ɱ��С���ڴ��.
 * @param allocator_type         [in]  allocator������, Ŀǰ֧��UPF_TOPLEVEL_PERMANENT_ALLOCATOR��
 *                                     UPF_TOPLEVEL_DISCARDABLE_ALLOCATOR, ȱʡΪUPF_TOPLEVEL_PERMANENT_ALLOCATOR
 * @param allocated_memory_limit [in]  �����ڴ������ֵ, ��Debug�汾�£���Client��
 *                                     ������ڴ泬����������ƣ��������ʧ��.
 * @param is_debug               [in]  �Ƿ�ʹ�õ��԰汾���ڴ��, ��ʹ�õ��԰汾
 *                                     ���ڴ��ʱ, Ҫʹ��allocate_debug�������ڴ�.
 * @param object_size            [in]  ���is_fixedΪtrue, 
 *                                     ���������ָ��ÿ������Ĵ�С��ȱʡֵΪ16.
 * @param num_objects_per_block  [in]  ���is_fixedΪtrue, 
 *                                     ���������ָ��ÿ���ڴ���а����Ķ������,
 *                                     ȱʡֵΪ256.
 * 
 * @retval һ���ڴ��ʵ��ָ��  �����ڴ�سɹ�. 
 * @retval 0                   �����ڴ��ʧ�� . 
 */
template < class Lock >
UPF_IMemory_Pool *
UPF_Memory_Pool_Factory_T< Lock >::create_memory_pool( const char *          memory_pool_name,
                                                       bool                  is_fixed,
                                                       toplevel_allocator_t  allocator_type,
                                                       size_t                allocated_memory_limit,
                                                       bool                  is_debug,
                                                       size_t                object_size,
                                                       size_t                num_objects_per_block )
{
    UPF_IMemory_Pool * memory_pool = 0;

    UPF_IMemory_Pool_Info_Manager * pool_info_manager = 
        UPF_Memory_Pool_Info_Manager::instance();

    // first check memory pool that's name be memory_pool_name if exists,
    // if exists, use the existed memory pool and trace a message.
    if ( 0 != pool_info_manager )
    {
        UPF_IMemory_Pool_Info * memory_pool_info = 
            pool_info_manager->get_memory_pool_info( memory_pool_name );

        if ( 0 != memory_pool_info )
        {
            memory_pool = memory_pool_info->get_memory_pool();

//            UPF_Memory_Pool_Util::trace( "Memory Pool [name: %s] has already existed, so we use the existed memory pool!!!\n",
//                                         memory_pool_name );
        }
    }

    // if memory pool that's name be memory_pool_name not exist,
    // then create a memory pool
    if ( 0 == memory_pool )
    {

        if ( false == is_debug )
        {
            is_debug = SELF_CLASS::get_debug_flag_from_env();
        }
        
        if ( is_fixed )
        {
            // if use debug version of memory pool, object size should add 
            // SIZE_OF_DEBUG_NODE(20) bytes.
            if ( is_debug )
            {
                if ( 0 == object_size )
                {
                    object_size = 1;
                }

                object_size += UPF_Memory_Pool_Debug::SIZE_OF_DEBUG_NODE;
            }
    
            memory_pool = new UPF_Fixed_Memory_Pool( memory_pool_name,
                                                     object_size,
                                                     num_objects_per_block,
                                                     allocator_type );
        }
        else
        {
            memory_pool = SELF_CLASS::create_memory_pool_from_env( 
                memory_pool_name, allocated_memory_limit, allocator_type );
        }

        if ( is_debug && memory_pool != 0 )
        {
            memory_pool = new UPF_Memory_Pool_Debug( memory_pool );
        }

        // add memory_pool into memory pool manager and memory pool info manager.
        if ( memory_pool != 0 )
        {
            // add into memory pool info manager
            if ( pool_info_manager != 0 )
            {
                pool_info_manager->add_memory_pool_info(
                     memory_pool->get_memory_pool_info() );
            }

            // add into memory pool manager
            UPF_Guard< Lock > guard( sm_lock );
    
            UPF_Memory_Pool_Map_Item item;
    
            item.memory_pool_name = memory_pool_name;
            item.memory_pool      = memory_pool;
    
            sm_memory_pool_mgr.push_back( item );
        }
    }

    return memory_pool;
}

/** 
 * ���ݸ������ڴ��������ȡ�ڴ�ؽӿ�ʵ��
 * 
 * @param memory_pool_name             [in]    �ڴ������
 * 
 * @retval    �ڴ�ؽӿ�ʵ��     ���ҳɹ�
 *            0                  ����ʧ��
 */
template < class Lock >
UPF_IMemory_Pool * 
UPF_Memory_Pool_Factory_T< Lock >::get_memory_pool( const char * memory_pool_name )
{
    UPF_IMemory_Pool * result = 0;

    if ( memory_pool_name != 0 )
    {
        UPF_Guard< Lock > guard( sm_lock );

        Memory_Pool_Mgr::iterator it_begin = sm_memory_pool_mgr.begin();
        Memory_Pool_Mgr::iterator it_end   = sm_memory_pool_mgr.end();

        Memory_Pool_Mgr::iterator it_find =  
            std::find_if( it_begin, it_end, 
                          UPF_Memory_Pool_Map_Item_Equal_Functor( memory_pool_name ) );

        if ( it_find != it_end )
        {
            result = it_find->memory_pool;
        }
    }

    return result;
}

/** 
 * ���ݸ������ڴ�������ͷ��ڴ��.
 * 
 * @param memory_pool_name        [in]  Ҫ�ͷŵ��ڴ������
 */
template < class Lock >
void 
UPF_Memory_Pool_Factory_T< Lock >::delete_memory_pool( const char * memory_pool_name )
{
    if ( memory_pool_name != 0 )
    {
        UPF_Guard< Lock > guard( sm_lock );

        Memory_Pool_Mgr::iterator it_begin = sm_memory_pool_mgr.begin();
        Memory_Pool_Mgr::iterator it_end   = sm_memory_pool_mgr.end();

        Memory_Pool_Mgr::iterator it_find =  
            std::find_if( it_begin, it_end, 
                          UPF_Memory_Pool_Map_Item_Equal_Functor( memory_pool_name ) );

        if ( it_find != it_end )
        {
            UPF_IMemory_Pool * memory_pool = it_find->memory_pool;

            // before delete memory_pool, need to remove memory pool info from
            // memory pool info manager.
            UPF_IMemory_Pool_Info_Manager * pool_info_manager = 
                UPF_Memory_Pool_Info_Manager::instance();

            if ( pool_info_manager != 0 )
            {
                pool_info_manager->remove_memory_pool_info( 
                    memory_pool->get_memory_pool_info() );
            }


            delete memory_pool;

            sm_memory_pool_mgr.erase( it_find );

        }
    }
}

/** 
 * ��ϵͳ����������ȡ���Ƿ���ڴ�ص��Թ��ܵı�־.
 * 
 * ��UPF_MEMORY_POOL_IS_DEBUG����������ȡ���Ƿ���ڴ�ص��Թ��ܵı�־,
 * ���UPF_MEMORY_POOL_IS_DEBUG����Ϊ1����򿪣����򲻴򿪡�ȱʡֵΪ
 * ���򿪵��Թ���.
 * 
 * @retval  true   ���ڴ�ص��Թ���.
 * @retval  false  �����ڴ�ص��Թ���.
 * 
 */
template < class Lock >
bool
UPF_Memory_Pool_Factory_T< Lock >::get_debug_flag_from_env( void )
{
    static int sl_debug_flag = -1; // first is -1, after processed, can be 0 or 1

    if ( -1 == sl_debug_flag )
    {
        // sl_debug_flag default is 0, namely be not debug.
        const char * const MEMORY_POOL_IS_DEBUG_ENV_VAR = "UPF_MEMORY_POOL_IS_DEBUG";

        const int MEMORY_POOL_DEBUG_OFF_VALUE = 0;
               
        sl_debug_flag = get_profile_string( MEMORY_POOL_IS_DEBUG_ENV_VAR,
                                            MEMORY_POOL_DEBUG_OFF_VALUE  );
    }

    return ( sl_debug_flag == 1 );
}

#ifdef UPF_OS_IS_WINCE
#include <dbgapi.h>        
#endif

/** 
 * ����ϵͳ��������USING_UPF_MEMORY_POOL_TYPE�������ڴ��.
 * 
 * ��USING_UPF_MEMORY_POOL_TYPE����������ȡ����Ҫʹ�õ��ڴ������, ȱʡΪ
 * UPF_Common_Memory_Pool.
 * 
 * USING_UPF_MEMORY_POOL_TYPE��ֵ��������:
 * 
 * <TABLE>
 * <TR><TD><B></b>USING_UPF_MEMORY_POOL_TYPEֵ</B></TD><TD><B>�ڴ��</B></TD></TR>
 * <TR><TD>1</TD><TD>UPF_Common_Memory_Pool</TD>
 * <TR><TD>2</TD><TD>UPF_Common_Memory_Pool2</TD>
 * <TR><TD>3</TD><TD>UPF_Memory_Pool_CRT_Debug_Alloc</TD>
 * </TABLE>
 * 
 * @param memory_pool_name              [in]  �ڴ������.
 * @param allocated_memory_limit        [in]  �����ڴ������ֵ.
 * @param allocator_type                [in]  allocator������, Ŀǰ֧��UPF_TOPLEVEL_PERMANENT_ALLOCATOR��
 *                                            UPF_TOPLEVEL_DISCARDABLE_ALLOCATOR, ȱʡΪUPF_TOPLEVEL_PERMANENT_ALLOCATOR
 * 
 * @return ʵ��UPF_IMemory_Pool�ӿڵ�ʵ��.
 * 
 */
template < class Lock >
UPF_IMemory_Pool * 
UPF_Memory_Pool_Factory_T< Lock >::create_memory_pool_from_env( 
        const char *                     memory_pool_name,
        size_t                           allocated_memory_limit,
        toplevel_allocator_t             allocator_type )
{
    // cache memory pool type.
    static int sl_memory_pool_type         = -1;

    const char * const USING_UPF_MEMORY_POOL_TYPE_ENV_VAR =
         "USING_UPF_MEMORY_POOL_TYPE";

    const char * const TOPLEVEL_PERMANENT_ALLOCATOR_SIZE =
         "TOPLEVEL_PERMANENT_ALLOCATOR_SIZE";

    const int COMMON_MEMORY_POOL_TYPE_VALUE          = 1; 
    const int COMMON_MEMORY_POOL2_TYPE_VALUE         = 2; 
    const int MEMORY_POOL_CRT_DEBUG_ALLOC_TYPE_VALUE = 3; 
    const int MEMORY_POOL_HEAP_ALLOC_TYPE_VALUE      = 4;
    const int GLOBAL_MEMORY_POOL_TYPE_VALUE          = 5;


    UPF_IMemory_Pool * memory_pool      = 0;

    if ( -1 == sl_memory_pool_type )
    {
        sm_env_lock.acquire();

        sl_memory_pool_type = get_profile_string( USING_UPF_MEMORY_POOL_TYPE_ENV_VAR,
                                                  GLOBAL_MEMORY_POOL_TYPE_VALUE );

        int quota_value = get_profile_string( TOPLEVEL_PERMANENT_ALLOCATOR_SIZE, 27 );

        UPF_Virtual_Memory_Allocator::set_managed_memory_size(
                            UPF_TOPLEVEL_PERMANENT_ALLOCATOR,
                            quota_value * 1024 * 1024 );

#ifdef UPF_OS_IS_WINCE
        NKDbgPrintfW( L"type: %d, quota value: %d\n", quota_value );
#endif
        sm_env_lock.release();
    }

    switch ( sl_memory_pool_type )
    {
    case COMMON_MEMORY_POOL2_TYPE_VALUE:
        memory_pool = new UPF_Common_Memory_Pool2( memory_pool_name,
                                                   allocated_memory_limit,
                                                   allocator_type ); 
        break;

#if defined(UPF_OS_IS_WINNT)
    // wince platform hasn't crt debug function, here use crt debug only in winnt platform.
    case MEMORY_POOL_CRT_DEBUG_ALLOC_TYPE_VALUE:
        memory_pool = new UPF_Memory_Pool_CRT_Debug_Alloc( memory_pool_name,
                                                           allocated_memory_limit,
                                                           allocator_type ); 
        break;
#endif
    case MEMORY_POOL_HEAP_ALLOC_TYPE_VALUE:
        memory_pool = new UPF_Memory_Pool_Heap_Alloc( memory_pool_name,
                                                      allocated_memory_limit,
                                                      allocator_type );
        break;

    case GLOBAL_MEMORY_POOL_TYPE_VALUE:

        // if allocator is tran allocator, namely allocator_type >=
        // UPF_STATIC_TOPLEVEL_ALLOCATOR_TYPES, then not use
        // GLOBAL_MEMORY_POOL_TYPE_VALUE, otherwise use profile's type.

        if ( allocator_type < UPF_STATIC_TOPLEVEL_ALLOCATOR_TYPES )
        {
            memory_pool = new UPF_Global_Memory_Pool( memory_pool_name,
                                                      allocated_memory_limit,
                                                      allocator_type );
        }
        else
        {
            memory_pool = new UPF_Common_Memory_Pool( memory_pool_name,
                                                      allocated_memory_limit,
                                                      allocator_type );
        }

        break;

    default:
        memory_pool = new UPF_Common_Memory_Pool( memory_pool_name,
                                                  allocated_memory_limit,
                                                  allocator_type );
        break;
    }

    return memory_pool;
}

/** 
 * ȡ��������Ϣ, ��winnt����ʱ��getenv, wince��δʵ��.
 * 
 * @param entry_name        [in]  ���������.
 * @param default_value     [in]  ȱʡֵ
 *
 * @return ����ֵ
 *
 * @todo �Ժ����������ļ�����.
 *
 */
template < class Lock >
int
UPF_Memory_Pool_Factory_T< Lock >::get_profile_string( const char * entry_name,
                                                       int          default_value )
{

#if defined(UPF_OS_IS_WINNT)

    char* env_value = getenv( entry_name );

    if ( NULL == env_value )
    {
        return ( default_value );
    }
    else
    {
        return ( atoi( env_value ) );
    }

#elif defined(UPF_OS_IS_WINCE)
    // read config file on wince platform

    char file_name[ 260 ];

    if ( -1 == UPF_OS::get_module_file_name( NULL,
										     file_name,
											 200 ) )
    {
        return (default_value);
    }

    int length = strlen( file_name );

    while ( length != 0 &&
            file_name[ length - 1 ] != '\\' &&
            file_name[ length - 1 ] != '/' )
    {
        --length;
    }

    if ( length != 0 )
    {
        file_name[ length ] = '\0';
    }

    strcat( file_name, "UPF.conf" );

    // set result be default value.
    int result  = default_value;

    FILE * fp = fopen( file_name, "r" );

    // if UPF.conf not found, then return default_value.
    if ( NULL == fp )
    {
        return ( default_value );
    }
    else
    {                        
        char line[ 100 ];

        // read param value.
        while ( fgets( line, 100, fp ) )
        {
            if ( line[0] != '\0' && line[0] != '\n' && line[0] != '#' )
            {
                if ( strstr( line, entry_name ) )
                {
                    char * value_ptr = strchr( line, '=' );
                    if ( value_ptr )
                    {
                        ++value_ptr;
                        
                        // skip leading space.
                        while ( *value_ptr && isspace( *value_ptr ) )
                        {
                            ++value_ptr;
                        }
                        
                        result = atoi( value_ptr );

                        break;
                    }
                }
            }
        }  
        
        fclose( fp );
    }   

    return ( result );
#endif

}

/**
 * ���ݸ�������Ϣ���������͵��ڴ��.
 * 
 * 
 * @param memory_pool_name       [in]  �ڴ������
 * 
 * @param allocator_type         [in]  allocator������. 
 *  
 * @param allocated_memory_limit [in]  �����ڴ������ֵ, ��Debug�汾�£���Client��
 *                                     ������ڴ泬����������ƣ��������ʧ��.
 * 
 * @return UPF_IMemory_Pool* 
 */
template < class Lock >
UPF_IMemory_Pool *
UPF_Memory_Pool_Factory_T< Lock >::create_trans_memory_pool( 
                const char *         memory_pool_name,
                toplevel_allocator_t allocator_type,
                size_t               allocated_memory_limit )
{
    UPF_IMemory_Pool * memory_pool = 0;

    bool is_debug = SELF_CLASS::get_debug_flag_from_env();

    memory_pool = SELF_CLASS::create_memory_pool_from_env( 
                    memory_pool_name, 
                    allocated_memory_limit, 
                    allocator_type );

    if ( is_debug && memory_pool != 0 )
    {
        memory_pool = new UPF_Memory_Pool_Debug( memory_pool );
    }

    return memory_pool;

}

