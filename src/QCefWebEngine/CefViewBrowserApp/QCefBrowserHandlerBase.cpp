#include "QCefBrowserHandlerBase.h"

#pragma region std_headers
#include <sstream>
#include <string>
#pragma endregion std_headers

#pragma region cef_headers
#include <include/cef_app.h>
#include <include/wrapper/cef_closure_task.h>
#include <include/wrapper/cef_helpers.h>
#include <include/wrapper/cef_resource_manager.h>
#pragma endregion cef_headers

#pragma region qt_headers
#include <QRect>
#include <QWindow>
#include <QVariant>
#include <QUrl>
// for open url.
#include <QDesktopServices>
// json
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#pragma endregion qt_headers

#include "QCefProtocol.h"

#include "public/QCefJavaScriptEngine.h"
#include "CCefSetting.h"


void CefDevToolClient::OnAfterCreated(CefRefPtr<CefBrowser> browser)
{
    CEF_REQUIRE_UI_THREAD();
    HWND hWnd = browser->GetHost()->GetWindowHandle();
    int width = QCefSetting::devToolWidth();
    int height = QCefSetting::devToolHeight();
    ::MoveWindow(hWnd, 0, 0, width, height, TRUE);
    emit sig_notifyBrowserEarly(browser);
}

void CefDevToolClient::OnBeforeClose(CefRefPtr<CefBrowser> browser)
{
    CEF_REQUIRE_UI_THREAD();
    emit sig_removeBrowser(browser->GetIdentifier());
}

//////////////////////////////////////////////////////////////////////////////////////////////

QCefBrowserHandlerBase::QCefBrowserHandlerBase()
    : resource_manager_(new CefResourceManager())
    , weak_factory_(this)
{}

QCefBrowserHandlerBase::~QCefBrowserHandlerBase() {}

bool QCefBrowserHandlerBase::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefProcessId source_process,
    CefRefPtr<CefProcessMessage> message)
{
    CEF_REQUIRE_UI_THREAD();

    if (DispatchNotifyRequest(browser, source_process, message))
        return true;

    return false;
}

void QCefBrowserHandlerBase::OnBeforeContextMenu(CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefContextMenuParams> params,
    CefRefPtr<CefMenuModel> model)
{
    CEF_REQUIRE_UI_THREAD();

    model->Clear();
}

bool QCefBrowserHandlerBase::OnContextMenuCommand(CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefContextMenuParams> params,
    int command_id,
    EventFlags event_flags)
{
    CEF_REQUIRE_UI_THREAD();

    return false;
}

void QCefBrowserHandlerBase::OnAddressChange(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& url)
{
    CEF_REQUIRE_UI_THREAD();
}

void QCefBrowserHandlerBase::OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title)
{
    CEF_REQUIRE_UI_THREAD();
}

bool QCefBrowserHandlerBase::OnConsoleMessage(CefRefPtr<CefBrowser> browser,
    cef_log_severity_t level,
    const CefString& message,
    const CefString& source,
    int line)
{
    CEF_REQUIRE_UI_THREAD();
    if (source.empty() || message.empty())
        return false;

    std::string src = source.ToString();
    std::size_t found = src.find_last_of("/\\");
    if (found != std::string::npos && found < src.length() - 1)
        src = src.substr(found + 1);

    __noop(src, message.ToString());
    return false;
}

void QCefBrowserHandlerBase::OnFullscreenModeChange(CefRefPtr<CefBrowser> browser, bool fullscreen)
{
    CEF_REQUIRE_UI_THREAD();
    emit sig_notifyFullScreenModeChanged(browser->GetIdentifier(), fullscreen);
}

bool QCefBrowserHandlerBase::OnDragEnter(CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefDragData> dragData,
    CefDragHandler::DragOperationsMask mask)
{
    CEF_REQUIRE_UI_THREAD();

    return true;
}

bool QCefBrowserHandlerBase::OnJSDialog(CefRefPtr<CefBrowser> browser,
    const CefString& origin_url,
    JSDialogType dialog_type,
    const CefString& message_text,
    const CefString& default_prompt_text,
    CefRefPtr<CefJSDialogCallback> callback,
    bool& suppress_message)
{
    CEF_REQUIRE_UI_THREAD();

    return false;
}

