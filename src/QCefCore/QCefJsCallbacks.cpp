#include "public/QCefJsCallbacks.h"
#include "public/QCefJavaScriptEngine.h"
#include "public/CefCoreBrowser.h"
#include "public/QCefCoreManagerBase.h"
#include "include/baseutility.h"
#include "include/jsonbuilder.h"

#include <QJsonArray>


JavaScriptCallback::JavaScriptCallback(const QString& signature, QCefCoreManagerBase* coreManager)
    : m_callbackSignature(signature), m_coreManager(coreManager)
{
}

JavaScriptCallback::JavaScriptCallback()
{
}

JavaScriptCallback::~JavaScriptCallback()
{
    if (m_callbackSignature.isEmpty() || !m_coreManager)
        return;
    m_coreManager->clearJavaScriptCallback(m_callbackSignature);
}

bool JavaScriptCallback::isValid()
{
    return !m_callbackSignature.isEmpty();
}

void JavaScriptCallback::clear()
{
    m_callbackSignature.clear();
}

void JavaScriptCallback::trigger(const QVariantList& vars)
{
    if (m_callbackSignature.isEmpty() || !m_coreManager)
        return;
    m_coreManager->invokeJavaScriptCallback(m_callbackSignature, vars);
}

int JavaScriptCallback::getBrowserId()
{
    if (!m_callbackSignature.isEmpty() && !m_browserId) {
        getSignatureIdentifier(m_callbackSignature, m_browserId, m_frameId);
    }
    return m_browserId;
}

qint64 JavaScriptCallback::getFrameId()
{
    if (!m_callbackSignature.isEmpty() && !m_frameId) {
        getSignatureIdentifier(m_callbackSignature, m_browserId, m_frameId);
    }
    return m_frameId;
}

//////////////////////////////////////////////////////////////////////////
JavaScriptReturnValueCallback::JavaScriptReturnValueCallback(const QString& signature)
    :JavaScriptCallback(signature, nullptr)
{
}

JavaScriptReturnValueCallback::JavaScriptReturnValueCallback()
    : JavaScriptCallback()
{
}

JavaScriptReturnValueCallback::~JavaScriptReturnValueCallback()
{
    execute(0);
}

void JavaScriptReturnValueCallback::execute(QVariant value)
{
    //写入返回值
    if (!isValid())
        return;
    bool bSucc = QCefJavaScriptEngine::get()->writeSynchronizeValue(m_callbackSignature, value);
    //如果已成功处理返回值，清空signature，避免析构时继续写入
    if (bSucc)
        m_callbackSignature.clear();
}


JavaScriptGetDataCallback::JavaScriptGetDataCallback()
    :JavaScriptCallback()
{
}

JavaScriptGetDataCallback::JavaScriptGetDataCallback(const QString& signature, class QCefCoreManagerBase* coreManager)
    : JavaScriptCallback(signature, coreManager)
{

}

void JavaScriptGetDataCallback::execute(int code, const QString& data, const QString& failReason)
{
    if (!isValid())
        return;
    QString strData = buildGetDataJsonResult(code, data, failReason);
    trigger({ strData });
}

QString JavaScriptGetDataCallback::buildGetDataJsonResult(int status, const QString& data, const QString& error)
{
    QJsonParseError jsonError;
    QString strResultCallback;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data.toUtf8(), &jsonError);
    if (jsonError.error != QJsonParseError::NoError) {
        strResultCallback = JsonBuilder().add("status", status).add("msg", error).add("data", data).build();
    } else {
        QJsonValue dataJsonValue;
        if (jsonDoc.isArray()) {
            dataJsonValue = jsonDoc.array();
        } else if (jsonDoc.isObject()) {
            dataJsonValue = jsonDoc.object();
        }
        strResultCallback = JsonBuilder().add("status", status).add("msg", error).add("data", dataJsonValue).build();
    }
    return strResultCallback;
}

////////////////////////////////////////////////////////////////////////////////////////

JavaScriptStubCallback::JavaScriptStubCallback()
    :JavaScriptCallback()
{
}

JavaScriptStubCallback::JavaScriptStubCallback(const QString& signature, class QCefCoreManagerBase* coreManager)
    : JavaScriptCallback(signature, coreManager)
{
}

void JavaScriptStubCallback::execute(const QString& json, const QByteArray& binary)
{
    if (isValid())
        trigger({ json, binary });
}

JavaScriptEventCallback::JavaScriptEventCallback()
    :JavaScriptCallback()
{

}

JavaScriptEventCallback::JavaScriptEventCallback(const QString& signature, class QCefCoreManagerBase* coreManager)
    : JavaScriptCallback(signature, coreManager)
{
}

void JavaScriptEventCallback::execute(const QString& strEventInfo)
{
    if (isValid())
        trigger({ strEventInfo });
}

JavaScriptReturnValueCallbackPtr JavaScriptCallbacksCollection::getReturnCallback() const
{
    return _returnCallback;
}
