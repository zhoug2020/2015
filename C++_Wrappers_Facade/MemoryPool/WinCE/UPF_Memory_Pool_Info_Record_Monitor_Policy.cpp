/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Memory_Pool_Info_Record_Monitor_Policy.cpp                                             *
*       CREATE DATE     : 2007-11-20                                                                                 *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]内存池信息监视策略的一些依赖于WINCE平台的实现![CN]                                     *
*********************************************************************************************************************/

/**
 * @file UPF_Memory_Pool_Info_Record_Monitor_Policy.cpp
 * 
 * @details 内存池信息监视策略的一些依赖于WINCE平台的实现
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
    const int SLEEP_MILLI_SECONDS = 2000;

    monitor_window_handle = ::FindWindow( NULL, UPF_TEXT(MONITOR_APP_WINDOW_TITLE) );
    
    if ( NULL == monitor_window_handle )
    {                             
        PROCESS_INFORMATION pi;
       
        ::ZeroMemory( &pi, sizeof( pi ) );
      
        if ( ::CreateProcess( NULL, 
                              UPF_TEXT( MONITOR_APP_NAME ),
                              NULL,
                              NULL,
                              FALSE,
                              0,
                              NULL,
                              NULL,
                              NULL,
                              &pi ) == FALSE )
        {
            return false;
        }
    
        // sleep some time for waiting for monitor app to be started.
        Sleep( SLEEP_MILLI_SECONDS );                 
    
        ::CloseHandle( pi.hProcess );
        ::CloseHandle( pi.hThread );
    
        monitor_window_handle = ::FindWindow( NULL, 
                                              UPF_TEXT(MONITOR_APP_WINDOW_TITLE) );
    
        if ( NULL == monitor_window_handle )
        {
            return false;
        }
    }

    return true;
}