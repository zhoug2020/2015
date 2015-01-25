/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Memory_Pool_Util.cpp                                                                   *
*       CREATE DATE     : 2007-11-22                                                                                 *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]�ڴ�ع������ʵ��![CN]                                                                *
*********************************************************************************************************************/

#include "UPF_Memory_Pool_Util.h"
#include <assert.h>
#include <stdio.h>
#include <windows.h>
#include <stdarg.h>
#include <string.h>

#include "..\..\OS_Adapter\UPF_OS.h"
#include "..\UPF_Thread_Mutex.h"

#ifdef UPF_OS_IS_WINCE
#define DUBHE_LOG_TXT "/Storage Card/UPF_Output.txt"
#endif

#ifdef UPF_OS_IS_WINNT
#define DUBHE_LOG_TXT "C:/UPF_Output.txt"
#endif

#define TRACE_TYPE_NULL						(0)
#define TRACE_TYPE_DEBUGGER					(1)
#define TRACE_TYPE_FILE_SYNC				(2)
#define TRACE_TYPE_FILE_NOSYNC				(3)
#define TRACE_TYPE_DEBUGGER_FILE_SYNC		(4)
#define TRACE_TYPE_DEBUGGER_FILE_NOSYNC		(5)

#if defined(_DEBUG) || defined(DEBUG)
#  define TRACE_TYPE (TRACE_TYPE_DEBUGGER_FILE_SYNC)
#else
#  define TRACE_TYPE (TRACE_TYPE_DEBUGGER_FILE_SYNC)
#endif /* _DEBUG */

// ʱ���,������,����ʱ���ϵ�����ļ��ĺ���ֱ�ӵ�����OS API,�Ժ���Ҫ�޸�
void	mos_printf(const char *format, ...)
{
	// ��ʱ����
	return;
}
void 	mos_break_program(int abort_code)
{
  //	::DebugBreak();
}

class UPF_Dubhe_File_Tracer
{
public:
	UPF_Dubhe_File_Tracer():m_log_handle(0)
	{
		m_log_handle = fopen(DUBHE_LOG_TXT,"a+c");
		if(m_log_handle)
		{
			fprintf(m_log_handle,"--------------[Begin]--------------------\n");
		}
	
	}
	~UPF_Dubhe_File_Tracer()
	{
			if(m_log_handle)
			{
				fclose(m_log_handle);
				m_log_handle = 0;
			}
	}

	void trace(const char * buffer)
	{
		if(m_log_handle)
			fprintf(m_log_handle,buffer);
	}

	void sync()
	{
		if(m_log_handle)
			fflush(m_log_handle);
	}

static	UPF_Dubhe_File_Tracer * instance()
	{
	      static UPF_Dubhe_File_Tracer * sl_instance = 0;
		   
		   if ( 0 == sl_instance )
		   {
			   UPF_Guard<UPF_Thread_Mutex> guard( sm_lock );
			   
			   if ( 0 == sl_instance )
			   {
				   sl_instance = new UPF_Dubhe_File_Tracer;
				   
			   }
		   }
		   
		   return sl_instance;
	}
private:
	FILE * m_log_handle;

	static UPF_Thread_Mutex sm_lock;

};
UPF_Thread_Mutex UPF_Dubhe_File_Tracer::sm_lock;


template<unsigned int TraceType>
struct trace_helper;

template<>
struct trace_helper<TRACE_TYPE_NULL>
{
	static  void trace(const char * buffer){}
};



template<>
struct trace_helper<TRACE_TYPE_DEBUGGER>
{
	static void trace(const char * buffer)
	{
		UPF_OS::trace(buffer);
	}

};


template<>
struct trace_helper<TRACE_TYPE_FILE_SYNC>
{
	static void trace(const char * buffer)
	{
		UPF_Dubhe_File_Tracer::instance()->trace(buffer);
		UPF_Dubhe_File_Tracer::instance()->sync();
	}
};

template<>
struct trace_helper<TRACE_TYPE_FILE_NOSYNC>
{
	static void trace(const char * buffer)
	{
		UPF_Dubhe_File_Tracer::instance()->trace(buffer);
	}
};

template<>
struct trace_helper<TRACE_TYPE_DEBUGGER_FILE_SYNC>
{
	static void trace(const char * buffer)
	{
		UPF_OS::trace(buffer);
		UPF_Dubhe_File_Tracer::instance()->trace(buffer);
		UPF_Dubhe_File_Tracer::instance()->sync();
	}

};

