#pragma once;

#include "../QCefCore/QCefCoreBrowserBase.h"
struct QOverlayImageInfo;

class QCefCoreBrowser : public QCefCoreBrowserBase {
public:
    QCefCoreBrowser(const CefRefPtr<CefBrowser>& browser, const QString& webId, QMap<QString,QString> extraData, QObject* parent = nullptr);


    virtual QOverlayImageInfo* getImageInfo() const override;

};