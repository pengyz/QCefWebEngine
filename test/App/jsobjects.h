#pragma once;
#include <QObject>
#include <QString>
#include "include/QCefJsCallbacks.h"

class JSObjectBase : public QObject {
    Q_OBJECT
public slots:
    int hello_world(const JavaScriptCallbacksCollection& cbCollections);
    QString hello_world2(const JavaScriptCallbacksCollection& cbCollections);
};