bool QCefBrowserHandlerBase::OnBeforeUnloadDialog(CefRefPtr<CefBrowser> browser,
    const CefString& message_text,
    bool is_reload,
    CefRefPtr<CefJSDialogCallback> callback)
{
    CEF_REQUIRE_UI_THREAD();

    return false;
}

void QCefBrowserHandlerBase::OnResetDialogState(CefRefPtr<CefBrowser> browser)
{
    CEF_REQUIRE_UI_THREAD();
}

bool QCefBrowserHandlerBase::OnPreKeyEvent(CefRefPtr<CefBrowser> browser,
    const CefKeyEvent& event,
    CefEventHandle os_event,
    bool* is_keyboard_shortcut)
{
    CEF_REQUIRE_UI_THREAD();
    if (CCefSetting::debug_enabled && event.type == KEYEVENT_RAWKEYDOWN && event.windows_key_code == VK_F12) {
        this->showDevTool(browser, 0, 0);
        return true;
    } else if (event.type == KEYEVENT_RAWKEYDOWN && event.windows_key_code == VK_ESCAPE) {
        CefRefPtr<CefFrame> frame = browser->GetMainFrame();
        if (NULL != frame) {
            frame->ExecuteJavaScript("{document.webkitExitFullscreen()}", frame->GetURL(), 0);
            return true;
        }
    }

    return false;
}

bool QCefBrowserHandlerBase::showDevTool(CefRefPtr<CefBrowser> browser, int x, int y)
{
    int debugPort = QCefSetting::remoteDebuggingPort();
    if (0 == debugPort) {
        //TRACEW("[!BP-CC-SDT!] not support,%d", debugPort);
        return false;
    }
    if (!browser)
        return false;

    CefWindowInfo windowInfo;
    windowInfo.SetAsPopup(NULL, L"BrowserDevTools");

    CefBrowserSettings settings;
    CefPoint pt(x, y);
    //TRACED("show...");
    //use the same client as the browser
    CefRefPtr<CefDevToolClient> client = new CefDevToolClient;
    connect(client.get(), &CefDevToolClient::sig_notifyBrowserEarly, this, &QCefBrowserHandlerBase::sig_notifyDevToolBrowser, Qt::DirectConnection);
    connect(client.get(), &CefDevToolClient::sig_removeBrowser, this, &QCefBrowserHandlerBase::sig_removeDevToolBrowser, Qt::DirectConnection);
    browser->GetHost()->ShowDevTools(windowInfo, client, settings, pt);
    return true;
}

bool QCefBrowserHandlerBase::OnBeforePopup(CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    const CefString& target_url,
    const CefString& target_frame_name,
    CefLifeSpanHandler::WindowOpenDisposition target_disposition,
    bool user_gesture,
    const CefPopupFeatures& popupFeatures,
    CefWindowInfo& windowInfo,
    CefRefPtr<CefClient>& client,
    CefBrowserSettings& settings,
    CefRefPtr<CefDictionaryValue>& extra_info,
    bool* no_javascript_access)
{
    CEF_REQUIRE_UI_THREAD();

    // open the url by browser.
    QDesktopServices::openUrl(QUrl(QString::fromStdWString(target_url.c_str())));
    return true;
}

void QCefBrowserHandlerBase::OnAfterCreated(CefRefPtr<CefBrowser> browser)
{
    CEF_REQUIRE_UI_THREAD();
    emit sig_notifyBrowserEarly(browser);
}

bool QCefBrowserHandlerBase::DoClose(CefRefPtr<CefBrowser> browser)
{
    CEF_REQUIRE_UI_THREAD();
    emit sig_closingBrowser(browser->GetIdentifier());
    // Closing the main window requires special handling. See the DoClose()
    // documentation in the CEF header for a detailed description of this
    // process.
    return false;
}

void QCefBrowserHandlerBase::OnBeforeClose(CefRefPtr<CefBrowser> browser)
{
    CEF_REQUIRE_UI_THREAD();
    emit sig_removeBrowser(browser->GetIdentifier());
}

void QCefBrowserHandlerBase::OnLoadingStateChange(CefRefPtr<CefBrowser> browser,
    bool isLoading,
    bool canGoBack,
    bool canGoForward)
{
    CEF_REQUIRE_UI_THREAD();
    emit sig_loadingStateChanged(browser->GetIdentifier(), isLoading, canGoBack, canGoForward);
}

