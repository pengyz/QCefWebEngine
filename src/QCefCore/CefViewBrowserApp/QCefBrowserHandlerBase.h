#ifndef QCEFVIEWHANDLER_H_
#define QCEFVIEWHANDLER_H_
#pragma once

#pragma region std_headers
#include <list>
#include <map>
#include <set>
#include <string>
#include <mutex>
#pragma endregion std_headers

#pragma region qt_headers
#include <QPointer>
#include <QObject>
#include <QMap>
#pragma endregion qt_headers

#pragma region cef_headers
#include <include/cef_client.h>
#include <include/wrapper/cef_message_router.h>
#include <include/wrapper/cef_resource_manager.h>
#pragma endregion cef_headers
#include "qcefcore_export.h"


class QCEFCORE_EXPORT CefDevToolClient
    : public QObject
    , public CefClient
    , public CefLifeSpanHandler {
    Q_OBJECT
public:
    virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() { return this; }
    virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
    virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

signals:
    void sig_notifyBrowserEarly(const CefRefPtr<CefBrowser>& browser);
    void sig_removeBrowser(int browserId);

public:
    IMPLEMENT_REFCOUNTING(CefDevToolClient);
};


class QCEFCORE_EXPORT QCefBrowserHandlerBase
    : public QObject
    , public CefClient
    , public CefContextMenuHandler
    , public CefDisplayHandler
    , public CefDragHandler
    , public CefJSDialogHandler
    , public CefKeyboardHandler
    , public CefLifeSpanHandler
    , public CefLoadHandler
    , public CefRequestHandler
    , public CefResourceRequestHandler
    , public CefRenderHandler
    , public CefDialogHandler
    , public CefDownloadHandler
    , public CefFindHandler {

    Q_OBJECT
public:
    /// <summary>
    ///
    /// </summary>
    enum {
        MAIN_FRAME = (int64_t)0,
        ALL_FRAMES = (int64_t)-1,
    };

    /// <summary>
    ///
    /// </summary>
    QCefBrowserHandlerBase();

    /// <summary>
    ///
    /// </summary>
    ~QCefBrowserHandlerBase();

public:

#pragma region CefClient

    //////////////////////////////////////////////////////////////////////////
    // CefClient methods:
    virtual CefRefPtr<CefContextMenuHandler> GetContextMenuHandler() override
    {
        return pContextMenuHandler_ ? pContextMenuHandler_ : this;
    }
    virtual CefRefPtr<CefDialogHandler> GetDialogHandler() override { return pDialogHandler_ ? pDialogHandler_ : this; }
    virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler() override { return pDisplayHandler_ ? pDisplayHandler_ : this; }
    virtual CefRefPtr<CefDownloadHandler> GetDownloadHandler() override { return pDownloadHandler_ ? pDownloadHandler_ : this; }
    virtual CefRefPtr<CefDragHandler> GetDragHandler() override { return this; }
    virtual CefRefPtr<CefJSDialogHandler> GetJSDialogHandler() override { return pJSDialogHandler_ ? pJSDialogHandler_ : this; }
    virtual CefRefPtr<CefKeyboardHandler> GetKeyboardHandler() override { return pKeyboardHandler_ ? pKeyboardHandler_ : this; }
    virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override { return pLiefSpanHandler_ ? pLiefSpanHandler_ : this; }
    virtual CefRefPtr<CefLoadHandler> GetLoadHandler() override { return pLoadHandler_ ? pLoadHandler_ : this; }
    virtual CefRefPtr<CefRequestHandler> GetRequestHandler() override { return pRequestHandler_ ? pRequestHandler_ : this; }
    virtual CefRefPtr<CefRenderHandler> GetRenderHandler() override { return pRenderHandler_ ? pRenderHandler_ : this; }
    virtual CefRefPtr<CefFindHandler> GetFindHandler() override { return pFindHandler_ ? pFindHandler_ : this; }

    virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        CefProcessId source_process,
        CefRefPtr<CefProcessMessage> message) override;

#pragma endregion CefClient

#pragma region CefContextMenuHandler

    // CefContextMenuHandler methods
    virtual void OnBeforeContextMenu(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        CefRefPtr<CefContextMenuParams> params,
        CefRefPtr<CefMenuModel> model) override;
    virtual bool OnContextMenuCommand(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        CefRefPtr<CefContextMenuParams> params,
        int command_id,
        EventFlags event_flags) override;

#pragma endregion CefContextMenuHandler

#pragma region CefDisplayHandler

    // CefDisplayHandler methods
    virtual void OnAddressChange(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& url) override;
    virtual void OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title) override;
    virtual bool OnConsoleMessage(CefRefPtr<CefBrowser> browser,
        cef_log_severity_t level,
        const CefString& message,
        const CefString& source,
        int line) override;
    virtual void OnFullscreenModeChange(CefRefPtr<CefBrowser> browser,
        bool fullscreen) override;


#pragma endregion CefDisplayHandler

