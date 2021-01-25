#include "QCefCoreBrowserBase.h"
#include "include/tracer.h"
#include "include/assertutil.h"
#include "QCefProtocol.h"

#include <QVariant>
#include <include/cef_task.h>
#include <include/base/cef_bind.h>
#include <include/wrapper/cef_closure_task.h>


QCefCoreBrowserBase::QCefCoreBrowserBase(const CefRefPtr<CefBrowser>& browser, const QString& webId, QMap<QString, QString> extraData, QObject* parent)
    :QObject(parent), _webId(webId), _browser(browser), _extraData(extraData)
{
}

const QString& QCefCoreBrowserBase::getTitle() const
{
    return _title;
}

void QCefCoreBrowserBase::setTitle(const QString& title)
{
    _title = title;
}

const QPoint& QCefCoreBrowserBase::getMousePoint() const
{
    return _mousePoint;
}

void QCefCoreBrowserBase::setMousePoint(const QPoint& point)
{
    _mousePoint = point;
}

const QSize& QCefCoreBrowserBase::getSize() const
{
    return _size;
}

void QCefCoreBrowserBase::setSize(const QSize& rect)
{
    _size = rect;
}

float QCefCoreBrowserBase::getDeviceScaleFactor() const
{
    return _deviceScaleFactor;
}

void QCefCoreBrowserBase::setDeviceScaleFactor(float scaleFactor)
{
    _deviceScaleFactor = scaleFactor;
}

bool QCefCoreBrowserBase::isMouseDown() const
{
    return _mouseDown;
}

void QCefCoreBrowserBase::setMouseDown(bool bDown)
{
    _mouseDown = bDown;
}

double QCefCoreBrowserBase::getScrollX() const
{
    return _scrollX;
}

void QCefCoreBrowserBase::setScrollX(double scrollX)
{
    _scrollX = scrollX;
}

double QCefCoreBrowserBase::getScrollY() const
{
    return _scrollY;
}

void QCefCoreBrowserBase::setScrollY(double scrollY)
{
    _scrollY = scrollY;
}

int QCefCoreBrowserBase::getBrowserId() const
{
    if (_browser)
        return _browser->GetIdentifier();
    return 0;
}

const QString& QCefCoreBrowserBase::getWebId() const
{
    return _webId;
}

void QCefCoreBrowserBase::setWebId(const QString& webId)
{
    _webId = webId;
}

HWND QCefCoreBrowserBase::getBrowserWnd() const
{
    if (_browser)
        return _browser->GetHost()->GetWindowHandle();
    return 0;
}

void QCefCoreBrowserBase::addHeader(const QString& key, const QString& value)
{
    headers.emplace(key, value);
}

const std::multimap<QString, QString>& QCefCoreBrowserBase::getHeaders()
{
    return headers;
}

bool QCefCoreBrowserBase::tryLock()
{
    return _objMutex.tryLock(100);
}

void QCefCoreBrowserBase::lock()
{
    return _objMutex.lock();
}

void QCefCoreBrowserBase::unlock()
{
    _objMutex.unlock();
}

const QString QCefCoreBrowserBase::getExtraData(const QString& key)
{
    return _extraData[key];
}

CefCoreBrowser::BrowserType QCefCoreBrowserBase::browserType()
{
    QString strValue = getExtraData(QCEF_SURFACE_TYPE_NAME);
    if (strValue == QCEF_SURFACE_TYPE_VALUE_OVERLAY) {
        return CefCoreBrowser::BrowserType::e_OverlayBrowser;
    } else if (strValue == QCEF_SURFACE_TYPE_VALUE_SDK) {
        return CefCoreBrowser::BrowserType::e_HtmlSurfaceBrowser;
    }
    return CefCoreBrowser::BrowserType::e_NormalBrowser;
}


bool QCefCoreBrowserBase::browserLoadUrl(const QString& url)
{
    if (!_browser)
        return false;
    CefString strUrl;
    strUrl.FromString(url.toStdString());
    _browser->GetMainFrame()->LoadURL(strUrl);
    return true;
}

