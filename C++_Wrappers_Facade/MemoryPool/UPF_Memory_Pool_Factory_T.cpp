/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Memory_Pool_Factory_T.cpp                                                              *
*       CREATE DATE     : 2007-11-12                                                                                 *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]内存池工厂的实现![CN]                                                                  *
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
 * @brief 一个辅助的查找UPF_Memory_Pool的函数对象
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
 * 根据给定的信息创建相应的内存池.
 * 
 * 如果is_fixed为true，则创建固定对象大小的内存池，这时还需指定每个对象的大小
 * (object_size), 每块内存中包含的对象个数(num_object_per_block), 固定对象大小
 * 的内存池会根据这两个信息创建一个对齐到页面大小(通常为4k)的初始内存块，
 * 然后在进行随后的分配内存操作.
 * 
 * 如果is_fixed为false, 则创建可变对象大小的内存池，object_size和
 * num_object_per_block并不使用.
 * 
 * 如果存在环境变量UPF_MEMORY_POOL_IS_DEBUG, 并设置为1, 则使用调试版本的内存池.
 * 
 * @param memory_pool_name       [in]  内存池名字
 * @param is_fixed               [in]  是否为固定大小的内存池, true为是固定大小
 *                                     的内存池,  false为可变大小的内存池.
 * @param allocator_type         [in]  allocator的类型, 目前支持UPF_TOPLEVEL_PERMANENT_ALLOCATOR和
 *                                     UPF_TOPLEVEL_DISCARDABLE_ALLOCATOR, 缺省为UPF_TOPLEVEL_PERMANENT_ALLOCATOR
 * @param allocated_memory_limit [in]  分配内存的上限值, 在Debug版本下，当Client端
 *                                     申请的内存超过了这个限制，将会断言失败.
 * @param is_debug               [in]  是否使用调试版本的内存池, 当使用调试版本
 *                                     的内存池时, 要使用allocate_debug来分配内存.
 * @param object_size            [in]  如果is_fixed为true, 
 *                                     则这个参数指定每个对象的大小，缺省值为16.
 * @param num_objects_per_block  [in]  如果is_fixed为true, 
 *                                     则这个参数指定每个内存块中包含的对象个数,
 *                                     缺省值为256.
 * 
 * @retval 一个内存池实例指针  创建内存池成功. 
 * @retval 0                   创建内存池失败 . 
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
 * 根据给定的内存池名来获取内存池接口实例
 * 
 * @param memory_pool_name             [in]    内存池名字
 * 
 * @retval    内存池接口实例     查找成功
 *            0                  查找失败
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
 * 根据给定的内存池名来释放内存池.
 * 
 * @param memory_pool_name        [in]  要释放的内存池名字
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
 * 从系统环境变量中取出是否打开内存池调试功能的标志.
 * 
 * 从UPF_MEMORY_POOL_IS_DEBUG环境变量中取出是否打开内存池调试功能的标志,
 * 如果UPF_MEMORY_POOL_IS_DEBUG被设为1，则打开，否则不打开。缺省值为
 * 不打开调试功能.
 * 
 * @retval  true   打开内存池调试功能.
 * @retval  false  不打开内存池调试功能.
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
 * 根据系统环境变量USING_UPF_MEMORY_POOL_TYPE来创建内存池.
 * 
 * 从USING_UPF_MEMORY_POOL_TYPE环境变量中取出想要使用的内存池类型, 缺省为
 * UPF_Common_Memory_Pool.
 * 
 * USING_UPF_MEMORY_POOL_TYPE的值含义如下:
 * 
 * <TABLE>
 * <TR><TD><B></b>USING_UPF_MEMORY_POOL_TYPE值</B></TD><TD><B>内存池</B></TD></TR>
 * <TR><TD>1</TD><TD>UPF_Common_Memory_Pool</TD>
 * <TR><TD>2</TD><TD>UPF_Common_Memory_Pool2</TD>
 * <TR><TD>3</TD><TD>UPF_Memory_Pool_CRT_Debug_Alloc</TD>
 * </TABLE>
 * 
 * @param memory_pool_name              [in]  内存池名称.
 * @param allocated_memory_limit        [in]  分配内存的上限值.
 * @param allocator_type                [in]  allocator的类型, 目前支持UPF_TOPLEVEL_PERMANENT_ALLOCATOR和
 *                                            UPF_TOPLEVEL_DISCARDABLE_ALLOCATOR, 缺省为UPF_TOPLEVEL_PERMANENT_ALLOCATOR
 * 
 * @return 实现UPF_IMemory_Pool接口的实例.
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
 * 取得配置信息, 在winnt下暂时用getenv, wince下未实现.
 * 
 * @param entry_name        [in]  配置入口名.
 * @param default_value     [in]  缺省值
 *
 * @return 配置值
 *
 * @todo 以后考虑用配置文件代替.
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
 * 根据给定的信息创建事务型的内存池.
 * 
 * 
 * @param memory_pool_name       [in]  内存池名字
 * 
 * @param allocator_type         [in]  allocator的类型. 
 *  
 * @param allocated_memory_limit [in]  分配内存的上限值, 在Debug版本下，当Client端
 *                                     申请的内存超过了这个限制，将会断言失败.
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

