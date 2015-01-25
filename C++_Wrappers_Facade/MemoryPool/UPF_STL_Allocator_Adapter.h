/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_STL_Allocator_Adapter.h                                                                *
*       CREATE DATE     : 2007-12-14                                                                                 *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]C++标准库的Allocator的一个适配器![CN]                                                  *
*********************************************************************************************************************/

#ifndef UPF_STL_ALLOCATOR_ADAPTER_H
#define UPF_STL_ALLOCATOR_ADAPTER_H

#include <cstddef>

#include "UPF_Memory_Pool_Singleton.h"
#include "UPF_Memory_Pool_Traits.h"

#define STL_ADAPTER             100

DEFINE_MEMORY_POOL_TRAITS(STL_ADAPTER, MAX_MEMORY_REQ_SIZE)

#ifdef _MSC_VER
# pragma pack(push, 8)
#endif /* _MSC_VER */

template < class Element_Type >
class UPF_STL_Allocator_Adapter
{
public:

    typedef size_t               size_type;
	typedef ptrdiff_t            difference_type;
	typedef Element_Type *       pointer;
	typedef const Element_Type * const_pointer;
	typedef Element_Type &       reference;
	typedef const Element_Type & const_reference;
	typedef Element_Type         value_type;

public:
    template < class Other_Element_Type >
    UPF_STL_Allocator_Adapter( const UPF_STL_Allocator_Adapter<Other_Element_Type> & ) { }

    template < class Other_Element_Type >
    struct rebind
    {
        typedef UPF_STL_Allocator_Adapter< Other_Element_Type > other;
    };

public:
    UPF_STL_Allocator_Adapter( void ) { }

    UPF_STL_Allocator_Adapter( const UPF_STL_Allocator_Adapter & ) { }

    ~UPF_STL_Allocator_Adapter( void ) {}

public:
    pointer address( reference element ) const 
    { 
        return &element; 
    }
    
    const_pointer address( const_reference element ) const 
    { 
        return &element; 
    }

    Element_Type * allocate( size_type count, const void * = 0 ) 
    {
        if ( 0 != count )
        {
            return static_cast< Element_Type * >(
                UPF_Malloc( UPF_Memory_Pool_Singleton( STL_ADAPTER )::instance(),
                     count * sizeof( Element_Type ) ) );
        }
        else
        {
            return 0;
        }
    }

#ifdef _MSC_VER
	char * _Charalloc( size_type count )
	{
        if ( 0 != count )
        {
            return static_cast< char * >(
                UPF_Malloc( UPF_Memory_Pool_Singleton( STL_ADAPTER )::instance(),
                     count * sizeof( char ) ) );
        }
        else
        {
            return 0;
        }
    }
#endif /* _MSC_VER */

    void deallocate( void * ptr, size_type count )
    { 
        UPF_Free( UPF_Memory_Pool_Singleton( STL_ADAPTER )::instance(),
                  ptr );
    }

    size_type max_size( void ) const
    { 
        return size_t( -1 ) / sizeof( Element_Type );
    }

    void construct( pointer ptr, const Element_Type & val )
    { 
        new (ptr) Element_Type( val ); 
    }

    void destroy( pointer ptr )
    { 
        ptr->~Element_Type();
    }

};

template< class Element_Type_lhs, class Element_Type_rhs > 
inline
bool operator ==( const UPF_STL_Allocator_Adapter< Element_Type_lhs >&, 
                  const UPF_STL_Allocator_Adapter< Element_Type_rhs >& )
{
    return (true); 
}

template< class Element_Type_lhs, class Element_Type_rhs > 
inline
bool operator !=( const UPF_STL_Allocator_Adapter< Element_Type_lhs >&, 
                  const UPF_STL_Allocator_Adapter< Element_Type_rhs >& )
{
    return (false); 
}

template<> 
class UPF_STL_Allocator_Adapter< void > 
{
public:
	typedef void                 Element_Type;
	typedef Element_Type *       pointer;
	typedef const Element_Type * const_pointer;
	typedef Element_Type         value_type;

public:
    template < class Other_Element_Type >
    struct rebind
    {
        typedef UPF_STL_Allocator_Adapter< Other_Element_Type > other;
    };

};

#ifdef  _MSC_VER
# pragma pack(pop)
#endif  /* _MSC_VER */

#endif /* UPF_STL_ALLOCATOR_ADAPTER_H */

