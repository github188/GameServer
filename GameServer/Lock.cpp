#include "StdAfx.h"
#include "Lock.h"

/**   @fn  CLock::CLock(void)
 *    @brief 构造函数，初始化临界区
 *    @param void 
 *    @return  
 */
CLock::CLock(void)
{
    InitializeCriticalSection(&m_cs);
}

/**   @fn  CLock::~CLock(void)
 *    @brief 析构函数，删除临界区
 *    @param void 
 *    @return  
 */
CLock::~CLock(void)
{
    DeleteCriticalSection(&m_cs);
}

/**   @fn void CLock::Lock()
 *    @brief 进入临界区
 *    @return void 
 */
void CLock::Lock()
{
    EnterCriticalSection(&m_cs);
}

/**   @fn void CLock::Unlock()
 *    @brief 退出临界区
 *    @return void 
 */
void CLock::Unlock()
{
    LeaveCriticalSection(&m_cs);
}

