#include "QCefObjectProtocol.h"
#include "QCefProtocol.h"

#include <qt_windows.h>
#include <QMetaMethod>
#include <QMetaProperty>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QByteArray>
#include <QByteArrayList>


QCefObjectProtocol* QCefObjectProtocol::m_instance = nullptr;

QCefObjectProtocol* QCefObjectProtocol::get()
{
    if (!m_instance)
        m_instance = new QCefObjectProtocol();
    return m_instance;
}

QCefObjectProtocol::QCefObjectProtocol()
{
    m_sharedMemory = new QSharedMemory(this);
}

QCefObjectProtocol::~QCefObjectProtocol()
{}

bool QCefObjectProtocol::init()
{
    m_sharedMemory->setKey(QString(QCEF_JSBRIDGE_NAME).arg(GetCurrentProcessId()));
    if (!m_sharedMemory->create(SHARED_MEMORY_SIZE, QSharedMemory::ReadWrite)) {
        //TRACEE("init QCefJavaScriptObjectBridge shared memory create failed! %s", qPrintable(m_sharedMemory->errorString()));
        return false;
    }
    if (m_sharedMemory->error() == QSharedMemory::NoError) {
        m_firstInit = true;
    } else if (m_sharedMemory->error() != QSharedMemory::AlreadyExists) {
        m_firstInit = false;
        //TRACEE("init QCefJavaScriptObjectBridge shared memory attach failed! %s", qPrintable(m_sharedMemory->errorString()));
        return false;
    } else {
        //use read write mode for the first init
        if (!m_sharedMemory->attach(QSharedMemory::ReadWrite)) {
            //TRACEE("init QCefJavaScriptObjectBridge shared memory attach failed! %s", qPrintable(m_sharedMemory->errorString()));
            return false;
        }
    }
    return true;
}

QString QCefObjectProtocol::readJsObjectRegisterInfo()
{
    QString jsBindingInfo;
    if (m_sharedMemory->lock()) {
        jsBindingInfo = QString::fromUtf8((char*)m_sharedMemory->data());
        m_sharedMemory->unlock();
    }
    return jsBindingInfo;
}

bool QCefObjectProtocol::writeJsObjectRegisterInfo(const QString& jsonInfo)
{
    if (!m_firstInit) {
        //TRACED("JSBinding can not write beacuse not first init");
        return false;
    }
    QByteArray jsonByts = jsonInfo.toUtf8();
    int iSize = jsonByts.size();
    Q_ASSERT(iSize < SHARED_MEMORY_SIZE); //check buffer size enough

    if (m_sharedMemory->lock()) {
        qstrncpy(reinterpret_cast<char*>(m_sharedMemory->data()), jsonByts.constData(), SHARED_MEMORY_SIZE);
        m_sharedMemory->unlock();
    }
    return true;
}