bool QCefCoreBrowserBase::browserPostUrl(const QString& url, const QByteArray& postBytes)
{
    if (!_browser)
        return false;
    CefRefPtr<CefRequest> request = CefRequest::Create();
    CefRefPtr<CefPostData> postData = CefPostData::Create();
    CefRefPtr<CefPostDataElement> element = CefPostDataElement::Create();
    element->SetToBytes(postBytes.length(), postBytes.data());
    postData->AddElement(element);
    CefRequest::HeaderMap headerMap;
    request->Set(url.toStdWString(), "POST", postData, headerMap);
    CefRefPtr<CefFrame> mainFrame = _browser->GetMainFrame();
    if (mainFrame) {
        mainFrame->LoadRequest(request);
        return true;
    }
    return false;
}

bool QCefCoreBrowserBase::browserCanGoBack()
{
    if (!_browser)
        return false;
    return _browser->CanGoBack();
}

bool QCefCoreBrowserBase::browserCanGoForward()
{
    if (!_browser)
        return false;
    return _browser->CanGoForward();
}

bool QCefCoreBrowserBase::browserGoBack()
{
    if (!_browser)
        return false;
    _browser->GoBack();
    return true;
}

bool QCefCoreBrowserBase::browserGoForward()
{
    if (!_browser)
        return false;
    _browser->GoForward();
    return true;
}

bool QCefCoreBrowserBase::browserIsLoading()
{
    if (!_browser)
        return false;
    return _browser->IsLoading();
}

bool QCefCoreBrowserBase::browserReload()
{
    if (!_browser)
        return false;
    _browser->Reload();
    return true;
}

bool QCefCoreBrowserBase::browserStopLoad()
{
    if (!_browser)
        return false;
    _browser->StopLoad();
    return false;
}

QString QCefCoreBrowserBase::browserGetUrl()
{
    QString strCurrUrl;
    if (!_browser)
        return false;

    strCurrUrl = QString::fromStdWString(_browser->GetMainFrame()->GetURL().ToWString());
    return strCurrUrl;
}

bool QCefCoreBrowserBase::notifyMoveOrResizeStarted()
{
    if (!_browser)
        return false;
    CefRefPtr<CefBrowserHost> host = _browser->GetHost();
    if (host) {
        host->NotifyMoveOrResizeStarted();
        return true;
    }
    return false;
}

bool QCefCoreBrowserBase::invokeJavaScriptCallback(qint64 frameId, const QString& jsCallbackSignature, QVariantList params)
{
    if (!_browser)
        return false;

    CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create(QCEF_INVOKENGLCALLBACK);
    auto paramValue = msg->GetArgumentList();
    int idx = 0;
    paramValue->SetString(idx++, jsCallbackSignature.toStdWString());
    for (const auto& value : params) {
        QVariant::Type vType = value.type();
        if (vType == QVariant::Type::String) {
            paramValue->SetString(idx++, value.toString().toStdWString());
        } else if (vType == QVariant::Type::Int) {
            paramValue->SetInt(idx++, value.toInt());
        } else if (vType == QVariant::Type::Bool) {
            paramValue->SetBool(idx++, value.toBool());
        } else if (vType == QVariant::Type::Double) {
            paramValue->SetDouble(idx++, value.toDouble());
        } else if (vType == QVariant::Type::ByteArray) {
            QByteArray data = value.toByteArray();
            if (!data.isEmpty()) {
                auto cefBinaryVal = CefBinaryValue::Create(data.constData(), data.size());
                paramValue->SetBinary(idx++, cefBinaryVal);
            } else {
                paramValue->SetNull(idx++);
            }
        } else {
            paramValue->SetNull(idx);
        }
    }
    CefRefPtr<CefFrame> frame = _browser->GetFrame(frameId);
    if (!frame) {
        TRACEE("browserId: %d get frame by frameId: %ld failed !", getBrowserId(), frameId);
        return false;
    }
    frame->SendProcessMessage(CefProcessId::PID_RENDERER, msg);
    return true;
}

