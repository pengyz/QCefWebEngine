#pragma once;
#include <QString>
#include <QVector>
#include <QVariant>

struct JavaScriptMetaParam {
    QString name;
    QString typeName;

    JavaScriptMetaParam(const QString& name, const QString& typeName)
        :name(name), typeName(typeName)
    {
    }

    JavaScriptMetaParam() {}

    bool isValid()
    {
        return !name.isEmpty();
    }
};

enum FunctionType {
    FunctionType_Signal,
    FunctionType_Slot,
};

struct JavaScriptMetaMethod {
    int retType;
    int functionType;
    QString name;
    QString signature;
    QString className;
    QVector<JavaScriptMetaParam> params;

    bool isValid()
    {
        return !name.isEmpty() && !signature.isEmpty();
    }
};
Q_DECLARE_METATYPE(JavaScriptMetaMethod);

struct JavaScriptMetaProperty {
    QString name;
    QString typeName;
    int typeId;
    bool isReadable;
    bool isWritable;
    QVariant value;
    bool isDynamicProperty;      //是否是动态属性，默认为false，需要手动指定

    bool isValid()
    {
        return !name.isEmpty() && !typeName.isEmpty();
    }
};
Q_DECLARE_METATYPE(JavaScriptMetaProperty);

struct JavaScriptMetaObject {
    QString className;
    QVector<JavaScriptMetaMethod> functions;
    QVector<JavaScriptMetaProperty> properties;
};
Q_DECLARE_METATYPE(JavaScriptMetaObject);
