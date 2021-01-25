#pragma once;
#include <QObject>
#include <QSharedMemory>
#include <QVariantList>

class QCefObjectProtocol : public QObject {
public:
    static QCefObjectProtocol* get();
    ~QCefObjectProtocol();
    bool init();
    //get signal/slot method infomation and write it to the shared memory
    bool registerJavaScriptHandlerObject(const QString& registerName, const QObject* registerObject);
    bool validateWrappedFunction(const QMetaObject* metaObj, const QMetaMethod& method, QString& error);

private:
    QCefObjectProtocol();
    QString readJsObjectRegisterInfo();
    bool writeJsObjectRegisterInfo(const QString& jsonInfo);

private:
    static QCefObjectProtocol* m_instance;
    QSharedMemory* m_sharedMemory = nullptr;
    const int SHARED_MEMORY_SIZE = 1024 * 1024 * 10;
    bool m_firstInit = true; //this shared memory must be write once.
};