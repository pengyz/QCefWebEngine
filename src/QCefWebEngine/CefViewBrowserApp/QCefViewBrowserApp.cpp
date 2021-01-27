#pragma region std_headers
#include <string>
#pragma endregion std_headers

#pragma region cef_headers
#include <include/cef_browser.h>
#include <include/cef_command_line.h>
#include <include/wrapper/cef_helpers.h>
#pragma endregion cef_headers

#include "QCefViewBrowserApp.h"
#include "QCefProtocol.h"
#include "BrowserDelegates/client_switches.h"
#include <QtGlobal>
#include <QString>
#include <QDebug>

QCefViewBrowserApp::QCefViewBrowserApp() {}

QCefViewBrowserApp::~QCefViewBrowserApp() {}


//////////////////////////////////////////////////////////////////////////
void QCefViewBrowserApp::OnBeforeCommandLineProcessing(const CefString& process_type, CefRefPtr<CefCommandLine> command_line)
{
    //common switches
    command_line->AppendSwitch(switches::kDisableSpellChecking);                        //禁用语法检测
    command_line->AppendSwitch(switches::kDisableExtensions);                           //禁用插件
    command_line->AppendSwitch(switches::kDisablePdfExtension);                         //禁用内置pdf插件
    command_line->AppendSwitch(switches::kEnableDirectWrite);                           //启用DirectWrite
    command_line->AppendSwitch(switches::kAllowFileAccessFromFiles);                    //启用跨文件访问
    command_line->AppendSwitch(switches::kNoProxyServer);                               //禁用代理服务器
    command_line->AppendSwitch(switches::kEnableGPU);                                   //启用GPU
    command_line->AppendSwitch(switches::kInProcessGpu);                                //启用进程内GPU
    command_line->AppendSwitch(switches::kDisableSiteIsolation);                        //禁用站点隔离（重要，必须禁用站点隔离，否则会影响callback调用）
    command_line->AppendSwitch(switches::kDisableWebSecurty);                           //禁用站点安全，允许CORS访问
    command_line->AppendSwitch(switches::kAllowRunningInsecureContent);                 //禁用https下加载http警告
    command_line->AppendSwitch(switches::kIgnoreCertificateErrors);                     //忽略SSL证书错误
    command_line->AppendSwitchWithValue(switches::kRenderProcessLimit, "1");            //最多仅有1个render进程
    command_line->AppendSwitchWithValue(switches::kDisableFeatures, "OutOfBlinkCors");  //禁用OOR-CORS

    if (command_line->HasSwitch(switches::kUseViews) &&
        !command_line->HasSwitch(switches::kTopChromeMd)) {
        // Use non-material mode on all platforms by default. Among other things
        // this causes menu buttons to show hover state. See usage of
        // MaterialDesignController::IsModeMaterial() in Chromium code.
        command_line->AppendSwitchWithValue(switches::kTopChromeMd, "non-material");
    }

    if (!command_line->HasSwitch(switches::kCachePath) &&
        !command_line->HasSwitch(switches::kDisableGpuShaderDiskCache)) {
        // Don't create a "GPUCache" directory when cache-path is unspecified.
        command_line->AppendSwitch(switches::kDisableGpuShaderDiskCache);
    }
}

void QCefViewBrowserApp::OnRegisterCustomSchemes(CefRawPtr<CefSchemeRegistrar> registrar)
{
    RegisterCustomSchemes(registrar);
}

CefRefPtr<CefResourceBundleHandler> QCefViewBrowserApp::GetResourceBundleHandler()
{
    return nullptr;
}

CefRefPtr<CefBrowserProcessHandler> QCefViewBrowserApp::GetBrowserProcessHandler()
{
    return this;
}

CefRefPtr<CefRenderProcessHandler> QCefViewBrowserApp::GetRenderProcessHandler()
{
    return nullptr;
}

//////////////////////////////////////////////////////////////////////////
void QCefViewBrowserApp::OnContextInitialized()
{
    CEF_REQUIRE_UI_THREAD();

    // create all browser delegates
    CreateBrowserDelegates(this, browser_delegates_);

    // Register cookieable schemes with the global cookie manager.
    CefRefPtr<CefCookieManager> manager = CefCookieManager::GetGlobalManager(nullptr);
    DCHECK(manager.get());
    typedef std::vector<CefString> CookiableSchemeSet;
    CookiableSchemeSet cookieable_schemes_;
    manager->SetSupportedSchemes(cookieable_schemes_, true, nullptr);

    RegisterCustomSchemesHandlerFactories();

    BrowserDelegateSet::iterator it = browser_delegates_.begin();
    for (; it != browser_delegates_.end(); ++it)
        (*it)->OnContextInitialized(this);
}

void QCefViewBrowserApp::OnBeforeChildProcessLaunch(CefRefPtr<CefCommandLine> command_line)
{
    BrowserDelegateSet::iterator it = browser_delegates_.begin();
    for (; it != browser_delegates_.end(); ++it)
        (*it)->OnBeforeChildProcessLaunch(this, command_line);
}

CefRefPtr<CefPrintHandler> QCefViewBrowserApp::GetPrintHandler()
{
    return nullptr;
}

void QCefViewBrowserApp::OnScheduleMessagePumpWork(int64 delay_ms)
{
}