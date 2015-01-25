/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Memory_Pool_Info_Record_Monitor_Policy.cpp                                             *
*       CREATE DATE     : 2007-11-20                                                                                 *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]�ڴ����Ϣ���Ӳ��Ե�ʵ��![CN]                                                          *
*********************************************************************************************************************/

#include "UPF_Memory_Pool_Info_Record_Monitor_Policy.h"
#include "UPF_Memory_Pool_Util.h"

#include <string.h>             /* for strncpy */
#include <stdio.h>              /* for _snprintf, _vsnprintf, BUFSIZ */
#include <new>                  /* for placement new */
#include <stdarg.h>             /* for va_list */

// Static data definitions
HWND UPF_Memory_Pool_Info_Record_CDMsg_Monitor_Policy::sm_monitor_window = NULL;

BOOL UPF_Memory_Pool_Info_Record_CDMsg_Monitor_Policy::sm_monitor_app_existed = TRUE;

HWND UPF_Memory_Pool_Info_Record_MMap_Monitor_Policy::sm_monitor_window = NULL;

BOOL UPF_Memory_Pool_Info_Record_MMap_Monitor_Policy::sm_monitor_app_existed = TRUE;


// Constant definitions

#define MONITOR_APP_NAME            "UPF_Memory_Pool_Monitor.exe"

#define MONITOR_APP_WINDOW_TITLE    "UPF Memory Pool Monitor"

// �ڴ�ӳ���������ǰ׺.
#define MMAP_NAME_PREFIX            "UPF_MMAP_@#$%"


// Help function definitions.

#if defined(UPF_OS_IS_WINNT)
/* WINNT Platform */
#  include "WinNT/UPF_Memory_Pool_Info_Record_Monitor_Policy.cpp"
#elif defined(UPF_OS_IS_WINCE)
/* WINCE Platform */
#  include "WinCE/UPF_Memory_Pool_Info_Record_Monitor_Policy.cpp"
#endif 

/********** UPF_Memory_Pool_Info_Record_Monitor_Policy implementation **********/

/** 
 * ���캯�������ڳ�ʼ���ڲ�״̬
 * 
 */
UPF_Memory_Pool_Info_Record_CDMsg_Monitor_Policy::
UPF_Memory_Pool_Info_Record_CDMsg_Monitor_Policy()
{
    // initialize copy data
    m_copy_data.cbData = sizeof( m_info_data );
    m_copy_data.lpData = &m_info_data;

    m_last_memory_peak = 0;
}

/** 
 * �����ڴ�ص�����
 * 
 * @param memory_pool_name   [in]   �ڴ�ص�����
 * 
 */
void 
UPF_Memory_Pool_Info_Record_CDMsg_Monitor_Policy::set_memory_pool_name(
     const char * memory_pool_name )
{
    // set info data
    strncpy( m_info_data.memory_pool_name, 
             memory_pool_name, 
             MAX_POOL_NAME_LENGTH );

    m_info_data.memory_pool_name[ MAX_POOL_NAME_LENGTH ] = '\0';

}

/** 
 * �����ڴ�ع�����ڴ�Ĵ�С��Ϣ�����ӳ���
 * 
 * @param size        [in]   �ڴ�ع�����ڴ�Ĵ�С
 */
void 
UPF_Memory_Pool_Info_Record_CDMsg_Monitor_Policy::set_managed_memory_size( 
    size_t size )
{
    Base_Class::set_managed_memory_size( size );

    set_send_info( MANAGED_MEMORY_SIZE, size );

    UPF_Memory_Pool_Info_Record_CDMsg_Monitor_Policy::send_memory_info( m_copy_data );
}

/** 
 * �����ڴ�ط����Client�˵��ڴ��С��Ϣ�����ӳ���
 * 
 * @param size       [in]  �ڴ�ط����Client�˵��ڴ��С
 */
void 
UPF_Memory_Pool_Info_Record_CDMsg_Monitor_Policy::set_allocated_memory_size(
     size_t size )
{
    Base_Class::set_allocated_memory_size( size );

    set_send_info( ALLOCATED_MEMORY_SIZE, size );

    UPF_Memory_Pool_Info_Record_CDMsg_Monitor_Policy::send_memory_info( m_copy_data );

    // if memory peak already changed, then send memory peak size info.
    size_t memory_peak_size = Base_Class::get_allocated_memory_peak();

    if ( m_last_memory_peak < memory_peak_size )
    {
        m_last_memory_peak = memory_peak_size;

        set_send_info( ALLOCATED_MEMORY_PEAK, m_last_memory_peak );

        UPF_Memory_Pool_Info_Record_CDMsg_Monitor_Policy::send_memory_info( m_copy_data );
    }
}

