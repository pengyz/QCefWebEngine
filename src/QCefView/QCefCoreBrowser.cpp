#include "QCefCoreBrowser.h"

QCefCoreBrowser::QCefCoreBrowser(const CefRefPtr<CefBrowser>& browser, const QString& webId, QMap<QString, QString> extraData, QObject* parent /*= nullptr*/)
    :QCefCoreBrowserBase(browser, webId, extraData, parent)
{
}

QOverlayImageInfo* QCefCoreBrowser::getImageInfo() const
{
    return nullptr;
}
