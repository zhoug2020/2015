/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Memory_Pool_Info_Record_Monitor_Policy.cpp                                             *
*       CREATE DATE     : 2007-11-20                                                                                 *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]�ڴ����Ϣ���Ӳ��Ե�һЩ������WINCEƽ̨��ʵ��![CN]                                     *
*********************************************************************************************************************/

/**
 * @file UPF_Memory_Pool_Info_Record_Monitor_Policy.cpp
 * 
 * @details �ڴ����Ϣ���Ӳ��Ե�һЩ������WINCEƽ̨��ʵ��
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