bool QCefCoreBrowserBase::clearJavaScriptCallback(qint64 frameId, const QString& jsCallbackSignature)
{
    if (!_browser)
        return false;

    CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create(QCEF_CLEARNGLCALLBACKS);
    auto paramValue = msg->GetArgumentList();
    int idx = 0;
    paramValue->SetString(idx++, jsCallbackSignature.toStdWString());
    CefRefPtr<CefFrame> frame = _browser->GetFrame(frameId);
    if (!frame) {
        TRACEE("browserId: %d get frame by frameId: %ld failed !", getBrowserId(), frameId);
        return false;
    }
    frame->SendProcessMessage(CefProcessId::PID_RENDERER, msg);
    return true;
}

bool QCefCoreBrowserBase::execJavaScript(const QString& javaScriptCode, const QString& scriptUrl, int startLine)
{
    if (!_browser)
        return false;
    _browser->GetMainFrame()->ExecuteJavaScript(javaScriptCode.toStdWString(), scriptUrl.toStdWString(), startLine);
    return true;
}

bool QCefCoreBrowserBase::closeBrowser(bool bForce)
{
    if (!_browser)
        return false;
    _browser->GetHost()->CloseBrowser(bForce);
    return true;
}

bool QCefCoreBrowserBase::resizeBrowser()
{
    if (!_browser)
        return false;
    //if (!CefCurrentlyOn(TID_UI)) {
    //    // Execute this method on the UI thread.
    //    CefPostTask(TID_UI, base::Bind(&CefBrowserHost::WasResized, _browser->GetHost()));
    //    return true;
    //} else {
    _browser->GetHost()->WasResized();
    //}
    return true;
}

bool QCefCoreBrowserBase::resizeBrowser(int width, int height)
{
    if (!_browser)
        return false;
    setSize(QSize(width, height));
    return resizeBrowser();
}

bool QCefCoreBrowserBase::getLinkAtPosition(int x, int y)
{
    if (!_browser)
        return false;
    CefRefPtr<CefProcessMessage> outMsg = CefProcessMessage::Create(QCEF_GET_LINK_AT_POSITION);
    auto args = outMsg->GetArgumentList();
    args->SetSize(2);
    args->SetInt(0, x);
    args->SetInt(1, y);
    _browser->GetMainFrame()->SendProcessMessage(PID_RENDERER, outMsg);
    return true;
}

bool QCefCoreBrowserBase::copy()
{
    if (!_browser)
        return false;
    CefRefPtr<CefFrame> focusFrame = _browser->GetFocusedFrame();
    if (focusFrame) {
        focusFrame->Copy();
        return true;
    }
    return false;
}


bool QCefCoreBrowserBase::past()
{
    if (!_browser)
        return false;
    CefRefPtr<CefFrame> focusFrame = _browser->GetFocusedFrame();
    if (focusFrame) {
        focusFrame->Paste();
        return true;
    }
    return false;
}

bool QCefCoreBrowserBase::cut()
{
    if (!_browser)
        return false;
    CefRefPtr<CefFrame> focusFrame = _browser->GetFocusedFrame();
    if (focusFrame) {
        focusFrame->Cut();
        return true;
    }
    return false;
}

bool QCefCoreBrowserBase::undo()
{
    if (!_browser)
        return false;
    CefRefPtr<CefFrame> focusFrame = _browser->GetFocusedFrame();
    if (focusFrame) {
        focusFrame->Undo();
        return true;
    }
    return false;
}

bool QCefCoreBrowserBase::redo()
{
    if (!_browser)
        return false;
    CefRefPtr<CefFrame> focusFrame = _browser->GetFocusedFrame();
    if (focusFrame) {
        focusFrame->Redo();
        return true;
    }
    return false;
}

bool QCefCoreBrowserBase::selectAll()
{
    if (!_browser)
        return false;
    CefRefPtr<CefFrame> focusFrame = _browser->GetFocusedFrame();
    if (focusFrame) {
        focusFrame->SelectAll();
        return true;
    }
    return false;
}

