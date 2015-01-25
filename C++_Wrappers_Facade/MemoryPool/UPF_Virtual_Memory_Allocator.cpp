/*----------------Copyright(C) 2008 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Virtual_Memory_Allocator.cpp                                                           *
*       CREATE DATE     : 2008-4-4                                                                                   *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]使用VirtualAlloc来分配虚拟内存的allocator![CN]                                         *
*********************************************************************************************************************/
#include "UPF_Virtual_Memory_Allocator.h"
#include "UPF_Memory_Pool_Define.h"
#include "UPF_Memory_Pool_Util.h"
#include "Memory_Pool_Info_Record_Policy.h"

#include <windows.h>
#include <memory>

#ifdef UPF_OS_IS_WINNT
#  include <stdio.h>
#endif

#if defined(WIN32) || defined(_WIN32)
#  define snprintf       _snprintf     /* on win32 platform, be _snprintf */
#endif /* defined(WIN32) || defined(_WIN32) */

////////////////////////////////////////////////////////////////////////////////

// macro definitions
#define UPF_MAX_VIRTUAL_MEMORY_ALLOC_SIZE            ( 30 * 1024 * 1024 )
#define UPF_MAX_VIRTUAL_MEMORY_REQ_SIZE              ( 6 * 1024 * 1024  )

#define UPF_ALLOC_MEMORY_BASE_SIZE                   ( MIN_ALLOC_MEM_SIZE  )
#define UPF_MAX_MEMPOOL_SIZE                         ( MAX_SMALL_BLOCK_SIZE)

#define VIRTUAL_ALLOCATOR_NODE_ALIGN_SIZE            ( 4 )

#define SIZE_MASK                                    ( 0xFFFFFFFC )
#define USED_STATUS_MASK                             ( 0x00000003 )

#define ALLOC_WATER_SIZE                             ( 1 * 1024 )

// used to return to memory accquirer.
#define ALLOCATED_NODE(node)                   ( (char *) (node) + NODE_USED_TAG_SIZE )

// used to obtain original node ptr.
#define ORIGINAL_NODE(ptr)                     ( (Free_Block_Node*) ((char *) (ptr) - NODE_USED_TAG_SIZE ) )

// used to get space size with used tag.
#define SPACE_SIZE(tag)                        ( (tag) & SIZE_MASK )

// used to split into a new freed node in original freed node's base from bottom.
#define NEW_FREED_NODE_FROM_BOTTOM(node, required_size )   ( (Free_Block_Node*) ((char *) (node) + required_size + EACH_NODE_WASTED_SIZE) )

// used to split into a new freed node in original freed node's base from top.
#define NEW_FREED_NODE_FROM_TOP(node, required_size )      ( (Free_Block_Node*) ((char *) (node) + \
SPACE_SIZE(node->used_tag.tag) - required_size) )

// used to get node's end ptr
#define NODE_END_PTR( node )                   ( (char *)node + EACH_NODE_WASTED_SIZE +\
    SPACE_SIZE(((node)->used_tag.tag)) )

// used to locate left neighbour node's used tag
#define LEFT_NEIGHBOUR_USED_TAG( node )         ( (Node_Used_Tag*) ((char *) (node) - NODE_USED_TAG_SIZE) )

// used to locate right neighbour node's used tag
#define RIGHT_NEIGHBOUR_USED_TAG( node )        ( (Node_Used_Tag*) ((char *) (node) + EACH_NODE_WASTED_SIZE +\
SPACE_SIZE(((node)->used_tag.tag)) ) )

// used to locate left original block node head
#define LEFT_BLOCK_NODE_HEAD( node ) \
  ( (Free_Block_Node*)( (char*)(node) - \
       SPACE_SIZE( LEFT_NEIGHBOUR_USED_TAG( node )->tag ) - \
         EACH_NODE_WASTED_SIZE ) )

// used to locate right original block node head
#define RIGHT_BLOCK_NODE_HEAD( node ) \
  ( (Free_Block_Node*)( RIGHT_NEIGHBOUR_USED_TAG( node  ) ) )

// remove node
#define REMOVE_NODE( node ) \
    node->previous_block_node->next_block_node = node->next_block_node;\
    node->next_block_node->previous_block_node = node->previous_block_node;

// insert node before a special node.
#define INSERT_NODE_BEFORE( node, next_node ) \
    node->next_block_node = next_node;\
    node->previous_block_node = next_node->previous_block_node;\
    next_node->previous_block_node->next_block_node = node;\
    next_node->previous_block_node = node;

// insert node after a special node.
#define INSERT_NODE_AFTER( node, previous_node ) \
    node->previous_block_node = previous_node;\
    node->next_block_node = previous_node->next_block_node;\
    previous_node->next_block_node->previous_block_node = node;\
    previous_node->next_block_node = node;

// update node's ptr
#define UPDATE_NODE( old_node, new_node ) \
   new_node->next_block_node = old_node->next_block_node;\
   old_node->next_block_node->previous_block_node = new_node;\
   new_node->previous_block_node = old_node->previous_block_node;\
   old_node->previous_block_node->next_block_node = new_node;
    

// used to locate to head tag node.
#define HEAD_USED_TAG(node)                    ( (Node_Used_Tag*) ( node ) )

// used to locate to foot tag node.
#define FOOT_USED_TAG(node, size)              ( (Node_Used_Tag*) ( (char *) (node) + NODE_USED_TAG_SIZE + size ) )

// use to judge if free
#define IS_FREE(tag)                           ( ( (tag) & USED_STATUS_MASK ) == NUS_Free )

