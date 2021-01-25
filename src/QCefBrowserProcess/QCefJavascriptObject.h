#pragma once;
#include <include/cef_v8.h>
#include "QCefMetaObject.h"

#include <QString>
#include <QVector>
#include <QPair>
#include <QMap>

/**
 * @brief ArrayBuffer释放回调类
 * @author Alex.peng
 */
class ReleaseCallback : public CefV8ArrayBufferReleaseCallback {
public:
    void ReleaseBuffer(void* buffer) override { std::free(buffer); }
    IMPLEMENT_REFCOUNTING(ReleaseCallback);
};

/**
 * @brief JS回调保存对象
 * @author Alex.peng
 */
struct QCefFunctionCallback {
    CefRefPtr<CefV8Context> context;
    CefRefPtr<CefV8Value> callback;
};

/**
 * @brief CEF函数对象
 * 回调函数信息记录在本对象内
 * @author Alex.peng
 */
class QCefFunctionObject : public CefV8Handler {
public:
    QCefFunctionObject(JavaScriptMetaMethod metaMethod, CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame);
    ~QCefFunctionObject();
    virtual bool Execute(const CefString& name, CefRefPtr<CefV8Value> object, const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval, CefString& exception) OVERRIDE;
    /**
     * @brief 执行回调函数
     * @author Alex.peng
     */
    bool ExecuteCallback(const CefString& signature, CefRefPtr<CefV8Value> object, CefRefPtr<CefListValue> arguments, CefRefPtr<CefV8Value>& retval, CefString& exception);
    /**
     * @brief 移除回调函数
     * @author Alex.peng
     */
    bool RemoveCallback(const CefString& signature);

    /**
     * @brief 读取同步值
     * @author Alex.peng
     */
    static CefRefPtr<CefV8Value> readSynchronizeValue(const QString& retTypeSignature, int retType);

private:
    JavaScriptMetaMethod m_metaMethod;                              //函数元对象信息
    CefRefPtr<CefBrowser> m_browser;
    CefRefPtr<CefFrame> m_frame;
    QMap<CefString, QCefFunctionCallback> m_callbacksMap;
    IMPLEMENT_REFCOUNTING(QCefFunctionObject);
};

/**
 * @brief Qt对象注册到JS中的代理对象
 *  主要实现Get和Set用于获取/设置属性/函数
 * @author Alex.peng
 */
class QCefJavaScriptObject : public CefV8Accessor {
public:
    QCefJavaScriptObject(JavaScriptMetaObject metaObj, CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame);
    ~QCefJavaScriptObject();
    
    /**
     * @brief 以指定的注册函数/属性到JS引擎
     * @author Alex.peng
     * @param
     *  pJsEnv                              JS环境对象
     *  registerName:                       要注册的对象名
     *  cefParentObj:                       父对象，注册对象将作为该对象的属性
     */
    bool registerObject(class QCefJavaScriptEnvironment* pJsEnv, CefString registerName, CefRefPtr<CefV8Value> cefParentObj);

    /**
     * @brief 根据函数名获取函数对象
     * @author Alex.peng
     */
    CefRefPtr<QCefFunctionObject> getFunction(const QString& functionName);

    /**
     * @brief 通过SetValue直接添加属性（不经过Get和Set）
     * @author Alex.peng
     */
    bool addNewV8ValueDirectly(const JavaScriptMetaProperty& propInfo, const CefRefPtr<CefV8Value> v8Value);

    /**
     * @brief 更新MetaProperty
     * @author Alex.peng
     */
    bool updateMetaProperty(JavaScriptMetaProperty* pProperty, const CefRefPtr<CefV8Value> v8Value);

public:
    virtual bool Get(const CefString& name, const CefRefPtr<CefV8Value> object, CefRefPtr<CefV8Value>& retval, CefString& exception) OVERRIDE;
    virtual bool Set(const CefString& name, const CefRefPtr<CefV8Value> object, const CefRefPtr<CefV8Value> value, CefString& exception) OVERRIDE;

private:

    /**
     * @brief 获取动态属性值，返回CefV8Value
     * @author Alex.peng
     */
    CefRefPtr<CefV8Value> retrieveDynamicProperty(const QString& registerName, const JavaScriptMetaProperty& propInfo);

    /**
     * @brief 更新动态属性
     * @author Alex.peng
     */
    bool updateDynamicProperty(const QString& registerName, const JavaScriptMetaProperty& propInfo, const CefRefPtr<CefV8Value>& value);

    /**
     * @brief 生成一个v8 value
     * @author Alex.peng
     */
    CefRefPtr<CefV8Value> genV8PropertyValue(const QVariant& value);

private:
    JavaScriptMetaObject m_metaObject;                                  //元对象信息，保存了一个要注册的Qt对象完整信息
    CefRefPtr<CefBrowser> m_browser;                                    //浏览器实例对象
    CefRefPtr<CefFrame> m_frame;                                        //Frame实例对象
    QMap<QString, CefRefPtr<QCefFunctionObject>> m_functionMap;         //对象函数表
    QString m_registerName;                                             //注册名，我们需要它来确定动态属性的pipe name
    class QCefJavaScriptEnvironment* m_pJsEnv = nullptr;                //js环境根对象
    IMPLEMENT_REFCOUNTING(QCefJavaScriptObject);
};
