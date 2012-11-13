#ifndef Lock_h__
#define Lock_h__
#include <windows.h>

class CLock
{
public:
    CLock(void);
    ~CLock(void);
    void Lock();
    void Unlock();

private:
    CRITICAL_SECTION m_cs;
};

class CAutoLock
{
public:
    CAutoLock(CLock& cs) 
        : m_rCs(cs)
    { 
        m_rCs.Lock(); 
    }
    ~CAutoLock()
    { 
        m_rCs.Unlock(); 
    }
private:
    CLock& m_rCs;
};



#endif // Lock_h__


