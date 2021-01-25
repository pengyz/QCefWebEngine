#include <QString>
#include <QStringList>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#pragma region project_headers
#include "QCefSurfaceRenderDelegate.h"
#include "QCefClient.h"
#include "QCefJavaScriptBinder.h"
#include "tracer.h"
#pragma endregion project_headers

namespace QCefViewSurfaceRenderDelegate {
    void
        CreateBrowserDelegate(QCefViewRenderApp::RenderDelegateSet& delegates)
    {
        delegates.insert(new RenderDelegate());
    }

    bool RenderDelegate::OnProcessMessageReceived(CefRefPtr<QCefViewRenderApp> app,
        CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        CefProcessId source_process,
        CefRefPtr<CefProcessMessage> message)
    {
        const std::string& messageName = message->GetName();
        if (messageName == QCEF_GET_LINK_AT_POSITION) {
            onGetLinkAtPosition(browser, message->GetArgumentList());
            return true;
        } else if (messageName == QCEF_SET_HORIZONTAL_SCROLL) {
            onSetHorizontalScroll(browser, message->GetArgumentList());
            return true;
        } else if (messageName == QCEF_SET_VERTICAL_SCROLL) {
            onSetVerticalScroll(browser, message->GetArgumentList());
            return true;
        }

        return false;
    }

    void RenderDelegate::onGetLinkAtPosition(CefRefPtr<CefBrowser> browser, const CefRefPtr<CefListValue>& args)
    {
        // MessageBoxA(NULL, "getLinkAtPosition", "", MB_OK);
        auto v8ctx = browser->GetMainFrame()->GetV8Context();
        if (!v8ctx) {
            TRACEE("v8 context is null.");
            return;
        }
        CefString url;
        int x = 0, y = 0;
        v8ctx->Enter();
        do {
            if (args->GetSize() != 2) {
                TRACEW("args size error, %d", args->GetSize());
                break;
            }
            x = args->GetInt(0);
            y = args->GetInt(1);

            std::string script = "document.elementFromPoint(" + std::to_string(x) + ", " + std::to_string(y) + ").href";

            CefRefPtr<CefV8Value> ret;
            CefRefPtr<CefV8Exception> excp;
            if (!v8ctx->Eval(script.c_str(), browser->GetMainFrame()->GetURL(), 0, ret, excp)) {
                TRACEW("eval js failed, %s", script.c_str());
                break;
            }
            if (!ret || !ret->IsString()) {
                TRACEW("eval js failed, no return: %s", script.c_str());
                break;
            }
            url = ret->GetStringValue();
        } while (false);
        CefRefPtr<CefProcessMessage> outMsg = CefProcessMessage::Create("getLinkAtPositionRet");
        auto argList = outMsg->GetArgumentList();
        argList->SetSize(3);
        argList->SetString(0, url);
        argList->SetInt(1, x);
        argList->SetInt(2, y);
        browser->GetFocusedFrame()->SendProcessMessage(PID_BROWSER, outMsg);
        v8ctx->Exit();
    }

    void RenderDelegate::onSetHorizontalScroll(CefRefPtr<CefBrowser> browser, const CefRefPtr<CefListValue>& args)
    {
        auto v8ctx = browser->GetMainFrame()->GetV8Context();
        if (!v8ctx) {
            TRACEE("v8 context is null.");
            return;
        }
        if (args->GetSize() != 2) {
            TRACEW("args size error, %d", args->GetSize());
            return;
        }

        int nAbsolutePixelScroll = 0, scrollY = 0;
        int docWidth = 0, windowWidth = 0;
        v8ctx->Enter();
        do {
            CefRefPtr<CefV8Value> ret;
            CefRefPtr<CefV8Exception> excp;

            nAbsolutePixelScroll = args->GetInt(0);
            scrollY = args->GetInt(1);

            std::string script = "window.scrollTo(";
            script = script + std::to_string(nAbsolutePixelScroll) + "," + std::to_string(scrollY) + ")";
            v8ctx->Eval(script.c_str(), browser->GetMainFrame()->GetURL(), 0, ret, excp);

            script = "document.documentElement.clientWidth";
            if (!v8ctx->Eval(script.c_str(), browser->GetMainFrame()->GetURL(), 0, ret, excp)) {
                TRACEW("eval js failed, %s", script.c_str());
                break;
            }
            if (!ret || !ret->IsInt()) {
                TRACEW("eval js failed, no return: %s", script.c_str());
                break;
            }
            docWidth = ret->GetIntValue();

            script = "window.innerWidth";
            if (!v8ctx->Eval(script.c_str(), browser->GetMainFrame()->GetURL(), 0, ret, excp)) {
                TRACEW("eval js failed, %s", script.c_str());
                break;
            }
            if (!ret || !ret->IsInt()) {
                TRACEW("eval js failed, no return: %s", script.c_str());
                break;
            }
            windowWidth = ret->GetIntValue();
        } while (false);
        CefRefPtr<CefProcessMessage> outMsg = CefProcessMessage::Create("onSetHorizontalScrollRet");
        auto argList = outMsg->GetArgumentList();
        argList->SetSize(2);
        argList->SetInt(0, docWidth);
        argList->SetInt(1, windowWidth);
        browser->GetFocusedFrame()->SendProcessMessage(PID_BROWSER, outMsg);
        v8ctx->Exit();
    }

    void RenderDelegate::onSetVerticalScroll(CefRefPtr<CefBrowser> browser, const CefRefPtr<CefListValue>& args)
    {
        auto v8ctx = browser->GetMainFrame()->GetV8Context();
        if (!v8ctx) {
            TRACEE("v8 context is null.");
            return;
        }
        if (args->GetSize() != 2) {
            TRACEW("args size error, %d", args->GetSize());
            return;
        }

        int nAbsolutePixelScroll = 0, scrollY = 0;
        int docHeight = 0, windowHeight = 0;
        v8ctx->Enter();
        do {
            CefRefPtr<CefV8Value> ret;
            CefRefPtr<CefV8Exception> excp;

            nAbsolutePixelScroll = args->GetInt(0);
            scrollY = args->GetInt(1);

            std::string script = "window.scrollTo(";
            script = script + std::to_string(nAbsolutePixelScroll) + "," + std::to_string(scrollY) + ")";
            v8ctx->Eval(script.c_str(), browser->GetMainFrame()->GetURL(), 0, ret, excp);

            script = "document.documentElement.clientHeight";
            if (!v8ctx->Eval(script.c_str(), browser->GetMainFrame()->GetURL(), 0, ret, excp)) {
                TRACEW("eval js failed, %s", script.c_str());
                break;
            }
            if (!ret || !ret->IsInt()) {
                TRACEW("eval js failed, no return: %s", script.c_str());
                break;
            }
            docHeight = ret->GetIntValue();

            script = "window.innerHeight";
            if (!v8ctx->Eval(script.c_str(), browser->GetMainFrame()->GetURL(), 0, ret, excp)) {
                TRACEW("eval js failed, %s", script.c_str());
                break;
            }
            if (!ret || !ret->IsInt()) {
                TRACEW("eval js failed, no return: %s", script.c_str());
                break;
            }
            windowHeight = ret->GetIntValue();
        } while (false);
        CefRefPtr<CefProcessMessage> outMsg = CefProcessMessage::Create("onSetVerticalScrollRet");
        auto argList = outMsg->GetArgumentList();
        argList->SetSize(2);
        argList->SetInt(0, docHeight);
        argList->SetInt(1, windowHeight);
        browser->GetFocusedFrame()->SendProcessMessage(PID_BROWSER, outMsg);
        v8ctx->Exit();
    }
}