/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Malloc.cpp                                                                             *
*       CREATE DATE     : 2007-12-11                                                                                 *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]封装了类似C标准库的内存分配函数, 包括malloc, realloc, free.![CN]                 *
*********************************************************************************************************************/

#pragma warning(disable: 4786)

#include <assert.h>                    /* for assert */
#include <new>
#include <windows.h>                   /* for OutputDebugString */
#include <stdio.h>                     /* for _vsnprintf */ 
#include <stdarg.h>                    /* for va_start, va_end */

#include "UPF_VM_Query.h"


#ifndef UPF_TEMPLATES_REQUIRE_SOURCE
#define UPF_TEMPLATES_REQUIRE_SOURCE
#endif 

// 定义在回收内存时对内存池是否仍然存在作检查, 正式发布时应去掉.
#define MEMORY_POOL_EXISTS_CHECK_WHEN_RECLAIM

#include "UPF_Malloc.h"
#include "UPF_Memory_Pool_Components.h"

#ifdef _DEBUG
# define UPF_MALLOC_TRACE(msg) UPF_Memory_Pool_Util::trace msg
#else
# define UPF_MALLOC_TRACE(msg) (void)0
#endif /* _DEBUG */

#ifdef UPF_MALLOC_DEBUG
#  undef UPF_Malloc
#  undef UPF_Realloc
#  undef UPF_Free
#endif

// malloc debug report
// #define UPF_MALLOC_DEBUG_REPORT

// 定义打印的最大长度.
#define MAX_PRINT_WIDTH                   (140)

#define MAX_SECTION_WIDTH                 (120)

/// 打印内存池信息.
static void UPF_Print_Memory_Pool_Info( FILE * fp );

/// 打印内存状态.
static void UPF_Print_Global_Memory_Status( FILE * fp );

/// 打印内存页详细信息
static void UPF_Print_Memory_Pages_Info( FILE * fp );

/** 
 * 一个辅助函数，查找分配ptr所指内存的内存池实例.
 * 
 * @param memory_pool       [in]   首先查找的内存池实例.
 * @param ptr               [in]   指向一有效内存的指针.
 * 
 * @retval 一个分配ptr所指内存的内存池实例  查找成功
 * @retval 0                                查找失败
 */
inline UPF_IMemory_Pool * 
get_memory_pool_owner( UPF_IMemory_Pool * memory_pool, void * ptr )
{
    if ( memory_pool->is_memory_block_exist( ptr ) )
    {
        return memory_pool;
    }
    else
    {
        UPF_IMemory_Pool * real_memory_pool = 
            UPF_Memory_Pool_Info_Manager::instance()->
                get_memory_pool_with_memory_addr( ptr, 
                                                  memory_pool );
        return real_memory_pool;
    }
}

/** 
 * 创建一个allocator.
 * 
 * 在调用UPF_Malloc, UPF_Realloc, UPF_Free之前需要调用UPF_New_Allocator来创建一个
 * allocator, 然后UPF_Malloc, UPF_Realloc, UPF_Free使用这个allocator进行实际的操作.
 * 
 * @param allocator_name           [in]   用于标识这个allocator的名字, 一般为相应的
 *                                        模块的名字.
 * @param allocated_memory_limit   [in]   Client端分配内存的上限值
 * 
 * @param allocator_type           [in]   allocator的类型, 目前支持UPF_TOPLEVEL_PERMANENT_ALLOCATOR和
 *                                        UPF_TOPLEVEL_DISCARDABLE_ALLOCATOR, 缺省为UPF_TOPLEVEL_PERMANENT_ALLOCATOR
 * @retval   allocator instance handle   创建成功.
 * @retval   0                           创建失败.
 */
UPF_Allocator_Handle
UPF_New_Allocator_By_Name( const char *           allocator_name,
                           size_t                 allocated_memory_limit,
                           toplevel_allocator_t   allocator_type )
{

    // not use UPF_TOPLEVEL_DISCARDABLE_ALLOCATOR temporarily.

    UPF_IMemory_Pool * memory_pool = 
        UPF_Memory_Pool_Factory::create_memory_pool( allocator_name, 
                                                     false,
                                                     UPF_TOPLEVEL_PERMANENT_ALLOCATOR,
                                                     allocated_memory_limit );

    UPF_Allocator_Handle result = 
        reinterpret_cast< UPF_Allocator_Handle >( memory_pool );

    return result;
}


