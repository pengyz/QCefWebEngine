#include "public/QCefCoreManager.h"
#include "QCefCoreManagerImpl.h"

QCefCoreManager* QCefCoreManager::m_instance = nullptr;


QCefCoreManager * QCefCoreManager::get()
{
    if (!m_instance)
        m_instance = new QCefCoreManager();
    return m_instance;
}

QCefCoreManagerBaseImpl* QCefCoreManager::doImplInit()
{
    return new QCefCoreManagerImpl(this);
}

