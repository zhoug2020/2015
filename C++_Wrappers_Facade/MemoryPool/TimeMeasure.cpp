/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : TimeMeasure.cpp                                                                            *
*       CREATE DATE     : 2007-11-14                                                                                 *
*       MODULE          : Time Measure                                                                               *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]![CN]                                                                                  *
*********************************************************************************************************************/

#include "TimeMeasure.h"

// Static member definitions.
__int64 UPF_Cpu_Cycle_Measure::sm_start_cycle_count = 0;
__int64 UPF_Cpu_Cycle_Measure::sm_end_cycle_count   = 0;

DWORD UPF_Milliseconds_Measure::sm_start_tick_count = 0;
DWORD UPF_Milliseconds_Measure::sm_end_tick_count   = 0;