/** 
 * 销毁一个allocator.
 * 
 * @param allocator               [in] 使用UPF_New_Allocator所创建的allocator. 
 */
void 
UPF_Delete_Allocator( UPF_Allocator_Handle allocator )
{
    UPF_IMemory_Pool * memory_pool =
         reinterpret_cast< UPF_IMemory_Pool * >( allocator );

    if ( 0 != memory_pool )
    {
        UPF_Memory_Pool_Factory::delete_memory_pool( 
            memory_pool->get_memory_pool_info()->get_memory_pool_name() );   
    }
}

/** 
 * 分配size大小的内存.
 * 
 * @param allocator               [in]  使用UPF_New_Allocator所创建的allocator. 
 * @param size                    [in]  要申请的内存大小, 如果size为0, 默认分配1
 *                                      字节的内存.
 * 
 * @retval 一个指向有效内存的指针    分配成功.
 * @retval 0                         当allocaotr为0或者分配失败时.
 */
void * 
UPF_Malloc( UPF_Allocator_Handle allocator, size_t size )
{
    void * result = 0;

    if ( 0 != allocator  )
    {
        UPF_IMemory_Pool * memory_pool =
            reinterpret_cast< UPF_IMemory_Pool * >( allocator );

        result = memory_pool->allocate( size );
    }

    return result;
}

/** 
 * UPF_Malloc的Debug版本, 可以用于记录一些附加的信息, dump已分配的内存块,
 * 检查内存泄露.
 * 
 * @param allocator              [in] 使用UPF_New_Allocator所创建的allocator. 
 * @param size                   [in] 要申请的内存大小, 如果size为0, 默认分配1
 *                                    字节的内存.
 * @param file_name              [in] 调用UPF_Malloc_Debug所在的源文件名.
 * @param line_no                [in] 调用UPF_Malloc_Debug所在的源文件行号.
 * 
 * @retval 一个指向有效内存的指针    分配成功.
 * @retval 0                         当allocaotr为0或者分配失败时.
 */

void my_trace( const char * fmt, ... )
{
    va_list arg_list;

    va_start( arg_list, fmt );
    char buffer[ BUFSIZ + 1 ];

    _vsnprintf( buffer, BUFSIZ, fmt, arg_list );

    buffer[ BUFSIZ ] = '\0';

    va_end( arg_list );

    UPF_Memory_Pool_Util::report( buffer );
}


void * 
UPF_Malloc_Debug( UPF_Allocator_Handle allocator, 
                  size_t               size,
                  const char *         file_name,
                  int                  line_no )
{
    void * result = 0;

    if ( 0 != allocator  )
    {
        const int PRINT_FLAGS_LENGTH = 5;

        static LONG  sl_print_flags[ PRINT_FLAGS_LENGTH ];

        UPF_IMemory_Pool * memory_pool =
            reinterpret_cast< UPF_IMemory_Pool * >( allocator );

        result = memory_pool->allocate_debug( size, 
                                              file_name, 
                                              line_no );

        size_t total_allocated_size = 
            UPF_Virtual_Memory_Allocator::get_total_allocated_size();

        if ( total_allocated_size >= 30 * 1024 * 1024 )
        {
            int index = (((total_allocated_size & 0xFFF00000) >> 20) - 30) / 5;

            if ( index >=0 && index <= 4 )
            {
                if ( InterlockedIncrement( & (sl_print_flags[ index ]) ) == 1 )
                {

#ifdef UPF_MALLOC_DEBUG_REPORT
                    UPF_Print_Memory_Statistic_ReportEx();
#endif /* UPF_MALLOC_DEBUG_REPORT */

                }
            }
        }

		if ( 0 == result )
		{
#ifdef UPF_MALLOC_DEBUG_REPORT
            UPF_Print_Memory_Statistic_Report();            
            my_trace( "malloc failed( file_name: %s, line_no: %d, size: %d )\n", file_name, line_no, size );
#endif /* UPF_MALLOC_DEBUG_REPORT */

		}

    }

    return result;
}


