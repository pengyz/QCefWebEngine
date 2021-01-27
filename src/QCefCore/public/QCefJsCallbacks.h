#pragma once;

#include "qcefcore_export.h"
#include <QVariantList>
#include <QString>
#include <QMetaType>
#include <QSharedPointer>
#include <QVector>

bool getSignatureIdentifier(const QString& jsCallbackSignature, int& browerId, qint64& frameId);
QStringList getCallbackSignatureList(const QString& jsCallbackSignature);


class QCEFCORE_EXPORT JavaScriptCallback {
public:
    JavaScriptCallback(const QString& signature, class QCefCoreManagerBase* coreManager);
    JavaScriptCallback();
    ~JavaScriptCallback();
    bool isValid();
    void clear();
    void trigger(const QVariantList& vars);
    int getBrowserId();
    qint64 getFrameId();

public:
    QString m_callbackSignature;
    class QCefCoreManagerBase* m_coreManager = nullptr;
protected:
    int m_browserId = 0;
    qint64 m_frameId = 0;
};

class QCEFCORE_EXPORT JavaScriptGetDataCallback : public JavaScriptCallback {
public:
    JavaScriptGetDataCallback(const QString& signature, class QCefCoreManagerBase* coreManager);
    JavaScriptGetDataCallback();
    void execute(int code, const QString& data, const QString& failReason);

protected:
    QString buildGetDataJsonResult(int status, const QString& data, const QString& error = "");
};
typedef QSharedPointer<JavaScriptGetDataCallback> JavaScriptGetDataCallbackPtr;
Q_DECLARE_METATYPE(JavaScriptGetDataCallback);
Q_DECLARE_METATYPE(JavaScriptGetDataCallbackPtr);

class QCEFCORE_EXPORT JavaScriptStubCallback : public JavaScriptCallback {
public:
    JavaScriptStubCallback();
    JavaScriptStubCallback(const QString& signature, class QCefCoreManagerBase* coreManager);
    void execute(const QString& json, const QByteArray& binary);
};
typedef QSharedPointer<JavaScriptStubCallback> JavaScriptStubCallbackPtr;
Q_DECLARE_METATYPE(JavaScriptStubCallback);
Q_DECLARE_METATYPE(JavaScriptStubCallbackPtr);

class QCEFCORE_EXPORT JavaScriptEventCallback : public JavaScriptCallback {
public:
    JavaScriptEventCallback();
    JavaScriptEventCallback(const QString& signature, class QCefCoreManagerBase* coreManager);
    void execute(const QString& strEventInfo);
};
typedef QSharedPointer<JavaScriptEventCallback> JavaScriptEventCallbackPtr;
Q_DECLARE_METATYPE(JavaScriptEventCallback);
Q_DECLARE_METATYPE(JavaScriptEventCallbackPtr);


class QCEFCORE_EXPORT JavaScriptCallbacksCollection {
    friend class QCefCoreManagerBase;
private:
    JavaScriptCallbacksCollection(const QVector<QSharedPointer<JavaScriptCallback>>& callbacks)
        :_callbacks(callbacks)
    {
    }
public:
    JavaScriptCallbacksCollection()
    {}

public:
    template<typename T>
    QSharedPointer<T> get(int index) const
    {
        if (index < 0 || index >= _callbacks.size())
            return QSharedPointer<T>(new T());
        auto baseCallback = _callbacks[index];
        auto result = qSharedPointerCast<T>(baseCallback);
        return result;
    }

    int size() const
    {
        return _callbacks.size();
    }

private:
    QVector<QSharedPointer<JavaScriptCallback>> _callbacks;
};
Q_DECLARE_METATYPE(JavaScriptCallbacksCollection);