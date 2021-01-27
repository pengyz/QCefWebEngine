#pragma region qt_headers
#include <QUuid>
#include <QLayout>
#include <QWidget>
#pragma endregion qt_headers

#include "include/CefCoreBrowser.h"
#include "QCefViewPrivate.h"
#include "CefViewBrowserApp/QCefBrowserHandlerBase.h"
#include "CCefWindowHook.h"
#include "public/QCefWebEngine.h"




QCefViewPrivate::QCefViewPrivate(QWidget* webViewWidget, const QString& url, const QString& webId)
    : webViewWidget_(webViewWidget)
    , webId_(webId)
    , hookWindow_(nullptr)
    , hookRender_(nullptr)
{
    hookWindow_ = new CCefWindowHook(this);
    hookRender_ = new CCefWindowHook(this);
    if (webId_.isEmpty()) {
        QString strUuid;
        strUuid = QUuid::createUuid().toString().toUpper();
        strUuid = strUuid.mid(1, strUuid.size() - 2);
        strUuid = strUuid.replace("-", "");
        webId_ = QString("WEB_%1").arg(strUuid);
    }
    QCefWebEngine::get()->createBrowser(webId_, url, "", (HWND)webViewWidget->winId(), {});
    connect(QCefWebEngine::get(), &QCefWebEngine::sig_notifyBrowserWindowIntergrate, this, &QCefViewPrivate::onBrowserWindowIntergrate);
    connect(QCefWebEngine::get(), &QCefWebEngine::sig_loadingStateChanged, this, &QCefViewPrivate::onLoadingStateChanged);
    connect(QCefWebEngine::get(), &QCefWebEngine::sig_loadStart, this, &QCefViewPrivate::onLoadStart);
    connect(QCefWebEngine::get(), &QCefWebEngine::sig_loadEnd, this, &QCefViewPrivate::onLoadEnd);
    connect(QCefWebEngine::get(), &QCefWebEngine::sig_loadError, this, &QCefViewPrivate::onLoadError);
    connect(QCefWebEngine::get(), &QCefWebEngine::sig_notifyFullScreenModeChanged, this, &QCefViewPrivate::onFullScreenModeChanged);
}

QCefViewPrivate::~QCefViewPrivate()
{
    auto browserInfo = QCefWebEngine::get()->getCoreBrowser(webId_);
    if (browserInfo) {
        LockCoreBrowser(browserInfo);
        browserInfo->closeBrowser(true);
    }
}

WId QCefViewPrivate::getCefWinId()
{
    auto browserInfo = QCefWebEngine::get()->getCoreBrowser(webId_);
    if (browserInfo) {
        LockCoreBrowser(browserInfo);
        return (WId)browserInfo->getBrowserWnd();
    }
    return 0;
}

void QCefViewPrivate::onBrowserWindowIntergrate(const QString& webId, int browserId, qint32 wnd)
{
    if (webId_ == webId) {
        emit sig_initializeCefGui((WId)wnd);
    }
}

void QCefViewPrivate::browserLoadUrl(const QString& url)
{
    auto browserInfo = QCefWebEngine::get()->getCoreBrowser(webId_);
    if (browserInfo) {
        LockCoreBrowser(browserInfo);
        browserInfo->browserLoadUrl(url);
    }
}

bool QCefViewPrivate::browserCanGoBack()
{
    auto browserInfo = QCefWebEngine::get()->getCoreBrowser(webId_);
    if (browserInfo) {
        LockCoreBrowser(browserInfo);
        browserInfo->browserCanGoBack();
    }
    return false;
}

bool QCefViewPrivate::browserCanGoForward()
{
    auto browserInfo = QCefWebEngine::get()->getCoreBrowser(webId_);
    if (browserInfo) {
        LockCoreBrowser(browserInfo);
        browserInfo->browserCanGoForward();
    }
    return false;
}

void QCefViewPrivate::browserGoBack()
{
    auto browserInfo = QCefWebEngine::get()->getCoreBrowser(webId_);
    if (browserInfo) {
        LockCoreBrowser(browserInfo);
        browserInfo->browserGoBack();
    }
}

void QCefViewPrivate::browserGoForward()
{
    auto browserInfo = QCefWebEngine::get()->getCoreBrowser(webId_);
    if (browserInfo) {
        LockCoreBrowser(browserInfo);
        browserInfo->browserGoForward();
    }
}

bool QCefViewPrivate::browserIsLoading()
{
    auto browserInfo = QCefWebEngine::get()->getCoreBrowser(webId_);
    if (browserInfo) {
        LockCoreBrowser(browserInfo);
        browserInfo->browserIsLoading();
    }
    return false;
}

void QCefViewPrivate::browserReload()
{
    auto browserInfo = QCefWebEngine::get()->getCoreBrowser(webId_);
    if (browserInfo) {
        LockCoreBrowser(browserInfo);
        browserInfo->browserReload();
    }
}

void QCefViewPrivate::browserStopLoad()
{
    auto browserInfo = QCefWebEngine::get()->getCoreBrowser(webId_);
    if (browserInfo) {
        LockCoreBrowser(browserInfo);
        browserInfo->browserStopLoad();
    }
}