// get toplevel allocator
#define DO_GET_TOPLEVEL_ALLOCATOR( type ) \
    assert( 0 <= type && type < ( sizeof( sm_toplevel_allocators ) / sizeof( *sm_toplevel_allocators) ) );\
    if ( 0 == sm_toplevel_allocators[ type ] )\
        create_toplevel_allocator( type );
    

////////////////////////////////////////////////////////////////////////////////
// type definitions

// 用于表示节点的使用状态.
enum  Node_Used_Status { NUS_Free = 0, NUS_Used = 1 };

/// 节点的使用标记, 在一个节点的首部，尾部都有一个
/// 这样的节点.
struct Node_Used_Tag
{
    /// 表示该节点所管理的空闲块大小和使用状态, 低2位表示
    /// 使用状态, 高30位表示块的大小.
    unsigned int tag;
};

/// 空闲内存块的节点类型 .
struct Free_Block_Node
{
    /// 节点的使用标记
    Node_Used_Tag    used_tag;

    /// 当在free状态时, 用于指向前一个Free Block.
    Free_Block_Node * previous_block_node;

    /// 当在free状态时, 用于指向下一个Free Block.
    Free_Block_Node * next_block_node;
};

/// Virtual Allocator 内部需要一些的size定义
enum { NODE_USED_TAG_SIZE = sizeof( Node_Used_Tag ),
       EACH_NODE_WASTED_SIZE = 2 * NODE_USED_TAG_SIZE,
       EACH_NODE_MIN_RESERVE_SIZE = sizeof(Free_Block_Node) + NODE_USED_TAG_SIZE };

/// 用于记录Memory Pool Info.
typedef UPF_Memory_Pool_Info_Record_Policy    UPF_Memory_Pool_Record_Policy_Type;


///////////////////////////////////////////////////////////////////////////////////////

/**
 * @class UPF_Virtual_Memory_Allocator_Impl
 *
 * @brief UPF_Virtual_Memory_Allocator's implementor.
 *
 */
class UPF_Virtual_Memory_Allocator_Impl
{
public:
    /**
     * initialize member.
     *
     * @param    type   [in]  UPF_TOPLEVEL_ALLOCATOR_TYPES or UPF_TOPLEVEL_PERMANENT_ALLOCATOR.
     *
     * @param    managed_memory_size  [in]  managed memory size, the Unit is Byte.

     *
     */
    UPF_Virtual_Memory_Allocator_Impl( unsigned int type,
                                       size_t       managed_memory_size );
   
    /**
     * release memory.
     */
    ~UPF_Virtual_Memory_Allocator_Impl( void );

public:
    /// alloc memory
    void * allocate( size_t size );

    /// dealloc memory
    void deallocate( void * ptr, size_t size );

    /// print memory info
    void print_memory_info( FILE * fp );

    /// get total allocated size.
    size_t get_total_allocated_size( void );

public:
    /**
     * set managed memory size, must be called before first memory request.
     *
     * @return true    set succeed
     *         false   set failed.
     */ 
    bool set_managed_memory_size( size_t managed_memory_size );

    /**
     * reclaim memory, return physical memory to os.
     */
    bool reclaim( void );

    /**
     * restore memory's manage.
     */
    bool restore( void );

private:
    /// 调节一个free block node在free block node list中的顺序, 一般为
    /// 升序排列.
    inline void adjust_free_block_node_order( Free_Block_Node * free_block_node,
                                              size_t            space_size,
                                              bool              is_greater );

    /// 查入空闲块
    inline void insert_free_block( Free_Block_Node * free_block_node,
                                   size_t            space_size );

    /// 设置一个块的foot used tag.
    inline void set_foot_used_tag( void *            ptr,
                                   size_t            size, 
                                   Node_Used_Status  used_status );

    /// 设置一个块的head used flag.
    inline void set_head_used_tag( void *            ptr,
                                   size_t            size,
                                   Node_Used_Status  used_status );

    /// 设置一个块的首尾 used tag
    inline void set_used_tag( void *            ptr,
                              size_t            size,
                              Node_Used_Status  used_status );


    /// 重置一个node.
    inline void reset_node( Free_Block_Node * node,
                            size_t            size );

private:

    bool is_reclaimed( void ) const
    {
        return m_is_reclaimed;
    }

    void set_reclaimed( bool is_reclaimed )
    {
        m_is_reclaimed = is_reclaimed;
    }

private:
    /// reset member
    void reset( void );

private:

    /// 用于存放内存映射的句柄, 初始为NULL
    HANDLE                  m_memfile_handle;

    /// 用于指示空闲块列表的头, 它的previous_block_node指向最后一个节点,
    /// 它的next_block_node指示第一个节点, used_tag未用.
    Free_Block_Node         m_free_block_list_head;

    /// 用于跟踪最初分配的空闲块
    Free_Block_Node *       m_org_free_block_head;

    /// 用于存储一个已分配的大块内存的头指针
    void *                  m_allocated_memory_head_ptr;

    /// 用于存储一个已分配的大块内存的尾指针
    void *                  m_allocated_memory_end_ptr;

    /// 用于virtual alloc 的lock
    UPF_Thread_Mutex        m_lock;

    /// 用于记录内存的分配情况.
    UPF_Memory_Pool_Record_Policy_Type   m_va_recorder;

    /// 用于记录当前的allocator类型.
    unsigned int            m_type;

    /// 用于记录所管理的内存大小.
    size_t                  m_managed_memory_size;

    /// 用于记录是否已经reclaimed, 初始为false.
    bool                    m_is_reclaimed;
};


////////////////////////////////////////////////////////////////////////////
//
// UPF_Virtual_Memory_Allocator_Impl's Implement
//
////////////////////////////////////////////////////////////////////////////