template<>
struct trace_helper<TRACE_TYPE_DEBUGGER_FILE_NOSYNC>
{
	static void trace(const char * buffer)
	{
		UPF_OS::trace(buffer);
		UPF_Dubhe_File_Tracer::instance()->trace(buffer);
	}

};

static void sg_osftl_trace(const char *format, ...)
{	
	char buf[512];

	va_list ap;
	va_start (ap, format);

	vsprintf (buf, format, ap);
	
	trace_helper<TRACE_TYPE>::trace(buf);

	va_end (ap);
}

// Inline function definitions.

/** 
 * ����size���������free small block list�е���Ӧ����ֵ.
 *  
 * @param size                [in]  ָ�����ڴ��С, ����1 <= size && size <= MAX_SMALL_BLOCK_SIZE
 * @param align_value         [out] ����������ͬʱ, ������Ӧ��Small Block����ֵ.
 * 
 * @return һ������ֵ, ��Χ�� 0 <= index < MAX_SMALL_BLOCKS_INDEX
 */
int 
UPF_Memory_Pool_Util::get_free_small_block_index( size_t   size,
                                                  size_t & align_value )
{
    assert( 1 <= size && size <= MAX_SMALL_BLOCK_SIZE );

    /*
     ��sm_small_block_index_table�в��Ҵ���ָ����size����Сhigh_block_sizeֵ���ڵ�
     item, �Ӹ�item����Ϣ���������Ӧ������.
     ���㹫ʽ: 
     ����ֵ = ��һ��������ȵ�high_block_index + 
              ((size + align_value) & ~align_value) >> right_shift_bits

     ������ڼ����������ƫ������ֵʱ(Ҳ����
     ((size + align_value) & align_value) >> right_shift_bits)
     ����Ӧ����UPF_MEMORY_ALIGN(size, ��������) >> right_shift_bits
     ��UPF_MEMORY_ALIGN�Ķ���Ϊ:
     #define UPF_MEMORY_ALIGN(size, boundary) ((size) + (boundary) - 1) & ~((boundary) - 1))
     ����Ϊʡȥ��1�Ĳ�������align_value��Ϊ��Ӧ�ķ�������ֵ��1.
    */
    int index = -1;

    for ( int i = 1; i < SMALL_BLOCK_INDEX_TABLE_SIZE; ++i )
    {
        if ( size <= sm_small_block_index_table[ i ].high_block_size )
        {
            size_t offset_size      = (size - sm_small_block_index_table[ i - 1 ].high_block_size);
            size_t right_shift_bits = sm_small_block_index_table[ i ].shift_bits;

            // set align_value, used to calculate below and used to return client yet.
            align_value = sm_small_block_index_table[ i ].align_value;

            int offset_index = UPF_MEMORY_ALIGN_LACK_ONE(offset_size, align_value) >> right_shift_bits;

            index = sm_small_block_index_table[ i - 1 ].high_block_index + offset_index;

            break;
        }
    }

    // ����������������� 0 <= ����ֵ < MAX_SMALL_BLOCKS_INDEX��
    assert( 0 <= index && index < MAX_SMALL_BLOCKS_INDEX );

    return index;
}

/** 
 * ����index���������free small block list�е���Ӧsizeֵ
 *  
 * @param index    [in]  ָ����free small block list�е�����ֵ,
 *                       ��Χ�� 0 <= inde < MAX_SMALL_BLOCKS_INDEX
 * 
 * @return һ��sizeֵ, ��Χ�� MIN_ALLOC_GRANULARITY <= size <= MAX_SMALL_BLOCK_SIZE
 */
size_t 
UPF_Memory_Pool_Util::get_free_small_block_size( int index )
{
    assert( 0 <= index && index < MAX_SMALL_BLOCKS_INDEX );

    size_t size = 0;

    for ( int i = 1; i < SMALL_BLOCK_INDEX_TABLE_SIZE; ++i )
    {
        if ( index <= sm_small_block_index_table[ i ].high_block_index )
        {
            int offset_index        = ( index - sm_small_block_index_table[ i - 1 ].high_block_index);            
            size_t left_shift_bits  = sm_small_block_index_table[ i ].shift_bits;

            size_t offset_size =  offset_index << left_shift_bits;

            size = sm_small_block_index_table[ i - 1 ].high_block_size + offset_size;

            break;
        }
    }

    // �������sizeֵ������ MIN_ALLOC_GRANULARITY <= sizeֵ <= MAX_SMALL_BLOCK_SIZE��
    assert( MIN_ALLOC_GRANULARITY <= size && size <= MAX_SMALL_BLOCK_SIZE );

    return size;
}