bool QCefCoreBrowserBase::browserSetFocus(bool hasFocus)
{
    if (!_browser)
        return false;
    _browser->GetHost()->SetFocus(hasFocus);
    return true;
}

bool QCefCoreBrowserBase::find(const QString & pchSearchStr, bool bCurrentlyInFind, bool bReverse)
{
    if (!_browser)
        return false;
    ++_findId;
    _browser->GetHost()->Find(0, pchSearchStr.toStdWString(), !bReverse, false, bCurrentlyInFind);
    return true;
}


bool QCefCoreBrowserBase::stopFinding()
{
    if (!_browser)
        return false;
    _browser->GetHost()->StopFinding(true);
    _findId = 0;
    return true;
}

bool QCefCoreBrowserBase::viewSource()
{
    if (!_browser)
        return false;
    auto focusedFrame = _browser->GetFocusedFrame();
    if (focusedFrame) {
        focusedFrame->ViewSource();
        return true;
    }
    return false;
}

bool QCefCoreBrowserBase::setScaleFactor(float scaleFactor)
{
    if (!_browser)
        return false;
    setDeviceScaleFactor(scaleFactor);
    return true;
}

bool QCefCoreBrowserBase::updateHorizontalScroll(double delta)
{
    if (!_browser)
        return false;
    CefRefPtr<CefProcessMessage> outMsg = CefProcessMessage::Create(QCEF_SET_HORIZONTAL_SCROLL);
    auto args = outMsg->GetArgumentList();
    args->SetSize(2);
    args->SetDouble(0, getScrollX() + delta);
    args->SetDouble(1, getScrollY());
    _browser->GetMainFrame()->SendProcessMessage(PID_RENDERER, outMsg);
    return true;
}

bool QCefCoreBrowserBase::updateVerticalScroll(double delta)
{
    if (!_browser)
        return false;
    setScrollY(getScrollY() + delta); //更新scroll y
    QString script = QString::asprintf(u8"if(window.__GDATA__.scrollTo) { window.__GDATA__.scrollTo(0, %1.2lf); } else { window.scrollBy(0, %1.2lf); }", delta, delta);
    return execJavaScript(script, "", 0);
}

bool QCefCoreBrowserBase::setBrowserVisible(bool bVisible)
{
    if (!_browser)
        return false;
    _browser->GetHost()->WasHidden(!bVisible);
    _visible = bVisible;
    return true;
}

bool QCefCoreBrowserBase::isVisible()
{
    return _visible;
}

void QCefCoreBrowserBase::setClosing(bool bClosing)
{
    _isClosing = bClosing;
}

bool QCefCoreBrowserBase::isClosing()
{
    return _isClosing;
}


/**
 * @brief 等待绘制，会暂停当前线程
 * @author Alex.peng
 */
void QCefCoreBrowserBase::waitPaint(const void* buffer, int width, int height, const QVector<QRect>& dirtyRects)
{
    //加锁定，修改，wait
    std::unique_lock<std::mutex> locker(_paintData.paintMutex);
    _paintData.needPaint = true;
    _paintData.buffer = buffer;
    _paintData.width = width;
    _paintData.height = height;
    _paintData.dirtyRects = dirtyRects;
    //则持续等待直到当前帧被消费掉
    while (_paintData.needPaint)
        _paintData.cv.wait(locker);
}

CEFPaintData& QCefCoreBrowserBase::paintData()
{
    return _paintData;
}

double QCefCoreBrowserBase::getZoomLevel()
{
    if (!_browser || !_browser->GetHost())
        return 0.0;
    return _browser->GetHost()->GetZoomLevel();
}

bool QCefCoreBrowserBase::setZoomLevel(double level)
{    
    if (!_browser || !_browser->GetHost())
        return false;
    if (!CefCurrentlyOn(TID_UI)) {
        // Must execute on the UI thread to access member variables.
        CefPostTask(TID_UI, base::Bind(&CefBrowserHost::SetZoomLevel, _browser->GetHost(), level));
        return true;
    }
    return true;
}

