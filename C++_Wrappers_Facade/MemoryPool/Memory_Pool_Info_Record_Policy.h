/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : Memory_Pool_Info_Record_Policy.h                                                           *
*       CREATE DATE     : 2007-11-19                                                                                 *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]内存池信息记录策略![CN]                                                                *
*********************************************************************************************************************/

#ifndef MEMORY_POOL_INFO_RECORD_POLICY_H
#define MEMORY_POOL_INFO_RECORD_POLICY_H

#include <stddef.h>        /* for size_t */

#include "..\..\OS_Adapter\UPF_OS.h"

/** 
 * @class UPF_Memory_Pool_Info_Record_Policy
 * 
 * @brief 记录内存池信息的策略类
 * 
 */
class /*UPF_Export*/ UPF_Memory_Pool_Info_Record_Policy
{
public:
    UPF_Memory_Pool_Info_Record_Policy()
    {
        m_managed_memory_size   = 0;
        m_allocated_memory_size = 0;
        m_allocated_memory_peak = 0;
        m_wasted_memory_size    = 0;
    }

public:
    inline void set_memory_pool_name( const char * memory_pool_name )
    {
        // empty method body
    }

    inline void set_memory_pool_type( const char * memory_pool_type )
    {
        // empty method body
    }

    inline size_t get_managed_memory_size( void ) const
    {
        return m_managed_memory_size;
    }

    inline void set_managed_memory_size( size_t size )
    {
        m_managed_memory_size = size;
    }

    inline size_t get_allocated_memory_size( void ) const
    {
        return m_allocated_memory_size;
    }

    inline void set_allocated_memory_size( size_t size )
    {
        m_allocated_memory_size = size;

        if ( m_allocated_memory_size > m_allocated_memory_peak )
        {
            m_allocated_memory_peak = m_allocated_memory_size;
        }
    }

    inline size_t get_allocated_memory_peak( void ) const
    {
        return m_allocated_memory_peak;
    }

    inline size_t get_wasted_memory_size( void ) const
    {
        return m_wasted_memory_size;
    }

    inline void set_wasted_memory_size( size_t size )
    {
        m_wasted_memory_size = size;
    }

    inline size_t get_allocated_memory_limit( void ) const
    {
        return m_allocated_memory_limit;
    }

    inline void set_allocated_memory_limit( size_t size )
    {
        m_allocated_memory_limit = size;
    }

    inline void set_memory_pool_id( size_t memory_pool_id )
    {
        m_memory_pool_id = memory_pool_id;
    }

    inline size_t get_memory_pool_id( void )
    {
        return m_memory_pool_id;
    }
    
private:
    /// managed memory size of memory pool.
    size_t m_managed_memory_size;

    /// allocated memory size of memory pool.
    size_t m_allocated_memory_size;

    /// allocated memory peak of memory pool.
    size_t m_allocated_memory_peak;

    /// wasted memory size of memory pool.
    size_t m_wasted_memory_size;

    /// allocated memory limit value of memory pool.
    size_t m_allocated_memory_limit;

    /// memory pool id
    size_t m_memory_pool_id;
};

/** 
 * @class UPF_Memory_Pool_Info_Record_Null_Policy
 * 
 * @brief 用于记录内存池信息的策略类, 但是它什么也不干.
 */
class UPF_Memory_Pool_Info_Record_Null_Policy
{
public:
    inline void set_memory_pool_name( const char * memory_pool_name )
    {
        // empty method body
    }

    inline void set_memory_pool_type( const char * memory_pool_type )
    {
        // empty method body
    }

    inline size_t get_managed_memory_size( void ) const
    {
        return 0;
    }

    inline void set_managed_memory_size( size_t size )
    {
    }

    inline size_t get_allocated_memory_size( void ) const
    {
        return 0;
    }

    inline void set_allocated_memory_size( size_t size )
    {
    }

    inline size_t get_allocated_memory_peak( void ) const
    {
        return 0;
    }

    inline size_t get_wasted_memory_size( void ) const
    {
        return 0;
    }

    inline void set_wasted_memory_size( size_t size )
    {
    }

    inline size_t get_allocated_memory_limit( void )
    {
        return 0xFFFFFFFF;
    }

    inline void set_allocated_memory_limit( size_t size )
    {
    }

    inline void set_memory_pool_id( size_t memory_pool_id )
    {
    }

    inline size_t get_memory_pool_id( void )
    {
        return 0;
    }
};


#endif /* MEMORY_POOL_INFO_RECORD_POLICY_H */