/** 
 * 重新分配内存, 并保留原有的内容.
 * 
 * 当分配内存时，如果在原来的位置还有足够的内存可以分配new_size大小, 
 * 则进行就地分配, 否则新申请一块new_size大小的内存, 并拷贝原有内存的内容, 同时
 * 释放原有的内存.
 * 
 * 
 * 有如下三种情况:
 *      -# 如果ptr为0, 则分配内存.
 *      -# 如果ptr不为0, new_size为0, 则释放内存.
 *      -# 如果ptr不为0, new_size不为0, 则进行重新分配内存操作.
 * 
 * @note 如果重新分配内存失败, 原有内存并不释放, 原来的指针仍然有效.
 * 
 * @param allocator              [in]  使用UPF_New_Allocator所创建的allocator. 
 * @param ptr                    [in]  使用UPF_Malloc或UPF_Realloc所申请的内存指针.
 * @param new_size               [in]  新的内存大小. 
 * 
 * @retval 一个指向有效内存的指针        重新申请内存成功. 
 * @retval 0                             重新申请内存失败, ptr所指向的内存
 *                                       并不释放, ptr仍然有效.
 */
void * 
UPF_Realloc( UPF_Allocator_Handle allocator, void * ptr, size_t new_size )
{
    void * result = 0;

    if ( 0 != allocator  )
    {
        UPF_IMemory_Pool * memory_pool =
            reinterpret_cast< UPF_IMemory_Pool * >( allocator );

        result = memory_pool->reallocate( ptr, 
                                          new_size );
    }

    return result;
}

/** 
 * UPF_Realloc的Debug版本. 
 * 
 * @param allocator              [in] 使用UPF_New_Allocator所创建的allocator.     
 * @param ptr                    [in] 使用UPF_Malloc_Debug或UPF_Realloc_Debug
 *                                    所申请的内存指针. 
 * @param new_size               [in] 新的内存大小.
 * @param file_name              [in] 调用UPF_Realloc_Debug所在的源文件名.      
 * @param line_no                [in] 调用UPF_Realloc_Debug所在的源文件行号.    
 * 
 * @return void*
 */
void *
UPF_Realloc_Debug( UPF_Allocator_Handle allocator, 
                   void *               ptr, 
                   size_t               new_size,
                   const char *         file_name,
                   int                  line_no )

{
    void * result = 0;

    if ( 0 != allocator  )
    {
        UPF_IMemory_Pool * memory_pool =
            reinterpret_cast< UPF_IMemory_Pool * >( allocator );

        result = memory_pool->reallocate_debug( ptr, 
                                                new_size, 
                                                file_name, 
                                                line_no );
    }

    return result;

}

/** 
 * 释放由UPF_Malloc或UPF_Realloc所申请的内存, 当allocator或者ptr为0时, 不作处理.
 * 
 * @param allocator               [in] 使用UPF_New_Allocator所创建的allocator. 
 * @param ptr                     [in] 使用UPF_Malloc或UPF_Realloc所申请的内存指针.
 */
void 
UPF_Free( UPF_Allocator_Handle allocator, void * ptr )
{
    if ( 0 != allocator && 0 != ptr )
    {
        UPF_IMemory_Pool * current_memory_pool = 
            reinterpret_cast< UPF_IMemory_Pool * >( allocator );

        UPF_IMemory_Pool * original_memory_pool = 
            get_memory_pool_owner( current_memory_pool, ptr );

        if ( current_memory_pool == original_memory_pool )
        {
            current_memory_pool->deallocate( ptr, 0 );
        }
        else if ( original_memory_pool != 0 )
        {   
            original_memory_pool->deallocate( ptr, 0 );

            UPF_MALLOC_TRACE( 
                ("<<< Warning: The memory block was allocated by %s, however it was freed by %s!!! >>>\n", 
                 original_memory_pool->get_memory_pool_info()->get_memory_pool_name(),
                 current_memory_pool->get_memory_pool_info()->get_memory_pool_name()) );

        }
        else
        {
            MESSAGE_ASSERT( 
                "The memory block is invalid, couldn't found its allocator!", 0 );
        }
    }
}

/** 
 * UPF_Free的调试版本.
 * 
 * @param allocator               [in] 使用UPF_New_Allocator所创建的allocator. 
 * @param ptr                     [in] 使用UPF_Malloc或UPF_Realloc所申请的内存指针.
 * @param file_name               [in] 调用UPF_Free_Debug所在的源文件名.
 * @param line_no                 [in] 调用UPF_Free_Debug所在的源文件行号.
 * 
 * @return void
 */
