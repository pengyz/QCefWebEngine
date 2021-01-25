#pragma once
#pragma region std_headers
#include <map>
#include <list>
#pragma endregion

#pragma region cef_headers
#include <include/cef_v8.h>
#pragma endregion cef_headers

#include <QCefProtocol.h>
#include "QCefJavaScriptEnvironment.h"

/// <summary>
///
/// </summary>
class QCefClient : public CefBaseRefCounted {
public:
    /// <summary>
    ///
    /// </summary>
    /// <param name="browser"></param>
    /// <param name="frame"></param>
    QCefClient(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame);

    ~QCefClient();

    /// <summary>
    ///
    /// </summary>
    /// <returns></returns>
    CefRefPtr<CefV8Value> GetObject();

    bool invokeCallBack(const QString& signature, CefRefPtr<CefListValue> argumentList);
    bool clearFunctionCallbacks(const QStringList& signatures);

private:
    /// <summary>
    ///
    /// </summary>
    CefRefPtr<CefV8Value> object_;

    /// <summary>
    ///
    /// </summary>
    CefRefPtr<CefBrowser> browser_;

    /// <summary>
    ///
    /// </summary>
    CefRefPtr<CefFrame> frame_;

private:
    void* jsEnv_;
    IMPLEMENT_REFCOUNTING(QCefClient);
};
