#include "public/QCefJsCallbacks.h"
#include "public/QCefJavaScriptEngine.h"
#include "public/CefCoreBrowser.h"
#include "public/QCefCoreManagerBase.h"
#include "QCefProtocol.h"

#include <QJsonArray>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>


bool getSignatureIdentifier(const QString& jsCallbackSignature, int& browerId, qint64& frameId)
{
    browerId = 0;
    frameId = 0;
    auto signatures = jsCallbackSignature.split(";", QString::SkipEmptyParts);
    if (signatures.size() != 1)
        return false;
    auto pairs = signatures[0].split(".", QString::SkipEmptyParts);
    if (pairs.size() != QCEF_SIGNATURE_VALID_PARTS_COUNT)
        return false;
    browerId = pairs[0].toInt();
    frameId = pairs[1].toLongLong();
    return true;
}

QStringList getCallbackSignatureList(const QString& jsCallbackSignature)
{
    //TRACED("jsCallbackSignature is: %s", qPrintable(jsCallbackSignature));
    QStringList signatureList;
    QJsonParseError jsonError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsCallbackSignature.toUtf8(), &jsonError);
    if (jsonError.error != QJsonParseError::NoError)
        return signatureList;
    QJsonObject jsonObj = jsonDoc.object();
    if (!jsonObj.contains("callbacks"))
        return signatureList;
    auto sigsValue = jsonObj["callbacks"].toString();
    signatureList = sigsValue.split(";", QString::SkipEmptyParts);
    return signatureList;
}

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
        QJsonObject jsonObj;
        jsonObj["status"] = status;
        jsonObj["msg"] = error;
        jsonObj["data"] = data;
        strResultCallback = QJsonDocument(jsonObj).toJson(QJsonDocument::Indented);
        //strResultCallback = JsonBuilder().add("status", status).add("msg", error).add("data", data).build();
    } else {
        QJsonValue dataJsonValue;
        if (jsonDoc.isArray()) {
            dataJsonValue = jsonDoc.array();
        } else if (jsonDoc.isObject()) {
            dataJsonValue = jsonDoc.object();
        }
        //strResultCallback = JsonBuilder().add("status", status).add("msg", error).add("data", dataJsonValue).build();
        QJsonObject jsonObj;
        jsonObj["status"] = status;
        jsonObj["msg"] = error;
        jsonObj["data"] = dataJsonValue;
        strResultCallback = QJsonDocument(jsonObj).toJson(QJsonDocument::Indented);
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