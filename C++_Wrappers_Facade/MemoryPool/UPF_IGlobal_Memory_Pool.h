
/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_IGlobal_Memory_Pool.h                                                                  *
*       CREATE DATE     : 2008-7-18                                                                                  *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]ȫ���ڴ�ؽӿڵĶ���![CN]                                                              *
*********************************************************************************************************************/
#ifndef UPF_IGLOBAL_MEMORY_POOL_H
#define UPF_IGLOBAL_MEMORY_POOL_H

#include <stddef.h>        /* for size_t */

/** 
 * @class UPF_IGlobal_Memory_Pool 
 *  
 * @brief UPF_IGlobal_Memory_PoolΪȫ���ڴ�صĽӿڣ�������ȫ���ڴ��ʵ������Ҫʵ������ӿ�������ķ���. 
 */
class UPF_IGlobal_Memory_Pool
{
public:
    /** 
     * ���ڴ���з����ڴ�.
     * 
     * @param size     [in] ��Ҫ������ڴ�Ĵ�С, ���sizeΪ0, ��ôĬ�Ϸ���1�ֽڵ��ڴ�.
     * @param hint_ptr [in] ����һ�������ptr, ���ڴ�����������Ϣ.
     * 
     * @retval һ��ָ����������ڴ��ָ��    �����ڴ�ɹ�. 
     * @retval 0                             �����ڴ�ʧ��. 
     */
    virtual void * allocate( size_t size, void * hint_ptr ) = 0;


    /** 
     * ���ڴ�������·����ڴ�, ������ԭ�е����ݣ��������ڴ�ʱ�������ԭ����λ��
     * �����㹻���ڴ���Է���, ����о͵ط���.
     * 
     * �������������:
     *      -# ���ptrΪ0, �����allocate���з����ڴ�.
     *      -# ���ptr��Ϊ0, new_sizeΪ0, �����deallocate�ͷ��ڴ�.
     *      -# ���ptr��Ϊ0, new_size��Ϊ0, ��������·����ڴ����, �����ԭ\n
     *         λ���п��õ��ڴ棬����о͵ط��䣬�������·���һ���ڴ�, ����ԭ\n
     *         ʼ�ڴ�����ݿ�������������ڴ���, ���ͷ�ԭ�����ڴ�.
     * 
     * @note ����·����ڴ�ʧ��, ԭ���ڴ沢���ͷ�, ԭ����ָ����Ȼ��Ч.
     * 
     * 
     * @param ptr           [in] ����allocate��reallocate��������ڴ�.
     * @param new_size      [in] �µ��ڴ��С
     * @param hint_ptr      [in] ����һ�������ptr, ���ڴ�����������Ϣ.
     * 
     * @retval һ��ָ����������ڴ��ָ��    ���������ڴ�ɹ�. 
     * @retval 0                             ���������ڴ�ʧ��, ptr��ָ����ڴ�\n
     *                                       �����ͷ�, ptr��Ȼ��Ч.
     * 
     */
    virtual void * reallocate( void *       ptr,
                               size_t       new_size,
                               void *       hint_ptr ) = 0;

    /** 
     * �ͷ��ڴ�.
     * 
     * @param ptr        [in] ����allocate��������ڴ��ָ��.
     * @param size       [in] ptr��ָ����ڴ�Ĵ�С.
     * @param hint_ptr   [in] ����һ�������ptr, ���ڴ�����������Ϣ.
     * 
     */
    virtual void deallocate( void *       ptr, 
                             size_t       size,
                             void *       hint_ptr ) = 0;


public:
    virtual ~UPF_IGlobal_Memory_Pool() { }
        
};

#endif /* UPF_IGLOBAL_MEMORY_POOL_H */

