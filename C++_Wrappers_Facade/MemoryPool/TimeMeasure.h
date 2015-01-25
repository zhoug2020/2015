/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : TimeMeasure.h                                                                              *
*       CREATE DATE     : 2007-11-14                                                                                 *
*       MODULE          : Time measure                                                                               *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]![CN]                                                                                  *
*********************************************************************************************************************/
#ifndef TIME_MEASURE_H
#define TIME_MEASURE_H

#include <string>
#include <stdio.h>
#include <windows.h>         /* for GetTickCount */

#pragma warning( disable : 4035 )

struct UPF_Cpu_Cycle_Measure
{
    static void start_time()
    {
        sm_start_cycle_count = get_cycle_count();
    }

    static void stop_time()
    {
        sm_end_cycle_count = get_cycle_count();
    }

    static const std::string get_measure_result( void )
    {
        char buf[ 100 ];

        sprintf( buf,  "%0.0f cpu cycles",
                (double) ( sm_end_cycle_count - sm_start_cycle_count ) );

        return buf;
    }

private:

    static __int64 get_cycle_count( void )
    {
#if 0
        __asm
        {
            rdtsc
        }
#endif 
    }

    static __int64 sm_start_cycle_count;
    static __int64 sm_end_cycle_count;    
};

#pragma warning( default : 4035 )

struct UPF_Milliseconds_Measure
{
    static void start_time()
    {
        sm_start_tick_count = GetTickCount();
    }

    static void stop_time()
    {
        sm_end_tick_count = GetTickCount();
    }

    static const std::string get_measure_result( void )
    {
        char buf[ 100 ];

        sprintf( buf,  "%d milliseconds",
                (int) ( sm_end_tick_count - sm_start_tick_count ) );

        return buf;
    }

private:

    static DWORD sm_start_tick_count;
    static DWORD sm_end_tick_count;    
};


#endif /* TIME_MEASURE_H */

