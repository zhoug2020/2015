/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Memory_Pool_Components.h                                                               *
*       CREATE DATE     : 2007-11-21                                                                                 *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]包含UPF Memory Pool的所有组件的定义![CN]                                               *
*********************************************************************************************************************/

#ifndef UPF_MEMORY_POOL_COMPONENTS_H
#define UPF_MEMORY_POOL_COMPONENTS_H

#if ! defined(UPF_MEMORY_POOL_DEFINE_H)
# include "UPF_Memory_Pool_Define.h"
#endif

#if ! defined(UPF_IMEMORY_POOL_H)
# include "UPF_IMemory_Pool.h"
#endif

#if ! defined(UPF_FIXED_MEMORY_POOL_T_H)
# include "UPF_Fixed_Memory_Pool_T.h"
#endif

#if ! defined(UPF_COMMON_MEMORY_POOL_T_H)
# include "UPF_Common_Memory_Pool_T.h"
#endif

#if ! defined(UPF_COMMON_MEMORY_POOL2_T_H)
# include "UPF_Common_Memory_Pool2_T.h"
#endif

#if ! defined(UPF_MEMORY_POOL_CRT_DEBUG_ALLOC_T_H) && (defined(UPF_OS_IS_WINNT))
# include "UPF_Memory_Pool_CRT_Debug_Alloc_T.h"
#endif

#if ! defined(UPF_IMEMORY_POOL_INFO_H)
# include "UPF_IMemory_Pool_Info.h"
#endif

#if ! defined(UPF_IMEMORY_POOL_INFO_MANAGER_H)
# include "UPF_IMemory_Pool_Info_Manager.h"
#endif

#if ! defined(UPF_MEMORY_POOL_INFO_MANAGER_IMPL_T_H)
# include "UPF_Memory_Pool_Info_Manager_Impl_T.h"
#endif

#if ! defined(UPF_MEMORY_POOL_DEBUG_T_H)
# include "UPF_Memory_Pool_Debug_T.h"
#endif

#if ! defined(UPF_MEMORY_POOL_HEAP_ALLOC_T_H)
# include "UPF_Memory_Pool_Heap_Alloc_T.h"
#endif

#if ! defined(UPF_PRINT_POLICY_H)
# include "UPF_Print_Policy.h"
#endif

#if ! defined(MEMORY_POOL_INFO_RECORD_POLICY_H)
# include "Memory_Pool_Info_Record_Policy.h"
#endif

#if ! defined(UPF_MEMORY_POOL_BASE_T_H)
# include "UPF_Memory_Pool_Base_T.h"
#endif

#if ! defined(UPF_GLOBAL_MEMORY_POOL_CREATOR_H)
# include "UPF_Global_Memory_Pool_Creator.h"
#endif

#if ! defined(UPF_GLOBAL_MEMORY_POOL_ADAPTER_T_H)
# include "UPF_Global_Memory_Pool_Adapter_T.h"
#endif

#if ! defined(UPF_Memory_Pool_Info_Record_Monitor_Policy_H) && (defined(WIN32) || defined(_WIN32))
# include "UPF_Memory_Pool_Info_Record_Monitor_Policy.h"
#endif

#if ! defined(UPF_THREAD_SYNC_H)
# include "UPF_Thread_Sync.h"
#endif

#if ! defined(UPF_MEMORY_POOL_FACTORY_T_H)
# include "UPF_Memory_Pool_Factory_T.h"
#endif

#include "UPF_Virtual_Memory_Allocator.h"
#include "UPF_One_Level_Memory_Manager.h"
// Type definitions.
#if ! defined(UPF_MEMORY_POOL_MULTI_THREAD_VERSION)
    typedef UPF_Null_Mutex 
            UPF_Memory_Pool_Lock_Type;
#else
    typedef UPF_Thread_Mutex 
            UPF_Memory_Pool_Lock_Type;
#endif

// allocator type
#if defined(UPF_OS_IS_WINNT)
/*    typedef   
        UPF_Memory_Pool_Alloc_Type;*/
typedef UPF_One_Level_Memory_Allocator
		UPF_Memory_Pool_Alloc_Type;

#elif defined(UPF_OS_IS_WINCE)
typedef UPF_One_Level_Memory_Allocator
		UPF_Memory_Pool_Alloc_Type;
#endif 

// record policy type
#if defined(UPF_OS_IS_WINNT)
typedef UPF_Memory_Pool_Info_Record_MMap_Monitor_Policy 
        UPF_Memory_Pool_Record_Policy_Type;
#elif defined(UPF_OS_IS_WINCE)
typedef UPF_Memory_Pool_Info_Record_Policy 
        UPF_Memory_Pool_Record_Policy_Type;
#endif

// fixe memory pool.
typedef UPF_Fixed_Memory_Pool_T< UPF_Memory_Pool_Alloc_Type, 
                                 UPF_Memory_Pool_Lock_Type,
                                 UPF_Memory_Pool_Record_Policy_Type >
        UPF_Fixed_Memory_Pool;

// common memory pool.
typedef UPF_Common_Memory_Pool_T< UPF_Memory_Pool_Alloc_Type, 
                                  UPF_Memory_Pool_Lock_Type,
                                  UPF_Memory_Pool_Record_Policy_Type >
        UPF_Common_Memory_Pool;

// common memory pool2.
typedef UPF_Common_Memory_Pool2_T< UPF_Memory_Pool_Alloc_Type, 
                                   UPF_Memory_Pool_Lock_Type,
                                   UPF_Memory_Pool_Record_Policy_Type >
        UPF_Common_Memory_Pool2;

// crt debug alloc
#if defined(UPF_OS_IS_WINNT)
typedef UPF_Memory_Pool_CRT_Debug_Alloc_T< UPF_Memory_Pool_Alloc_Type,
                                           UPF_Memory_Pool_Lock_Type,
                                           UPF_Memory_Pool_Record_Policy_Type >
        UPF_Memory_Pool_CRT_Debug_Alloc;
#endif /* UPF_OS_IS_WINNT */

// debug memory pool.
typedef UPF_Memory_Pool_Debug_T< UPF_Thread_Mutex >
        UPF_Memory_Pool_Debug;

// heap alloc memory pool.
typedef UPF_Memory_Pool_Heap_Alloc_T< UPF_Memory_Pool_Alloc_Type,
                                      UPF_Memory_Pool_Lock_Type,
                                      UPF_Memory_Pool_Record_Policy_Type >
        UPF_Memory_Pool_Heap_Alloc;


// global memory pool.
typedef UPF_Global_Memory_Pool_Adapter_T< UPF_Memory_Pool_Alloc_Type,
                                          UPF_Memory_Pool_Lock_Type,
                                          UPF_Memory_Pool_Record_Policy_Type >
        UPF_Global_Memory_Pool;


// UPF Memory Pool Info Manager, print info to console.
typedef UPF_Memory_Pool_Info_Manager_Impl< UPF_Memory_Pool_Lock_Type,
                                           UPF_Console_Print_Policy >
        UPF_Memory_Pool_Info_Manager;

// UPF Memory Pool Factory.
typedef UPF_Memory_Pool_Factory_T< UPF_Memory_Pool_Lock_Type >
        UPF_Memory_Pool_Factory;


#endif /* UPF_MEMORY_POOL_COMPONENTS_H */