///////////////////////////////////
bool QCefCoreBrowserBase::browserMouseEvent(unsigned int message, int x, int y, unsigned int flags)
{
    TRACET();
    if (!_browser)
        return false;
    auto pHost = _browser->GetHost();
    if (NULL == pHost) return false;
    CefMouseEvent mouse_event;
    mouse_event.x = x;
    mouse_event.y = y;
    mouse_event.modifiers = flags;
    TRACED("message = %d,pos = (%d,%d)", message, x, y);
    if (WM_LBUTTONDOWN == message) {
        pHost->SendMouseClickEvent(mouse_event, MBT_LEFT, false, 1);
    } else if (WM_RBUTTONDOWN == message) {
        pHost->SendMouseClickEvent(mouse_event, MBT_RIGHT, false, 1);
    } else if (WM_LBUTTONUP == message) {
        pHost->SendMouseClickEvent(mouse_event, MBT_LEFT, true, 1);
    } else if (WM_LBUTTONDBLCLK == message) {
        pHost->SendMouseClickEvent(mouse_event, MBT_LEFT, false, 2);
    } else if (WM_MOUSEMOVE == message) {
        pHost->SendMouseMoveEvent(mouse_event, false);
    } else if (WM_MOUSELEAVE == message) {
        pHost->SendMouseMoveEvent(mouse_event, true);
    } else {

    }
    return true;
}

bool QCefCoreBrowserBase::browserMouseMove(int x, int y)
{
    if (!_browser)
        return false;
    auto pHost = _browser->GetHost();
    setMousePoint(QPoint(x, y));
    if (NULL == pHost) return false;
    CefMouseEvent mouse_event;
    mouse_event.x = x;
    mouse_event.y = y;
    mouse_event.modifiers = 0;
    if (isMouseDown()) {
        mouse_event.modifiers = EVENTFLAG_LEFT_MOUSE_BUTTON;
    }
    pHost->SendMouseMoveEvent(mouse_event, false);
    return true;
}

bool QCefCoreBrowserBase::browserMouseUp(int type)
{
    // type: MBT_LEFT
    if (!_browser)
        return false;
    auto pHost = _browser->GetHost();
    if (NULL == pHost) return false;
    CefMouseEvent mouse_event;
    mouse_event.x = getMousePoint().x();
    mouse_event.y = getMousePoint().y();
    if (type == CefBrowserHost::MouseButtonType::MBT_LEFT) {
        setMouseDown(false);
        mouse_event.modifiers = EVENTFLAG_LEFT_MOUSE_BUTTON;
    } else if (type == CefBrowserHost::MouseButtonType::MBT_MIDDLE)
        mouse_event.modifiers = EVENTFLAG_MIDDLE_MOUSE_BUTTON;
    else if (type == CefBrowserHost::MouseButtonType::MBT_RIGHT)
        mouse_event.modifiers = EVENTFLAG_RIGHT_MOUSE_BUTTON;
    else
        mouse_event.modifiers = 0;
    pHost->SendMouseClickEvent(mouse_event, (CefBrowserHost::MouseButtonType)type, true, 1);
    return true;
}

bool QCefCoreBrowserBase::browserMouseDown(int type)
{
    if (!_browser)
        return false;
    auto pHost = _browser->GetHost();
    if (NULL == pHost) return false;
    CefMouseEvent mouse_event;
    mouse_event.x = getMousePoint().x();
    mouse_event.y = getMousePoint().y();
    if (type == CefBrowserHost::MouseButtonType::MBT_LEFT) {
        setMouseDown(true);
        mouse_event.modifiers = EVENTFLAG_LEFT_MOUSE_BUTTON;
    } else if (type == CefBrowserHost::MouseButtonType::MBT_MIDDLE)
        mouse_event.modifiers = EVENTFLAG_MIDDLE_MOUSE_BUTTON;
    else if (type == CefBrowserHost::MouseButtonType::MBT_RIGHT)
        mouse_event.modifiers = EVENTFLAG_RIGHT_MOUSE_BUTTON;
    else
        mouse_event.modifiers = 0;
    pHost->SendMouseClickEvent(mouse_event, (CefBrowserHost::MouseButtonType)type, false, 1);
    return true;
}