/** 
 * �������free small block list�е���ӽ�size����Ӧ����ֵ.
 *  
 * @param size               [in]  ָ�����ڴ��С, ����1 <= size && size <= MAX_SMALL_BLOCK_SIZE
 * @param block_size         [out] ����������ͬʱ, ������ӽ�size��Small Block ��С.
 * 
 * @return һ������ֵ, ��Χ�� 0 <= index < MAX_SMALL_BLOCKS_INDEX
 */
int 
UPF_Memory_Pool_Util::get_nearest_free_small_block_index( size_t   size, 
                                                          size_t & block_size )
{
    assert( 1 <= size && size <= MAX_SMALL_BLOCK_SIZE );

    int    index       = -1;
	size_t align_value = 0;

    block_size = 0;

    for ( int i = 1; i < SMALL_BLOCK_INDEX_TABLE_SIZE; ++i )
    {
        if ( size <= sm_small_block_index_table[ i ].high_block_size )
        {
            size_t offset_size      = (size - sm_small_block_index_table[ i - 1 ].high_block_size);
            size_t right_shift_bits = sm_small_block_index_table[ i ].shift_bits;

            align_value = sm_small_block_index_table[ i ].align_value;

            size_t aligned_offset_size = UPF_MEMORY_ALIGN_LACK_ONE(offset_size, align_value);

            // set block_size, used to return client.
            block_size = 
                sm_small_block_index_table[ i - 1 ].high_block_size + aligned_offset_size;

            int offset_index = aligned_offset_size >> right_shift_bits;

            index = sm_small_block_index_table[ i - 1 ].high_block_index + offset_index;

			// size less than or equal to block_size.
			// if size less than block_size, then decrement index, and adjust block_size value.
            if ( size < block_size )
            {
                --index;
				block_size -= (sm_small_block_index_table[ i ].align_value + 1);

            }

            break;
        }
    }

    // ����������������� 0 <= ����ֵ < MAX_SMALL_BLOCKS_INDEX��
    assert( 0 <= index && index < MAX_SMALL_BLOCKS_INDEX );

    return index;    
}

/** 
 * ����size, ����sm_small_block_index_table, ��ȡ����Ӧ�ķ�������
 * 
 * @param size       [in]    ������ڴ��С, ���� 1 <= size <=MAX_SMALL_BLOCK_SIZE
 * 
 * @return ��sm_small_block_index_table����ָ����size���Ӧ�ķ�������
 */
size_t 
UPF_Memory_Pool_Util::get_allocate_granularity( size_t size )
{
    assert( 1 <= size && size <= MAX_SMALL_BLOCK_SIZE );

    for ( int i = 1; i < SMALL_BLOCK_INDEX_TABLE_SIZE; ++i )
    {
        if ( size <= sm_small_block_index_table[ i ].high_block_size )
        {
            // because align_value within sm_small_block_index_table equal to
            // allocate granularity - 1, so when return, 
            // need the value that eqaul to align_value + 1.
            return ( sm_small_block_index_table[ i ].align_value + 1 );
        }
    }

    assert( 0 );

    return 0xffffffff;
}



// Static member defines.

/**
 * sm_small_block_index_tableΪ�˼���ķ�������룬
 * Ҳ�ɲ��ã�����Ҫ����һЩ������Ӱ��Ч��.
 * �˱���Ҫ�Ǹ����±����:
 * <br><pre>
 * -------------------------------------------
 * |  ��������  |   ��������    |  ����Χ   |
 * -------------------------------------------
 * |  1-32      |      8        |   1-256     | 
 * |  33-48     |      16       |  257-512    |
 * |  49-64     |      32       |  513-1024   | 
 * |  65-80     |      64       | 1025-2048   |
 * |  81-96     |     128       | 2049-4096   |
 * | 97-112     |     256       | 4097-8192   |
 * | 113-128    |     512       | 8193-16384  |
 * -------------------------------------------
 * </pre>
 */
