#pragma once;
#include "../QCefCore/QCefCoreManagerBaseImpl.h"

enum class HtmlSurfaceConsumerType;

class QCefCoreManagerImpl : public QCefCoreManagerBaseImpl {
    Q_OBJECT
public:
    QCefCoreManagerImpl(QObject* parent = nullptr);
protected:
    virtual CefRefPtr<QCefBrowserHandlerBase> doInitCefBrowserHandler() override;
    virtual QSharedPointer<QCefCoreBrowserBase> doCreateCoreBrowser(const CefRefPtr<CefBrowser>& browser, const QString& webId, const QString& shmName, QMap<QString, QString> extraData, QObject* parent) override;
    virtual bool modifyCefConfig(CefSettings& settings) override;
};
