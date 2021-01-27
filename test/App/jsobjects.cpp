#include "jsobjects.h"

int JSObjectBase::hello_world(const JavaScriptCallbacksCollection& cbCollections)
{
    auto cb = cbCollections.get<JavaScriptGetDataCallback>(0);
    if (cb)
        cb->execute(100, "hello world !", "");
    return -1;
}

QString JSObjectBase::hello_world2(const JavaScriptCallbacksCollection& cbCollections)
{
    return "hello world !";
}
