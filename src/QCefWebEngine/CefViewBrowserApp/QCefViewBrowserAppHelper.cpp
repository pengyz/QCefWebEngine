#pragma region std_headers
#include <string>
#pragma endregion std_headers

#pragma region cef_headers
#include <include/cef_browser.h>
#include <include/cef_command_line.h>
#pragma endregion cef_headers

#include "QCefViewBrowserApp.h"
#include "BrowserDelegates/QCefViewDefaultBrowserDelegate.h"
#include "SchemeHandlers/QCefViewDefaultSchemeHandler.h"
#include "public/QCefSetting.h"

void
QCefViewBrowserApp::CreateBrowserDelegates(CefRefPtr<QCefViewBrowserApp> app, BrowserDelegateSet& delegates)
{
    QCefViewDefaultBrowserDelegate::CreateBrowserDelegate(delegates);
}

void
QCefViewBrowserApp::RegisterCustomSchemesHandlerFactories()
{
    QCefViewDefaultSchemeHandler::RegisterSchemeHandlerFactory();
}

void
QCefViewBrowserApp::RegisterCustomSchemes(CefRawPtr<CefSchemeRegistrar> registrar)
{
    QCefViewDefaultSchemeHandler::RegisterScheme(registrar);
}