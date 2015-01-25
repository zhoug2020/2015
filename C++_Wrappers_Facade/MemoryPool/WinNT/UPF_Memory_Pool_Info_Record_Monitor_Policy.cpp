/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Memory_Pool_Info_Record_Monitor_Policy.cpp                                             *
*       CREATE DATE     : 2007-11-20                                                                                 *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]�ڴ����Ϣ���Ӳ��Ե�һЩ������WINNTƽ̨��ʵ��![CN]                                     *
*********************************************************************************************************************/

/**
 * @file UPF_Memory_Pool_Info_Record_Monitor_Policy.cpp
 * 
 * @details �ڴ����Ϣ���Ӳ��Ե�һЩ������WINNTƽ̨��ʵ��
 */

/** 
 * �������ӳ���.
 * ����������ҵ����ӳ��������ھ��, �򷵻ظþ��, �����������ӳ���, ���²���
 * �����ھ��.
 *  
 * @param monitor_window_handle       [out]   ���ӳ���������ھ��.
 * 
 * @retval true  �������ӳ���ɹ�, monitor_window_handleΪ���ӳ���������ھ��.
 *         false �������ӳ���ʧ��, monitor_window_handleΪNULL.
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