#pragma region CefDragHandler

    // CefDragHandler methods
    virtual bool OnDragEnter(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefDragData> dragData,
        CefDragHandler::DragOperationsMask mask) override;

#pragma endregion CefDragHandler

#pragma region CefJSDialogHandler

    // CefJSDialogHandler methods
    virtual bool OnJSDialog(CefRefPtr<CefBrowser> browser,
        const CefString& origin_url,
        JSDialogType dialog_type,
        const CefString& message_text,
        const CefString& default_prompt_text,
        CefRefPtr<CefJSDialogCallback> callback,
        bool& suppress_message) override;

    virtual bool OnBeforeUnloadDialog(CefRefPtr<CefBrowser> browser,
        const CefString& message_text,
        bool is_reload,
        CefRefPtr<CefJSDialogCallback> callback) override;
    virtual void OnResetDialogState(CefRefPtr<CefBrowser> browser) override;

#pragma endregion CefJSDialogHandler

#pragma region CefKeyboardHandler

    // CefKeyboardHandler methods
    virtual bool OnPreKeyEvent(CefRefPtr<CefBrowser> browser,
        const CefKeyEvent& event,
        CefEventHandle os_event,
        bool* is_keyboard_shortcut) override;

#pragma endregion CefKeyboardHandler

#pragma region CefLifeSpanHandler

    // CefLifeSpanHandler methods:
    virtual bool OnBeforePopup(CefRefPtr<CefBrowser> browser,
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
        bool* no_javascript_access) override;
    virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
    virtual bool DoClose(CefRefPtr<CefBrowser> browser) override;
    virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

#pragma endregion CefLifeSpanHandler

#pragma region CefLoadHandler

    // CefLoadHandler methods
    virtual void OnLoadingStateChange(CefRefPtr<CefBrowser> browser,
        bool isLoading,
        bool canGoBack,
        bool canGoForward) override;
    virtual void OnLoadStart(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        TransitionType transition_type) override;
    virtual void OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode) override;
    virtual void OnLoadError(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        ErrorCode errorCode,
        const CefString& errorText,
        const CefString& failedUrl) override;

#pragma endregion CefLoadHandler

#pragma region CefRequestHandler

    // CefRequestHandler methods
    virtual bool OnBeforeBrowse(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        CefRefPtr<CefRequest> request,
        bool user_gesture,
        bool is_redirect) override;

    virtual bool OnOpenURLFromTab(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        const CefString& target_url,
        CefRequestHandler::WindowOpenDisposition target_disposition,
        bool user_gesture) override;

    virtual CefRefPtr<CefResourceRequestHandler> GetResourceRequestHandler(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        CefRefPtr<CefRequest> request,
        bool is_navigation,
        bool is_download,
        const CefString& request_initiator,
        bool& disable_default_handling) override;

    virtual bool OnQuotaRequest(CefRefPtr<CefBrowser> browser,
        const CefString& origin_url,
        int64 new_size,
        CefRefPtr<CefRequestCallback> callback) override;

    virtual void OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser, TerminationStatus status) override;

#pragma endregion CefRequestHandler

#pragma region CefResourceRequestHandler

    virtual ReturnValue OnBeforeResourceLoad(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        CefRefPtr<CefRequest> request,
        CefRefPtr<CefRequestCallback> callback) override;

    virtual CefRefPtr<CefResourceHandler> GetResourceHandler(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        CefRefPtr<CefRequest> request) override;

    virtual void OnProtocolExecution(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        CefRefPtr<CefRequest> request,
        bool& allow_os_execution) override;

#pragma endregion CefResourceRequestHandler

#pragma region CefRenderHandler

    virtual CefRefPtr<CefAccessibilityHandler> GetAccessibilityHandler() override;

    virtual bool GetRootScreenRect(CefRefPtr<CefBrowser> browser, CefRect& rect) override;

    virtual void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) override;

    virtual bool GetScreenPoint(CefRefPtr<CefBrowser> browser,
        int viewX,
        int viewY,
        int& screenX,
        int& screenY) override;

    virtual bool GetScreenInfo(CefRefPtr<CefBrowser> browser,
        CefScreenInfo& screen_info) override;

    virtual void OnPopupShow(CefRefPtr<CefBrowser> browser, bool show) override;

    virtual void OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect& rect) override;

    virtual void OnPaint(CefRefPtr<CefBrowser> browser,
        PaintElementType type,
        const RectList& dirtyRects,
        const void* buffer,
        int width,
        int height) override;

    virtual void OnAcceleratedPaint(CefRefPtr<CefBrowser> browser,
        PaintElementType type,
        const RectList& dirtyRects,
        void* shared_handle) override;

    virtual void OnCursorChange(CefRefPtr<CefBrowser> browser,
        CefCursorHandle cursor,
        CursorType type,
        const CefCursorInfo& custom_cursor_info) override;

    virtual bool StartDragging(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefDragData> drag_data,
        CefDragHandler::DragOperationsMask allowed_ops,
        int x,
        int y) override;

    virtual void UpdateDragCursor(CefRefPtr<CefBrowser> browser,
        DragOperation operation) override;

    virtual void OnScrollOffsetChanged(CefRefPtr<CefBrowser> browser,
        double x,
        double y) override;

    virtual void OnImeCompositionRangeChanged(CefRefPtr<CefBrowser> browser,
        const CefRange& selected_range,
        const RectList& character_bounds) override;

    virtual void OnTextSelectionChanged(CefRefPtr<CefBrowser> browser,
        const CefString& selected_text,
        const CefRange& selected_range) override;

    virtual void OnVirtualKeyboardRequested(CefRefPtr<CefBrowser> browser,
        TextInputMode input_mode) override;

