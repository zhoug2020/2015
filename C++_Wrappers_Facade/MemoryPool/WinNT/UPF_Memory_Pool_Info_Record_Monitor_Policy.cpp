/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Memory_Pool_Info_Record_Monitor_Policy.cpp                                             *
*       CREATE DATE     : 2007-11-20                                                                                 *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]内存池信息监视策略的一些依赖于WINNT平台的实现![CN]                                     *
*********************************************************************************************************************/

/**
 * @file UPF_Memory_Pool_Info_Record_Monitor_Policy.cpp
 * 
 * @details 内存池信息监视策略的一些依赖于WINNT平台的实现
 */

/** 
 * 启动监视程序.
 * 首先如果查找到监视程序主窗口句柄, 则返回该句柄, 否则启动监视程序, 重新查找
 * 主窗口句柄.
 *  
 * @param monitor_window_handle       [out]   监视程序的主窗口句柄.
 * 
 * @retval true  启动监视程序成功, monitor_window_handle为监视程序的主窗口句柄.
 *         false 启动监视程序失败, monitor_window_handle为NULL.
 */
static bool
startup_monitor_app( HWND & monitor_window_handle )
{
    monitor_window_handle = ::FindWindow( NULL, MONITOR_APP_WINDOW_TITLE );
    
    if ( NULL == monitor_window_handle )
    {                             
        STARTUPINFO         si;
        PROCESS_INFORMATION pi;
    
        ::ZeroMemory( &si, sizeof( si ) );
        si.cb = sizeof( si );
    
        ::ZeroMemory( &pi, sizeof( pi ) );
    
        ::GetStartupInfo( &si );
    
        if ( ::CreateProcess( NULL, 
                              MONITOR_APP_NAME,
                              NULL,
                              NULL,
                              FALSE,
                              NORMAL_PRIORITY_CLASS,
                              NULL,
                              NULL,
                              &si,
                              &pi ) == FALSE )
        {
            return false;
        }
    
        ::WaitForInputIdle( pi.hProcess, INFINITE );                  
    
        ::CloseHandle( pi.hProcess );
        ::CloseHandle( pi.hThread );
    
        monitor_window_handle = ::FindWindow( NULL, MONITOR_APP_WINDOW_TITLE );
    
        if ( NULL == monitor_window_handle )
        {
            return false;
        }
    }

    return true;
}