#pragma region qt_headers
#include <QVBoxLayout>
#include <QWindow>
#include <QResizeEvent>
#include <QVariant>
#include <QUuid>
#include <QMutex>
#include <QShortcut>
#include <QKeySequence>
#include <QPoint>
#pragma endregion qt_headers

#pragma region cef_headers
#include <include/cef_app.h>
#include <include/cef_browser.h>
#include <include/cef_frame.h>
#include <include/cef_sandbox_win.h>
#pragma endregion cef_headers

#include <QCefProtocol.h>


#include "public/QCefView.h"
#include "CefViewBrowserApp/QCefBrowserHandlerBase.h"
#include "CCefWindowHook.h"
#include "QCefViewPrivate.h"
#include "CCefSetting.h"
#include "public/QCefWebEngine.h"
#include "include/QCefJavaScriptEngine.h"
#include "include/CefCoreBrowser.h"

#include <QMouseEvent>
#include <QLayout>


static QMap<HWND, LPVOID> g_hwndDataMap;
static QMutex g_hwndDataMutex;

QCefView::QCefView(const QString url, const QString& webId, QWidget* parent /*= 0*/, int width /*= 0*/, int height /*= 0*/)
    : QWidget(parent)
    , pPrivate_(nullptr)
{
    setMouseTracking(true);
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet("background-color: #ff252529");
    auto mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    setLayout(mainLayout);

    pPrivate_ = new QCefViewPrivate(this, url, webId);
    this->layout()->setContentsMargins(0, 0, 0, 0);
    connect(pPrivate_, &QCefViewPrivate::sig_initializeCefGui, this, [=](WId winId) {
        //notify cef initialize
        QWindow* pWindow = QWindow::fromWinId(winId);
        QWidget* pContainer = createWindowContainer(pWindow, this);
        //这一句是必须的，用于解决窗口刚打开的时候页面标题跳动问题，给窗口一个合适的默认宽高
        if (width && height)
            pContainer->setGeometry(0, 0, width, height);
        if (!layout()->isEmpty()) {
            layout()->removeItem(layout()->itemAt(0));
        }
        layout()->addWidget(pContainer);
        layout()->update();
    }, Qt::QueuedConnection);

    connect(pPrivate_, &QCefViewPrivate::sig_loadingStateChanged, this, &QCefView::loadingStateChanged);
    connect(pPrivate_, &QCefViewPrivate::sig_loadStart, this, &QCefView::loadStart);
    connect(pPrivate_, &QCefViewPrivate::sig_loadEnd, this, &QCefView::loadEnd);
    connect(pPrivate_, &QCefViewPrivate::sig_loadError, this, &QCefView::loadError);
    connect(pPrivate_, &QCefViewPrivate::sig_notifyFullScreenModeChanged, this, &QCefView::sig_notifyFullScreenModeChanged);
}

QCefView::QCefView(QWidget* parent /*= 0*/)
    : QCefView("about:blank", "", parent)
{}

QCefView::~QCefView()
{
    if (pPrivate_) {
        pPrivate_ = nullptr;
    }

    disconnect();
}

WId QCefView::getCefWinId()
{
    if (pPrivate_)
        return pPrivate_->getCefWinId();

    return 0;
}

void QCefView::browserLoadUrl(const QString& url)
{
    if (pPrivate_)
        pPrivate_->browserLoadUrl(url);
}

bool QCefView::browserCanGoBack()
{
    if (pPrivate_)
        return pPrivate_->browserCanGoBack();

    return false;
}

bool QCefView::browserCanGoForward()
{
    if (pPrivate_)
        return pPrivate_->browserCanGoForward();

    return false;
}

void QCefView::browserGoBack()
{
    if (pPrivate_)
        pPrivate_->browserGoBack();
}

void QCefView::browserGoForward()
{
    if (pPrivate_)
        pPrivate_->browserGoForward();
}

bool QCefView::browserIsLoading()
{
    if (pPrivate_)
        return pPrivate_->browserIsLoading();

    return false;
}

void QCefView::browserReload()
{
    if (pPrivate_)
        pPrivate_->browserReload();
}

void QCefView::browserStopLoad()
{
    if (pPrivate_)
        pPrivate_->browserStopLoad();
}

QString QCefView::getUrl()
{
    return pPrivate_->getUrl();
}

int QCefView::getBrowserId()
{
    int browserId = -1;
    if (pPrivate_)
        browserId = pPrivate_->getBrowserId();
    return browserId;
}

QString QCefView::getWebId()
{
    QString webId;
    if (pPrivate_)
        webId = pPrivate_->getWebId();
    return webId;
}

void QCefView::execJavaScript(const QString& javaScriptCode, const QString& scriptUrl, int startLine)
{
    if (pPrivate_)
        pPrivate_->execJavaScript(javaScriptCode, scriptUrl, startLine);
}

void QCefView::resizeEvent(QResizeEvent * event)
{
    pPrivate_->onToplevelWidgetMoveOrResize();
}

bool QCefView::setInputEventHook()
{
    return pPrivate_->setInputEventHook();
}

double QCefView::getZoomLevel()
{
    return pPrivate_->getZoomLevel();
}

void QCefView::setZoomLevel(double level)
{
    pPrivate_->setZoomLevel(level);
}

void QCefView::mouseMoveEvent(QMouseEvent *event)
{
    QWidget::mouseMoveEvent(event);
}

void QCefView::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
}

void QCefView::mouseReleaseEvent(QMouseEvent *event)
{
    QWidget::mouseReleaseEvent(event);
}