void 
UPF_Free_Debug( UPF_Allocator_Handle allocator, 
                void *               ptr,
                const char *         file_name,
                int                  line_no )
{
    if ( 0 != allocator && 0 != ptr )
    {
        UPF_IMemory_Pool * current_memory_pool = 
            reinterpret_cast< UPF_IMemory_Pool * >( allocator );

        UPF_IMemory_Pool * original_memory_pool = 
            get_memory_pool_owner( current_memory_pool, ptr );

        if ( current_memory_pool == original_memory_pool )
        {
            current_memory_pool->deallocate( ptr, 0 );
        }
        else if ( original_memory_pool != 0 )
        {   
            original_memory_pool->deallocate( ptr, 0 );

            UPF_MALLOC_TRACE( 
                ("%s(%d): <<< warning: The memory block(0x%p) was allocated by %s, however it was freed by %s!!! >>>\n", 
                 file_name,
                 line_no,
                 ptr,
                 original_memory_pool->get_memory_pool_info()->get_memory_pool_name(),
                 current_memory_pool->get_memory_pool_info()->get_memory_pool_name()) );

        }
        else
        {
            UPF_MALLOC_TRACE( 
                ("%s(%d): <<< error: The memory block(0x%p) is invalid, couldn't found its allocator!!! >>>\n", 
                 file_name,
                 line_no,
                 ptr) );

            MESSAGE_ASSERT( 
                "The memory block is invalid, couldn't found its allocator!", 0 );
        }
    }
}

/**
 * 获取allocator的信息, 在调用UPF_Query_Allocator前, info_ptr所指向的结构体的内容
 * 不需要清0.
 * 
 * @param allocator        [in]  allocator handle, 应该为通过UPF_New_Allocator
 *                               创建所得的handle.
 * @param info_ptr         [out] 用于存储allocator的一些信息.
 * 
 * @retval      0    获取信息成功, info_ptr所指向的结构中包含allocator的信息.
 * @retval      -1   获取信息失败, 通常是因为allocator为0, 或者info_ptr为0, 当
 *                   失败时, info_ptr所指向的结构中的内容并不改变.
 * 
 */
int 
UPF_Query_Allocator( UPF_Allocator_Handle allocator,
                     UPF_Allocator_Info * info_ptr )
{
    int result = -1;

    if ( 0 != allocator && 0 != info_ptr )
    {
        // get memory pool info interface ptr.
        UPF_IMemory_Pool * current_memory_pool = 
            reinterpret_cast< UPF_IMemory_Pool * >( allocator );

        UPF_IMemory_Pool_Info * memory_pool_info = 
            current_memory_pool->get_memory_pool_info();

        // memory_pool_info must not be 0.
        assert( memory_pool_info != 0 );

        // get allocate size.
        info_ptr->allocated_size = memory_pool_info->get_allocated_memory_size();

        // get managed size.
        info_ptr->managed_size   = memory_pool_info->get_managed_memory_size();

        result = 0;

    }

    return result;
}


void 
UPF_Print_Memory_Statistic_Report( void )
{
    static LONG  sl_print_flag  = 0;
    
    // only do once.
    if ( 0 == sl_print_flag )
    {
        InterlockedIncrement( &sl_print_flag );
        
        if ( 1 != sl_print_flag )
        {
            return;
        }
    }
    else
    {
        return;
    }

    UPF_Print_Memory_Statistic_ReportEx();
}


