/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Pool_Object_Base.h                                                                     *
*       CREATE DATE     : 2007-12-13                                                                                 *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]从内存池中分配的对象的基类模板定义![CN]                                                *
*********************************************************************************************************************/

#ifndef UPF_POOL_OBJECT_BASE_H
#define UPF_POOL_OBJECT_BASE_H

#include <stddef.h>                        /* for size_t */
#include <new>                             /* for nothrow_t */
#include "UPF_Memory_Pool_Singleton.h"

/** 
 * @class UPF_Pool_Object_Base
 * 
 * @brief 从内存池中分配的对象的基类模板.
 * 
 * @details 当创建一个对象时, 想要使用从内存池中分配的内存时, 那么以这个类作为基类
 *          派生即可.
 * 
 * @note 当各模块使用UPF_Pool_Object_Base作为基类时，首先要使用UPF_Memory_Pool_Traits.h
 *       中的DEFINE_MEMORY_POOL_TRAITS定一个UPF_Memory_Pool_Traits.
 * 
 * - module_id 模块id.
 * 
 */
template < size_t module_id >
class UPF_Pool_Object_Base
{
#define POOL_INSTANCE    UPF_Memory_Pool_Singleton( module_id )::instance()

public:

    /********************** Single Object new and delete operator **************/

    // single object new and delete operator overload
    static void * operator new( size_t size )
    {
        return UPF_Malloc( POOL_INSTANCE, size );
    }

    static void operator delete( void * p )
    {
        UPF_Free( POOL_INSTANCE, p );
    }

    // non-throwing single object new and delete operator overload
    static void * operator new( size_t size, const std::nothrow_t & )
    {
        return UPF_Malloc( POOL_INSTANCE, size );
    }

    static void operator delete( void * p, const std::nothrow_t & )
    {
        UPF_Free( POOL_INSTANCE, p );
    }

    // placement single object new and delete operator overload
    static void * operator new( size_t size, void * place )
    {
        return ::operator new( size, place );
    }

    static void operator delete( void * p, void * place )
    {
        ::operator delete( p, place );
    }

    // single object new and delete operator with debug info overload
    static void * operator new( size_t size, const char * file_name, int line_no )
    {
        return UPF_Malloc_Debug( POOL_INSTANCE, size, file_name, line_no );
    }

    static void operator delete( void * p, const char * file_name, int line_no )
    {
        UPF_Free_Debug( POOL_INSTANCE, p, file_name, line_no );
    }

    /********************** Array Object new and delete operator **************/

    // array object new and delete operator overload
    static void * operator new[]( size_t size )
    {
        return UPF_Malloc( POOL_INSTANCE, size );
    }

    static void operator delete[]( void * p )
    {
        UPF_Free( POOL_INSTANCE, p );
    }

    // non-throwing array object new and delete operator overload
    static void * operator new[]( size_t size, const std::nothrow_t & )
    {
        return UPF_Malloc( POOL_INSTANCE, size );
    }

    static void operator delete[]( void * p, const std::nothrow_t & )
    {
        UPF_Free( POOL_INSTANCE, p );
    }

    // placement array object new and delete operator overload
    static void * operator new[]( size_t size, void * place )
    {
        return ::operator new( size, place );
    }

    static void operator delete[]( void * p, void * place )
    {
        ::operator delete( p, place );
    }

    // array object new and delete operator with debug info overload
    static void * operator new[]( size_t size, const char * file_name, int line_no )
    {
        return UPF_Malloc_Debug( POOL_INSTANCE, size, file_name, line_no );
    }

    static void operator delete[]( void * p, const char * file_name, int line_no )
    {
        UPF_Free_Debug( POOL_INSTANCE, p, file_name, line_no );
    }

protected:

    inline UPF_Pool_Object_Base( void ) { }

    inline UPF_Pool_Object_Base( const UPF_Pool_Object_Base & ) { }

    inline UPF_Pool_Object_Base & operator = ( const UPF_Pool_Object_Base & )
    {
        return *this;
    }

    inline ~UPF_Pool_Object_Base( void ) { }

#undef POOL_INSTANCE
};

#endif /* UPF_POOL_OBJECT_BASE_H */

