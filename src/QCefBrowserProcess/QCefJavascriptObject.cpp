#include "QCefJavascriptObject.h"
#include "QCefJavaScriptEnvironment.h"
#include <QCefProtocol.h>
#include <QUuid>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonDocument>
#include <QUuid>

QCefJavaScriptObject::QCefJavaScriptObject(JavaScriptMetaObject metaObj, CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame)
    :m_metaObject(metaObj), m_browser(browser), m_frame(frame)
{
}

QCefJavaScriptObject::~QCefJavaScriptObject()
{
    //TRACED("called to clear all !!!");
    m_functionMap.clear();
}

bool QCefJavaScriptObject::registerObject(QCefJavaScriptEnvironment* pJsEnv, CefString registerName, CefRefPtr<CefV8Value> cefParentObj)
{
    m_registerName = QString::fromStdWString(registerName);
    m_pJsEnv = pJsEnv;
    QStringList registerNames = QString::fromStdWString(registerName.ToWString()).split(".");
    QString regNameFinal = registerNames.last();
    registerNames.removeLast();
    CefRefPtr<CefV8Value> parentNode = cefParentObj;
    //check and get the parent object.
    for (const QString& registerName : registerNames) {
        //the parent tree must be registered previously
        Q_ASSERT(parentNode->HasValue(registerName.toStdWString()));
        //cefParentObj->DeleteValue(registerName);
        auto objValue = parentNode->GetValue(registerName.toStdWString());
        Q_ASSERT(objValue->IsObject());
        parentNode = objValue;
    }

    CefRefPtr<CefV8Value> currObjValue;
    if (parentNode->HasValue(regNameFinal.toStdWString())) {
        currObjValue = parentNode->GetValue(regNameFinal.toStdWString());
        if (!currObjValue->IsObject()) {
            parentNode->DeleteValue(regNameFinal.toStdWString());
            currObjValue = nullptr;
        }
    }

    if (!currObjValue)
        currObjValue = CefV8Value::CreateObject(this, nullptr);
    for (const JavaScriptMetaMethod& funcInfo : m_metaObject.functions) {
        CefRefPtr<QCefFunctionObject> functionHandler = new QCefFunctionObject(funcInfo, m_browser, m_frame);
        CefRefPtr<CefV8Value> func = CefV8Value::CreateFunction(funcInfo.name.toStdWString(), functionHandler);
        m_functionMap.insert(funcInfo.name, functionHandler);
        currObjValue->SetValue(funcInfo.name.toStdWString(), func, V8_PROPERTY_ATTRIBUTE_NONE);
    }
    for (const JavaScriptMetaProperty& propInfo : m_metaObject.properties) {
        //通过SetValue(name, control, property)函数设置的才会走Get和Set
        //通过SetValue(name, value, property)不会走Get和Set
        //所以根据是否是动态属性，要调用不同的重载函数进行处理
        if (!propInfo.isDynamicProperty) {
            //非动态属性，直接设置，不走Get和Set
            CefV8Value::PropertyAttribute propAttr = V8_PROPERTY_ATTRIBUTE_NONE;
            Q_ASSERT(propInfo.isReadable); //property must be readable at least.
            CefRefPtr<CefV8Value> v8Value;
            if (propInfo.typeName == "QString") {
                v8Value = CefV8Value::CreateString(propInfo.value.toString().toStdWString());
            } else if (propInfo.typeName == "int") {
                v8Value = CefV8Value::CreateInt(propInfo.value.toInt());
            } else if (propInfo.typeName == "double") {
                v8Value = CefV8Value::CreateDouble(propInfo.value.toDouble());
            } else if (propInfo.typeName == "bool") {
                v8Value = CefV8Value::CreateBool(propInfo.value.toBool());
            } else {
                v8Value = CefV8Value::CreateUndefined();
            }
            currObjValue->SetValue(propInfo.name.toStdWString(), V8_ACCESS_CONTROL_DEFAULT, propAttr);
        } else {
            //动态属性，走Get和Set
            currObjValue->SetValue(propInfo.name.toStdWString(), propInfo.isWritable ? V8_ACCESS_CONTROL_DEFAULT :
                V8_ACCESS_CONTROL_PROHIBITS_OVERWRITING, V8_PROPERTY_ATTRIBUTE_NONE);
        }
    }
    //do not allow user to change this value
    parentNode->SetValue(regNameFinal.toStdWString(), currObjValue, V8_PROPERTY_ATTRIBUTE_READONLY);
    //添加到env对象中
    m_pJsEnv->addMetaObjectInfo(QString::fromStdWString(registerName), this, currObjValue);
    return true;
}