bool QCefCoreBrowserBase::browserMouseDoubleClick(int type)
{
    if (!_browser)
        return false;
    auto pHost = _browser->GetHost();
    if (NULL == pHost) return false;
    CefMouseEvent mouse_event;
    mouse_event.x = getMousePoint().x();
    mouse_event.y = getMousePoint().y();
    mouse_event.modifiers = EVENTFLAG_LEFT_MOUSE_BUTTON;
    pHost->SendMouseClickEvent(mouse_event, (CefBrowserHost::MouseButtonType)type, false, 2);
    return true;
}

bool QCefCoreBrowserBase::browserMouseWheel(int delta)
{
    if (!_browser)
        return false;
    auto pHost = _browser->GetHost();
    if (NULL == pHost) return false;
    CefMouseEvent mouse_event;
    mouse_event.x = getMousePoint().x();
    mouse_event.y = getMousePoint().y();
    // mouse_event.modifiers = flags;
    TRACED("pos = (%d,%d)", mouse_event.x, mouse_event.y);
    pHost->SendMouseWheelEvent(mouse_event, 0, delta);
    return true;
}

void OnKeyEvent(CefKeyEvent& event, unsigned int virtualKeyCode, unsigned int flags, bool systemKey, cef_key_event_type_t type)
{
    event.windows_key_code = virtualKeyCode;
    event.native_key_code = virtualKeyCode;			// lParam
    event.is_system_key = systemKey;
    event.type = type;
    event.modifiers = flags;
}

bool QCefCoreBrowserBase::browserKeyDown(unsigned int virtualKeyCode, unsigned int flags, bool systemKey)
{
    if (!_browser)
        return false;
    auto pHost = _browser->GetHost();
    CefKeyEvent event;
    OnKeyEvent(event, virtualKeyCode, flags, systemKey, KEYEVENT_KEYDOWN);
    pHost->SendKeyEvent(event);
    return true;
}

bool QCefCoreBrowserBase::browserKeyUp(unsigned int virtualKeyCode, unsigned int flags, bool systemKey)
{
    if (!_browser)
        return false;
    auto pHost = _browser->GetHost();
    CefKeyEvent event;
    OnKeyEvent(event, virtualKeyCode, flags, systemKey, KEYEVENT_KEYUP);
    pHost->SendKeyEvent(event);
    return true;
}

bool QCefCoreBrowserBase::browserKeyPress(unsigned int charCode, unsigned int flags, bool systemKey)
{
    if (!_browser)
        return false;
    auto pHost = _browser->GetHost();
    CefKeyEvent event;
    OnKeyEvent(event, charCode, flags, systemKey, KEYEVENT_CHAR);
    pHost->SendKeyEvent(event);
    return true;
}

/**
 * @brief QCefCoreBrowserLocker Implementation
 * @author Alex.peng
 */
QCefCoreBrowserLocker::QCefCoreBrowserLocker(const QSharedPointer<CefCoreBrowser>& browser, const char* file, const char* function, int line)
    :pBrowserPtr(browser), _file(QString::fromLocal8Bit(file)), _function(function), _line(line)
{
    TRACED("%s: file: %s, function: %s, line: %d", __FUNCTION__, qPrintable(_file), qPrintable(_function), _line);
    Q_ASSERT(pBrowserPtr);
    pBrowserPtr->lock();
}

QCefCoreBrowserLocker::~QCefCoreBrowserLocker()
{
    TRACED("%s: file: %s, function: %s, line: %d", __FUNCTION__, qPrintable(_file), qPrintable(_function), _line);
    Q_ASSERT(pBrowserPtr);
    pBrowserPtr->unlock();
}