UPF_Virtual_Memory_Allocator_Impl::UPF_Virtual_Memory_Allocator_Impl( unsigned int type,
                                                                      size_t       managed_memory_size )
{
    // reset member be 0.
    reset();

    // set recalimed be false.
    this->set_reclaimed( false );

    // set other members.
    m_type                = type;
    m_managed_memory_size = managed_memory_size;
}

/////////////////////////////////////////////////////////////////////////

UPF_Virtual_Memory_Allocator_Impl::~UPF_Virtual_Memory_Allocator_Impl( void )
{
    this->reclaim();
}

/////////////////////////////////////////////////////////////////////////

bool 
UPF_Virtual_Memory_Allocator_Impl::set_managed_memory_size( size_t managed_memory_size )
{
    if ( 0 != m_org_free_block_head )
    {
        return false;
    }

    m_managed_memory_size  = managed_memory_size;

    return true;
}


/////////////////////////////////////////////////////////////////////////

bool 
UPF_Virtual_Memory_Allocator_Impl::reclaim( void )
{
    UPF_Guard< UPF_Thread_Mutex > guard( m_lock );

    if ( this->is_reclaimed() )
    {
        return false;
    }
    else
    {
        // unmap memory.
        if ( 0 != m_allocated_memory_head_ptr )
        {
            UnmapViewOfFile( m_allocated_memory_head_ptr );
        }

        // close mem mapping handle.
        if ( NULL != m_memfile_handle )
        {
            CloseHandle( m_memfile_handle );
        }

        // do final process.
        this->reset();
        this->set_reclaimed( true );

        return true;
    }
}


/////////////////////////////////////////////////////////////////////////

bool 
UPF_Virtual_Memory_Allocator_Impl::restore( void )
{
    UPF_Guard< UPF_Thread_Mutex > guard( m_lock );

    if ( this->is_reclaimed() )
    {
        this->set_reclaimed( false );

        return true;
    }
    else
    {
        return false;
    }
}


/////////////////////////////////////////////////////////////////////////

void 
UPF_Virtual_Memory_Allocator_Impl::reset( void )
{
    m_org_free_block_head  = 0;

    m_allocated_memory_head_ptr = 0;
    m_allocated_memory_end_ptr  = 0;

    m_memfile_handle = NULL;

    // init m_free_block_list_head be empty.
    m_free_block_list_head.next_block_node     = &m_free_block_list_head;
    m_free_block_list_head.previous_block_node = &m_free_block_list_head;

    // set size info
    m_va_recorder.set_managed_memory_size( 0 );
    m_va_recorder.set_allocated_memory_size( 0 );
    m_va_recorder.set_wasted_memory_size( 0 );
}


/////////////////////////////////////////////////////////////////////////

