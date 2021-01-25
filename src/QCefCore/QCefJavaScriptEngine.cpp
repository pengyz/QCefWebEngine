#include "public/QCefJavaScriptEngine.h"
#include "public/QCefJsCallbacks.h"

#include "QCefObjectProtocol.h"
#include "QCefProtocol.h"
#include "public/QCefCoreManagerBase.h"

#include <QMetaObject>
#include <QArgument>
#include <QVector>
#include <QVariant>
#include <QMetaMethod>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonValue>


QCefJavaScriptEngine* QCefJavaScriptEngine::m_instance = nullptr;


bool QCefJavaScriptEngine::writeSynchronizeValueInner(const QString returnTypeSignature, void* value, int len)
{
    //TRACET();
    QString pipeName = QString("\\\\.\\pipe\\%1").arg(returnTypeSignature);
    HANDLE hPipe = [pipeName]()->HANDLE {
        while (1) {
            HANDLE hPipe = CreateFile(pipeName.toStdWString().c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
            if (hPipe != INVALID_HANDLE_VALUE) {
                //TRACED("open pipe success!");
                return hPipe;
            }
            int nErrorId = GetLastError();
            if (nErrorId != ERROR_PIPE_BUSY) {
                //TRACEE("client createfile error: %d", nErrorId);
                return NULL;
            }

            //TRACED("WaitNamedPipeA ...");
            if (!WaitNamedPipe(pipeName.toStdWString().c_str(), 10000)) {
                if (GetLastError() == ERROR_SEM_TIMEOUT) {
                    //TRACEE("WaitNamePipeA timeOut!");
                } else {
                    //TRACEE("WaitNamePipeA Failed: %d", GetLastError());
                    break;
                }
            } else {
                //TRACED("waitNamedPipe success!");
                continue;
            }
        }
        return NULL;
    }();
    if (hPipe == INVALID_HANDLE_VALUE || !hPipe) {
        //TRACEE("connect server failed!");
        return false;
    }
    DWORD dwMode = PIPE_READMODE_MESSAGE;
    if (!SetNamedPipeHandleState(hPipe, &dwMode, NULL, NULL)) {
        //TRACEE("SetNamedPipeHandleState failed !");
        return false;
    }
    DWORD dwWritten = 0;
    if (!WriteFile(hPipe, value, len, &dwWritten, NULL)) {
        int nLastError = ::GetLastError();
        if (ERROR_NO_DATA == nLastError) {
            //TRACEE("pipi already closeed !");
        } else {
            //TRACEE("client writefile failed: %d", nLastError);
        }
        return false;
    }
    if (dwWritten != len) {
        //TRACEE("client writefile failed dwWritten != len");
        return false;
    }
    FlushFileBuffers(hPipe);
    CloseHandle(hPipe);
    return true;
}

bool QCefJavaScriptEngine::retieveProperty(const QString& propertySignature, QVariant& propValue)
{
    //borwserId;frameId;registerName;propName;propType;guid
    if (propertySignature.isEmpty())
        return false;
    QStringList paramParts = propertySignature.split(";", QString::SkipEmptyParts);
    if (paramParts.size() != 6)
        return false;
    int browserId = paramParts[0].toInt();
    qint64 frameId = paramParts[1].toLongLong();
    QString registerName = paramParts[2];
    QString propName = paramParts[3];
    QString propType = paramParts[4];
    QString uuid = paramParts[5];

    //通过registerName找到QObject对象
    if (!m_jsObjectBindingMap.contains(registerName))
        return false;
    QObject* pTarget = m_jsObjectBindingMap[registerName];
    int propIndex = pTarget->metaObject()->indexOfProperty(qUtf8Printable(propName));
    if (-1 == propIndex)
        return false;
    QMetaProperty metaProp = pTarget->metaObject()->property(propIndex);
    if (!metaProp.isReadable())
        return false;
    QVariant value = metaProp.read(pTarget);
    QVariant::Type targetType = QVariant::nameToType(qUtf8Printable(propType));
    if (value.type() != targetType) {
        //TRACEE("type mismatch !!!");
        return false;
    }
    propValue = value;
    return true;
}

bool QCefJavaScriptEngine::setProperty(const QString& propertySignature, const QString& propValue)
{
    //borwserId;frameId;registerName;propName;propType;guid
    if (propertySignature.isEmpty())
        return false;
    QStringList paramParts = propertySignature.split(";", QString::SkipEmptyParts);
    if (paramParts.size() != 6)
        return false;
    int browserId = paramParts[0].toInt();
    qint64 frameId = paramParts[1].toLongLong();
    QString registerName = paramParts[2];
    QString propName = paramParts[3];
    QString propType = paramParts[4];
    QString uuid = paramParts[5];

    //通过registerName找到QObject对象
    if (!m_jsObjectBindingMap.contains(registerName))
        return false;
    QObject* pTarget = m_jsObjectBindingMap[registerName];
    int propIndex = pTarget->metaObject()->indexOfProperty(qUtf8Printable(propName));
    if (-1 == propIndex)
        return false;
    QMetaProperty metaProp = pTarget->metaObject()->property(propIndex);
    if (!metaProp.isWritable())
        return false;
    QVariant::Type targetType = QVariant::nameToType(qUtf8Printable(propType));
    QVariant newValue;
    switch (targetType) {
    case QVariant::String: {
        newValue = propValue;
    }break;
    case QVariant::Double: {
        newValue = propValue.toDouble();
    }break;
    case  QVariant::Int: {
        newValue = propValue.toInt();
    }break;
    case QVariant::Bool: {
        newValue = QVariant::fromValue<bool>(propValue.toInt());
    }break;
    default:
        break;
    }
    bool bOk = metaProp.write(pTarget, newValue);
    return bOk;
}

bool QCefJavaScriptEngine::writeSynchronizeValue(const QString strSignature, const QVariant& value)
{
    bool bSucc = false;
    switch (value.type()) {
    case QVariant::Invalid: {
        char buffer[1] = { 0 };
        bSucc = writeSynchronizeValueInner(strSignature, buffer, sizeof(buffer));
    }break;
    case QVariant::Int:
    case QMetaType::Long: {
        int iValue = value.toInt();
        bSucc = writeSynchronizeValueInner(strSignature, &iValue, sizeof(iValue));
    }break;
    case QVariant::UInt:
    case QMetaType::ULong: {
        auto iValue = value.toUInt();
        bSucc = writeSynchronizeValueInner(strSignature, &iValue, sizeof(iValue));
    }break;
    case QVariant::Bool: {
        bool bValue = value.toBool();
        bSucc = writeSynchronizeValueInner(strSignature, &bValue, sizeof(bValue));
    }break;
    case QVariant::Double: {
        auto dValue = value.toDouble();
        bSucc = writeSynchronizeValueInner(strSignature, &dValue, sizeof(dValue));
    }break;
    case QVariant::String: {
        auto sValue = value.toString().toUtf8();
        int len = sValue.length();
        char* buffer = new char[sizeof(len) + len];
        *(int*)buffer = len;
        memcpy_s(buffer + sizeof(len), len, sValue.constData(), len);
        bSucc = writeSynchronizeValueInner(strSignature, buffer, len + sizeof(len));

    }break;
    case QVariant::LongLong: {
        auto lValue = value.toLongLong();
        bSucc = writeSynchronizeValueInner(strSignature, &lValue, sizeof(lValue));
    }break;
    case QVariant::ULongLong: {
        auto lValue = value.toULongLong();
        bSucc = writeSynchronizeValueInner(strSignature, &lValue, sizeof(lValue));
    }break;
    case QVariant::Char: {
        auto cValue = value.toChar();
        bSucc = writeSynchronizeValueInner(strSignature, &cValue, sizeof(cValue));
    }break;
    default: { //未知类型，写入一个byte，避免卡死
        bool bValue = false;
        writeSynchronizeValueInner(strSignature, &bValue, sizeof(bValue));
    }break;
    }
    return bSucc;
}

//template<typename T>
//bool invokeFunction(QObject* obj, const QString& methodName, const QString& retTypeSignature, const QVector<QGenericArgument>& argList)
//{
//    T val;
//    bool bRet = QMetaObject::invokeMethod(obj, methodName.toStdString().c_str(), Qt::DirectConnection, QReturnArgument<decltype(val)>(QMetaType::typeName(qMetaTypeId<decltype(val)>()), val), argList[0],
//        argList[1], argList[2], argList[3], argList[4], argList[5], argList[6], argList[7], argList[8], argList[9]);
//    writeReturnValue(retTypeSignature, &val, sizeof(val));
//    return bRet;
//}


QCefJavaScriptEngine::QCefJavaScriptEngine()
{
    qRegisterMetaType<JavaScriptGetDataCallback>();
    qRegisterMetaType<JavaScriptGetDataCallbackPtr>();
    qRegisterMetaType<JavaScriptStubCallback>();
    qRegisterMetaType<JavaScriptStubCallbackPtr>();
}

QCefJavaScriptEngine* QCefJavaScriptEngine::get()
{
    if (!m_instance)
        m_instance = new QCefJavaScriptEngine();
    return m_instance;
}

bool QCefJavaScriptEngine::init()
{
    if (!QCefObjectProtocol::get()->init())
        return false;
    return true;
}

void QCefJavaScriptEngine::registerObject(const QString& registerName, QObject* object)
{
    object->setObjectName(registerName);
    m_jsObjectBindingMap.insert(registerName, object);
    QCefObjectProtocol::get()->registerJavaScriptHandlerObject(registerName, object);
    m_registeredMetaObjectMap[registerName] = object->metaObject();
}

QString QCefJavaScriptEngine::getReturnValueSignature(const QString& jsCallbackSignature)
{
    QJsonParseError jsonError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsCallbackSignature.toUtf8(), &jsonError);
    if (jsonError.error != QJsonParseError::NoError)
        return "";
    QJsonObject jsonObj = jsonDoc.object();
    if (!jsonObj.contains("retType"))
        return "";
    QString strRetType = jsonObj["retType"].toString();
    return strRetType;
}

