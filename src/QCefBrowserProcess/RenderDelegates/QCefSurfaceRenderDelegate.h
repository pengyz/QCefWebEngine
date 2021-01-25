#pragma once
#pragma region std_headers
#include <unordered_map>
#pragma endregion

#pragma region cef_headers
#include <include/wrapper/cef_message_router.h>
#pragma endregion cef_headers

#pragma region project_headers
#include "../QCefViewRenderApp.h"
#include "QCefClient.h"
#pragma endregion project_headers

namespace QCefViewSurfaceRenderDelegate {
    /// <summary>
    ///
    /// </summary>
    void
        CreateBrowserDelegate(QCefViewRenderApp::RenderDelegateSet& delegates);

    /// <summary>
    ///
    /// </summary>
    class RenderDelegate : public QCefViewRenderApp::RenderDelegate {

    public:
        /// <summary>
        ///
        /// </summary>
        RenderDelegate() = default;

        /// <summary>
        ///
        /// </summary>
        /// <param name="app"></param>
        /// <param name="browser"></param>
        /// <param name="source_process"></param>
        /// <param name="message"></param>
        /// <returns></returns>
        virtual bool OnProcessMessageReceived(CefRefPtr<QCefViewRenderApp> app,
            CefRefPtr<CefBrowser> browser,
            CefRefPtr<CefFrame> frame,
            CefProcessId source_process,
            CefRefPtr<CefProcessMessage> message);

    protected:
        void onGetLinkAtPosition(CefRefPtr<CefBrowser> browser, const CefRefPtr<CefListValue>& args);
        void onSetHorizontalScroll(CefRefPtr<CefBrowser> browser, const CefRefPtr<CefListValue>& args);
        void onSetVerticalScroll(CefRefPtr<CefBrowser> browser, const CefRefPtr<CefListValue>& args);



    private:
        IMPLEMENT_REFCOUNTING(RenderDelegate);
    };
}