CefRefPtr<QCefFunctionObject> QCefJavaScriptObject::getFunction(const QString& functionName)
{
    if (!m_functionMap.contains(functionName))
        return nullptr;
    return m_functionMap[functionName];
}

//发消息从远端获取属性值
CefRefPtr<CefV8Value> QCefJavaScriptObject::retrieveDynamicProperty(const QString& registerName, const JavaScriptMetaProperty& propInfo)
{
    if (!m_frame || !m_browser)
        return CefV8Value::CreateUndefined();

    //borwserId;frameId;registerName;propName;propType;guid
    QString strSignature = QString("%1;%2;%3;%4;%5;%6").arg(m_browser->GetIdentifier()).arg(m_frame->GetIdentifier())
        .arg(registerName).arg(propInfo.name).arg(propInfo.typeName).arg(QUuid::createUuid().toString(QUuid::Id128));

    //发送消息给Browser进程，通知读取属性
    CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create(QCEF_RETRIEVEPROPERTY);
    CefRefPtr<CefListValue> args = msg->GetArgumentList();
    int idx = 0;
    args->SetString(idx++, QString::number(m_browser->GetIdentifier()).toStdString());
    args->SetString(idx++, QString::number(m_frame->GetIdentifier()).toStdString());
    args->SetString(idx++, strSignature.toStdWString());
    m_frame->SendProcessMessage(PID_BROWSER, msg);
    CefRefPtr<CefV8Value> v8Value = QCefFunctionObject::readSynchronizeValue(strSignature, propInfo.typeId);
    return v8Value;
}

bool QCefJavaScriptObject::updateDynamicProperty(const QString& registerName, const JavaScriptMetaProperty& propInfo, const CefRefPtr<CefV8Value>& value)
{
    if (!propInfo.isWritable)
        return false;
    if ((propInfo.typeName == "QString" && !value->IsString()) || (propInfo.typeName == "int" && !value->IsInt()) ||
        (propInfo.typeName == "double" && !value->IsDouble()) || (propInfo.typeName == "bool" && !value->IsBool())) {
        //TRACEE("property typeName is: %s but value type mismatch !", qPrintable(propInfo.typeName));
        return false;
    }
    //发送更新消息
    QString strSignature = QString("%1;%2;%3;%4;%5;%6").arg(m_browser->GetIdentifier()).arg(m_frame->GetIdentifier())
        .arg(registerName).arg(propInfo.name).arg(propInfo.typeName).arg(QUuid::createUuid().toString(QUuid::Id128));
    CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create(QCEF_SETPROPERTY);
    CefRefPtr<CefListValue> args = msg->GetArgumentList();
    int idx = 0;
    args->SetString(idx++, QString::number(m_browser->GetIdentifier()).toStdString());
    args->SetString(idx++, QString::number(m_frame->GetIdentifier()).toStdString());
    args->SetString(idx++, strSignature.toStdWString());
    QString strValue;
    if (value->IsString()) {
        strValue = QString::fromStdWString(value->GetStringValue());
    } else if (value->IsDouble()) {
        strValue = QString::number(value->GetDoubleValue());
    } else if (value->IsInt()) {
        strValue = QString::number(value->GetIntValue());
    } else if (value->IsBool()) {
        strValue = QString::number(value->GetBoolValue());
    } else {
        //TRACEE("property type not allowed !");
        return false;
    }
    args->SetString(idx++, strValue.toStdWString());
    m_frame->SendProcessMessage(PID_BROWSER, msg);
    return true;
}

CefRefPtr<CefV8Value> QCefJavaScriptObject::genV8PropertyValue(const QVariant& value)
{
    CefRefPtr<CefV8Value> v8Value;
    if (value.type() == QVariant::String) {
        v8Value = CefV8Value::CreateString(value.toString().toStdWString());
    } else if (value.type() == QVariant::Int) {
        v8Value = CefV8Value::CreateInt(value.toInt());
    } else if (value.type() == QVariant::Double) {
        v8Value = CefV8Value::CreateDouble(value.toDouble());
    } else if (value.type() == QVariant::Bool) {
        v8Value = CefV8Value::CreateBool(value.toBool());
    } else {
        v8Value = CefV8Value::CreateUndefined();
    }
    return v8Value;
}