const UPF_Memory_Pool_Util::Small_Block_Index_Table 
UPF_Memory_Pool_Util::
sm_small_block_index_table[ SMALL_BLOCK_INDEX_TABLE_SIZE ] = {
    { -1,  0,     0,   0 },
    { 31,  256,   7,   3 },   
    { 47,  512,   15,  4 },
    { 63,  1024,  31,  5 },
    { 79,  2048,  63,  6 },
    { 95,  4096,  127, 7 },
    { 111, 8192,  255, 8 }
};

/** 
 * ��ʾ����ϵͳ�Ĵ�����Ϣ.
 * 
 * @todo ���������������win32��, �Ժ��Ƶ���.
 * 
 *  error_msg_fmt     [in]    ������Ϣ��ʽ��
 */
void
UPF_Memory_Pool_Util::display_os_error( const char * error_msg_fmt, ... )
{
    const int CUSTOM_MSG_BUF_SIZE = 100;

    LPVOID lpMsgBuf = NULL;

    if ( GetLastError() != 0 )
    {
        if ( ! FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | 
            FORMAT_MESSAGE_FROM_SYSTEM | 
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            GetLastError(),
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
            (LPTSTR) &lpMsgBuf,
            0,
            NULL ))
        {           
            UPF_Memory_Pool_Util::report( "FormatMessage error" );
            return;
        }

    }
    
    char    custom_msg_buffer[ CUSTOM_MSG_BUF_SIZE + 1 ];
    va_list arg_list;

    va_start( arg_list, error_msg_fmt );

    _vsnprintf( custom_msg_buffer, 
                CUSTOM_MSG_BUF_SIZE, 
                error_msg_fmt, 
                arg_list );

    custom_msg_buffer[ CUSTOM_MSG_BUF_SIZE ] = '\0';

    va_end( arg_list );

    char error_msg_buf[ BUFSIZ + 1 ];
    
    if ( NULL != lpMsgBuf )
    {
        _snprintf( error_msg_buf, BUFSIZ, "%s\nDetails: %s", custom_msg_buffer, lpMsgBuf );
    }
    else
    {
        strncpy( error_msg_buf, custom_msg_buffer, BUFSIZ );
    }
    
    error_msg_buf[ BUFSIZ ] = '\0';
    
    UPF_Memory_Pool_Util::report( error_msg_buf );

    // Free the buffer.
    LocalFree( lpMsgBuf );

}

/** 
 * traceһ����Ϣ
 * 
 * @param fmt     [in]      message��ʽ��
 * 
 */
void 
UPF_Memory_Pool_Util::trace( const char * fmt, ... )
{
    va_list arg_list;

    va_start( arg_list, fmt );
    char buffer[ BUFSIZ + 1 ];

    _vsnprintf( buffer, BUFSIZ, fmt, arg_list );

    buffer[ BUFSIZ ] = '\0';

    va_end( arg_list );

//    OutputDebugString( UPF_A2T(buffer) );
}


/** 
 * ����һ����Ϣ
 * 
 * @param fmt     [in]      message��ʽ��
 * 
 */
void 
UPF_Memory_Pool_Util::report( const char * fmt, ... )
{
    va_list arg_list;

    va_start( arg_list, fmt );
    char buffer[ BUFSIZ + 1 ];

    _vsnprintf( buffer, BUFSIZ, fmt, arg_list );

    buffer[ BUFSIZ ] = '\0';

    va_end( arg_list );

#ifdef UPF_OS_IS_WINNT
    MessageBox( NULL, 
                UPF_A2T(buffer), 
                UPF_TEXT("UPF MemoryPool Report"), 
                MB_OK );
#else
    sg_osftl_trace( "UPF MemoryPool Report: %s\n", buffer );
#endif

}


const char * 
UPF_Memory_Pool_Util::num_to_string( unsigned int number, char * buf )
{
    wchar_t  num_buf[ 50 ];
    wchar_t  fmt_buf[ 100 ];

    swprintf( num_buf, L"%u", number );

    NUMBERFMTW nf;

    nf.NumDigits     = 0;
    nf.LeadingZero   = FALSE;
    nf.Grouping      = 3;
    nf.lpDecimalSep  = L".";
    nf.lpThousandSep = L",";
    nf.NegativeOrder = 0;

    GetNumberFormatW( LOCALE_USER_DEFAULT, 
                      0, 
                      num_buf, 
                      &nf, 
                      fmt_buf, 
                      100 );

    WideCharToMultiByte( GetACP(), 
                         0,
                         fmt_buf,
                         -1,
                         buf,
                         100,
                         NULL,
                         NULL );

    return ( buf );

}


