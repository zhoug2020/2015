/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Memory_Pool_Info_Record_Monitor_Policy.h                                               *
*       CREATE DATE     : 2007-11-20                                                                                 *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]�����ڴ����Ϣ��¼, ����¼�ڴ����Ϣʱ, �����Ӧ�����ݷ��͸�һ�����ӳ����ɼ��ӳ���   *
*                             ��ʵʱ��ʾ����, ������win32 platform.                                                  *
*                         ![CN]                                                                                      *
*********************************************************************************************************************/

#ifndef UPF_Memory_Pool_Info_Record_Monitor_Policy_H
#define UPF_Memory_Pool_Info_Record_Monitor_Policy_H

#include "Memory_Pool_Info_Record_Policy.h"
#include "UPF_Memory_Pool_Define.h"

#include <stddef.h>            /* for size_t */
#include <windows.h>           /* for COPYDATASTRUCT */
#include <memory.h>            /* for memset */

/** 
 * @class UPF_Memory_Pool_Info_Record_CDMsg_Monitor_Policy
 * 
 * @brief һ���ڴ����Ϣ������, ͨ������WM_COPYDATA��Ϣ��һ��
 *        UPF_Memory_Pool_Monitor.exe�ĳ���, �������������ʾ����.
 * 
 */
class UPF_Memory_Pool_Info_Record_CDMsg_Monitor_Policy 
    : public UPF_Memory_Pool_Info_Record_Policy
{
public:
    typedef UPF_Memory_Pool_Info_Record_Policy Base_Class;

public:
    UPF_Memory_Pool_Info_Record_CDMsg_Monitor_Policy();

    void set_memory_pool_name( const char * memory_pool_name );
    

    void set_managed_memory_size( size_t size );

    void set_allocated_memory_size( size_t size );    

    void set_wasted_memory_size( size_t size );

private:

    enum Field_Type {
        MANAGED_MEMORY_SIZE   = 0x1,
        ALLOCATED_MEMORY_SIZE = 0x2,
        ALLOCATED_MEMORY_PEAK = 0x4,
        WASTED_MEMORY_SIZE    = 0x8
    };

    struct Memory_Pool_Info_Data
    {
       size_t memory_size;
       char   memory_pool_name[ MAX_POOL_NAME_LENGTH + 1 ];
    };

private:
    /// �����ڴ����Ϣ�����ӳ���
    static void send_memory_info( COPYDATASTRUCT & copy_data );

private:

    /// ���÷��͵���Ϣ
    void set_send_info( DWORD  send_flag,
                        size_t size )
    {
        m_copy_data.dwData      = send_flag;
        m_info_data.memory_size = size;
    }

private:
    /// ���Ӵ��ھ��
    static HWND sm_monitor_window;

    /// ��ż��ӳ����Ƿ����, ȱʡΪTRUE
    static BOOL sm_monitor_app_existed;

private:
    /// ���͸����ӳ����copy data
    COPYDATASTRUCT         m_copy_data;

    /// Ҫ���͸����ӳ�����ڴ����Ϣ
    Memory_Pool_Info_Data  m_info_data;

    /// ������һ�ε��ڴ��ֵ.
    size_t                 m_last_memory_peak;

};

/** 
 * @class UPF_Memory_Pool_Info_Record_MMap_Monitor_Policy
 * 
 * @brief һ���ڴ����Ϣ������, ͨ���ڴ�ӳ����ϵͳ�п���һ�鹲���ڴ���, ����¼
 *        �ڴ�صķ�������ʱ, ����������ڴ���д����.
 *        �����и�UPF_Memory_Pool_Monitor.exe�ĳ���, ��ʱ��ȥ�����ڴ���ȡ���ݲ���
 *        ��ʾ�ڴ�����.
 * 
 */
class /*UPF_Export*/ UPF_Memory_Pool_Info_Record_MMap_Monitor_Policy 
    : public UPF_Memory_Pool_Info_Record_Policy
{
public:
    typedef UPF_Memory_Pool_Info_Record_Policy Base_Class;

public:
    /// ���캯��, ���ڴ�ӳ�����.
    UPF_Memory_Pool_Info_Record_MMap_Monitor_Policy( void );

    /// ��������, �ر��ڴ�ӳ�����.
    ~UPF_Memory_Pool_Info_Record_MMap_Monitor_Policy( void );

    void set_memory_pool_name( const char * memory_pool_name );
    

    void set_managed_memory_size( size_t size );

    void set_allocated_memory_size( size_t size );    

    void set_wasted_memory_size( size_t size );

    void set_allocated_memory_limit( size_t size );

private:

    struct Memory_Pool_MMap_Data
    {       
        Memory_Pool_MMap_Data()
        {
            ::memset( m_memory_pool_name, 0, sizeof( m_memory_pool_name ) );

            m_managed_memory_size    = 0;
            m_allocated_memory_size  = 0;
            m_wasted_memory_size     = 0;
            m_memory_peak_size       = 0;
            m_allocated_memory_limit = 0;
        }

        char   m_memory_pool_name[ MAX_POOL_NAME_LENGTH + 1 ];

        size_t m_managed_memory_size;
        size_t m_allocated_memory_size;
        size_t m_wasted_memory_size;
        size_t m_memory_peak_size;
        size_t m_allocated_memory_limit;
    };

    /// ���ڷ��͸����Ӵ��ڵ���Ϣ, ��Ϣ��WPARAM����Ϊm_mmap_id, ���ӳ������
    /// ���id, ���Եõ��ڴ�ӳ����������, �Ӷ����Ի�ȡ�����ڴ�����.
    enum MMap_Window_Message
    {
        WM_UPF_MMAP_ADD    = WM_USER + 111,    /**< ���ڴ�ӳ����󴴽�ʱ���͵���Ϣ. */
        WM_UPF_MMAP_REMOVE = WM_USER + 112     /**< ���ڴ�ӳ�����ر�ʱ���͵���Ϣ. */
    };

private:
    /// ���Ӵ��ھ��
    static HWND sm_monitor_window;

    /// ��ż��ӳ����Ƿ����, ȱʡΪTRUE
    static BOOL sm_monitor_app_existed;

private:
    /// ȡ���ڴ�ӳ�����ID, �ڴ�ӳ�������������ڴ�ӳ���������ǰ׺+�ڴ�ӳ�����ID���.
    static long get_mmap_id( void )
    {
        static volatile long sm_mmap_id;
        return ::InterlockedIncrement( const_cast< long * >( &sm_mmap_id ) );
    }

    
private:
    /// ָ�����ڴ��ָ��.
    Memory_Pool_MMap_Data *  m_mmap_data;

    /// ��Memory_Pool_MMap_Data���Ӧ���ڴ�ӳ�����ľ��.
    HANDLE                   m_mmap_data_handle;

    /// ���ڴ�ŵ�ǰ�ڴ�ӳ������Ӧ��ID.
    long                     m_mmap_id;

    /// ������һ�ε��ڴ��ֵ.
    size_t                   m_last_memory_peak;
};
#endif /* UPF_Memory_Pool_Info_Record_Monitor_Policy_H */

