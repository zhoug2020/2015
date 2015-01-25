/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : Benchmark.h                                                                                *
*       CREATE DATE     : 2007-11-13                                                                                 *
*       MODULE          :                                                                                            *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]测量时间，只用于x86![CN]                                                               *
*********************************************************************************************************************/

#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <iostream>      /* for cout, endl */
#include <iomanip>       /* for setw */

#include <cmath>         /* for log10 */

#include <string>
#include <list>
#include <memory>

class UPF_ITime_Measure_Item
{
public:
    // get test item name.
    virtual const char * get_item_name() const = 0;

	virtual void do_before_execute()
	{
	}

	virtual void do_after_execute()
	{
	}

    // execute test
    virtual void do_execute( void ) const = 0;

	virtual ~UPF_ITime_Measure_Item() {}
};

template < class TimeMeasure >
class UPF_Benchmark
{
public:
    UPF_Benchmark( int test_count = 1 )
        : m_test_count( test_count )
    {
    }

    void add_measure_item( UPF_ITime_Measure_Item * measure_item )
    {

        m_measure_items.push_back( 
            std::auto_ptr< UPF_ITime_Measure_Item >( measure_item ) );
    }

    void run_benchmark( void )
    {
        Measure_Item_Iter measure_iter = m_measure_items.begin();

        for ( ; measure_iter != m_measure_items.end(); ++measure_iter )
        {

			(*measure_iter)->do_before_execute();

			for ( int i = 0; i < m_test_count; ++i )
			{

				TimeMeasure::start_time();
				
				(*measure_iter)->do_execute();
				
				TimeMeasure::stop_time();
				
				if ( m_test_count > 1 )
				{
					std::cout << std::right 
						      << std::setw( (int) log10( m_test_count ) + 1 );

					std::cout << i + 1 << ". ";
				}				

				std::cout << std::left << std::setw( 20 )
					      << (*measure_iter)->get_item_name()
						  << std::right << std::setw( 30 )
					      << TimeMeasure::get_measure_result() << std::endl;
			}

			(*measure_iter)->do_after_execute();

			std::cout << std::setfill( '=' ) 
				      << std::setw( 60 ) << "" 
					  << std::setfill( ' ' ) << std::endl;
        }
        
    }

private:
    typedef std::list< std::auto_ptr< UPF_ITime_Measure_Item > > Measure_Item_List;
    typedef Measure_Item_List::iterator                          Measure_Item_Iter;

    Measure_Item_List m_measure_items;
    int               m_test_count;
};

struct UPF_Cpu_Cycle_Measure;
struct UPF_Milliseconds_Measure;

typedef UPF_Benchmark< UPF_Cpu_Cycle_Measure >    UPF_Cpu_Cycle_Benchmark;
typedef UPF_Benchmark< UPF_Milliseconds_Measure > UPF_Milliseconds_Benchmark;


#endif /* BENCHMARK_H */