void QCefBrowserHandlerBase::OnLoadStart(CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    TransitionType transition_type)
{
    CEF_REQUIRE_UI_THREAD();
    emit sig_loadStart(browser->GetIdentifier());
}

void QCefBrowserHandlerBase::OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode)
{
    CEF_REQUIRE_UI_THREAD();
    emit sig_loadEnd(browser->GetIdentifier(), httpStatusCode);
}

void QCefBrowserHandlerBase::OnLoadError(CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    ErrorCode errorCode,
    const CefString& errorText,
    const CefString& failedUrl)
{
    CEF_REQUIRE_UI_THREAD();
    if (errorCode == ERR_ABORTED)
        return;

    if (ERR_UNKNOWN_URL_SCHEME == errorCode) {
        QDesktopServices::openUrl(QString::fromStdWString(failedUrl));
    }
    emit sig_loadError(browser->GetIdentifier(), errorCode, QString::fromStdWString(failedUrl));
}

bool QCefBrowserHandlerBase::OnBeforeBrowse(CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefRequest> request,
    bool user_gesture,
    bool is_redirect)
{
    CEF_REQUIRE_UI_THREAD();

    return false;
}

bool QCefBrowserHandlerBase::OnOpenURLFromTab(CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    const CefString& target_url,
    CefRequestHandler::WindowOpenDisposition target_disposition,
    bool user_gesture)
{
    CEF_REQUIRE_UI_THREAD();

    return false; // return true to cancel this navigation.
}

CefRefPtr<CefResourceRequestHandler> QCefBrowserHandlerBase::GetResourceRequestHandler(CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefRequest> request,
    bool is_navigation,
    bool is_download,
    const CefString& request_initiator,
    bool& disable_default_handling)
{
    CEF_REQUIRE_IO_THREAD();
    return this;
}

bool QCefBrowserHandlerBase::OnQuotaRequest(CefRefPtr<CefBrowser> browser,
    const CefString& origin_url,
    int64 new_size,
    CefRefPtr<CefRequestCallback> callback)
{
    CEF_REQUIRE_IO_THREAD();
    static const int maxSize = 10 * 1024 * 1024;
    callback->Continue(new_size <= maxSize);
    return true;
}

void QCefBrowserHandlerBase::OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser, TerminationStatus status)
{
    CEF_REQUIRE_UI_THREAD();
    browser->Reload();
}

CefResourceRequestHandler::ReturnValue QCefBrowserHandlerBase::OnBeforeResourceLoad(CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefRequest> request,
    CefRefPtr<CefRequestCallback> callback)
{
    return resource_manager_->OnBeforeResourceLoad(browser, frame, request, callback);
}

CefRefPtr<CefResourceHandler> QCefBrowserHandlerBase::GetResourceHandler(CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefRequest> request)
{
    return resource_manager_->GetResourceHandler(browser, frame, request);
}

void QCefBrowserHandlerBase::OnProtocolExecution(CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefRequest> request,
    bool& allow_os_execution)
{}

