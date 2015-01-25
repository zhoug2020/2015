/*----------------Copyright(C) 2008 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Memory_Pool_ID_Manager.cpp                                                             *
*       CREATE DATE     : 2008-7-21                                                                                  *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]一个ID管理器, 用于生成唯一ID值![CN]                                                    *
*********************************************************************************************************************/
#include "UPF_Memory_Pool_ID_Manager.h"

/////////////////////////////////////////////////////////////////////////
// static data definitions.
std::bitset< UPF_Memory_Pool_ID_Manager::MAX_ID_VALUE >   
UPF_Memory_Pool_ID_Manager::sm_id_generator;

UPF_Thread_Mutex              UPF_Memory_Pool_ID_Manager::sm_lock;

/////////////////////////////////////////////////////////////////////////

size_t 
UPF_Memory_Pool_ID_Manager::get_id( void )
{
    UPF_Guard< UPF_Thread_Mutex > guard( sm_lock );

    size_t index; 
    
    for ( index = 0; index < sm_id_generator.size(); ++index )
    {
        if ( ! sm_id_generator[ index ] )
        {
            sm_id_generator[ index ] = true;

            break;
        }
    }

    if ( index >= sm_id_generator.size() )
    {
        return ( INVALID_ID );
    }

    return ( index );
}

/////////////////////////////////////////////////////////////////////////

void 
UPF_Memory_Pool_ID_Manager::release_id( size_t id )
{
    UPF_Guard< UPF_Thread_Mutex > guard( sm_lock );

    if ( id >= 0 && id < sm_id_generator.size() )
    {
        sm_id_generator[ id ] = false;
    }
}
