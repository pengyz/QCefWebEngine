#pragma once;
#include <QObject>
#include <QMap>
#include <QVector>
#include <QGenericArgument>
#include <QVariantList>

#include "QCefSetting.h"
#include "qcefwebengine_export.h"


class QCEFWEBENGINE_EXPORT QCefJavaScriptEngine : public QObject {
    Q_OBJECT
private:
    QCefJavaScriptEngine();
public:
    static QCefJavaScriptEngine* get();
    bool init();
    /**
     * @brief 通过给定的名字注册QObject对象
     * 获取注册QObject对象的元信息，记录对象，记录元信息等
     * @author Alex.peng
     */
    void registerObject(const QString& registerName, QObject* object);
    /**
     * @brief 获取返回值signature
     * @author Alex.peng
     */
    QString getReturnValueSignature(const QString& jsCallbackSignature);
    /**
     * @brief 通过信号槽的反射调用执行Qt槽函数，完成JS到C++的函数调用过程
     * @author Alex.peng
     */
    bool inovkeMethod(int browserId, const QVariantList& cefArguments, QString& callbackSignature);

    /**
     * @brief 获取属性值
     * @author Alex.peng
     */
    bool retieveProperty(const QString& propertySignature, QVariant& propValue);

    /**
     * @brief 设置属性值
     * @author Alex.peng
     */
    bool setProperty(const QString& propertySignature, const QString& propValue);

    /**
     * @brief 写入同步值
     * @author Alex.peng
     */
    bool writeSynchronizeValue(const QString returnTypeSignature, const QVariant& value);

private:
    /**
    * @brief 通过管道回写同步值，用于跨进程同步通信
    * @author Alex.peng
    */
    bool writeSynchronizeValueInner(const QString returnTypeSignature, void* value, int len);

private:
    static QCefJavaScriptEngine* m_instance;
    QMap<QString, QObject*> m_jsObjectBindingMap;
    QMap<QString, const QMetaObject*> m_registeredMetaObjectMap;
};
