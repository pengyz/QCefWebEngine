#include <QString>
#include <QStringList>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#pragma region project_headers
#include "QCefViewDefaultRenderDelegate.h"
#include "QCefClient.h"
#include "QCefJavaScriptBinder.h"
#pragma endregion project_headers

namespace QCefViewDefaultRenderDelegate {
    void CreateBrowserDelegate(QCefViewRenderApp::RenderDelegateSet& delegates)
    {
        delegates.insert(new RenderDelegate());
    }

    RenderDelegate::RenderDelegate() {}

    void RenderDelegate::OnWebKitInitialized(CefRefPtr<QCefViewRenderApp> app)
    {
        //weblit initialized, so notify page creation
    }

    void RenderDelegate::OnContextCreated(CefRefPtr<QCefViewRenderApp> app,
            CefRefPtr<CefBrowser> browser,
            CefRefPtr<CefFrame> frame,
            CefRefPtr<CefV8Context> context)
    {
        int64 frameId = frame->GetIdentifier();
        auto it = frame_id_to_client_map_.find(frameId);
        if (it == frame_id_to_client_map_.end()) {
            // create and insert the QCefClient Object into this frame.window object
            CefRefPtr<CefV8Value> objWindow = context->GetGlobal();
            CefRefPtr<QCefClient> objClient = new QCefClient(browser, frame);
            objWindow->SetValue(QCEF_OBJECT_NAME, objClient->GetObject(), V8_PROPERTY_ATTRIBUTE_READONLY);
            frame_id_to_client_map_[frameId] = objClient;
        }
    }

    void RenderDelegate::OnContextReleased(CefRefPtr<QCefViewRenderApp> app,
            CefRefPtr<CefBrowser> browser,
            CefRefPtr<CefFrame> frame,
            CefRefPtr<CefV8Context> context)
    {
        int64 frameId = frame->GetIdentifier();
        auto it = frame_id_to_client_map_.find(frameId);
        if (it != frame_id_to_client_map_.end()) {
            frame_id_to_client_map_.erase(it);
        }
    }

    bool RenderDelegate::OnProcessMessageReceived(CefRefPtr<QCefViewRenderApp> app,
            CefRefPtr<CefBrowser> browser,
            CefRefPtr<CefFrame> frame,
            CefProcessId source_process,
            CefRefPtr<CefProcessMessage> message)
    {
        if (OnTriggerEventNotifyMessage(browser, frame, source_process, message)) {
            return true;
        }

        return false;
    }

    void RenderDelegate::OnBrowserCreated(CefRefPtr<QCefViewRenderApp> app,
        CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefDictionaryValue> extra_info)
    {
    }

    bool RenderDelegate::OnTriggerEventNotifyMessage(CefRefPtr<CefBrowser> browser,
            CefRefPtr<CefFrame> frame,
            CefProcessId source_process,
            CefRefPtr<CefProcessMessage> message)
    {
        //TRACET();
        CefString messageName = message->GetName();
        if (messageName == QCEF_INVOKENGLCALLBACK) {
            CefRefPtr<CefListValue> messageArguments = message->GetArgumentList();
            if (!messageArguments)
                return false;
            if (messageArguments->GetSize() < 2)
                return false;
            size_t idx = 0;
            if (CefValueType::VTYPE_STRING != messageArguments->GetType(idx))
                return false;
            QString strSignature;
            strSignature = QString::fromStdWString(messageArguments->GetString(idx++).ToWString());
            auto sigList = strSignature.split(".");
            if (sigList.size() != QCEF_SIGNATURE_VALID_PARTS_COUNT)
                return false;
            int browserId = sigList[0].toInt();
            int64 frameId = sigList[1].toLongLong();
            //not the same browser, return it.
            if (browserId != browser->GetIdentifier())
                return false;
            CefRefPtr<CefListValue> newArguments = CefListValue::Create();
            int iNewIdx = 0;
            for (idx; idx < messageArguments->GetSize(); idx++) {
                newArguments->SetValue(iNewIdx++, messageArguments->GetValue(idx));
            }
            //int64 frameId = frame->GetIdentifier();
            auto it = frame_id_to_client_map_.find(frameId);
            if (it != frame_id_to_client_map_.end()) {
                it->second->invokeCallBack(strSignature, newArguments);
            } else {
                //TRACEE("QCEF_INVOKENGLCALLBACK can't find QCefClient by id: %ld", frameId);
            }
        } else if (messageName == QCEF_CLEARNGLCALLBACKS) {
            CefRefPtr<CefListValue> messageArguments = message->GetArgumentList();
            if (!messageArguments)
                return false;
            if (messageArguments->GetSize() < 1)
                return false;
            int idx = 0;
            if (CefValueType::VTYPE_STRING != messageArguments->GetType(idx))
                return false;
            QString strSignature;
            strSignature = QString::fromStdWString(messageArguments->GetString(idx++).ToWString());

            //handle callback signatures, compatible with json and splited string list.
            QJsonParseError jsonError;
            QJsonDocument jsonDoc = QJsonDocument::fromJson(strSignature.toUtf8(), &jsonError);
            if (jsonError.error == QJsonParseError::NoError) {
                //get from json
                QJsonObject jsonObj = jsonDoc.object();
                if (jsonObj.contains("callbacks")) {
                    strSignature = jsonObj["callbacks"].toString();
                }
            }

            QMap<int64, QStringList> frameCallbackMap;
            QStringList sigList = strSignature.split(";", QString::SkipEmptyParts);
            for (const auto& callbackSig : sigList) {
                auto sigParts = callbackSig.split(".");
                if (sigParts.size() != QCEF_SIGNATURE_VALID_PARTS_COUNT)
                    continue;
                int browserId = sigParts[0].toInt();
                int64 frameId = sigParts[1].toLongLong();
                if (!frameCallbackMap.contains(frameId)) {
                    frameCallbackMap[frameId] = QStringList() << callbackSig;
                } else {
                    frameCallbackMap[frameId] = frameCallbackMap[frameId] << callbackSig;

                }
            }
            for (auto iter = frameCallbackMap.begin(); iter != frameCallbackMap.end(); ++iter) {
                int64 frameId = iter.key();
                QStringList sigs = iter.value();
                auto it = frame_id_to_client_map_.find(frameId);
                if (it != frame_id_to_client_map_.end()) {
                    it->second->clearFunctionCallbacks(sigs);
                } else {
                    //TRACEE("QCEF_CLEARNGLCALLBACKS can't find QCefClient by id: %ld", frameId);
                }
            }
        }

        return false;
    }
}