bool QCefJavaScriptObject::addNewV8ValueDirectly(const JavaScriptMetaProperty& propInfo, const CefRefPtr<CefV8Value> v8Value)
{
    //TRACET();
    JSMetaObjectPairInfo pair = m_pJsEnv->getMetaObjectInfo(m_registerName);
    if (!pair.second || !pair.second->IsValid() || pair.second->IsNull()) {
        //TRACEE("get getMetaObjectInfo failed !");
        return false;
    }
    //注册为允许读写的属性
    bool bOk = pair.second->SetValue(propInfo.name.toStdWString(), v8Value, V8_PROPERTY_ATTRIBUTE_NONE);
    return bOk;
}

bool QCefJavaScriptObject::updateMetaProperty(JavaScriptMetaProperty* pProperty, const CefRefPtr<CefV8Value> v8Value)
{
    bool bOk = true;
    if (!v8Value)
        return false;
    if (v8Value->IsString()) {
        pProperty->typeName = "QString";
        pProperty->value = QString::fromStdWString(v8Value->GetStringValue());
    } else if (v8Value->IsInt()) {
        pProperty->typeName = "int";
        pProperty->value = v8Value->GetIntValue();
    } else if (v8Value->IsDouble()) {
        pProperty->typeName = "double";
        pProperty->value = v8Value->GetDoubleValue();
    } else if (v8Value->IsBool()) {
        pProperty->typeName = "bool";
        pProperty->value = v8Value->GetBoolValue();
    } else {
        bOk = false;
    }
    return bOk;
}

bool QCefJavaScriptObject::Get(const CefString& name, const CefRefPtr<CefV8Value> object, CefRefPtr<CefV8Value>& retval, CefString& exception)
{
    //TRACED("name is: %s", qPrintable(QString::fromStdWString(name)));
    auto pos = std::find_if(m_metaObject.properties.begin(), m_metaObject.properties.end(), [=](const JavaScriptMetaProperty& prop) {
        if (prop.name == QString::fromStdWString(name))
            return true;
        return false;
    });
    //can not find in meta property info
    if (pos == m_metaObject.properties.end()) {
        exception = "property not exist!";
        retval = CefV8Value::CreateUndefined();
        return false;
    }
    if (pos->isDynamicProperty) {
        //need retreive from remote
        retval = retrieveDynamicProperty(m_registerName, *pos);

    } else {
        retval = genV8PropertyValue(pos->value);
    }
    return true;
}

