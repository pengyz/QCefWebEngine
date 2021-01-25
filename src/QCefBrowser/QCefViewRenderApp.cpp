#pragma region std_headers
#include <string>
#pragma endregion std_headers

#pragma region cef_headers
#include <include/wrapper/cef_helpers.h>
#include <include/cef_browser.h>
#include <include/cef_command_line.h>
#pragma endregion cef_headers

#pragma region project_headers
#include "QCefViewRenderApp.h"
#include "RenderDelegates/QCefViewDefaultRenderDelegate.h"
#include "RenderDelegates/QCefSurfaceRenderDelegate.h"
#include "tracer.h"
#pragma endregion project_headers


QCefViewRenderApp::QCefViewRenderApp()
{}

QCefViewRenderApp::~QCefViewRenderApp() {}

void QCefViewRenderApp::CreateRenderDelegates(RenderDelegateSet& delegates)
{
    QCefViewDefaultRenderDelegate::CreateBrowserDelegate(delegates);
    //对于HtmlSurface的特殊处理 主要用于响应几个特殊消息
    QCefViewSurfaceRenderDelegate::CreateBrowserDelegate(delegates);
}

void QCefViewRenderApp::OnBeforeCommandLineProcessing(const CefString& process_type, CefRefPtr<CefCommandLine> command_line)
{
    CefString logLevel = command_line->GetSwitchValue(QCEF_LOGLEVEL_OPTION_NAME);
    if (!logLevel.empty()) {
        int iLogLevel = QString::fromStdWString(logLevel).toInt();
        // set log level
        Tracer::Initial(GetModuleHandle(NULL), iLogLevel, Tracer::TraceMode::AllTrace);
    }
    //read and set parentId as the js binding shared memory name key
    CefString parentId = command_line->GetSwitchValue(QCEF_PARENT_ID_NAME);
    QCefJavaScriptBinder::get()->setParentId(QString::fromStdWString(parentId));

    RenderDelegateSet::iterator it = render_delegates_.begin();
    for (; it != render_delegates_.end(); ++it)
        (*it)->OnBeforeCommandLineProcessing(process_type, command_line);
}

void QCefViewRenderApp::OnRegisterCustomSchemes(CefRawPtr<CefSchemeRegistrar> registrar)
{
}

CefRefPtr<CefRenderProcessHandler> QCefViewRenderApp::GetRenderProcessHandler()
{
    return this;
}

CefRefPtr<CefResourceBundleHandler> QCefViewRenderApp::GetResourceBundleHandler()
{
    return nullptr;
}

CefRefPtr<CefBrowserProcessHandler> QCefViewRenderApp::GetBrowserProcessHandler()
{
    return nullptr;
}

//////////////////////////////////////////////////////////////////////////
void QCefViewRenderApp::OnWebKitInitialized()
{
    CEF_REQUIRE_RENDERER_THREAD();
    CreateRenderDelegates(render_delegates_);
    RenderDelegateSet::iterator it = render_delegates_.begin();
    for (; it != render_delegates_.end(); ++it)
        (*it)->OnWebKitInitialized(this);
}

void QCefViewRenderApp::OnBrowserCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefDictionaryValue> extra_info)
{
    CEF_REQUIRE_RENDERER_THREAD();
    RenderDelegateSet::iterator it = render_delegates_.begin();
    for (; it != render_delegates_.end(); ++it)
        (*it)->OnBrowserCreated(this, browser, extra_info);

    CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create(QCEF_NOTIFYBROWSERCREATED);
    auto arguments = msg->GetArgumentList();
    arguments->SetInt(0, browser->GetIdentifier());
    arguments->SetDictionary(1, extra_info);
    if (browser->GetMainFrame())
        browser->GetMainFrame()->SendProcessMessage(PID_BROWSER, msg);
}

void QCefViewRenderApp::OnBrowserDestroyed(CefRefPtr<CefBrowser> browser)
{
    CEF_REQUIRE_RENDERER_THREAD();
    RenderDelegateSet::iterator it = render_delegates_.begin();
    for (; it != render_delegates_.end(); ++it)
        (*it)->OnBrowserDestroyed(this, browser);
}

CefRefPtr<CefLoadHandler> QCefViewRenderApp::GetLoadHandler()
{
    CefRefPtr<CefLoadHandler> load_handler;
    RenderDelegateSet::iterator it = render_delegates_.begin();
    for (; it != render_delegates_.end() && !load_handler.get(); ++it)
        load_handler = (*it)->GetLoadHandler(this);

    return load_handler;
}

void QCefViewRenderApp::OnContextCreated(CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefV8Context> context)
{
    CEF_REQUIRE_RENDERER_THREAD();
    RenderDelegateSet::iterator it = render_delegates_.begin();
    for (; it != render_delegates_.end(); ++it)
        (*it)->OnContextCreated(this, browser, frame, context);
}

void QCefViewRenderApp::OnContextReleased(CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefV8Context> context)
{
    CEF_REQUIRE_RENDERER_THREAD();
    RenderDelegateSet::iterator it = render_delegates_.begin();
    for (; it != render_delegates_.end(); ++it)
        (*it)->OnContextReleased(this, browser, frame, context);
}

void QCefViewRenderApp::OnUncaughtException(CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefV8Context> context,
    CefRefPtr<CefV8Exception> exception,
    CefRefPtr<CefV8StackTrace> stackTrace)
{
    CEF_REQUIRE_RENDERER_THREAD();
    RenderDelegateSet::iterator it = render_delegates_.begin();
    for (; it != render_delegates_.end(); ++it)
        (*it)->OnUncaughtException(this, browser, frame, context, exception, stackTrace);
}

void QCefViewRenderApp::OnFocusedNodeChanged(CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefDOMNode> node)
{
    CEF_REQUIRE_RENDERER_THREAD();
    RenderDelegateSet::iterator it = render_delegates_.begin();
    for (; it != render_delegates_.end(); ++it)
        (*it)->OnFocusedNodeChanged(this, browser, frame, node);
}

bool QCefViewRenderApp::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefProcessId source_process,
    CefRefPtr<CefProcessMessage> message)
{
    CEF_REQUIRE_RENDERER_THREAD();
    DCHECK_EQ(source_process, PID_BROWSER);

    bool handled = false;
    RenderDelegateSet::iterator it = render_delegates_.begin();
    for (; it != render_delegates_.end() && !handled; ++it)
        handled = (*it)->OnProcessMessageReceived(this, browser, frame, source_process, message);

    return handled;
}