void 
UPF_Memory_Pool_Util::print_memory_size_headers( FILE * fp )
{
    assert( fp );

    fprintf( fp, 
             "%-22s %-10s %-10s %-10s %-10s %-10s %-15s %s\n",
             "MemPoolName",
             "Managed",
             "Allocated",
             "Peak",
             "Free",
             "Wasted",
             "Limit",
             "IsExceedLimit" );

    print_chars( fp, 120, '-' );
}


void 
UPF_Memory_Pool_Util::print_memory_size_infos( FILE *       fp,
                                               const char * name,
                                               size_t       managed_memory_size,
                                               size_t       allocated_memory_size,
                                               size_t       allocated_memory_peak,
                                               size_t       wasted_memory_size,
                                               size_t       allocated_memory_limit )
{
    assert( fp );

    char buf[ 100 ];

    // output name
    fprintf( fp, "%-22s ", name );

    // output managed memory size
    fprintf( fp, "%-10s ", num_to_string( managed_memory_size, buf ) );

    // output allocated memory size
    fprintf( fp, "%-10s ", num_to_string( allocated_memory_size, buf ) );

    // output allocated memory peak
    fprintf( fp, "%-10s ", num_to_string( allocated_memory_peak, buf ) );

    // output free memory size
    size_t free_memory_size = (managed_memory_size - allocated_memory_size - wasted_memory_size);

    if ( free_memory_size > managed_memory_size )
    {
        fprintf( fp, "%-10s ", "NA" );
    }
    else
    {
        fprintf( fp, "%-10s ", num_to_string( free_memory_size , buf ) );
    }

    // output wasted memory size
    fprintf( fp, "%-10s ", num_to_string( wasted_memory_size, buf ) );

    // output allocated memory limit
    fprintf( fp, "%-15s ", num_to_string( allocated_memory_limit, buf ) );

    // output is limit.
    fprintf( fp, 
             "%s\n", 
             ( allocated_memory_size >= allocated_memory_limit ? "Y" : "N" ) );

}


void 
UPF_Memory_Pool_Util::print_line( FILE * fp,
                                  int    width )
{
    assert( fp );
    
    print_chars( fp, width, '_' );

}


void 
UPF_Memory_Pool_Util::print_framed_caption( FILE *        fp,
                                            const char *  caption, 
                                            int           width,
                                            int           gap_width,
                                            const char    border_char )
{
    assert( fp );

    int caption_length = strlen( caption );
    int caption_needed_width = (caption_length + 2 * gap_width + 2);

    if ( width < caption_needed_width )
    {
        width = caption_needed_width;
    }

    // calculate left pos and right pos that want to print.
    int left  = ( width - caption_needed_width ) / 2;

    // start print
    int line_count;

    for ( line_count = 0; line_count < 3; ++line_count )
    {
        // print left space.
        print_chars( fp, left, ' ',  false );
        
        if ( line_count == 1 )
        {
            // print caption.
            fputc( border_char, fp );
            print_chars( fp, gap_width, ' ', false );

            fprintf( fp, "%s", caption );

            print_chars( fp, gap_width, ' ', false );
            fputc( border_char, fp );

        }
        else
        {
            // print  border line.
            print_chars( fp, caption_needed_width, border_char, false );

        }

        // print line char.
        fputc( '\n', fp );

    }
}


void 
UPF_Memory_Pool_Util::print_lined_caption( FILE *        fp,
                                           const char *  caption, 
                                           int           width,
                                           int           gap_width,
                                           const char    line_char )
{
    assert( fp );

    int caption_length = strlen( caption );
    int caption_needed_width = (caption_length + 2 * gap_width + 2);

    if ( width < caption_needed_width )
    {
        width = caption_needed_width;
    }

    // calculate left pos and right pos that want to print.
    int left  = ( width - caption_needed_width ) / 2;

    print_chars( fp, left,      line_char, false );
    print_chars( fp, gap_width, ' ',       false );

    fputs( caption, fp );

    print_chars( fp, gap_width, ' ',       false );
    print_chars( fp, left,      line_char, false );

    fputc( '\n', fp );
}