void 
UPF_Print_Memory_Statistic_ReportEx( void )
{

    // print report.

    const wchar_t * MEMORY_REPORT_SUB_PATH  = L"\\UPF_Memory_Report";
    const char * MEMORY_REPORT_FILE_NAME    = "\\Memory_Statistic.txt";

    // 1. determine memory report's path and create it 
    //    if memory report's path not exist.
    wchar_t  * temp_wchar_ptr  =  0 ;
	wchar_t exe_path[MAX_PATH] = {0};
   
	GetModuleFileNameW( NULL, exe_path, MAX_PATH );
	
	temp_wchar_ptr = wcsrchr(exe_path, '\\');
	
	if(temp_wchar_ptr != NULL)
	{
		*(temp_wchar_ptr) = '\0';
	}
	
    // get memory report's sub path
    wcscat( exe_path, MEMORY_REPORT_SUB_PATH );


    // judge memory report's sub path if exists.
    WIN32_FIND_DATAW find_data;

    HANDLE hFind = FindFirstFileW( exe_path, &find_data );
    if ( INVALID_HANDLE_VALUE == hFind )
    {
        if ( ! CreateDirectoryW( exe_path, NULL ) )
        {
            UPF_Memory_Pool_Util::report( "%s create failed", exe_path );
            return;
        }
    }
    else
    {
        FindClose( hFind );
    }

    // then, convert memory report's path to ansi version.
    char * temp_report_path_ptr = (char *) exe_path;
    temp_wchar_ptr              = exe_path;

#pragma warning(push)
#pragma warning(disable: 4244)

    while ( ( *temp_report_path_ptr++ = *temp_wchar_ptr++ ) != 0 )
    {
        ;
    }

#pragma warning(pop)

    char * report_path     = (char *) exe_path;

     // set report end path for use later.
    char * report_end_path = temp_report_path_ptr - 1;

    // then, determine report file name.

	strcat( report_path, MEMORY_REPORT_FILE_NAME );

    char * report_file_name = (char *) report_path;
   

    // open file and print
    FILE * fp = fopen( report_file_name, "a+b" );

    if ( NULL == fp )
    {
        UPF_Memory_Pool_Util::report( "%s open failed", report_file_name );
        return;
    }

    // start print

    // print a delemit line.

    fprintf( fp, "\n\n" );

    // print title
    UPF_Memory_Pool_Util::print_lined_caption( fp,
                                               "Memory Statistic Report",
                                               MAX_PRINT_WIDTH,
                                               2 );

    // print current time etc.
    SYSTEMTIME current_time;

    GetLocalTime( &current_time );

    fprintf( fp, "\nTime: %04d-%02d-%02d %02d:%02d:%02d.%03d\n",
             current_time.wYear,
             current_time.wMonth,
             current_time.wDay,
             current_time.wHour,
             current_time.wMinute,
             current_time.wSecond,
             current_time.wMilliseconds );

     // reset report path end char is \0.
    *report_end_path = '\0';

    // print memory pool info.
    UPF_Print_Memory_Pool_Info( fp );

    // print global memory status.
    UPF_Print_Global_Memory_Status( fp );

    // print memory page infos.
    UPF_Print_Memory_Pages_Info( fp );

    fclose( fp );

    // print UPF's all memory pool summary infos

    UPF_Memory_Pool_Info_Manager::instance()->
        print_all_memory_pool_summary_infos( report_path );

}


void 
UPF_Print_Memory_Pool_Info( FILE * fp )
{
    // print memory pool info

    fprintf( fp, "\n\n" );
    UPF_Memory_Pool_Util::print_framed_caption( fp,
                                                "UPF Memory Pool Info",
                                                MAX_SECTION_WIDTH ,
                                                4,
                                                '+' );

    fprintf( fp, "\n" );
    UPF_Memory_Pool_Info_Manager::instance()->print_all_memory_pool_infos( 
        fp );


    // print virtual allocator info.
    
    fprintf( fp, "\n\n" );
    UPF_Memory_Pool_Util::print_framed_caption( fp,
                                                "Virtual Allocator Info",
                                                MAX_SECTION_WIDTH ,
                                                4,
                                                '+' );
    fprintf( fp, "\n" );

    UPF_Virtual_Memory_Allocator::print_memory_info( fp );
    
}


void 
UPF_Print_Global_Memory_Status( FILE * fp )
{
    fprintf( fp, "\n\n" );
    UPF_Memory_Pool_Util::print_framed_caption( fp,
                                                "Global Memory Status",
                                                MAX_SECTION_WIDTH,
                                                4,
                                                '+' );
    fprintf( fp, "\n" );

    fprintf( fp, 
             "%-15s %-15s %-15s %-15s %-15s %-15s %-15s\n",
             "MemoryLoad",
             "TotalPhys",
             "AvailPhys",
             "TotalPageFile",
             "AvailPageFile",
             "TotalVirtual",
             "AvailVirtual" );

    UPF_Memory_Pool_Util::print_chars( fp, 121, '-' );

    char buf[ 100 ];

    MEMORYSTATUS memory_status;

    GlobalMemoryStatus( &memory_status );

    fprintf( fp, "%2d%%", memory_status.dwMemoryLoad );
    fprintf( fp, "%-12s ", " " );

    fprintf( fp, "%-15s ",
             UPF_Memory_Pool_Util::num_to_string( memory_status.dwTotalPhys, buf ) );

    fprintf( fp, "%-15s ", 
             UPF_Memory_Pool_Util::num_to_string( memory_status.dwAvailPhys, buf ) );

    fprintf( fp, "%-15s ", 
             UPF_Memory_Pool_Util::num_to_string( memory_status.dwTotalPageFile, buf ) );

    fprintf( fp, "%-15s ", 
             UPF_Memory_Pool_Util::num_to_string( memory_status.dwAvailPageFile, buf ) );

    fprintf( fp, "%-15s ", 
             UPF_Memory_Pool_Util::num_to_string( memory_status.dwTotalVirtual, buf ) );

    fprintf( fp, "%-15s\n", 
             UPF_Memory_Pool_Util::num_to_string( memory_status.dwAvailVirtual, buf ) );

}


