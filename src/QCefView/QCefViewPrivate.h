
#pragma region cef_headers
#include <include/cef_app.h>
#include <include/cef_browser.h>
#include <include/cef_frame.h>
#include <include/cef_sandbox_win.h>
#pragma endregion cef_headers

#include <QCefProtocol.h>
#include <QObject>
#include <QMouseEvent>

#include "public/QCefView.h"

class QWidget;

/**
 * @brief QCefView私有实现类
 * 主要是为了隐藏私有接口，仅暴露必要的共有接口给外部调用者
 * QCefView中的函数大部分会代理到此类
 * @author Alex.peng
 */
class QCefViewPrivate : public QObject {
    Q_OBJECT
public:
    explicit QCefViewPrivate(QWidget* webViewWidget, const QString& url, const QString& webId);
    ~QCefViewPrivate();

public:
    WId getCefWinId();
    void onBrowserWindowIntergrate(const QString& webId, int browserId, qint32 wnd);
    void browserLoadUrl(const QString& url);
    bool browserCanGoBack();
    bool browserCanGoForward();
    void browserGoBack();
    void browserGoForward();
    bool browserIsLoading();
    void browserReload();
    void browserStopLoad();
    QString getUrl();
    void notifyMoveOrResizeStarted();
    bool sendObjectRegisterMessage(const QString& regName, const QObject* registerObject);
    void onToplevelWidgetMoveOrResize();
    int getBrowserId();
    QString getWebId();
    void execJavaScript(const QString& javaScriptCode, const QString& scriptUrl, int startLine);
    bool setInputEventHook();
    double getZoomLevel();
    void setZoomLevel(double level);

private slots:
    void onLoadingStateChanged(int browserId, bool isLoading, bool canGoBack, bool canGoForward);
    void onLoadStart(int browserId);
    void onLoadEnd(int browserId, int httpStatusCode);
    void onLoadError(int browserId, int errorCode, QString url);
    void onFullScreenModeChanged(int browserId, bool fullscreen);


signals:
    void sig_initializeCefGui(WId winId);

    void sig_loadingStateChanged(bool isLoading, bool canGoBack, bool canGoForward);
    void sig_loadStart();
    void sig_loadEnd(int httpStatusCode);
    void sig_loadError(int errorCode, QString url);
    void sig_notifyFullScreenModeChanged(bool fullscreen);

private:
    QString webId_;
    QWidget* webViewWidget_;

    class CCefWindowHook* hookWindow_;
    class CCefWindowHook* hookRender_;
};