#pragma endregion CefRenderHandler

#pragma region CefDownloadHandler

    virtual void OnBeforeDownload(
        CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefDownloadItem> download_item,
        const CefString& suggested_name,
        CefRefPtr<CefBeforeDownloadCallback> callback) override;

#pragma endregion CefDownloadHandler

    //////////////////////////////////////////////////////////////////////////

    bool DispatchNotifyRequest(CefRefPtr<CefBrowser> browser,
        CefProcessId source_process,
        CefRefPtr<CefProcessMessage> message);

    void SetContextMenuHandler(CefRefPtr<CefContextMenuHandler> handler) { pContextMenuHandler_ = handler; }

    void SetDialogHandler(CefRefPtr<CefDialogHandler> handler) { pDialogHandler_ = handler; }

    void SetDisplayHandler(CefRefPtr<CefDisplayHandler> handler) { pDisplayHandler_ = handler; }

    void SetLifeSpanHandler(CefRefPtr<CefLifeSpanHandler> handler) { pLiefSpanHandler_ = handler; }

    void SetDownloadHandler(CefRefPtr<CefDownloadHandler> handler) { pDownloadHandler_ = handler; }

    void SetJSDialogHandler(CefRefPtr<CefJSDialogHandler> handler) { pJSDialogHandler_ = handler; }

    void SetKeyboardHandler(CefRefPtr<CefKeyboardHandler> handler) { pKeyboardHandler_ = handler; }

    void SetFindHandler(CefRefPtr<CefFindHandler> handler) { pFindHandler_ = handler; }

    void SetRequestHandler(CefRefPtr<CefRequestHandler> handler) { pRequestHandler_ = handler; }

    void SetRenderHandler(CefRefPtr<CefRenderHandler> handler) { pRenderHandler_ = handler; }

    void SetLoadHandler(CefRefPtr<CefLoadHandler> handler) { pLoadHandler_ = handler; }

    bool showDevTool(CefRefPtr<CefBrowser> browser, int x, int y);


    /////////////////////////////////////////////////////////////////////////////////'
signals:
    void sig_notifyBrowserEarly(const CefRefPtr<CefBrowser>& browser);
    void sig_loadingStateChanged(int browserId, bool isLoading, bool canGoBack, bool canGoForward);
    void sig_loadStart(int browserId);
    void sig_loadEnd(int browserId, int httpStatusCode);
    void sig_loadError(int browserId, int errorCode, QString url);
    void sig_notifyFullScreenModeChanged(int browserId, bool fullscreen);

    //new signals
    void sig_addBrowser(const QString& webId, const CefRefPtr<CefBrowser>& browser, const QString& shmName, QMap<QString, QString> extraData);
    void sig_removeBrowser(int browserId);
    void sig_closingBrowser(int browserId); //当前正在关闭浏览器

    // devTool signals
    void sig_notifyDevToolBrowser(const CefRefPtr<CefBrowser>& browser);
    void sig_removeDevToolBrowser(int browserId);

private:

    /// <summary>
    ///
    /// </summary>
    CefRefPtr<CefResourceManager> resource_manager_;

    CefRefPtr<CefContextMenuHandler> pContextMenuHandler_;
    CefRefPtr<CefDisplayHandler> pDisplayHandler_;
    CefRefPtr<CefJSDialogHandler> pJSDialogHandler_;
    CefRefPtr<CefKeyboardHandler> pKeyboardHandler_;
    CefRefPtr<CefDialogHandler> pDialogHandler_;
    CefRefPtr<CefDownloadHandler> pDownloadHandler_;
    CefRefPtr<CefLifeSpanHandler> pLiefSpanHandler_;
    CefRefPtr<CefFindHandler> pFindHandler_;
    CefRefPtr<CefRequestHandler> pRequestHandler_;
    CefRefPtr<CefRenderHandler> pRenderHandler_;
    CefRefPtr<CefLoadHandler> pLoadHandler_;

    bool _bWindowless = false;

    // Must be the last member.
    base::WeakPtrFactory<QCefBrowserHandlerBase> weak_factory_;

    // Include the default reference counting implementation.
    IMPLEMENT_REFCOUNTING(QCefBrowserHandlerBase);
};
#endif
