#pragma once;
#include "QCefCoreManagerBase.h"
#include "qcefview_export.h"

class QCEFVIEW_EXPORT QCefWebEngine : public QCefCoreManagerBase {
    Q_OBJECT
public:
    static QCefWebEngine* get();
protected:
    virtual QCefCoreManagerBaseImpl* doImplInit() override;

private:
    static QCefWebEngine* m_instance;
};