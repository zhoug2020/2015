/*----------------Copyright(C) 2008 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Global_Memory_Pool_Creator.cpp                                                         *
*       CREATE DATE     : 2008-7-23                                                                                  *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]用于提供Global Memory Pool的创建者![CN]                                                *
*********************************************************************************************************************/
#include "UPF_Global_Memory_Pool_Creator.h"

#define UPF_TEMPLATES_REQUIRE_SOURCE

#include "UPF_Common_Global_Memory_Pool_T.h"
#include "UPF_Memory_Pool_Components.h"

//////////////////////////////////////////////////////////////////////
// type definitions

// common global memory pool 
typedef UPF_Common_Global_Memory_Pool_T< UPF_Memory_Pool_Alloc_Type,
                                         UPF_Memory_Pool_Lock_Type,
                                         UPF_Memory_Pool_Record_Policy_Type >
        UPF_Common_Global_Memory_Pool;

//////////////////////////////////////////////////////////////////////
// static data definitions

UPF_Thread_Mutex 
UPF_Global_Memory_Pool_Creator::sm_lock;

size_t
UPF_Global_Memory_Pool_Creator::sm_object_ref_count = 0;

UPF_IGlobal_Memory_Pool *
UPF_Global_Memory_Pool_Creator::sm_object_ptr = 0;

////////////////////////////////////////////////////////////////////////

UPF_IGlobal_Memory_Pool * 
UPF_Global_Memory_Pool_Creator::get_instance( void )
{
    UPF_Guard< UPF_Thread_Mutex > guard( sm_lock );

    if ( 0 == sm_object_ptr )
    {
        sm_object_ptr = new UPF_Common_Global_Memory_Pool;
    }

    ++sm_object_ref_count;

    return sm_object_ptr;
}


void
UPF_Global_Memory_Pool_Creator::release_instance( void )
{
    UPF_Guard< UPF_Thread_Mutex > guard( sm_lock );

    if ( sm_object_ref_count > 0 && --sm_object_ref_count == 0 )
    {
        delete sm_object_ptr;

        sm_object_ptr = 0;
    }
}

