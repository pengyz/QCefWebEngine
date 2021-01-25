#pragma once;
#include "QCefCoreManagerBase.h"
#include "qcefview_export.h"

class QCEFVIEW_EXPORT QCefCoreManager : public QCefCoreManagerBase {
    Q_OBJECT
public:
    static QCefCoreManager* get();
protected:
    virtual QCefCoreManagerBaseImpl* doImplInit() override;

private:
    static QCefCoreManager* m_instance;
};