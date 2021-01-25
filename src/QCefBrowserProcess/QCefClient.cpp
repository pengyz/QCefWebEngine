#pragma region projet_headers
#include "QCefClient.h"
#include "QCefJavaScriptBinder.h"
#pragma endregion projet_headers

//////////////////////////////////////////////////////////////////////////

QCefClient::QCefClient(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame)
    : object_(CefV8Value::CreateObject(nullptr, nullptr))
    , browser_(browser)
    , frame_(frame)
    , jsEnv_(nullptr)
{
    //bind all functions and properties.
    bool bOk = QCefJavaScriptBinder::get()->init();
    if (bOk) {
        if (jsEnv_) {
            delete jsEnv_;
            jsEnv_ = nullptr;
        }
        jsEnv_ = QCefJavaScriptBinder::get()->bindAllObjects(object_, browser_, frame_);
    }
}

QCefClient::~QCefClient()
{
    //TRACED("called to release all !");
    delete jsEnv_;
    jsEnv_ = nullptr;
}

CefRefPtr<CefV8Value>
QCefClient::GetObject()
{
    return object_;
}

bool QCefClient::invokeCallBack(const QString & signature, CefRefPtr<CefListValue> argumentList)
{
    if (jsEnv_) {
        return ((QCefJavaScriptEnvironment*)jsEnv_)->invokeCallBack(signature, argumentList);
    }
    return false;
}

bool QCefClient::clearFunctionCallbacks(const QStringList& signatures)
{
    if (jsEnv_) {
        return ((QCefJavaScriptEnvironment*)jsEnv_)->clearFunctionCallbacks(signatures);
    }
    return false;
}
