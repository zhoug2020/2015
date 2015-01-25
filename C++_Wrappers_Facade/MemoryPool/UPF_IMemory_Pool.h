/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_IMemory_Pool.h                                                                         *
*       CREATE DATE     : 2007-11-12                                                                                 *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]�ڴ�ؽӿڵĶ���![CN]                                                                  *
*********************************************************************************************************************/
#ifndef UPF_IMEMORY_POOL_H
#define UPF_IMEMORY_POOL_H

#include <stddef.h>        /* for size_t */

class UPF_IMemory_Pool_Info;

/** 
 * @class UPF_IMemory_Pool 
 *  
 * @brief UPF_IMemory_PoolΪ�ڴ�صĽӿڣ��������ڴ��ʵ������Ҫʵ������ӿ�������ķ���. 
 */
class UPF_IMemory_Pool
{
public:
    /** 
     * ���ڴ���з����ڴ�.
     * 
     * @param size    [in] ��Ҫ������ڴ�Ĵ�С, ���sizeΪ0, ��ôĬ�Ϸ���1�ֽڵ��ڴ�.
     * 
     * @retval һ��ָ����������ڴ��ָ��    �����ڴ�ɹ�. 
     * @retval 0                             �����ڴ�ʧ��. 
     */
    virtual void * allocate( size_t size ) = 0;

    /** 
     * allocate�ĵ��԰汾, ��Ҫ���ڸ��ٵ�ǰ�����ڴ��λ�ã���������
     * �ڴ�й¶�ļ��.
     * 
     * @param size          [in] ��Ҫ������ڴ�Ĵ�С, ���sizeΪ0, \n
     *                           ��ôĬ�Ϸ���1�ֽڵ��ڴ�.
     * @param file_name     [in] ����allocate_debug��Դ�ļ���
     * @param line_no       [in] ����allocate_debug���к�
     * 
     * @retval һ��ָ����������ڴ��ָ��    �����ڴ�ɹ�. 
     * @retval 0                             �����ڴ�ʧ��. 
     */
    virtual void * allocate_debug( size_t       size, 
                                   const char * file_name,
                                   int          line_no ) = 0;

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
     * 
     * @retval һ��ָ����������ڴ��ָ��    ���������ڴ�ɹ�. 
     * @retval 0                             ���������ڴ�ʧ��, ptr��ָ����ڴ�\n
     *                                       �����ͷ�, ptr��Ȼ��Ч.
     * 
     */
    virtual void * reallocate( void * ptr,
                               size_t new_size ) = 0;

    /** 
     * reallocate�ĵ��԰汾, ��Ҫ���ڸ��ٵ�ǰ�����ڴ��λ�ã���������
     * �ڴ�й¶�ļ��.
     * 
     * @param ptr           [in] ����allocate��reallocate��������ڴ�.
     * @param new_size      [in] �µ��ڴ��С
     * @param file_name     [in] ����reallocate_debug��Դ�ļ���
     * @param line_no       [in] ����reallocate_debug���к�
     * 
     * @retval һ��ָ����������ڴ��ָ��    ���������ڴ�ɹ�. 
     * @retval 0                             ���������ڴ�ʧ��.
     * 
     */
    virtual void * reallocate_debug( void *       ptr,
                                     size_t       new_size,
                                     const char * file_name,
                                     int          line_no ) = 0;

    /** 
     * �ͷ��ڴ�.
     * 
     * @param ptr     [in] ����allocate��������ڴ��ָ��.
     * @param size    [in] ptr��ָ����ڴ�Ĵ�С.
     */
    virtual void deallocate( void * ptr, size_t size ) = 0;

    /** 
     * �ж��ڴ���Ƿ�������ڴ����.
     * 
     * @param ptr     [in] һ��ָ���ڴ��ĵ�ַ.
     * 
     * @retval true  �ڴ������ڸ��ڴ����.
     * @retval false �ڴ�鲻�����ڸ��ڴ����.
     */
    virtual bool is_memory_block_exist( void * ptr ) = 0;

    /** 
     * ��ȡָ��UPF_IMemory_Pool_Infoʵ����ָ��.
     *  
     * @return ָ��UPF_IMemory_Pool_Infoʵ����ָ��, ��ǰʵ��ȷ������ֵ�϶���Ϊ0
     */
    virtual UPF_IMemory_Pool_Info * get_memory_pool_info( void ) = 0;

public:
    virtual ~UPF_IMemory_Pool() { }
        
};

#endif /* UPF_IMEMORY_POOL_H */