void * 
UPF_Virtual_Memory_Allocator_Impl::allocate( size_t size )
{
    // size must be large than or equal to UPF_ALLOC_MEMORY_BASE_SIZE
    assert( size >= UPF_ALLOC_MEMORY_BASE_SIZE );

    UPF_Guard< UPF_Thread_Mutex > guard( m_lock );

    // if already reclaimed, then return 0.
    if ( is_reclaimed() )
    {
        UPF_Memory_Pool_Util::report( 
            "VirtualAllocator[%d]: memory already reclaimed!",
            m_type );

        return 0;
    }

    size_t  wasted_size = 0;
    void *  result_ptr  = 0;

    if ( size > UPF_MAX_VIRTUAL_MEMORY_REQ_SIZE )
    {
        UPF_Memory_Pool_Util::report( 
            "VirtualAllocator[%d]: Allocated size (%u) is large than max acceped size (%u)!",
            m_type,
            size,
            UPF_MAX_VIRTUAL_MEMORY_REQ_SIZE );

        return 0;
    }

    // if m_allocated_memory_ptr is 0, then allocate virtual memory,
    // allocate virtual memory only on first allocate.
    if ( 0 == m_allocated_memory_head_ptr )
    {
        m_memfile_handle = CreateFileMapping( INVALID_HANDLE_VALUE,
                                              NULL, 
                                              PAGE_READWRITE, 
                                              0, 
                                              m_managed_memory_size, 
                                              NULL );

        if ( m_memfile_handle == NULL )
        {
            UPF_Memory_Pool_Util::report(
                "VirtualAllocator[%d]: CreateFileMapping failed! ",
                m_type );

            return 0;                 
        }
        
        LPVOID commit_memory =
            MapViewOfFile( m_memfile_handle, 
                           FILE_MAP_WRITE | FILE_MAP_READ, 
                           0, 
                           0, 
                           m_managed_memory_size
                         );
        
        if ( commit_memory == NULL )
        {
            CloseHandle( m_memfile_handle );
            m_memfile_handle = NULL;

            UPF_Memory_Pool_Util::report(
                "VirtualAllocator[%d]: CreateFileMapping failed! ",
                m_type );

            return 0;                 
        }
        
        // reset m_free_block_list_head.used_tag.
        m_free_block_list_head.used_tag.tag = 0;
        
        m_allocated_memory_head_ptr = commit_memory;
        m_allocated_memory_end_ptr  = static_cast< char * >( 
            m_allocated_memory_head_ptr ) + m_managed_memory_size;
        
        Free_Block_Node * free_block_head = 
            static_cast< Free_Block_Node * >( m_allocated_memory_head_ptr );
        
        // used to record orginal free block head for alloc memory pool's memory.
        m_org_free_block_head = free_block_head;
                 
        reset_node( free_block_head, m_managed_memory_size - EACH_NODE_WASTED_SIZE );                                        
        
        m_va_recorder.set_wasted_memory_size( EACH_NODE_WASTED_SIZE );
        m_va_recorder.set_managed_memory_size( m_managed_memory_size );
    }

    // set required size, default align to 4 bytes.
    size_t required_size = UPF_MEMORY_ALIGN( size, VIRTUAL_ALLOCATOR_NODE_ALIGN_SIZE );

    wasted_size = required_size - size;
    
    // free block node.
    Free_Block_Node * free_block_node = 0;

    // store found block size.
    size_t            space_size = 0;

    // used to store flag if is mempool block, mempool block is allocated from
    // bottom to top, non mempool block is allocated from top to bottom.
    bool              is_mempool_block = true;

    // set start free block node that want to find.
    if ( size <= UPF_MAX_MEMPOOL_SIZE )
    {
        if ( 0 != m_org_free_block_head )
        {
            free_block_node  = m_org_free_block_head;
        }
        else
        {
            free_block_node = m_free_block_list_head.next_block_node;
        }

        is_mempool_block = true;
    }
    else
    {
        free_block_node  = m_free_block_list_head.next_block_node;
        is_mempool_block = false;
    }


    // find block node of suited size.
    while ( free_block_node != &m_free_block_list_head )
    {
        space_size = SPACE_SIZE(free_block_node->used_tag.tag);

        if ( space_size >= required_size )
        {
            break;
        }

        free_block_node = free_block_node->next_block_node;
    }


    if ( free_block_node != &m_free_block_list_head && 
         space_size >= required_size )
    {  
        // if space size equal to required size, then remove the block node.
        if ( ! is_mempool_block && ( space_size - required_size ) <= ALLOC_WATER_SIZE )
        { 
            REMOVE_NODE( free_block_node );

            if ( m_org_free_block_head == free_block_node )
            {
                m_org_free_block_head = 0;
            }

            // set alloated block's used tag.
            set_used_tag( free_block_node, space_size, NUS_Used );

            // set wasted size
            wasted_size += ( space_size - required_size ); 
        }
        else
        {
            if ( is_mempool_block )
            {
                if ( (space_size - required_size) <= EACH_NODE_MIN_RESERVE_SIZE )
                {

                    REMOVE_NODE( free_block_node );


                    if ( m_org_free_block_head == free_block_node )
                    {
                        m_org_free_block_head = 0;
                    }

                    // set alloated block's used tag.
                    set_used_tag( free_block_node, space_size, NUS_Used );

                    // set wasted size
                    wasted_size += ( space_size - required_size ); 

                }
                else
                {
                    Free_Block_Node * new_free_block_node =
                        NEW_FREED_NODE_FROM_BOTTOM( free_block_node, required_size );
                    
                    // if free_block_node is m_org_free_block_head,
                    // then update m_org_free_block_head is new_free_block_node.
                    if ( m_org_free_block_head == free_block_node )
                    {
                        m_org_free_block_head = new_free_block_node;
                    }
                    
                    // set new free size and used status.
                    size_t new_size = space_size - required_size - 
                        EACH_NODE_WASTED_SIZE;
                    
                    // modify link
                    UPDATE_NODE( free_block_node, new_free_block_node );
                    
                    // set new free block node's used tag.               
                    set_used_tag( new_free_block_node, new_size, NUS_Free );
                    
                    // because size decrement,
                    // so if new_free_block_node is not the first node, then adjust node's size order.
                    if ( m_free_block_list_head.next_block_node != new_free_block_node )
                    {
                        // adjust new free block node's order in free block list.
                        adjust_free_block_node_order( new_free_block_node,
                                                      new_size,
                                                      false );
                    }
                    
                    // because split a new node, need 2 * used tag size to record          
                    // size and used status infos.
                    wasted_size += EACH_NODE_WASTED_SIZE;  

                    // set alloated block's used tag.
                    set_used_tag( free_block_node, required_size, NUS_Used );

                }

            }
            else
            {
                Free_Block_Node *  new_free_block_node =
                    NEW_FREED_NODE_FROM_TOP( free_block_node, required_size );

                // set new free size and used status.
                size_t new_size = space_size - required_size - 
                    EACH_NODE_WASTED_SIZE;

                set_used_tag( free_block_node, new_size, NUS_Free );

                // if free_block_node is not the first node, then adjust node's size order.
                if ( m_free_block_list_head.next_block_node != free_block_node )
                {
                    // because size decrement,
                    // so if free_block_node is not the first node, then adjust node's size order.
                    adjust_free_block_node_order( free_block_node,
                                                  new_size,
                                                  false );
                }


                // update free_block_node as return free block node.
                free_block_node = new_free_block_node;

                wasted_size += EACH_NODE_WASTED_SIZE;

                // set alloated block's used tag.
                set_used_tag( free_block_node, required_size, NUS_Used );
            }        

        }

        // free_block_node is return node.
        result_ptr = ALLOCATED_NODE(free_block_node);
    }


    if ( 0 == result_ptr )
    {    
        UPF_Memory_Pool_Util::report( "[size: %u B (%u K)] virtual allocate failed!",
                                      size, size / 1024 );
    }
    else
    {
        // set allocated memory size
        m_va_recorder.set_allocated_memory_size( 
                m_va_recorder.get_allocated_memory_size() + size );

        // set wasted memory size
        m_va_recorder.set_wasted_memory_size(
                m_va_recorder.get_wasted_memory_size() + wasted_size );
    }

    return ( result_ptr );
}

/////////////////////////////////////////////////////////////////////////

