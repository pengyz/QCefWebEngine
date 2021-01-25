#include "QCefJavaScriptBinder.h"
#include "QCefProtocol.h"
#include "QCefClient.h"
#include "QCefMetaObject.h"
#include "tracer.h"

#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>

QCefJavaScriptBinder* QCefJavaScriptBinder::m_instance = nullptr;

QCefJavaScriptBinder* QCefJavaScriptBinder::get()
{
    if (!m_instance)
        m_instance = new QCefJavaScriptBinder();
    return m_instance;
}

bool QCefJavaScriptBinder::init()
{
    TRACET();
    if (!m_sharedMemory) {
        m_sharedMemory = new QSharedMemory(this);
        Q_ASSERT(!m_parentId.isEmpty());
        m_sharedMemory->setKey(QString(QCEF_JSBRIDGE_NAME).arg(m_parentId));
    }
    if (!m_sharedMemory->isAttached()) {
        if (!m_sharedMemory->attach(QSharedMemory::AccessMode::ReadOnly)) {
            TRACEE("attach name: %s failed: %s", qPrintable(QString(QCEF_JSBRIDGE_NAME).arg(m_parentId)),
                qPrintable(m_sharedMemory->errorString()));
            return false;
        }
    }
    QString jsonInfo = QString::fromUtf8((char*)m_sharedMemory->data());
    return initRegisterObjectsData(jsonInfo);
}

bool QCefJavaScriptBinder::initRegisterObjectsData(const QString& jsonData)
{
    if (!jsonData.length()) {
        //do not have register object
        return true;
    }
    QJsonParseError jsonError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData.toUtf8().data(), &jsonError);
    if (jsonError.error != QJsonParseError::NoError) {
        TRACEE("json parse error: %s", qPrintable(jsonError.errorString()));
        return false;
    }

    m_javaScriptMetaObjectMap.clear();

    if (!jsonDoc.isArray())
        return false;

    auto getJsonValue = [](QJsonObject& jsonObj, const QString& key)->QJsonValue {
        if (!jsonObj.contains(key))
            return QJsonValue();
        return jsonObj[key];
    };

    auto jsonArray = jsonDoc.array();
    for (int i = 0; i < jsonArray.size(); i++) {
        auto infoObj = jsonArray[i].toObject();
        QString regName = getJsonValue(infoObj, "registerName").toString();
        QString className = getJsonValue(infoObj, "className").toString();
        if (regName.isEmpty() || className.isEmpty())
            continue;
        auto funcObj = getJsonValue(infoObj, "functions").toObject();
        auto propertyArray = getJsonValue(infoObj, "properties").toArray();
        QJsonArray signalsArray = getJsonValue(funcObj, "signals").toArray();
        QJsonArray slotsArray = getJsonValue(funcObj, "slots").toArray();

        auto parseFunction = [&](JavaScriptMetaObject& metaObject, QJsonArray& jsonArray, FunctionType type) {
            for (int i = 0; i < jsonArray.size(); i++) {
                auto oneFuncObj = jsonArray[i].toObject();
                JavaScriptMetaMethod funcInfo;
                funcInfo.name = getJsonValue(oneFuncObj, "functionName").toString();
                funcInfo.signature = getJsonValue(oneFuncObj, "functionSignature").toString();
                funcInfo.className = regName; //always use javascript object name instead of C++ class name.
                funcInfo.functionType = type;
                funcInfo.retType = getJsonValue(oneFuncObj, "returnType").toInt();
                auto paramsArray = getJsonValue(oneFuncObj, "params").toArray();
                for (int i = 0; i < paramsArray.size(); i++) {
                    auto paramObj = paramsArray[i].toObject();
                    QString paramName = getJsonValue(paramObj, "name").toString();
                    QString paramType = getJsonValue(paramObj, "type").toString();
                    JavaScriptMetaParam jsParamInfo(paramName, paramType);
                    if (!jsParamInfo.isValid())
                        continue;
                    funcInfo.params.append(jsParamInfo);
                }
                if (funcInfo.isValid())
                    metaObject.functions.append(funcInfo);
            }
        };

        auto parseProperty = [&](JavaScriptMetaObject& metaObject, QJsonArray& jsonArray) {
            for (int i = 0; i < jsonArray.size(); i++) {
                QJsonObject propObj = jsonArray[i].toObject();
                JavaScriptMetaProperty property;
                property.name = propObj.value("name").toString();
                property.typeName = propObj.value("typeName").toString();
                property.typeId = propObj.value("typeId").toInt();
                property.isReadable = propObj.value("readable").toBool();
                property.isWritable = propObj.value("writable").toBool();
                property.value = propObj.value("value").toVariant();
                property.isDynamicProperty = propObj.value("isDynamicProperty").toBool();
                metaObject.properties << property;
            }
        };

        JavaScriptMetaObject objectInfo;
        objectInfo.className = className;
        parseFunction(objectInfo, signalsArray, FunctionType_Signal);
        parseFunction(objectInfo, slotsArray, FunctionType_Slot);
        parseProperty(objectInfo, propertyArray);

        m_javaScriptMetaObjectMap[regName] = objectInfo;
    }
    return true;
}

void* QCefJavaScriptBinder::bindAllObjects(CefRefPtr<CefV8Value> parentObj, CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame)
{
    QCefJavaScriptEnvironment* pJsEnv = new QCefJavaScriptEnvironment();
    QStringList allRegisterNames = m_javaScriptMetaObjectMap.keys();
    for (const auto& regNameKey : allRegisterNames) {
        if (regNameKey.isEmpty())
            continue;
        if (!m_javaScriptMetaObjectMap.contains(regNameKey)) {
            TRACEE("register name for %s not found !", qPrintable(regNameKey));
            continue;
        }
        JavaScriptMetaObject& metaInfo = m_javaScriptMetaObjectMap[regNameKey];
        CefRefPtr<QCefJavaScriptObject> jsObj = new QCefJavaScriptObject(metaInfo, browser, frame);
        //注册的时候将QCefJavaScriptEnvironment的实例传入
        jsObj->registerObject(pJsEnv, regNameKey.toStdWString(), parentObj);
    }

    return pJsEnv;
}

void QCefJavaScriptBinder::setParentId(const QString& parentId)
{
    m_parentId = parentId;
}
