#pragma once;
#include <QObject>
#include <include/cef_v8.h>

#include "QCefJavascriptObject.h"
#include "QCefJavaScriptBinder.h"


using JSMetaObjectPairInfo = QPair<CefRefPtr<QCefJavaScriptObject>, CefRefPtr<CefV8Value>>;

class QCefJavaScriptEnvironment {
    friend class QCefJavaScriptBinder;
private:
    QCefJavaScriptEnvironment(QObject* parent = nullptr);
public:
    virtual ~QCefJavaScriptEnvironment();

public:
    void addMetaObjectInfo(const QString& key, const CefRefPtr<QCefJavaScriptObject>& jsObject, const CefRefPtr<CefV8Value >& jsV8Value);
    /**
     * @brief 获取注册的JS对象信息
     * @author Alex.peng
     */
    JSMetaObjectPairInfo getMetaObjectInfo(const QString& key);
    bool invokeCallBack(const QString& signature, CefRefPtr<CefListValue> argumentList);
    bool clearFunctionCallbacks(const QStringList& signatures);

private:
    QMap<QString, JSMetaObjectPairInfo> m_javaScriptObjectMap;
};