void 
UPF_Virtual_Memory_Allocator_Impl::deallocate( void * ptr, size_t size )
{
    assert( size >= UPF_ALLOC_MEMORY_BASE_SIZE &&
            size <= UPF_MAX_VIRTUAL_MEMORY_REQ_SIZE );

    UPF_Guard< UPF_Thread_Mutex > guard( m_lock );

    // free block node.
    Free_Block_Node * free_block_node = 0;

    free_block_node = ORIGINAL_NODE( ptr );

    // assert used status is used to prevent from doubled free.
    assert( ! IS_FREE( free_block_node->used_tag.tag ) );

    size_t space_size  = SPACE_SIZE( free_block_node->used_tag.tag );

    assert( space_size >= size );

    // record wasted size and saved size of merging free block node.
    size_t wasted_size      = space_size - size;
    size_t extra_saved_size = 0;

    // locate left neighbour node and right neighbour node,
    // and get corresponding used status.
    Node_Used_Tag * left_neighbour_tag_ptr  = 
        LEFT_NEIGHBOUR_USED_TAG( free_block_node );

    Node_Used_Tag * right_neighbour_tag_ptr = 
        RIGHT_NEIGHBOUR_USED_TAG( free_block_node );

    bool is_left_neighbour_free  = false;
    bool is_right_neighbour_free = false;

    if ( left_neighbour_tag_ptr >= m_allocated_memory_head_ptr &&
         IS_FREE( left_neighbour_tag_ptr->tag )  )
    {
        is_left_neighbour_free = true;
    }

    if ( right_neighbour_tag_ptr < m_allocated_memory_end_ptr &&
         IS_FREE( right_neighbour_tag_ptr->tag )  )
    {
        is_right_neighbour_free = true;
    }

    // start to merge free node.
    if ( is_left_neighbour_free && is_right_neighbour_free )
    {
        Free_Block_Node * left_block_node =
            LEFT_BLOCK_NODE_HEAD( free_block_node );

        Free_Block_Node * right_block_node =
            RIGHT_BLOCK_NODE_HEAD( free_block_node );

        size_t  new_space_size = 0;

        // remove right_block_node
        REMOVE_NODE( right_block_node );

        new_space_size = ( SPACE_SIZE( left_block_node->used_tag.tag ) + 
                           2 * EACH_NODE_WASTED_SIZE +
                           space_size + 
                           SPACE_SIZE( right_block_node->used_tag.tag ) );

        // merge left, right free node.
        set_used_tag( left_block_node, 
                      new_space_size,
                      NUS_Free );

        // because size increase,
        // so if left_block_node is not the last node, then adjust order.
        if ( m_free_block_list_head.previous_block_node != left_block_node )
        {
            // adjust left block node order
            adjust_free_block_node_order( left_block_node,
                                          new_space_size,
                                          true );
        }

        // update m_org_free_block_head
        if ( right_block_node == m_org_free_block_head  )
        {
            m_org_free_block_head = left_block_node;
        }

        // save 2 block node's tag size.
        extra_saved_size = 2 * EACH_NODE_WASTED_SIZE;

    }
    else if ( is_left_neighbour_free )
    {
        // merge left free node
        Free_Block_Node * left_block_node =
            LEFT_BLOCK_NODE_HEAD( free_block_node );

        size_t  new_space_size = 0;

        new_space_size = ( SPACE_SIZE( left_block_node->used_tag.tag ) + 
                           EACH_NODE_WASTED_SIZE + space_size );

        set_used_tag( left_block_node, 
                      new_space_size,
                      NUS_Free );

        // because size increase,
        // so if left_block_node is not the last node, then adjust order.
        if ( m_free_block_list_head.previous_block_node != left_block_node )
        {
            // adjust left block node order
            adjust_free_block_node_order( left_block_node,
                                          new_space_size,
                                          true );
        }

        // save 1 block node's tag size.
        extra_saved_size = EACH_NODE_WASTED_SIZE;
    }
    else if ( is_right_neighbour_free )
    {
        // merge right free node
        Free_Block_Node * right_block_node =
            RIGHT_BLOCK_NODE_HEAD( free_block_node );

        size_t  new_space_size = 0;

        new_space_size = ( space_size + EACH_NODE_WASTED_SIZE +
                           SPACE_SIZE( right_block_node->used_tag.tag ) );

        // modify link
        UPDATE_NODE( right_block_node, free_block_node );

        // set free_block_node'size and used status
        set_used_tag( free_block_node,
                      new_space_size,
                      NUS_Free );


        // because size increase,
        // so if free_block_node is not the last node, then adjust order.
        if ( m_free_block_list_head.previous_block_node != free_block_node )
        {
            // adjust left block node order
            adjust_free_block_node_order( free_block_node,
                                          new_space_size,
                                          true );
        }

        // update m_org_free_block_head
        if ( right_block_node == m_org_free_block_head  )
        {
            m_org_free_block_head = free_block_node;
        }

        // save 1 block node's tag size.
        extra_saved_size = EACH_NODE_WASTED_SIZE;
    }
    else
    {
        // if left neighbour node and right neighbour node is not
        // free, then make the new freed node inserting into
        // free block list.
        set_used_tag( free_block_node,
                      space_size,
                      NUS_Free );

        insert_free_block( free_block_node, space_size );                       
    
    }

    // set allocated memory size
    m_va_recorder.set_allocated_memory_size( 
        m_va_recorder.get_allocated_memory_size() - size );

    // set wasted memory size
    m_va_recorder.set_wasted_memory_size(
        m_va_recorder.get_wasted_memory_size() - wasted_size - extra_saved_size );

    return;
}


///////////////////////////////////////////////////////////////////////////