QString QCefViewPrivate::getUrl()
{
    auto browserInfo = QCefWebEngine::get()->getCoreBrowser(webId_);
    if (browserInfo) {
        LockCoreBrowser(browserInfo);
        return browserInfo->browserGetUrl();
    }
    return "";
}

void QCefViewPrivate::notifyMoveOrResizeStarted()
{
    auto browserInfo = QCefWebEngine::get()->getCoreBrowser(webId_);
    if (browserInfo) {
        LockCoreBrowser(browserInfo);
        browserInfo->notifyMoveOrResizeStarted();
    }
}

bool QCefViewPrivate::sendObjectRegisterMessage(const QString& regName, const QObject* registerObject)
{
    return false;
}

void QCefViewPrivate::onToplevelWidgetMoveOrResize() { notifyMoveOrResizeStarted(); }

int QCefViewPrivate::getBrowserId()
{
    auto browserInfo = QCefWebEngine::get()->getCoreBrowser(webId_);
    if (browserInfo) {
        LockCoreBrowser(browserInfo);
        return browserInfo->getBrowserId();
    }
    return 0;
}

QString QCefViewPrivate::getWebId()
{
    return webId_;
}

void QCefViewPrivate::execJavaScript(const QString& javaScriptCode, const QString& scriptUrl, int startLine)
{
    auto browserInfo = QCefWebEngine::get()->getCoreBrowser(webId_);
    if (browserInfo) {
        LockCoreBrowser(browserInfo);
        browserInfo->execJavaScript(javaScriptCode, scriptUrl, startLine);
    }
}

bool QCefViewPrivate::setInputEventHook()
{
    hookWindow_->installWinProcHook((HWND)getCefWinId(), (HWND)webViewWidget_->winId(), "Chrome_WidgetWin_0"); //filter WM_LBUTTONUP
    hookRender_->installWinProcHook((HWND)getCefWinId(), (HWND)webViewWidget_->winId(), "Chrome_RenderWidgetHostHWND"); //filter WM_LBUTTONDOWN
    return true;
}

double QCefViewPrivate::getZoomLevel()
{
    auto browserInfo = QCefWebEngine::get()->getCoreBrowser(webId_);
    if (!browserInfo)
        return 0.0;
    LockCoreBrowser(browserInfo);
    return browserInfo->getZoomLevel();
}

void QCefViewPrivate::setZoomLevel(double level)
{
    auto browserInfo = QCefWebEngine::get()->getCoreBrowser(webId_);
    if (!browserInfo)
        return;
    LockCoreBrowser(browserInfo);
    browserInfo->setZoomLevel(level);
}

void QCefViewPrivate::onLoadingStateChanged(int browserId, bool isLoading, bool canGoBack, bool canGoForward)
{
    auto browserInfo = QCefWebEngine::get()->getCoreBrowser(webId_);
    if (!browserInfo)
        return;
    int currBrowserId = 0;
    {
        LockCoreBrowser(browserInfo);
        currBrowserId = browserInfo->getBrowserId();
    }
    if (currBrowserId == browserId) {
        emit sig_loadingStateChanged(isLoading, canGoBack, canGoForward);
    }
}

void QCefViewPrivate::onLoadStart(int browserId)
{
    auto browserInfo = QCefWebEngine::get()->getCoreBrowser(webId_);
    if (!browserInfo)
        return;
    int currBrowserId = 0;
    {
        LockCoreBrowser(browserInfo);
        currBrowserId = browserInfo->getBrowserId();
    }
    if (currBrowserId == browserId) {
        emit sig_loadStart();
    }
}

void QCefViewPrivate::onLoadEnd(int browserId, int httpStatusCode)
{
    auto browserInfo = QCefWebEngine::get()->getCoreBrowser(webId_);
    if (!browserInfo)
        return;
    int currBrowserId = 0;
    {
        LockCoreBrowser(browserInfo);
        currBrowserId = browserInfo->getBrowserId();
    }
    if (currBrowserId == browserId) {
        emit sig_loadEnd(httpStatusCode);
    }
}

void QCefViewPrivate::onLoadError(int browserId, int errorCode, QString url)
{
    auto browserInfo = QCefWebEngine::get()->getCoreBrowser(webId_);
    if (!browserInfo)
        return;
    int currBrowserId = 0;
    {
        LockCoreBrowser(browserInfo);
        currBrowserId = browserInfo->getBrowserId();
    }
    if (currBrowserId == browserId) {
        emit sig_loadError(errorCode, url);
    }
}

void QCefViewPrivate::onFullScreenModeChanged(int browserId, bool fullscreen)
{
    auto browserInfo = QCefWebEngine::get()->getCoreBrowser(webId_);
    if (!browserInfo)
        return;
    int currBrowserId = 0;
    {
        LockCoreBrowser(browserInfo);
        currBrowserId = browserInfo->getBrowserId();
    }
    if (currBrowserId == browserId) {
        emit sig_notifyFullScreenModeChanged(fullscreen);
    }
}