/** 
 * �����ڴ�����˷ѵ��ڴ��С��Ϣ�����ӳ���
 * 
 * @param size       [in]  �ڴ�����˷ѵ��ڴ��С
 */
void 
UPF_Memory_Pool_Info_Record_CDMsg_Monitor_Policy::set_wasted_memory_size( 
    size_t size )
{
    Base_Class::set_wasted_memory_size( size );

    set_send_info( WASTED_MEMORY_SIZE, size );

    UPF_Memory_Pool_Info_Record_CDMsg_Monitor_Policy::send_memory_info( m_copy_data );
}

/** 
 *  �����ڴ����Ϣ�����ӳ���
 *  
 *  ����һ�η�����Ϣʱ����Ϊ���ӳ�������Ѿ�����, �������Ȳ��Ҽ��ӳ����������,
 *  ������Ҳ���, ���������ӳ���UPF_Memory_Pool_Monitor.exe), ��������ɹ�
 *  ����������ھ�������һ��˳���Ļ�������WM_COPYDATA, ���ʧ�ܣ�������
 *  sm_monitor_app_existedΪFALSE, �Ա�ʾ���ӳ��򲻴��ڣ������Ժ�Ҳ���ڲ���.
 * 
 * @param copy_data          [in]   Ҫ���͵�WM_COPYDATA��Ϣ����
 */
void
UPF_Memory_Pool_Info_Record_CDMsg_Monitor_Policy::send_memory_info( 
    COPYDATASTRUCT & copy_data )
{
    if ( TRUE == sm_monitor_app_existed )
    {
        if ( ( NULL == sm_monitor_window || 
              ::IsWindow( sm_monitor_window ) == FALSE) )
        {
            if ( startup_monitor_app( sm_monitor_window ) == false )
            {
                sm_monitor_app_existed = FALSE;
                return;
            }
        }

        SendMessage( sm_monitor_window, WM_COPYDATA, 0, (LPARAM) &copy_data );
    }
}


/********** UPF_Memory_Pool_Info_Record_MMap_Monitor_Policy implementation **********/

/** 
 * ���캯��, ���ڴ�ӳ�����. ������ӳ���û������, ���������ӳ���.
 * 
 */
UPF_Memory_Pool_Info_Record_MMap_Monitor_Policy::
UPF_Memory_Pool_Info_Record_MMap_Monitor_Policy( void )
{
    m_mmap_id          = 0;
    m_mmap_data_handle = NULL;
    m_mmap_data        = NULL;
    m_last_memory_peak = 0;

    if ( TRUE == sm_monitor_app_existed )
    {
        if ( ( NULL == sm_monitor_window || 
              ::IsWindow( sm_monitor_window ) == FALSE) )
        {
            if ( startup_monitor_app( sm_monitor_window ) == false )
            {
                sm_monitor_app_existed = FALSE;
                return;
            }
        }

        // construct a memory map object name
        char mmap_name[ 100 ];

        m_mmap_id = UPF_Memory_Pool_Info_Record_MMap_Monitor_Policy::get_mmap_id();

        _snprintf( mmap_name, sizeof( mmap_name ) - 1, "%s_%d",
                   MMAP_NAME_PREFIX,
                   m_mmap_id );

        mmap_name[ sizeof( mmap_name ) - 1 ] = '\0';

        m_mmap_data_handle = ::CreateFileMapping( INVALID_HANDLE_VALUE, 
                                                  NULL, 
                                                  PAGE_READWRITE,
                                                  0,
                                                  sizeof( Memory_Pool_MMap_Data ),
                                                  UPF_A2T(mmap_name) );

        if ( NULL == m_mmap_data_handle )
        {
            UPF_Memory_Pool_Util::display_os_error( "Create File Mapping Object (%s) failed", mmap_name );
        }
        else
        {
            m_mmap_data = static_cast< Memory_Pool_MMap_Data * >(
                ::MapViewOfFile( m_mmap_data_handle, FILE_MAP_ALL_ACCESS, 0, 0, 0 ) );

            if ( NULL ==  m_mmap_data )
            {
                UPF_Memory_Pool_Util::display_os_error( "Map file view (%s) failed", mmap_name );

                ::CloseHandle( m_mmap_data_handle );

                m_mmap_data_handle = NULL;
            }
            else
            {
                new (m_mmap_data) Memory_Pool_MMap_Data;
            }
        }

    }    
}