bool QCefObjectProtocol::registerJavaScriptHandlerObject(const QString& registerName, const QObject* registerObject)
{
    QString jsonInfo = readJsObjectRegisterInfo();
    QJsonDocument jsonDoc;
    if (!jsonInfo.isEmpty()) {
        QJsonParseError jsonError;
        jsonDoc = QJsonDocument::fromJson(jsonInfo.toUtf8(), &jsonError);
        Q_ASSERT_X(jsonError.error == QJsonParseError::NoError, __FUNCTION__, "parse json failed!");
        if (jsonError.error != QJsonParseError::NoError) {
            //TRACEE("parse json failed ! %s", qPrintable(jsonError.errorString()));
            return false;
        }
    }

    auto metaObj = registerObject->metaObject();
    QJsonObject registerObjInfo;
    registerObjInfo.insert("registerName", registerName);
    registerObjInfo.insert("className", metaObj->className());
    QJsonObject functionsObject;
    QJsonArray signalFunctionsArray;
    QJsonArray slotsFunctionsArray;
    for (int i = 0; i < metaObj->methodCount(); i++) {
        QMetaMethod metaMethod = metaObj->method(i);
        if (metaMethod.access() != QMetaMethod::Public)
            continue;
        bool bSignalSlot = metaMethod.methodType() != QMetaMethod::Signal || metaMethod.methodType() != QMetaMethod::Slot;
        if (!bSignalSlot)
            continue;
        //ignore some useless function
        // __Wrapped__ function should not be registered to JS side, it just an ugly work around.
        if (metaMethod.name() == "destroyed" || metaMethod.name() == "deleteLater"
            || metaMethod.name() == "objectNameChanged" || metaMethod.name().endsWith("___Wrapped___"))
            continue;

        QByteArrayList paramNameList = metaMethod.parameterNames();
        QByteArrayList paramTypeList = metaMethod.parameterTypes();
        QJsonObject functionInfo;
        functionInfo.insert("functionName", QString::fromUtf8(metaMethod.name()));
        functionInfo.insert("functionSignature", QString::fromUtf8(metaMethod.methodSignature()));
        functionInfo.insert("returnType", metaMethod.returnType());
        QJsonArray paramList;
        QString errorInfo;
        bool bCheckOk = validateWrappedFunction(metaObj, metaMethod, errorInfo);
        Q_ASSERT_X(bCheckOk, "validateWrappedFunction", qPrintable(errorInfo));

        for (int j = 0; j < metaMethod.parameterCount(); j++) {
            int paramTypeId = metaMethod.parameterType(j);
            QJsonObject paramData;
            if (j == 0) {
                Q_ASSERT_X(paramTypeList[j] == "JavaScriptCallbacksCollection", "check",
                    "first param type must be JavaScriptCallbacksCollection");
                if (paramTypeList[j] != "JavaScriptCallbacksCollection") {
                    //TRACEE("method: %s not valid !", metaMethod.name().constData());
                }
            }
            paramData.insert("name", QString::fromUtf8(paramNameList[j]));
            paramData.insert("type", QString::fromUtf8(paramTypeList[j]));
            paramList.append(paramData);
        }
        functionInfo.insert("params", paramList);
        if (metaMethod.methodType() == QMetaMethod::Signal)
            signalFunctionsArray.append(functionInfo);
        if (metaMethod.methodType() == QMetaMethod::Slot)
            slotsFunctionsArray.append(functionInfo);
    }
    functionsObject.insert("signals", signalFunctionsArray);
    functionsObject.insert("slots", slotsFunctionsArray);
    registerObjInfo.insert("functions", functionsObject);

    //properties
    QJsonArray propertyArray;
    for (int i = 0; i < metaObj->propertyCount(); i++) {
        QMetaProperty prop = metaObj->property(i);
        if (!prop.isReadable() || !strcmp(prop.name(), "objectName"))
            continue;

        QJsonObject propObj;
        propObj.insert("name", prop.name());
        propObj.insert("typeName", prop.typeName());
        propObj.insert("typeId", prop.userType());
        propObj.insert("readable", prop.isReadable());
        propObj.insert("writable", prop.isWritable());
        propObj.insert("value", QJsonValue::fromVariant(prop.read(registerObject)));
        propObj.insert("isDynamicProperty", prop.isFinal()); //我们把final属性作为是否是动态属性的标记,默认应该是false
        propertyArray.append(propObj);
    }
    registerObjInfo.insert("properties", propertyArray);

    QJsonArray jsonArray = jsonDoc.array();
    if (!jsonArray.isEmpty()) {
        for (int i = 0; i < jsonArray.size(); i++) {
            auto obj = jsonArray[i].toObject();
            if (obj.contains("registerName") && obj.value("registerName").toString() == registerName) {
                jsonArray.takeAt(i);
                break;
            }
        }
    } else {
        jsonArray = QJsonArray();
    }

    jsonArray.append(registerObjInfo);
    jsonDoc.setArray(jsonArray);
    QString jsonStr = jsonDoc.toJson(QJsonDocument::Compact);
    //TRACED("jsonStr is: %s", qPrintable(jsonStr));
    writeJsObjectRegisterInfo(jsonStr); //write to shared memory
    return true;
}

bool QCefObjectProtocol::validateWrappedFunction(const QMetaObject* metaObj, const QMetaMethod& method, QString& error)
{
    if (method.parameterCount() <= 10)
        return true;
    //check for ___Wrapped___
    QString newName = method.name().append("___Wrapped___");
    auto params = method.parameterTypes().mid(0, 9);
    params.append(QByteArray("QVariantList"));
    QString paramList = params.join(",");
    QString newFunctionSignature = QMetaObject::normalizedSignature(QString("%1(%2)").arg(newName).arg(paramList).toLocal8Bit().data());
    int iWrapped = metaObj->indexOfMethod(newFunctionSignature.toUtf8());
    if (iWrapped == -1) {
        error = QString("need function: %1 but not found !").arg(newFunctionSignature);
        return false;
    }
    auto wrapped = metaObj->method(iWrapped);
    if (wrapped.access() != QMetaMethod::Public) {
        error = QString("function : %1 must be public").arg(newFunctionSignature);
        return false;
    }
    return true;
}