bool QCefJavaScriptObject::Set(const CefString& name, const CefRefPtr<CefV8Value> object, const CefRefPtr<CefV8Value> value, CefString& exception)
{
    //TRACED("name is: %s", qPrintable(QString::fromStdWString(name)));
    bool bOk = false;
    if (!value->IsValid()) {
        exception = "value is invalid!";
        return bOk;
    }

    auto pos = std::find_if(m_metaObject.properties.begin(), m_metaObject.properties.end(), [=](const JavaScriptMetaProperty& prop) {
        if (prop.name == QString::fromStdWString(name))
            return true;
        return false;
    });
    //can not find in meta property info
    if (pos != m_metaObject.properties.end()) {
        if (!pos->isWritable) {
            exception = "property is readonly!";
            return bOk;
        }
        //能找到这个值，我们根据是否是动态属性判断是否要更新它
        if (pos->isDynamicProperty) {
            //动态属性，通过IPC消息配合命名管道进行更新，仅实现了异步更新
            bOk = updateDynamicProperty(m_registerName, *pos, value);
        } else {
            //静态属性，直接更新到property中即可
            bOk = updateMetaProperty(pos, value);
        }
    } else {
        //直接更新
        bOk = addNewV8ValueDirectly(*pos, value);
        if (!bOk) {
            exception = "set new property failed !";
        }
    }
    return bOk;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QCefFunctionObject::QCefFunctionObject(JavaScriptMetaMethod metaMethod, CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame)
    :m_metaMethod(metaMethod), m_browser(browser), m_frame(frame)
{
}

QCefFunctionObject::~QCefFunctionObject()
{
    m_callbacksMap.clear();
}

//js function call handler
bool QCefFunctionObject::Execute(const CefString& name, CefRefPtr<CefV8Value> object,
    const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval, CefString& exception)
{
    /*TRACED("name is: %s, className is: %s, signature is: %s", qPrintable(QString::fromStdWString(name)),
        qPrintable(m_metaMethod.className), qPrintable(m_metaMethod.signature));*/
    //create invokeNglMethod message then send to browser process.
    CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create(QCEF_INVOKENGLMETHOD);

    CefRefPtr<CefListValue> args = msg->GetArgumentList();
    int browserId = m_browser->GetIdentifier();
    int64 frameId = m_frame->GetIdentifier();

    int idx = 0;
    //call param: [browserId, frameId, className, functionName, callbackSignatures], function params
    args->SetString(idx++, QString::number(browserId).toStdString());
    args->SetString(idx++, QString::number(frameId).toStdString());
    args->SetString(idx++, m_metaMethod.className.toStdWString());
    args->SetString(idx++, name);
    args->SetString(idx++, m_metaMethod.signature.toStdWString());
    //TRACED("class name is: %s, signature is: %s", qPrintable(m_metaMethod.className), qPrintable(m_metaMethod.signature));

    //check param count
    int acceptParamCount = m_metaMethod.params.size();
    QStringList signatureList;
    QString callbackSignatures;
    int iSigIndex = idx;
    args->SetString(idx++, callbackSignatures.toStdWString());

    //set all params
    for (std::size_t i = 0; i < arguments.size(); i++) {
        if (arguments[i]->IsBool()) {
            args->SetBool(idx++, arguments[i]->GetBoolValue());
        } else if (arguments[i]->IsInt()) {
            args->SetInt(idx++, arguments[i]->GetIntValue());
        } else if (arguments[i]->IsDouble()) {
            double dValue = arguments[i]->GetDoubleValue();
            if (isnan(dValue)) {
                //the double value is NAN, raise an exception.
                exception = QString(u8"argument %1 is nan !").arg(i).toStdWString();
                retval = CefV8Value::CreateUndefined();
                return false;
            }
            args->SetDouble(idx++, dValue);
        } else if (arguments[i]->IsString()) {
            args->SetString(idx++, arguments[i]->GetStringValue());
        } else if (arguments[i]->IsArrayBuffer()) {
            //TODO: do not support CefV8ArrayBuffer, we can't get raw data from it!
            CefRefPtr<CefV8Value> value = arguments[i];
            args->SetNull(idx++);
        } else if (arguments[i]->IsFunction()) {
            QString strUuid;
            strUuid = QUuid::createUuid().toString().toUpper();
            strUuid = strUuid.mid(1, strUuid.size() - 2);
            strUuid = strUuid.replace("-", "");
            QString callbackSig = QString("%1.%2.%3.%4.%5.%6").arg(m_browser->GetIdentifier()).arg(frameId).arg(m_metaMethod.className).arg(m_metaMethod.name)
                .arg(i).arg(strUuid);
            signatureList << callbackSig;

            QCefFunctionCallback functionCallback;
            functionCallback.callback = arguments[i];
            functionCallback.context = CefV8Context::GetCurrentContext();

            CefString callbackSignature = callbackSig.toStdWString();
            m_callbacksMap.insert(callbackSignature, functionCallback);
            //TRACED("callback found at: %d, signature is: %s", i, qPrintable(callbackSig));
        } else {
            args->SetNull(idx++);
        }
    }

    bool bNeedReturn = m_metaMethod.retType != QMetaType::Void;
    QString retTypeSignature;
    if (!signatureList.isEmpty() || bNeedReturn) {
        callbackSignatures = signatureList.join(";");
        QJsonObject jsonObj;
        jsonObj["callbacks"] = callbackSignatures;
        if (m_metaMethod.retType != QMetaType::Void) {
            //generate a return type signature
            QString strUuid;
            strUuid = QUuid::createUuid().toString().toUpper();
            strUuid = strUuid.mid(1, strUuid.size() - 2);
            strUuid = strUuid.replace("-", "");
            retTypeSignature = QString("%1.%2.%3.%4.%5.%6").arg(m_browser->GetIdentifier()).arg(frameId).arg(m_metaMethod.className).arg(m_metaMethod.name)
                .arg(m_metaMethod.retType).arg(strUuid);
            jsonObj["retType"] = retTypeSignature;
        }
        callbackSignatures = QJsonDocument(jsonObj).toJson(QJsonDocument::Compact);
        args->SetString(iSigIndex, callbackSignatures.toStdWString());
        //TRACED("json callback signature is: %s", qPrintable(callbackSignatures));
    }

    if (m_browser && m_frame) {
        m_frame->SendProcessMessage(PID_BROWSER, msg);
        retval = readSynchronizeValue(retTypeSignature, m_metaMethod.retType);
    } else {
        retval = CefV8Value::CreateUndefined();
    }

    return true;
}

bool QCefFunctionObject::ExecuteCallback(const CefString& signature, CefRefPtr<CefV8Value> object, CefRefPtr<CefListValue> arguments, CefRefPtr<CefV8Value>& retval, CefString& exception)
{
    //TRACED("signature is: %s", qPrintable(QString::fromStdWString(signature)));
    if (!m_callbacksMap.contains(signature)) {
        exception = "can't get callback !";
        return false;
    }
    QCefFunctionCallback funcCallback = m_callbacksMap[signature];
    if (!funcCallback.context->Enter()) {
        exception = L"enter current context error !";
        retval = CefV8Value::CreateUndefined();
        return false;
    }
    CefV8ValueList v8Arguments(arguments->GetSize());
    if (arguments) {
        for (size_t i = 0; i < arguments->GetSize(); i++) {
            CefValueType vType = arguments->GetType(i);
            if (vType == CefValueType::VTYPE_STRING) {
                v8Arguments[i] = CefV8Value::CreateString(arguments->GetString(i));
            } else if (vType == CefValueType::VTYPE_INT) {
                v8Arguments[i] = CefV8Value::CreateInt(arguments->GetInt(i));
            } else if (vType == CefValueType::VTYPE_BOOL) {
                v8Arguments[i] = CefV8Value::CreateBool(arguments->GetBool(i));
            } else if (vType == CefValueType::VTYPE_DOUBLE) {
                v8Arguments[i] = CefV8Value::CreateDouble(arguments->GetDouble(i));
            } else if (vType == CefValueType::VTYPE_BINARY) {
                auto binVal = arguments->GetBinary(i);
                auto size = binVal->GetSize();
                void* buf = new char[size];
                memset(buf, 0, size);
                auto readSize = binVal->GetData(buf, size, 0);
                if (readSize != size) {
                    //TRACEE("GetData want %d, but got %d", size, readSize);
                    size = 0;
                }
                v8Arguments[i] = CefV8Value::CreateArrayBuffer(buf, size, new ReleaseCallback());
            } else {
                v8Arguments[i] = CefV8Value::CreateNull();
            }
        }
    }
    retval = funcCallback.callback->ExecuteFunction(object, v8Arguments);
    if (!funcCallback.context->Exit()) {
        exception = L"exit current context error !";
    }
    return true;
}

bool QCefFunctionObject::RemoveCallback(const CefString& signature)
{
    bool bRet = false;
    if (m_callbacksMap.contains(signature)) {
        m_callbacksMap.take(signature);
        bRet = true;
    }
    //TRACED("after remove, callback count is: %d", m_callbacksMap.size());
    return bRet;
}

CefRefPtr<CefV8Value> QCefFunctionObject::readSynchronizeValue(const QString& retTypeSignature, int retType)
{
    //TRACET();
    CefRefPtr<CefV8Value> retval = CefV8Value::CreateUndefined();
    //check if return value type valid
    if (retType != QMetaType::Int && retType != QMetaType::UInt && retType != QMetaType::Bool &&
        retType != QMetaType::Double && retType != QMetaType::QString && retType != QMetaType::Long &&
        retType != QMetaType::LongLong && retType != QMetaType::ULong && retType != QMetaType::ULongLong &&
        retType != QMetaType::Char && retType != QMetaType::UChar &&retType != QMetaType::Short &&
        retType != QMetaType::UShort &&  retType != QMetaType::Float)
        return retval;

    QString lpszPipename = QString("\\\\.\\pipe\\%1").arg(retTypeSignature);
    HANDLE hPipe = CreateNamedPipe(lpszPipename.toStdWString().c_str(), PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, PIPE_UNLIMITED_INSTANCES,
        QCEF_RET_PIPE_BUFFER_LEN, QCEF_RET_PIPE_BUFFER_LEN, 0, NULL);
    if (hPipe == INVALID_HANDLE_VALUE)
        return retval;
    BOOL fConnected = ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
    if (!fConnected) {
        CloseHandle(hPipe);
        return retval;
    }

    DWORD bytes;
    switch (retType) {
    case QMetaType::Int: {
        int val = 0;
        ::ReadFile(hPipe, &val, sizeof(val), &bytes, NULL);
        retval = CefV8Value::CreateInt(val);
    }break;
    case QMetaType::UInt: {
        unsigned int val = 0;
        ::ReadFile(hPipe, &val, sizeof(val), &bytes, NULL);
        retval = CefV8Value::CreateUInt(val);
    }break;
    case QMetaType::Bool: {
        bool val = false;
        ::ReadFile(hPipe, &val, sizeof(val), &bytes, NULL);
        retval = CefV8Value::CreateBool(val);
    }break;
    case QMetaType::Double: {
        double val = 0.0;
        ::ReadFile(hPipe, &val, sizeof(val), &bytes, NULL);
        retval = CefV8Value::CreateDouble(val);
    }break;
    case QMetaType::QString: { //字符串需要特殊处理
        CefString val;
        int len = 0;
        ::ReadFile(hPipe, &len, sizeof(len), &bytes, NULL);
        if (len && len < 1024 * 1024) {
            char* buffer = new char[len + 1];
            char* start = buffer;
            memset(buffer, 0, len + 1);
            int totalBytes = 0;
            while (totalBytes < len) {
                if (!::ReadFile(hPipe, start, len, &bytes, NULL))
                    break;
                totalBytes += bytes;
                start += bytes;
            }
            val = QString::fromUtf8(buffer).toStdWString();
            delete[] buffer;
        } else {
            //TRACEE("string length is too long !!!!");
        }
        retval = CefV8Value::CreateString(val);
    }break;
    case QMetaType::Long: {
        long val = 0;
        ::ReadFile(hPipe, &val, sizeof(val), &bytes, NULL);
        retval = CefV8Value::CreateDouble(val);
    }break;
    case QMetaType::LongLong: {
        long long val = 0;
        ::ReadFile(hPipe, &val, sizeof(val), &bytes, NULL);
        retval = CefV8Value::CreateDouble(val);
    }break;
    case QMetaType::ULong: {
        unsigned long val = 0;
        ::ReadFile(hPipe, &val, sizeof(val), &bytes, NULL);
        retval = CefV8Value::CreateDouble(val);
    }break;
    case QMetaType::ULongLong: {
        unsigned long long val = 0;
        ::ReadFile(hPipe, &val, sizeof(val), &bytes, NULL);
        retval = CefV8Value::CreateDouble(val);
    }break;
    case QMetaType::Char: {
        char val = 0;
        ::ReadFile(hPipe, &val, sizeof(val), &bytes, NULL);
        retval = CefV8Value::CreateInt(val);
    }break;
    case QMetaType::UChar: {
        unsigned char val = 0;
        ::ReadFile(hPipe, &val, sizeof(val), &bytes, NULL);
        retval = CefV8Value::CreateUInt(val);
    }break;
    case QMetaType::Short: {
        short val = 0;
        ::ReadFile(hPipe, &val, sizeof(val), &bytes, NULL);
        retval = CefV8Value::CreateInt(val);
    }break;
    case QMetaType::UShort: {
        unsigned short val = 0;
        ::ReadFile(hPipe, &val, sizeof(val), &bytes, NULL);
        retval = CefV8Value::CreateUInt(val);
    }break;
    case QMetaType::Float: {
        float val = 0.f;
        ::ReadFile(hPipe, &val, sizeof(val), &bytes, NULL);
        retval = CefV8Value::CreateDouble(val);
    }break;
    }
    CloseHandle(hPipe);
    return retval;
}
