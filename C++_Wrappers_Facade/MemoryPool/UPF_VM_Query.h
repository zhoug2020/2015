/*----------------Copyright(C) 2008 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_VM_Query.h                                                                             *
*       CREATE DATE     : 2008-6-26                                                                                  *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]��ѯ�����ڴ�ҳ��״̬![CN]                                                            *
*********************************************************************************************************************/
#ifndef UPF_VM_QUERY_H
#define UPF_VM_QUERY_H

#include <windows.h>

/**
 * @file UPF_VM_Query.h
 *
 * @details ��ѯ�����ڴ�ҳ��״̬, 
 *          ������ Jeffrey Richter��ʦ������<<WINDOWS���ı��>>��VMQuery.
 *
 */


typedef struct {
   // Region information
   PVOID  pvRgnBaseAddress;
   DWORD  dwRgnProtection;  // PAGE_*
   SIZE_T RgnSize;
   DWORD  dwRgnStorage;     // MEM_*: Free, Image, Mapped, Private
   DWORD  dwRgnBlocks;
   DWORD  dwRgnGuardBlks;   // If > 0, region contains thread stack
   BOOL   fRgnIsAStack;     // TRUE if region contains thread stack

   // Block information
   PVOID  pvBlkBaseAddress;
   DWORD  dwBlkProtection;  // PAGE_*
   SIZE_T BlkSize;
   DWORD  dwBlkStorage;     // MEM_*: Free, Reserve, Image, Mapped, Private
} VMQUERY, *PVMQUERY;



///////////////////////////////////////////////////////////////////////////////

BOOL VMQuery(LPCVOID pvAddress, PVMQUERY pVMQ);

void ConstructRgnInfoLine( PVMQUERY pVMQ, PSTR szLine, int nMaxLen );

void ConstructBlkInfoLine( PVMQUERY pVMQ, PSTR szLine, int nMaxLen );

#endif /* UPF_VM_QUERY_H */
