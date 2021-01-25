#pragma once;
#include <QObject>
#include <QSharedMemory>
#include <QVector>
#include <QMap>
#include <QVariantList>
#include <QPair>

#include "QCefJavaScriptEnvironment.h"

class QCefJavaScriptBinder : public QObject {
    Q_OBJECT
private:
    QCefJavaScriptBinder() = default;
public:
    static QCefJavaScriptBinder* get();
    bool init();
    /**
     * @brief 对json注册信息进行反序列化
     * @author Alex.peng
     */
    bool initRegisterObjectsData(const QString& jsonData);
    /**
     * @brief 自动绑定Qt对象到JS，绑定根据Qt元对象信息，从共享内存中获取所有的注册信息并完成绑定
     * @param
     *  obj                     要绑定到的JS对象
     *  browser                 执行绑定的浏览器对象
     *  frame                   执行绑定的Frame对象
     * @author Alex.peng
     */
    void* bindAllObjects(CefRefPtr<CefV8Value> obj, CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame);
    /**
     * @brief 设置父进程Id，该Id用于给共享内存名做区分
     * @author Alex.peng
     */
    void setParentId(const QString& parentId);

private:
    static QCefJavaScriptBinder* m_instance;
    QSharedMemory* m_sharedMemory = nullptr;
    QMap<QString, JavaScriptMetaObject> m_javaScriptMetaObjectMap;      //函数绑定信息，从共享内存的json串中得到
    QString m_parentId;                                                 //launcher的PID，用于生成不同的共享内存名
};