#pragma once;
#include <QObject>
#include <qt_windows.h>
#include <QVector>
#include <QSharedPointer>
#include <QVariantList>
#include "qcefcore_export.h"
#include "QCefJsCallbacks.h"

class QCefCoreManagerBaseImpl;
class CefCoreBrowser;

/**
 * @brief CEF核心管理器类
 * CEF的初始化，析构，浏览器对象的生命周期管理，浏览器功能执行都通过该类实现
 * QCefView和QCefOverlay应该直接使用此类与CEF进行交互
 * @author Alex.peng
 */
class QCEFCORE_EXPORT QCefCoreManagerBase : public QObject {
    Q_OBJECT

protected:
    QCefCoreManagerBase(QObject* parent = nullptr);
public:
    ~QCefCoreManagerBase();

public:
    static QCefCoreManagerBase* get();
    void init(bool bWindowless, bool bForceEnableDevTool);
    bool createBrowser(const QString& webId, const QString& url, const QString& shmName, HWND hParentWnd, QMap<QString, QString> extraData);
    void deInit();
    QSharedPointer<CefCoreBrowser> getCoreBrowser(int browserId);
    QSharedPointer<CefCoreBrowser> getCoreBrowser(const QString& webId);
    void closeAllBrowsers(bool bForce);
    bool setCookie(const QString& pchHostname, const QString& pchKey, const QString& pchValue, const QString& pchPath /*= "/"*/, float nExpires /*= 0*/, bool bSecure /*= false*/, bool bHTTPOnly /*= false*/);
    bool clearJavaScriptCallback(const QString& jsCallbackSignature);
    bool invokeJavaScriptCallback(const QString& jsCallbackSignature, const QVariantList& params);
    /**
     * @brief 解析signature，获取js callback集合
     * @author Alex.peng
     */
    JavaScriptCallbacksCollection genJavaScriptCallbackCollection(const QString& signatures);

protected:
    virtual QCefCoreManagerBaseImpl* doImplInit() = 0;

signals:
    /**
     * @brief 浏览器创建完成通知，用于通知CEF窗口句柄
     * @author Alex.peng
     */
    void sig_notifyBrowserWindowIntergrate(const QString& webId, int browserId, qint32 hwnd);
    /**
     * @brief 浏览器对象销毁通知
     * @author Alex.peng
     */
    void sig_removeBrowserNotify(const QString& webId, int browserId);
    void sig_browserAddedNotify(const QString& webId, int browserId);


    void sig_setCefBrowserWindow(int browserId, HWND hwnd);
    void sig_loadingStateChanged(int browserId, bool isLoading, bool canGoBack, bool canGoForward);
    void sig_loadStart(int browserId);
    void sig_loadEnd(int browserId, int httpStatusCode);
    void sig_loadError(int browserId, int errorCode, QString url);
    void sig_notifyFullScreenModeChanged(int browserId, bool fullscreen);

    void sig_allBrowserClosed();


private:
    static QCefCoreManagerBase* m_instance;
    QCefCoreManagerBaseImpl* _pImpl = nullptr;


};