#ifdef UPF_OS_IS_WINCE
    extern "C" __declspec(dllimport) DWORD SetProcPermissions(
            DWORD newperms );
#endif

void 
UPF_Print_Memory_Pages_Info( FILE * fp )
{
    assert( fp );

#ifdef UPF_OS_IS_WINCE
    SetProcPermissions( 0xFFFFFFFF );
#endif

    BOOL fOk = TRUE;

#ifdef UPF_OS_IS_WINCE
    SetProcPermissions( 0xFFFFFFFF );
    PVOID pvAddress = (PVOID) 0x10000;
#else
    PVOID pvAddress = NULL;
#endif
    
    fprintf( fp, "\n\n" );
    UPF_Memory_Pool_Util::print_framed_caption( fp,
                                                "Memory Pages Info",
                                                86,
                                                4,
                                                '+' );
    fprintf( fp, "\n" );

    fprintf( fp, 
             "%-13s %-18s %-9s %-7s %-12s %-10s\n",
             "BaseAddress",
             "State",
             "Size",
             "Blocks",
             "Protect",
             "Comment" );

    UPF_Memory_Pool_Util::print_chars( fp, 85, '-' );

    while (fOk) 
    {      
        VMQUERY vmq;
        fOk = VMQuery( pvAddress, &vmq );
        
        if (fOk) 
        {
            // Construct the line to be displayed, and add it to the list box.
            char szLine[1024];

            ConstructRgnInfoLine( &vmq, szLine, sizeof(szLine) );

            fprintf( fp, "%s\n", szLine );
                
            for ( DWORD dwBlock = 0;
                  fOk && (dwBlock < vmq.dwRgnBlocks);
                  dwBlock++ ) 
            {
                    
                ConstructBlkInfoLine(&vmq, szLine, sizeof(szLine));
                fprintf( fp, "%s\n", szLine );
                    
                // Get the address of the next region to test.
                pvAddress = ((PBYTE) pvAddress + vmq.BlkSize);
                if (dwBlock < vmq.dwRgnBlocks - 1) 
                {
                    // Don't query the memory info after the last block.
                    fOk = VMQuery(pvAddress, &vmq);
                }

            }
           
            // Get the address of the next region to test.
            pvAddress = ((PBYTE) vmq.pvRgnBaseAddress + vmq.RgnSize);

            if ( (UINT) pvAddress >= 0x2000000 )
            {
                break;
            }
        }
    }

}


UPF_Allocator_Handle 
UPF_Get_Allocator_By_Name( const char * allocator_name )
{
    UPF_IMemory_Pool * memory_pool = 
        UPF_Memory_Pool_Factory::get_memory_pool( allocator_name );

    UPF_Allocator_Handle result = 
        reinterpret_cast< UPF_Allocator_Handle >( memory_pool );

    return result;
}


void 
UPF_Delete_Allocator_By_Name( const char * allocator_name )
{
    if ( 0 != allocator_name )
    {
        UPF_Memory_Pool_Factory::delete_memory_pool( allocator_name );  
    }
}


int 
UPF_Set_Toplevel_Allocator_Quota( toplevel_allocator_t  toplevel_allocator_type,
                                  int                   quota_value )
{
    // not use UPF_Set_Toplevel_Allocator_Quota temporarily.
    return 0;


    /*if ( toplevel_allocator_type == UPF_TOPLEVEL_DISCARDABLE_ALLOCATOR )
    {
        return 0;
    }
    else
    {
        bool result = ( UPF_Virtual_Memory_Allocator::set_managed_memory_size(
                            toplevel_allocator_type,
                            quota_value * 1024 * 1024 ) );

        return ( result ? 0 : -1 );
    }*/
}


