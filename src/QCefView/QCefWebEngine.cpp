#include "public/QCefWebEngine.h"
#include "QCefCoreManagerImpl.h"

QCefWebEngine* QCefWebEngine::m_instance = nullptr;


QCefWebEngine * QCefWebEngine::get()
{
    if (!m_instance)
        m_instance = new QCefWebEngine();
    return m_instance;
}

QCefCoreManagerBaseImpl* QCefWebEngine::doImplInit()
{
    return new QCefCoreManagerImpl(this);
}

