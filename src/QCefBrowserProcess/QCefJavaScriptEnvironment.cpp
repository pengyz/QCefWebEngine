#include "QCefJavaScriptEnvironment.h"
#include "QCefProtocol.h"

QCefJavaScriptEnvironment::QCefJavaScriptEnvironment(QObject * parent)
{

}

QCefJavaScriptEnvironment::~QCefJavaScriptEnvironment()
{
    m_javaScriptObjectMap.clear();
}

void QCefJavaScriptEnvironment::addMetaObjectInfo(const QString & key, const CefRefPtr<QCefJavaScriptObject>& jsObject, const CefRefPtr<CefV8Value>& jsV8Value)
{
    m_javaScriptObjectMap.insert(key, JSMetaObjectPairInfo(jsObject, jsV8Value));
}

JSMetaObjectPairInfo QCefJavaScriptEnvironment::getMetaObjectInfo(const QString& key)
{
    if (!m_javaScriptObjectMap.contains(key))
        return JSMetaObjectPairInfo();
    return m_javaScriptObjectMap[key];
}

bool QCefJavaScriptEnvironment::invokeCallBack(const QString& signature, CefRefPtr<CefListValue> argumentList)
{
    //TRACED("signature is: %s", qPrintable(signature));
    auto sigList = signature.split(".");
    if (sigList.size() != QCEF_SIGNATURE_VALID_PARTS_COUNT)
        return false;
    int browserId = sigList[0].toInt();
    int frameId = sigList[1].toInt();
    const QString& className = sigList[2];
    const QString& method = sigList[3];
    //TRACED("className is: %s, method is: %s", qPrintable(className), qPrintable(method));

    if (!m_javaScriptObjectMap.contains(className)) {
        //TRACEE("js object map not found %s", qPrintable(className));
        return false;
    }
    QPair<CefRefPtr<QCefJavaScriptObject>, CefRefPtr<CefV8Value>> jsObjectPair = m_javaScriptObjectMap[className];
    if (!jsObjectPair.first || !jsObjectPair.second) {
        //TRACEE("%s: jsObjectPair invalid !", qPrintable(className));
        return false;
    }
    CefRefPtr<QCefFunctionObject> functionObj = jsObjectPair.first->getFunction(method);
    if (!functionObj) {
        //TRACEE("get functionObj failed !");
        return false;
    }

    CefRefPtr<CefV8Value> retVal;
    CefString exception;
    bool bRet = functionObj->ExecuteCallback(signature.toStdWString(), jsObjectPair.second, argumentList, retVal, exception);
    /*if (bRet) {
        TRACED("execute callback using signature: %s success !", qPrintable(signature));
    } else {
        TRACEE("execute callback using signature: %s failed !!!", qPrintable(signature));
    }*/
    return bRet;
}

bool QCefJavaScriptEnvironment::clearFunctionCallbacks(const QStringList& signatures)
{
    for (const auto& signature : signatures) {
        //TRACED("signature is: %s", qPrintable(signature));
        auto sigList = signature.split(".");
        if (sigList.size() != QCEF_SIGNATURE_VALID_PARTS_COUNT)
            return false;
        int browserId = sigList[0].toInt();
        int frameId = sigList[1].toInt();
        const QString& className = sigList[2];
        const QString& method = sigList[3];
        //TRACED("class is: %s, method is: %s", qPrintable(className), qPrintable(method));

        if (!m_javaScriptObjectMap.contains(className))
            return false;
        QPair<CefRefPtr<QCefJavaScriptObject>, CefRefPtr<CefV8Value>> jsObjectPair = m_javaScriptObjectMap[className];
        if (!jsObjectPair.first || !jsObjectPair.second)
            return false;
        CefRefPtr<QCefFunctionObject> functionObj = jsObjectPair.first->getFunction(method);
        if (!functionObj)
            return false;

        CefRefPtr<CefV8Value> retVal;
        CefString exception;
        functionObj->RemoveCallback(signature.toStdWString());
    }
    return true;
}