inline void
UPF_Virtual_Memory_Allocator_Impl::set_foot_used_tag( void *            ptr,
                                                 size_t            size, 
                                                 Node_Used_Status  used_status )
{
    FOOT_USED_TAG( ptr, size )->tag = size | used_status;
}

///////////////////////////////////////////////////////////////////////////

inline void
UPF_Virtual_Memory_Allocator_Impl::set_head_used_tag( void *            ptr,
                                                  size_t            size, 
                                                  Node_Used_Status  used_status )
{
    HEAD_USED_TAG( ptr )->tag = size | used_status;
}


///////////////////////////////////////////////////////////////////////////

inline void 
UPF_Virtual_Memory_Allocator_Impl::set_used_tag( void *            ptr,
                                            size_t            size,
                                            Node_Used_Status  used_status )
{
    HEAD_USED_TAG( ptr )->tag       = size | used_status;
    FOOT_USED_TAG( ptr, size )->tag = size | used_status;
}

///////////////////////////////////////////////////////////////////////////

inline void 
UPF_Virtual_Memory_Allocator_Impl::adjust_free_block_node_order( Free_Block_Node * free_block_node,
                                                            size_t            space_size,
                                                            bool              is_greater )
{   
    // if free_block_node's size greater, then find toward right, otherwise
    // find toward left.
    assert( free_block_node != &m_free_block_list_head );

    if ( is_greater )
    {
        Free_Block_Node * next_node = free_block_node->next_block_node;

        // when node's size is greater, if free block node is last node or
        // maller than next node's size, then 
        // need not to adjust order, return directly.
        if ( next_node == &m_free_block_list_head ||
             space_size <= SPACE_SIZE( next_node->used_tag.tag ) )
        {
            return;
        }
        else
        {

            // remove the free block node
            REMOVE_NODE( free_block_node );

            Free_Block_Node * node = next_node->next_block_node;

            while ( node != &m_free_block_list_head )
            {
                if ( space_size <= SPACE_SIZE( node->used_tag.tag ) )
                {
                    break;
                }

                node = node->next_block_node;
            }

            INSERT_NODE_BEFORE( free_block_node, node );
        }
    }
    else
    {
        Free_Block_Node * previous_node = free_block_node->previous_block_node;

        // when node's size is smaller, if free block node is head node or
        // greater than previous node's size, then 
        // need not to adjust order, return directly.
        if ( previous_node == &m_free_block_list_head ||
             space_size >= SPACE_SIZE( previous_node->used_tag.tag )  )
        {
            return;
        }
        else
        {
            // remove the free block node
            REMOVE_NODE( free_block_node );

            Free_Block_Node * node = previous_node->previous_block_node;

            while ( node != &m_free_block_list_head )
            {
                if ( space_size >= SPACE_SIZE( node->used_tag.tag ) )
                {
                    break;
                }

                node = node->previous_block_node;
            }

            INSERT_NODE_AFTER( free_block_node, node );
        }
    }        
}

/////////////////////////////////////////////////////////////////////////

void 
UPF_Virtual_Memory_Allocator_Impl::insert_free_block( Free_Block_Node * free_block_node,
                                                 size_t            space_size )
{
    Free_Block_Node * node  = m_free_block_list_head.next_block_node;


    while ( node != &m_free_block_list_head  )
    {
        if ( SPACE_SIZE( node->used_tag.tag ) >= space_size )
        {
            break;
        }        

        node = node->next_block_node;
    }
 
    INSERT_NODE_BEFORE( free_block_node, node );

}


///////////////////////////////////////////////////////////////////////////

inline void
UPF_Virtual_Memory_Allocator_Impl::reset_node( Free_Block_Node * node,
                                               size_t            size )
{
    node->next_block_node      = &m_free_block_list_head;
    node->previous_block_node  = &m_free_block_list_head;

    m_free_block_list_head.next_block_node     = node;
    m_free_block_list_head.previous_block_node = node;

    HEAD_USED_TAG( node )->tag       = size | NUS_Free;
    FOOT_USED_TAG( node, size )->tag = size | NUS_Free;
}


///////////////////////////////////////////////////////////////////////////

size_t 
UPF_Virtual_Memory_Allocator_Impl::get_total_allocated_size( void )
{
    return m_va_recorder.get_allocated_memory_size();
}

/////////////////////////////////////////////////////////////////////////