bool QCefJavaScriptEngine::inovkeMethod(int browserId, const QVariantList& messageArguments, QString& callbackSignature)
{
    if (messageArguments.size() < 4) {
        return false;
    }

    int idx = 0;
    if (QVariant::Type::String != messageArguments[idx].type() ||
        QVariant::Type::String != messageArguments[idx + 1].type()) {
        return false;
    }

    int messageBrowserId = QString::fromStdString(messageArguments[idx++].toString().toStdString()).toInt();
    qint64 frameId = QString::fromStdString(messageArguments[idx++].toString().toStdString()).toLongLong();
    //TRACED("browserId is: %d, messageBrowserId is: %d, frameId is: %ld", browserId, messageBrowserId, frameId);
    //not current browser, ignore call
    if (messageBrowserId != browserId) {
        return false;
    }

    //browserId className method methodSignature
    if (QVariant::Type::String != messageArguments[idx].type() ||
        QVariant::Type::String != messageArguments[idx + 1].type() ||
        QVariant::Type::String != messageArguments[idx + 2].type() ||
        QVariant::Type::String != messageArguments[idx + 3].type())
        return false;

    QString className = messageArguments[idx++].toString();
    QString method = messageArguments[idx++].toString();
    QString methodSignature = messageArguments[idx++].toString();
    QString callbackSig = messageArguments[idx].toString(); //do not increase idx, this argument is used again below.
    //retrive signatures, without browserId and frameId
    callbackSignature = callbackSig;
    QVector<QGenericArgument> argList;
    argList.resize(10);
    auto cbCollection = QCefCoreManagerBase::get()->genJavaScriptCallbackCollection(callbackSig);
    //argList.replace(0, Q_ARG(QString, callbackSig));
    argList.replace(0, Q_ARG(JavaScriptCallbacksCollection, cbCollection));

    //get meta object
    if (!m_registeredMetaObjectMap.contains(className)) {
        //TRACEE("meta method for name: %s not found !", qPrintable(className));
        return false;
    }
    const QMetaObject* metaObj = m_registeredMetaObjectMap[className];
    int iMethod = metaObj->indexOfMethod(metaObj->normalizedSignature(methodSignature.toStdString().c_str()).toStdString().c_str());
    if (iMethod == -1) {
        //TRACEE("method %s index not found !", qPrintable(method));
        return false;
    }
    QMetaMethod metaMethod = m_registeredMetaObjectMap[className]->method(iMethod);

    //get calling object
    if (!m_jsObjectBindingMap.contains(className)) {
        //TRACEE("class name: %s not found !", qPrintable(className));
        return false;
    }
    QObject* obj = m_jsObjectBindingMap[className];
    bool bNeedWrap = false;
    if (metaMethod.parameterCount() > argList.size())
        //需要折叠，折叠的意思是，为了传递超过10个参数，我们把大于9的参数放入QVariantList，同时创建一个同名的但是以___Wrapped___结尾的函数
        //该函数的作用仅仅是用来突破10个参数的限制，注册到js引擎时，该后缀的函数会被忽略
        bNeedWrap = true;
    //--------------------------------------------------------------------------------------------------------------------
    //NOTE:
    //QGenericArgument只能获取常量引用，本身并不持有数据，所以必须将QVariant转为具体类型后再次进行存储，保证在调用过程中被
    //引用的参数有效。如果直接使用类型 Q_ARG(QString, messageArguments[messageIndex].toString())这种方式，会导致被引用的值
    //在调用前就被析构，造成反射调用时传递的参数不正确。
    //基于这个原因，invokeMethod使用DirectConnect进行调用，如果需要转到主线程，则在处理函数中再发信号进行处理
    //我们保存所有的参数到数组，保证Q_ARG包装的参数在调用期间有效
    int iValueVecs[10] = { 0 };
    int iIValIdx = 0;
    uint uValueVecs[10] = { 0 };
    int iUValIdx = 0;
    bool bValueVecs[10] = { false };
    int iBValIdx = 0;
    double dValueVecs[10] = { 0.0 };
    int iDValIdx = 0;
    QString sValueVecs[10];
    int iSValIdx = 0;
    QByteArray byteValueVecs[10];
    int iByteValIdx = 0;
    //参数获取时严格按照函数的元信息进行，如遇到参数类型不匹配，则尝试转型，无法转换时会给出默认参数（基于QVariant）
    //如果参数数量不匹配则做相同处理，多则仅传递指定数量忽略后续参数，少则使用默认参数补齐。
    //根据QMetaObject::invokeMethod接口的限制，最多支持10个调用参数
    //如果实际的调用参数超过10个，则前9个正常传递，剩余的参数折叠为QVariantList传递
    QVariantList varList;
    for (int i = 1; i < metaMethod.parameterCount(); i++) {
        int paramTypeId = metaMethod.parameterType(i);
        int messageIndex = idx + i;
        if (paramTypeId == QMetaType::Int) {
            if (messageIndex >= messageArguments.size() || !messageArguments[messageIndex].canConvert(QVariant::Type::Int)) {
                iValueVecs[iIValIdx] = QVariant().toInt();
            } else {
                iValueVecs[iIValIdx] = messageArguments[messageIndex].toInt();
            }
            if (i < 9) {
                argList[i] = Q_ARG(int, iValueVecs[iIValIdx]);
            }
            iIValIdx++;
        } else if (paramTypeId == QMetaType::UInt) {
            if (messageIndex >= messageArguments.size() || !messageArguments[messageIndex].canConvert(QVariant::Type::UInt)) {
                uValueVecs[iUValIdx] = QVariant().toUInt();
            } else {
                uValueVecs[iUValIdx] = messageArguments[messageIndex].toUInt();
            }
            if (i < 9) {
                argList[i] = Q_ARG(uint, uValueVecs[iUValIdx]);
            }
            iUValIdx++;
        } else if (paramTypeId == QMetaType::Bool) {
            if (messageIndex >= messageArguments.size() || !messageArguments[messageIndex].canConvert(QVariant::Type::Bool)) {
                bValueVecs[iBValIdx] = QVariant().toBool();
            } else {
                bValueVecs[iBValIdx] = messageArguments[messageIndex].toBool();
            }
            if (i < 9) {
                argList[i] = Q_ARG(bool, bValueVecs[iBValIdx]);
            }
            iBValIdx++;
        } else if (paramTypeId == QMetaType::Double) {
            if (messageIndex >= messageArguments.size() || !messageArguments[messageIndex].canConvert(QVariant::Type::Double)) {
                dValueVecs[iDValIdx] = QVariant().toDouble();
            } else {
                dValueVecs[iDValIdx] = messageArguments[messageIndex].toDouble();
            }
            if (i < 9) {
                argList[i] = Q_ARG(double, dValueVecs[iDValIdx]);
            }
            iDValIdx++;
        } else if (paramTypeId == QMetaType::QString) {
            if (messageIndex >= messageArguments.size() || !messageArguments[messageIndex].canConvert(QVariant::Type::String)) {
                sValueVecs[iSValIdx] = QVariant().toString();
            } else {
                sValueVecs[iSValIdx] = messageArguments[messageIndex].toString();
            }
            if (i < 9) {
                argList[i] = Q_ARG(QString, sValueVecs[iSValIdx]);
            }
            iSValIdx++;
        } else if (paramTypeId == QMetaType::QByteArray) {
            if (messageIndex >= messageArguments.size() || !messageArguments[messageIndex].canConvert(QVariant::Type::ByteArray)) {
                byteValueVecs[iByteValIdx] = QVariant().toByteArray();
            } else {
                byteValueVecs[iByteValIdx] = messageArguments[messageIndex].toByteArray();
            }
            if (i < 9) {
                argList[i] = Q_ARG(QByteArray, byteValueVecs[iByteValIdx]);
            }
            iSValIdx++;
        } else {
        }
    }

    if (bNeedWrap) {
        method.append("___Wrapped___");
        varList = messageArguments.mid(idx + 9);
        argList[9] = Q_ARG(QVariantList, varList);
    }

    //带返回值的调用处理
    bool bRet = false;
    QGenericReturnArgument retArg;
    bRet = QMetaObject::invokeMethod(obj, qUtf8Printable(method), Qt::DirectConnection, retArg, argList[0],
        argList[1], argList[2], argList[3], argList[4], argList[5], argList[6], argList[7], argList[8], argList[9]);
    return bRet;
}