/** 
 * ��������, �ر��ڴ�ӳ�����, �������ӳ��������ڷ���WM_UPF_MMAP_REMOVE��Ϣ.
 * WM_UPF_MMAP_REMOVE��Ϣ��WPARAM����Ϊm_mmap_id.
 * 
 */
UPF_Memory_Pool_Info_Record_MMap_Monitor_Policy::
~UPF_Memory_Pool_Info_Record_MMap_Monitor_Policy( void )
{
    if ( m_mmap_data != NULL )
    {
        // send memory mapping object remove message.
        if ( sm_monitor_window != NULL && ::IsWindow( sm_monitor_window ) )
        {
            ::SendMessage( sm_monitor_window, WM_UPF_MMAP_REMOVE, m_mmap_id, 0 );
        }

        ::UnmapViewOfFile( m_mmap_data );

        ::CloseHandle( m_mmap_data_handle );

        m_mmap_data_handle = NULL;
        m_mmap_data        = NULL;
    }
}

/** 
 * �����ڴ�ص�����, �������ӳ��������ڷ���WM_UPF_MMAP_ADD��Ϣ.
 * WM_UPF_MMAP_ADD��Ϣ��WPARAM����Ϊm_mmap_id, ��Ϊ��ʱ���Ѿ����ڴ�ص�������
 * ������ʱ����WM_UPF_MMAP_ADD��Ϣ���ʺϵ�, ���ӳ����յ���Ϣ��, ��ȡ���ڴ�ӳ�����
 * �ľ��, Ȼ��ˢ���ڴ�ط�������.
 * 
 * @param memory_pool_name   [in]   �ڴ�ص�����
 * 
 */
void 
UPF_Memory_Pool_Info_Record_MMap_Monitor_Policy::set_memory_pool_name( 
    const char * memory_pool_name )
{
    if ( m_mmap_data )
    {
        strncpy( m_mmap_data->m_memory_pool_name, 
                 memory_pool_name, 
                 MAX_POOL_NAME_LENGTH );

        m_mmap_data->m_memory_pool_name[ MAX_POOL_NAME_LENGTH ] = '\0';

        // send memory mapping object add message
        if ( sm_monitor_window != NULL && ::IsWindow( sm_monitor_window ) )
        {
            ::SendMessage( sm_monitor_window, WM_UPF_MMAP_ADD, m_mmap_id, 0 );
        }
    }
}

/** 
 * �����ڴ�ع�����ڴ�Ĵ�С��Ϣ
 * 
 * @param size        [in]   �ڴ�ع�����ڴ�Ĵ�С
 * 
 * @param size
 */
void 
UPF_Memory_Pool_Info_Record_MMap_Monitor_Policy::set_managed_memory_size( 
    size_t size )
{
    Base_Class::set_managed_memory_size( size );

    if ( m_mmap_data != NULL  )
    {
        m_mmap_data->m_managed_memory_size = size;
    }
}

/** 
 * �����ڴ�ط����Client�˵��ڴ��С��Ϣ
 * 
 * @param size       [in]   �ڴ�ط����Client�˵��ڴ��С
 */
void 
UPF_Memory_Pool_Info_Record_MMap_Monitor_Policy::set_allocated_memory_size(
     size_t size )    
{
    Base_Class::set_allocated_memory_size( size );

    if ( m_mmap_data != NULL  )
    {
        m_mmap_data->m_allocated_memory_size = size;
        m_mmap_data->m_memory_peak_size = Base_Class::get_allocated_memory_peak();
    }
}

/** 
 * �����ڴ�����˷ѵ��ڴ��С��Ϣ
 * 
 * @param size       [in]    �ڴ�����˷ѵ��ڴ��С
 */
void 
UPF_Memory_Pool_Info_Record_MMap_Monitor_Policy::set_wasted_memory_size( 
    size_t size )
{
    Base_Class::set_wasted_memory_size( size );

    if ( m_mmap_data != NULL  )
    {
        m_mmap_data->m_wasted_memory_size = size;
    }
}

/** 
 * �����ڴ���з����ڴ������ֵ.
 * 
 * @param size       [in]   �ڴ���з����ڴ������ֵ
 */
void 
UPF_Memory_Pool_Info_Record_MMap_Monitor_Policy::set_allocated_memory_limit( 
    size_t size )
{
    Base_Class::set_allocated_memory_limit( size );

    if ( m_mmap_data != NULL )
    {
        m_mmap_data->m_allocated_memory_limit = size;
    }
}
