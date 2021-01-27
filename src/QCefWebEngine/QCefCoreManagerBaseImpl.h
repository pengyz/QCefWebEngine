#pragma once;
#include <QObject>
#include <QMap>
#include <QMutex>
#include <QSharedPointer>
#include <QQueue>
#include <QPair>
#include <tuple>
#include <include/cef_client.h>
#include <include/cef_app.h>
#include "CefViewBrowserApp/QCefBrowserHandlerBase.h"
#include "QCefCoreBrowserBase.h"
#include "qcefwebengine_export.h"

enum class HtmlSurfaceConsumerType;


/**
 * @brief CEF核心管理器基础实现
 * 负责管理CEF库的初始化/释放 浏览器的创建，关闭，获取指定浏览器对象等
 * @author Alex.peng
 */
class QCEFWEBENGINE_EXPORT QCefCoreManagerBaseImpl : public QObject {
    Q_OBJECT
public:
    QCefCoreManagerBaseImpl(QObject* parent = nullptr);

    /**
     * @brief 创建浏览器
     * @param
     *  webId                   浏览器唯一标识
     *  url                     加载的url
     *  shmName                 共享内存名，用于离屏渲染时共享渲染数据
     *  hParentWnd              父进程窗口句柄
     *  extraData               额外信息，会传递给CefHandler
     *  width                   初始宽度
     *  height                  初始高度
     * @author Alex.peng
     */
    bool createBrowser(const QString& webId, const QString& url, const QString& shmName, HWND hParentWnd,
        QMap<QString, QString> extraData);

    /**
     * @brief 初始化
     * @param
     *  bWindowless             是否启用离屏渲染
     * @author Alex.peng
     */
    void init(bool bWindowless, bool bForceEnableDevTool = false);

    /**
     * @brief 反初始化
     * @author Alex.peng
     */
    void deInit();

    /**
     * @brief 根据browserId获取浏览器包装对象
     * @author Alex.peng
     */
    QSharedPointer<QCefCoreBrowserBase> getCoreBrowser(int browserId);

    /**
     * @brief 根据webId获取浏览器包装对象（该方式较快）
     * @author Alex.peng
     */
    QSharedPointer<QCefCoreBrowserBase> getCoreBrowser(const QString& webId);

    /**
     * @brief 关闭所有浏览器
     * @author Alex.peng
     */
    void closeAllBrowsers(bool bForce);

protected:

    /**
     * @brief 创建一个BrowserHandler实例，由子类负责实现以实现不同创建策略
     * @author Alex.peng
     */
    virtual CefRefPtr<QCefBrowserHandlerBase> doInitCefBrowserHandler() = 0;

    /**
    * @brief 创建一个浏览器包装对象实例，由子类负责实现以实现不同创建策略
    * @author Alex.peng
    */
    virtual QSharedPointer<QCefCoreBrowserBase> doCreateCoreBrowser(const CefRefPtr<CefBrowser>& browser, const QString& webId, const QString& shmName, QMap<QString, QString> extraData, QObject* parent) = 0;

    /**
     * @brief 通过浏览器包装对象来关闭指定浏览器
     * @author Alex.peng
     */
    bool closeBrowser(QSharedPointer<QCefCoreBrowserBase> browser, bool bForce);

    /**
     * @brief 用来修改CEF配置的模板方法
     * @author Alex.peng
     */
    virtual bool modifyCefConfig(CefSettings& setting) = 0;

signals:
    void sig_notifyBrowserWindowIntergrate(const QString& webId, int browserId, qint32 hwnd);
    void sig_browserAddedNotify(const QString& webId, int browserId);
    void sig_removeBrowserNotify(const QString& webId, int browserId);

    void sig_setCefBrowserWindow(int browserId, HWND hwnd);
    void sig_loadingStateChanged(int browserId, bool isLoading, bool canGoBack, bool canGoForward);
    void sig_loadStart(int browserId);
    void sig_loadEnd(int browserId, int httpStatusCode);
    void sig_loadError(int browserId, int errorCode, QString url);
    void sig_allBrowserClosed();
    void sig_notifyFullScreenModeChanged(int browserId, bool fullscreen);

private:
    bool initCef(bool bForceEnableDevTool);

private slots:
    void onAddBrowser(const QString& webId, const CefRefPtr<CefBrowser>& browser, const QString& shmName, const QMap<QString, QString> extraData);
    void onRemoveBrowser(int browserId);
    void onAddBrowserEarly(const CefRefPtr<CefBrowser>& browser);
    void onClosingBrowser(int browserId);

    void onAddDevToolBrowser(const CefRefPtr<CefBrowser>& browser);
    void onRemoveDevToolBrowser(int browserId);

protected:
    CefRefPtr<QCefBrowserHandlerBase> _browserHandler;
    QMap<QString, QSharedPointer<QCefCoreBrowserBase>> _browserMap;
    QMap<int, CefRefPtr<CefBrowser>> _devToolsMap;
    QMutex _browserMapMutex;

    bool _bWindowless = false;
    CefRefPtr<CefApp> _app;
    CefSettings _cef_settings;
    int64_t _nBrowserRefCount = 0;
    int _findId = 0;
    QQueue<std::tuple<QString, QString, QMap<QString, QString>>> _webIdQueue;
    QMutex _webIdQueueMutex;
};