void 
UPF_Virtual_Memory_Allocator_Impl::print_memory_info( FILE * fp )
{
    assert( fp != 0 );

    // first lock to prevent from allocating memory in printing.
    UPF_Guard< UPF_Thread_Mutex > guard( m_lock );

    // print header
    UPF_Memory_Pool_Util::print_chars( fp, 120, '-' );

    UPF_Memory_Pool_Util::print_memory_size_headers( fp );

    // print content

    // generate allocator name with m_type.

    char allocator_name[ 40 ];

    snprintf( allocator_name, 
              sizeof( allocator_name ),
              "VirtualAllocator [%d]",
              m_type );   
    allocator_name[ sizeof( allocator_name ) - 1 ] = '\0';

    UPF_Memory_Pool_Util::print_memory_size_infos(
        fp,
        allocator_name,
        m_va_recorder.get_managed_memory_size(),
        m_va_recorder.get_allocated_memory_size(),
        m_va_recorder.get_allocated_memory_peak(),
        m_va_recorder.get_wasted_memory_size(),
        m_va_recorder.get_managed_memory_size()
    );

//#ifdef UPF_OS_IS_WINNT
    // print virtual allocator's free block list.
    fprintf( fp, "\n\n" );
    UPF_Memory_Pool_Util::print_framed_caption( fp,
                                                "Virtual Allocator Free Blocks Info",
                                                120 ,
                                                4,
                                                '+' );

    
    fprintf( fp,
             "\n[  allocated ptr head: 0x%08X, allocated ptr end: 0x%08X, free block head: 0x%08X ]\n",
             m_allocated_memory_head_ptr,
             m_allocated_memory_end_ptr,
             m_free_block_list_head.next_block_node );

    UPF_Memory_Pool_Util::print_chars( fp, 120, '-' );

    fprintf( fp, "\n" );

    Free_Block_Node * free_block = NULL;
    size_t            block_size = 0;
    size_t            total_size = 0;

    free_block = m_free_block_list_head.next_block_node;
    int i = 0;

    while ( free_block != &m_free_block_list_head )
    {
        block_size = SPACE_SIZE( free_block->used_tag.tag );
        total_size += block_size;

        fprintf( fp,
                 "%d. start address: 0x%p, end address: 0x%p, next: 0x%p, pre: 0x%p, size: %u B [%u K]\n", 
                 ++i, 
                 free_block,
                 NODE_END_PTR( free_block ),
                 free_block->next_block_node,
                 free_block->previous_block_node,
                 block_size,
                 block_size / 1024 );

        free_block = free_block->next_block_node;
    }

    UPF_Memory_Pool_Util::print_chars( fp, 120, '=' );

    fprintf( fp, 
             "\n\tTotal Free Block Count: %d.\n\tTotal Free Block Size: %u B[%u K].\n\n",
             i,
             total_size,
             total_size / 1024 );

//#endif /* UPF_OS_IS_WINNT */
}

////////////////////////////////////////////////////////////////////////////////
//
// UPF_Virtual_Memory_Allocator::Destroyer's Implement
//
////////////////////////////////////////////////////////////////////////////////

UPF_Virtual_Memory_Allocator::Destroyer::~Destroyer()
{
    for ( int i = 0; i < UPF_TOPLEVEL_ALLOCATOR_TYPES; ++i )
    {
        if ( 0 != UPF_Virtual_Memory_Allocator::sm_toplevel_allocators[i] )
        {
            delete ( UPF_Virtual_Memory_Allocator::sm_toplevel_allocators[i] );

            UPF_Virtual_Memory_Allocator::sm_toplevel_allocators[i] = 0;
        }
    }
}


////////////////////////////////////////////////////////////////////////////////
//
// UPF_Virtual_Memory_Allocator's Implement
//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// UPF_Virtual_Memory_Allocator's static data initialization.

// define toplevel allocators.
UPF_Virtual_Memory_Allocator_Impl * 
UPF_Virtual_Memory_Allocator::sm_toplevel_allocators[ UPF_TOPLEVEL_ALLOCATOR_TYPES ] =
{ 0 };

// define dynamic allocator's states.
UPF_Virtual_Memory_Allocator::Dynamic_Allocator_State
    UPF_Virtual_Memory_Allocator::sm_toplevel_dynamic_allocators_states[ 
        UPF_DYNAMIC_TOPLEVEL_ALLOCATOR_TYPES ] =
{ DAS_NONE };

// define first use dynamic allocator's flag, init be true.
bool UPF_Virtual_Memory_Allocator::sm_first_use_dynamic_allocator = true;

// define lock.
UPF_Thread_Mutex
UPF_Virtual_Memory_Allocator::sm_creator_lock;

UPF_Thread_Mutex
UPF_Virtual_Memory_Allocator::sm_manage_lock;

// define a virtual memory allocator destroyer.
UPF_Virtual_Memory_Allocator::Destroyer 
UPF_Virtual_Memory_Allocator::sm_virtual_memory_allocator_destroyer;

///////////////////////////////////////////////////////////////////////////

size_t 
UPF_Virtual_Memory_Allocator::get_total_allocated_size( void )
{
    size_t total_allocated_size = 0;
    
    UPF_Virtual_Memory_Allocator_Impl * allocator_impl = 0;

    // get permanent allocator's size.
    allocator_impl = sm_toplevel_allocators[ UPF_TOPLEVEL_PERMANENT_ALLOCATOR ];

    if ( 0 != allocator_impl )
    {
        total_allocated_size += allocator_impl->get_total_allocated_size();
    }

    // get discardable allocator's size.
    allocator_impl = sm_toplevel_allocators[ UPF_TOPLEVEL_DISCARDABLE_ALLOCATOR ];

    if ( 0 != allocator_impl )
    {
        total_allocated_size += allocator_impl->get_total_allocated_size();
    }

    // return total allocated size.
    return ( total_allocated_size );
}

/////////////////////////////////////////////////////////////////////////

void 
UPF_Virtual_Memory_Allocator::print_memory_info( FILE * fp )
{
    UPF_Virtual_Memory_Allocator_Impl * allocator_impl = 0;

    // print permanent allocator
    allocator_impl = sm_toplevel_allocators[ UPF_TOPLEVEL_PERMANENT_ALLOCATOR ];

    if ( 0 != allocator_impl )
    {
        allocator_impl->print_memory_info( fp );
    }

    // print discardable allocator
    allocator_impl = sm_toplevel_allocators[ UPF_TOPLEVEL_DISCARDABLE_ALLOCATOR ];

    if ( 0 != allocator_impl )
    {
        allocator_impl->print_memory_info( fp );
    }
}


/////////////////////////////////////////////////////////////////////////

inline void
UPF_Virtual_Memory_Allocator::create_toplevel_allocator( toplevel_allocator_t type )
{
    UPF_Guard< UPF_Thread_Mutex > guard( sm_creator_lock );

    if ( 0 == sm_toplevel_allocators[ type ] )
    {
        sm_toplevel_allocators[ type ] = 
            new UPF_Virtual_Memory_Allocator_Impl( type,
                                                   UPF_MAX_VIRTUAL_MEMORY_ALLOC_SIZE );

    }
}


