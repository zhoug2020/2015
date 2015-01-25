/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Print_Policy.cpp                                                                       *
*       CREATE DATE     : 2007-11-20                                                                                 *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]打印策略类实现![CN]                                                                    *
*********************************************************************************************************************/

#if defined(UPF_OS_IS_WINNT)
/* WINNT platform */
#  include "WinNT/UPF_Print_Policy.cpp"
#elif defined(UPF_OS_IS_WINCE)
/* WINCE platform */
#  include "WinCE/UPF_Print_Policy.cpp"
#endif

