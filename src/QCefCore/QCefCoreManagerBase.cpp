#include "public/QCefCoreManagerBase.h"
#include "QCefCoreManagerBaseImpl.h"
#include "public/CefCoreBrowser.h"
#include "public/QCefJsCallbacks.h"
#include "public/QCefJavaScriptEngine.h"

QCefCoreManagerBase* QCefCoreManagerBase::m_instance = nullptr;

QCefCoreManagerBase::QCefCoreManagerBase(QObject* parent /*= nullptr*/)
{
    qRegisterMetaType<JavaScriptCallbacksCollection>();
    m_instance = this;
}

QCefCoreManagerBase::~QCefCoreManagerBase()
{

}

QCefCoreManagerBase* QCefCoreManagerBase::get()
{
    Q_ASSERT(m_instance);
    return m_instance;
}

void QCefCoreManagerBase::init(bool bWindowless, bool bForceEnableDevTool)
{
    _pImpl = doImplInit();
    _pImpl->init(bWindowless, bForceEnableDevTool);

    connect(_pImpl, &QCefCoreManagerBaseImpl::sig_removeBrowserNotify, this, &QCefCoreManagerBase::sig_removeBrowserNotify, Qt::QueuedConnection);
    connect(_pImpl, &QCefCoreManagerBaseImpl::sig_browserAddedNotify, this, &QCefCoreManagerBase::sig_browserAddedNotify, Qt::QueuedConnection);
    connect(_pImpl, &QCefCoreManagerBaseImpl::sig_notifyBrowserWindowIntergrate, this, &QCefCoreManagerBase::sig_notifyBrowserWindowIntergrate, Qt::QueuedConnection);

    connect(_pImpl, &QCefCoreManagerBaseImpl::sig_loadingStateChanged, this, &QCefCoreManagerBase::sig_loadingStateChanged, Qt::QueuedConnection);
    connect(_pImpl, &QCefCoreManagerBaseImpl::sig_loadStart, this, &QCefCoreManagerBase::sig_loadStart, Qt::QueuedConnection);
    connect(_pImpl, &QCefCoreManagerBaseImpl::sig_loadEnd, this, &QCefCoreManagerBase::sig_loadEnd, Qt::QueuedConnection);
    connect(_pImpl, &QCefCoreManagerBaseImpl::sig_loadError, this, &QCefCoreManagerBase::sig_loadError, Qt::QueuedConnection);
    connect(_pImpl, &QCefCoreManagerBaseImpl::sig_notifyFullScreenModeChanged, this, &QCefCoreManagerBase::sig_notifyFullScreenModeChanged, Qt::QueuedConnection);

    connect(_pImpl, &QCefCoreManagerBaseImpl::sig_allBrowserClosed, this, &QCefCoreManagerBase::sig_allBrowserClosed, Qt::QueuedConnection);
}

bool QCefCoreManagerBase::createBrowser(const QString& webId, const QString& url, const QString& shmName,
    HWND hParentWnd, QMap<QString, QString> extraData)
{
    return _pImpl->createBrowser(webId, url, shmName, hParentWnd, extraData);
}

void QCefCoreManagerBase::deInit()
{
    _pImpl->deInit();
}

QSharedPointer<CefCoreBrowser> QCefCoreManagerBase::getCoreBrowser(int browserId)
{
    return _pImpl->getCoreBrowser(browserId).dynamicCast<CefCoreBrowser>();
}

QSharedPointer<CefCoreBrowser> QCefCoreManagerBase::getCoreBrowser(const QString& webId)
{
    return _pImpl->getCoreBrowser(webId).staticCast<CefCoreBrowser>();
}

void QCefCoreManagerBase::closeAllBrowsers(bool bForce)
{
    _pImpl->closeAllBrowsers(bForce);
}

bool QCefCoreManagerBase::setCookie(const QString& pchHostname, const QString& pchKey, const QString& pchValue, const QString& pchPath /*= "/"*/, float nExpires /*= 0*/, bool bSecure /*= false*/, bool bHTTPOnly /*= false*/)
{
    if (pchHostname.isEmpty() || pchKey.isEmpty() || pchValue.isEmpty()) {
        //TRACEE("param error.");
        return false;
    }

    auto cookieMgr = CefCookieManager::GetGlobalManager(nullptr);
    if (!cookieMgr) {
        //TRACEE("cookie manager not found.");
        return false;
    }
    CefCookie cookie;
    CefString(&cookie.domain) = pchHostname.toStdWString();
    CefString(&cookie.name) = pchKey.toStdWString();
    CefString(&cookie.value) = pchValue.toStdWString();
    if (!pchPath.isEmpty()) {
        CefString(&cookie.path) = pchPath.toStdWString();
    }
    if (nExpires != 0) {
        cookie.has_expires = true;
        cef_time_from_doublet(nExpires, &cookie.expires);
    }
    cookie.secure = bSecure;
    cookie.httponly = bHTTPOnly;
    cookieMgr->SetCookie(pchHostname.toStdWString(), cookie, nullptr);
    return true;
}

bool QCefCoreManagerBase::clearJavaScriptCallback(const QString& jsCallbackSignatures)
{
    int browserId = 0;
    qint64 frameId = 0;
    if (!getSignatureIdentifier(jsCallbackSignatures, browserId, frameId))
        return false;
    auto browserInfo = getCoreBrowser(browserId);
    if (!browserInfo)
        return false;
    LockCoreBrowser(browserInfo);
    if (browserInfo->isClosing()) //关闭中，不再执行清理callback操作
        return false;
    browserInfo->clearJavaScriptCallback(frameId, jsCallbackSignatures);
    return true;
}

bool QCefCoreManagerBase::invokeJavaScriptCallback(const QString& jsCallbackSignature, const QVariantList& params)
{
    int browserId = 0;
    qint64 frameId = 0;
    if (!getSignatureIdentifier(jsCallbackSignature, browserId, frameId))
        return false;
    auto browserInfo = getCoreBrowser(browserId);
    if (!browserInfo)
        return false;
    LockCoreBrowser(browserInfo);
    if (browserInfo->isClosing()) //关闭中则不再执行callback
        return false;
    browserInfo->invokeJavaScriptCallback(frameId, jsCallbackSignature, params);
    return true;
}

JavaScriptCallbacksCollection QCefCoreManagerBase::genJavaScriptCallbackCollection(const QString& signatures)
{
    QVector<QSharedPointer<JavaScriptCallback>> callbacks;
    QStringList signaturesList = getCallbackSignatureList(signatures);
    for (const QString& sig : signaturesList) {
        callbacks << QSharedPointer<JavaScriptCallback>(new JavaScriptCallback(sig, this));
    }
    return JavaScriptCallbacksCollection(callbacks);
}