/////////////////////////////////////////////////////////////////////////


void * 
UPF_Virtual_Memory_Allocator::allocate( size_t               size,
                                        toplevel_allocator_t type )
{
    DO_GET_TOPLEVEL_ALLOCATOR( type );

    return ( sm_toplevel_allocators[type]->allocate( size ) );
}

/////////////////////////////////////////////////////////////////////////

void 
UPF_Virtual_Memory_Allocator::deallocate( void *               ptr, 
                                          size_t               size,
                                          toplevel_allocator_t type )
{
    DO_GET_TOPLEVEL_ALLOCATOR( type );

    sm_toplevel_allocators[type]->deallocate( ptr, size );
}

/////////////////////////////////////////////////////////////////////////

void * 
UPF_Virtual_Memory_Allocator::reallocate( void *               ptr, 
                                          size_t               new_size,
                                          toplevel_allocator_t type )
{
    UPF_Memory_Pool_Util::report("Virtaull Memory Allocator reallocate not implemented!");
    
    return 0;
}

/////////////////////////////////////////////////////////////////////////

bool 
UPF_Virtual_Memory_Allocator::set_managed_memory_size( toplevel_allocator_t type,
                                                       size_t               managed_memory_size )
{
    DO_GET_TOPLEVEL_ALLOCATOR( type );

    return ( sm_toplevel_allocators[type]->set_managed_memory_size( 
                 managed_memory_size )
            );
}

/////////////////////////////////////////////////////////////////////////

bool 
UPF_Virtual_Memory_Allocator::reclaim( toplevel_allocator_t type )
{
    DO_GET_TOPLEVEL_ALLOCATOR( type );

    return ( sm_toplevel_allocators[type]->reclaim() );
}

/////////////////////////////////////////////////////////////////////////

bool 
UPF_Virtual_Memory_Allocator::restore( toplevel_allocator_t type )
{
    DO_GET_TOPLEVEL_ALLOCATOR( type );

    return ( sm_toplevel_allocators[type]->restore() );
}


/////////////////////////////////////////////////////////////////////////

bool 
UPF_Virtual_Memory_Allocator::get_dynamic_toplevel_allocator( 
    toplevel_allocator_t & type )
{
    UPF_Guard< UPF_Thread_Mutex > guard( sm_manage_lock );

    if ( sm_first_use_dynamic_allocator )
    {
        // init sm_toplevel_dynamic_allocators_states
        memset( sm_toplevel_dynamic_allocators_states,
                DAS_NONE,
                sizeof( sm_toplevel_dynamic_allocators_states ) /
                sizeof( *sm_toplevel_dynamic_allocators_states ) );

        // set sm_first_use_dynamic_allocator be false for prevent from
        // initialize again.
        sm_first_use_dynamic_allocator = false;
    }

    // find a usable dynamic virtual allocator, find from DAS_UNUSED's 
    // virtual allocator first, then find from DAS_NONE.
    int i;

    for ( i = 0; i < UPF_DYNAMIC_TOPLEVEL_ALLOCATOR_TYPES; ++i )
    {
        if ( DAS_UNUSED == sm_toplevel_dynamic_allocators_states[ i ] )
        {
            assert( sm_toplevel_allocators[ 
                        UPF_STATIC_TOPLEVEL_ALLOCATOR_TYPES + i ] != 0 );

            // restore allocator's memory request.
            sm_toplevel_allocators[ 
                UPF_STATIC_TOPLEVEL_ALLOCATOR_TYPES + i ]->restore();

            break;
        }
        else if ( DAS_NONE == sm_toplevel_dynamic_allocators_states[ i ] )
        {
            // create virtual allocator impl
            toplevel_allocator_t type_index = 
                UPF_STATIC_TOPLEVEL_ALLOCATOR_TYPES + i;

            assert( sm_toplevel_allocators[ type_index ] == 0 );

            if ( 0 == sm_toplevel_allocators[ type_index ] )
            {
                create_toplevel_allocator( type_index );
            }

            break;
        }
    }

    if ( i >= UPF_DYNAMIC_TOPLEVEL_ALLOCATOR_TYPES )
    {
        return false;
    }
    else
    {
        sm_toplevel_dynamic_allocators_states[ i ] = DAS_USED;

        type = ( UPF_STATIC_TOPLEVEL_ALLOCATOR_TYPES + i );

        return true;
    }

}

/////////////////////////////////////////////////////////////////////////

bool 
UPF_Virtual_Memory_Allocator::release_dynamic_toplevel_allocator( 
    toplevel_allocator_t type )
{
    UPF_Guard< UPF_Thread_Mutex > guard( sm_manage_lock );

    assert( type >= UPF_STATIC_TOPLEVEL_ALLOCATOR_TYPES &&
            type < UPF_TOPLEVEL_ALLOCATOR_TYPES  &&
            sm_toplevel_allocators[ type ] != 0 );

    int dynamic_allocator_index = 
        type - UPF_STATIC_TOPLEVEL_ALLOCATOR_TYPES;

    if ( 
         ( DAS_USED !=
           sm_toplevel_dynamic_allocators_states[ dynamic_allocator_index ] 
         ) ||
         ( 0 == sm_toplevel_allocators[ type ] )
       )
    {
        return false;
    }

    if ( false == sm_toplevel_allocators[ type ]->reclaim() )
    {
        return false;
    }

    sm_toplevel_dynamic_allocators_states[ dynamic_allocator_index ] = DAS_UNUSED;

    return true;
}
