#include "QCefCoreManagerImpl.h"
#include "QCefCoreBrowser.h"
#include "QCefBrowserHandler.h"


QCefCoreManagerImpl::QCefCoreManagerImpl(QObject* parent /*= nullptr*/)
    :QCefCoreManagerBaseImpl(parent)
{
}

CefRefPtr<QCefBrowserHandlerBase> QCefCoreManagerImpl::doInitCefBrowserHandler()
{
    return new QCefBrowserHandler();
}

QSharedPointer<QCefCoreBrowserBase> QCefCoreManagerImpl::doCreateCoreBrowser(const CefRefPtr<CefBrowser>& browser, const QString& webId, const QString& shmName, QMap<QString, QString> extraData, QObject* parent)
{
    Q_UNUSED(shmName);
    return QSharedPointer<QCefCoreBrowserBase>(new QCefCoreBrowser(browser, webId, extraData, nullptr));
}

bool QCefCoreManagerImpl::modifyCefConfig(CefSettings& settings)
{
    //do nothing.
    return true;
}