bool QCefBrowserHandlerBase::DispatchNotifyRequest(CefRefPtr<CefBrowser> browser,
    CefProcessId source_process,
    CefRefPtr<CefProcessMessage> message)
{
    CefString messageName = message->GetName();
    CefRefPtr<CefListValue> messageArguments = message->GetArgumentList();
    if (!messageArguments)
        return false;

    int browserId = browser->GetIdentifier();

    if (messageName == QCEF_INVOKENGLMETHOD) {

        QVariantList varList;
        //convert from CefListValue to QVariantList
        //TRACED("messageArguments size is: %d", messageArguments->GetSize());
        for (size_t i = 0; i < messageArguments->GetSize(); i++) {
            switch (messageArguments->GetType(i)) {
            case VTYPE_INT: {
                varList.append(messageArguments->GetInt(i));
            }break;
            case VTYPE_DOUBLE: {
                varList.append(messageArguments->GetDouble(i));
            }break;
            case VTYPE_BOOL: {
                varList.append(messageArguments->GetBool(i));
            }break;
            case VTYPE_STRING: {
                varList.append(QString::fromStdWString(messageArguments->GetString(i).ToWString()));
            }break;
            case VTYPE_BINARY: {
                CefRefPtr<CefBinaryValue> binaryVal = messageArguments->GetBinary(i);
                size_t size = binaryVal->GetSize();
                QByteArray dataBuf;
                dataBuf.resize(size);
                auto readSize = binaryVal->GetData(dataBuf.data(), size, 0);
                Q_ASSERT(readSize == size);
                varList.append(dataBuf);
            }break;
            case VTYPE_NULL: {
                varList.append(QVariant());
            }break;
            default: {
                //TRACEE("index; %d, type: %d", i, messageArguments->GetType(i));
                Q_ASSERT(false);
            }break;
            }
        }

        int idx = 0;
        if (QVariant::Type::String != varList[idx].type() ||
            QVariant::Type::String != varList[idx + 1].type()) {
            return false;
        }

        int messageBrowserId = QString::fromStdString(varList[idx++].toString().toStdString()).toInt();
        int64 frameId = QString::fromStdString(varList[idx++].toString().toStdString()).toLongLong();
        if (messageBrowserId != browserId)
            return false;

        //invoke method by meta object
        QString strCallbackSignatures;
        bool bOk = QCefJavaScriptEngine::get()->inovkeMethod(browserId, varList, strCallbackSignatures);
        if (!bOk) {
            //send clear callbacks
            CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create(QCEF_CLEARNGLCALLBACKS);
            auto paramValue = msg->GetArgumentList();
            int idx = 0;
            paramValue->SetString(idx++, strCallbackSignatures.toStdWString());
            browser->GetMainFrame()->SendProcessMessage(CefProcessId::PID_RENDERER, msg);
        }
        return bOk;
    } else if (messageName == QCEF_NOTIFYBROWSERCREATED) {
        Q_ASSERT(messageArguments->GetSize() == 2);
        int senderBrowserId = messageArguments->GetInt(0);
        if (senderBrowserId != browserId)
            return false;
        CefRefPtr<CefDictionaryValue> dictValue = messageArguments->GetDictionary(1);
        QString webId;
        if (dictValue->HasKey("webId")) {
            webId = QString::fromStdWString(dictValue->GetString("webId").ToWString());
        }
        QString shmName;
        if (dictValue->HasKey("shmName")) {
            shmName = QString::fromStdWString(dictValue->GetString("shmName").ToWString());
        }
        //剩余的参数，我们转换成QMap，保存到QCefCoreBrowser中，方便后续处理
        QMap<QString, QString> extraData;
        CefDictionaryValue::KeyList keyList;
        if (dictValue->GetKeys(keyList) && keyList.size()) {
            for (const auto& key : keyList) {
                if (dictValue->GetType(key) == CefValueType::VTYPE_STRING) {
                    extraData[QString::fromStdWString(key.ToWString())] = QString::fromStdWString(dictValue->GetString(key).ToWString());
                }
            }
        }

        emit sig_addBrowser(webId, browser, shmName, extraData);
        return true;
    } else if (messageName == QCEF_RETRIEVEPROPERTY) {
        if (messageArguments->GetSize() != 3) {
            //TRACEE("QCEF_RETRIEVEPROPERTY argument size mismatch !");
            return false;
        }
        int messageBrowserId = QString::fromStdString(messageArguments->GetString(0).ToString()).toInt();
        int64 messageFrameId = QString::fromStdString(messageArguments->GetString(1).ToString()).toLongLong();
        QString signature = QString::fromStdString(messageArguments->GetString(2).ToString());
        if (messageBrowserId != browserId)
            return false;
        //获取动态属性，当前处于多线程中
        //TOOD：可能存在多线程竞争，暂不处理
        QVariant value;
        bool bOk = QCefJavaScriptEngine::get()->retieveProperty(signature, value);
        if (bOk) {
            //发送回去
            bOk = QCefJavaScriptEngine::get()->writeSynchronizeValue(signature, value);
        }
        return bOk;
    } else if (messageName == QCEF_SETPROPERTY) {
        if (messageArguments->GetSize() != 4) {
            //TRACEE("QCEF_SETPROPERTY argument size mismatch !");
            return false;
        }
        int messageBrowserId = QString::fromStdWString(messageArguments->GetString(0)).toInt();
        int64 messageFrameId = QString::fromStdWString(messageArguments->GetString(1)).toLongLong();
        QString signature = QString::fromStdWString(messageArguments->GetString(2));
        QString strValue = QString::fromStdWString(messageArguments->GetString(3));
        if (messageBrowserId != browserId)
            return false;
        bool bOk = QCefJavaScriptEngine::get()->setProperty(signature, strValue);
        return bOk;
    }

    return false;
}