int 
UPF_Reclaim_Toplevel_Allocator( toplevel_allocator_t  toplevel_allocator_type )
{
    // not use UPF_TOPLEVEL_DISCARDABLE_ALLOCATOR temporarily.
    if ( toplevel_allocator_type == UPF_TOPLEVEL_DISCARDABLE_ALLOCATOR )
    {
        return 0;
    }
    else
    {
    #ifdef MEMORY_POOL_EXISTS_CHECK_WHEN_RECLAIM

        char failed_msg[ 100 ];

        sprintf( failed_msg, 
                 "[%d] reclaim memory failed!",
                 toplevel_allocator_type );

        if ( UPF_Memory_Pool_Info_Manager::instance()->is_exist_memory_pool(
                toplevel_allocator_type,
                true,
                failed_msg ) )
        {
            DebugBreak();
        }

    #endif /* MEMORY_POOL_EXISTS_CHECK_WHEN_RECLAIM */

        bool result = ( UPF_Virtual_Memory_Allocator::reclaim( 
                            toplevel_allocator_type ) );

        return ( result ? 0 : -1 );
    }
}


int 
UPF_Restore_Toplevel_Allocator( toplevel_allocator_t  toplevel_allocator_type )
{
    // not use UPF_TOPLEVEL_DISCARDABLE_ALLOCATOR temporarily.
    if ( toplevel_allocator_type == UPF_TOPLEVEL_DISCARDABLE_ALLOCATOR )
    {
        return 0;
    }
    else
    {
        bool result = ( UPF_Virtual_Memory_Allocator::restore(
                            toplevel_allocator_type ) );

        return ( result ? 0 : -1 );
    }
}


UPF_Trans_Allocator_Handle 
UPF_New_Trans_Allocator_By_Name( const char *         allocator_name,
                                 size_t               max_needed_memory_size )
{

    toplevel_allocator_t    allocator_type;

    // first get allocator type.
    if ( false == 
         UPF_Virtual_Memory_Allocator::get_dynamic_toplevel_allocator(
              allocator_type ) 
       )
    {
        return 0;
    }

    // calculate actual needed memory size by bytes.
    size_t max_needed_memory_size_by_bytes =
        max_needed_memory_size * 1024 * 1024;

    // set virtual memory allocator's managed memory size.
    if ( false == UPF_Virtual_Memory_Allocator::set_managed_memory_size(
            allocator_type,
            max_needed_memory_size_by_bytes ) )
    {
        // if set failed, release allocator type.
        UPF_Virtual_Memory_Allocator::release_dynamic_toplevel_allocator(
            allocator_type );
         
        return 0;
    }

    UPF_IMemory_Pool * memory_pool = 
        UPF_Memory_Pool_Factory::create_trans_memory_pool( 
            allocator_name, 
            allocator_type,
            max_needed_memory_size_by_bytes );

    if ( 0 == memory_pool )
    {
        // if create memory pool failed, then release allocator type.
        
        UPF_Virtual_Memory_Allocator::release_dynamic_toplevel_allocator(
            allocator_type );
         
        return 0;
    }

    UPF_Trans_Allocator_Handle result = 
        reinterpret_cast< UPF_Trans_Allocator_Handle >( memory_pool );

    return result;
}


void
UPF_Delete_Trans_Allocator( UPF_Trans_Allocator_Handle allocator )
{
    if ( 0 != allocator )
    {
        UPF_IMemory_Pool * memory_pool =
             reinterpret_cast< UPF_IMemory_Pool * >( allocator );        

        // save allocator_type.
        toplevel_allocator_t   allocator_type =
            memory_pool->get_memory_pool_info()->get_allocator_type();

        // must first delete memory pool
        delete memory_pool;

        // release toplevel allocator at last.
        UPF_Virtual_Memory_Allocator::release_dynamic_toplevel_allocator(
             allocator_type );
       
    }
}


void * 
UPF_Trans_Malloc( UPF_Trans_Allocator_Handle allocator, size_t size )
{
    void * result = 0;

    if ( 0 != allocator )
    {
        UPF_IMemory_Pool * memory_pool =
            reinterpret_cast< UPF_IMemory_Pool * >( allocator );

        result = memory_pool->allocate( size );
    }

    return result;
}


void 
UPF_Trans_Free( UPF_Trans_Allocator_Handle allocator, void * ptr )
{
    if ( 0 != allocator && 0 != ptr )
    {
        UPF_IMemory_Pool * memory_pool = 
            reinterpret_cast< UPF_IMemory_Pool * >( allocator );

        memory_pool->deallocate( ptr, 0 );
    }            
}
