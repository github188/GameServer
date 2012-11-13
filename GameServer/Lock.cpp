#include "StdAfx.h"
#include "Lock.h"

/**   @fn  CLock::CLock(void)
 *    @brief ���캯������ʼ���ٽ���
 *    @param void 
 *    @return  
 */
CLock::CLock(void)
{
    InitializeCriticalSection(&m_cs);
}

/**   @fn  CLock::~CLock(void)
 *    @brief ����������ɾ���ٽ���
 *    @param void 
 *    @return  
 */
CLock::~CLock(void)
{
    DeleteCriticalSection(&m_cs);
}

/**   @fn void CLock::Lock()
 *    @brief �����ٽ���
 *    @return void 
 */
void CLock::Lock()
{
    EnterCriticalSection(&m_cs);
}

/**   @fn void CLock::Unlock()
 *    @brief �˳��ٽ���
 *    @return void 
 */
void CLock::Unlock()
{
    LeaveCriticalSection(&m_cs);
}