#pragma region CefRenderHandler

CefRefPtr<CefAccessibilityHandler> QCefBrowserHandlerBase::GetAccessibilityHandler()
{
    return NULL;
}

bool QCefBrowserHandlerBase::GetRootScreenRect(CefRefPtr<CefBrowser> browser, CefRect& rect)
{
    return false;
}

void QCefBrowserHandlerBase::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect)
{
    //RECT winRect;
    //GetWindowRect(hwnd_, &winRect);
    ////QSize size = pQCefWindow_->geometry().size();
    //int width = winRect.right - winRect.left;
    //int height = winRect.bottom - winRect.top;
    //rect.Set(0, 0, width > 0 ? width : 1, height > 0 ? height : 1);
}

bool QCefBrowserHandlerBase::GetScreenPoint(CefRefPtr<CefBrowser> browser,
    int viewX,
    int viewY,
    int& screenX,
    int& screenY)
{
    return false;
}

bool QCefBrowserHandlerBase::GetScreenInfo(CefRefPtr<CefBrowser> browser,
    CefScreenInfo& screen_info)
{
    return false;
}

void QCefBrowserHandlerBase::OnPopupShow(CefRefPtr<CefBrowser> browser, bool show)
{
    CEF_REQUIRE_UI_THREAD();

    /*if (!show) {
        renderer_.ClearPopupRects();
        browser->GetHost()->Invalidate(PET_VIEW);
    }

    renderer_.OnPopupShow(browser, show);*/
}

void QCefBrowserHandlerBase::OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect& rect)
{
    CEF_REQUIRE_UI_THREAD();
    //renderer_.OnPopupSize(browser, rect);
}

void QCefBrowserHandlerBase::OnPaint(CefRefPtr<CefBrowser> browser,
    PaintElementType type,
    const RectList& dirtyRects,
    const void* buffer,
    int width,
    int height)
{
    CEF_REQUIRE_UI_THREAD();
}

void QCefBrowserHandlerBase::OnAcceleratedPaint(CefRefPtr<CefBrowser> browser,
    PaintElementType type,
    const RectList& dirtyRects,
    void* shared_handle)
{}

void QCefBrowserHandlerBase::OnCursorChange(CefRefPtr<CefBrowser> browser,
    CefCursorHandle cursor,
    CursorType type,
    const CefCursorInfo& custom_cursor_info)
{}

bool QCefBrowserHandlerBase::StartDragging(CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefDragData> drag_data,
    CefDragHandler::DragOperationsMask allowed_ops,
    int x,
    int y)
{
    return false;
}

void QCefBrowserHandlerBase::UpdateDragCursor(CefRefPtr<CefBrowser> browser,
    DragOperation operation)
{}

void QCefBrowserHandlerBase::OnScrollOffsetChanged(CefRefPtr<CefBrowser> browser,
    double x,
    double y)
{}

void QCefBrowserHandlerBase::OnImeCompositionRangeChanged(CefRefPtr<CefBrowser> browser,
    const CefRange& selected_range,
    const RectList& character_bounds)
{}

void QCefBrowserHandlerBase::OnTextSelectionChanged(CefRefPtr<CefBrowser> browser,
    const CefString& selected_text,
    const CefRange& selected_range)
{}

void QCefBrowserHandlerBase::OnVirtualKeyboardRequested(CefRefPtr<CefBrowser> browser,
    TextInputMode input_mode)
{}

void QCefBrowserHandlerBase::OnBeforeDownload(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefDownloadItem> download_item,
    const CefString& suggested_name,
    CefRefPtr<CefBeforeDownloadCallback> callback)
{
    callback->Continue(download_item->GetURL(), false);
}


#pragma endregion