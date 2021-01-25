#ifndef QCEFVIEW_H
#define QCEFVIEW_H
#pragma once

#pragma region std_headers
#include <memory>
#pragma endregion std_headers

#pragma region qt_headers
#include <QtCore/qglobal.h>
#include <QWidget>
#include <QVariantList>
#pragma endregion qt_headers

#include "QCefSetting.h"
#include "qcefview_export.h"

class QCefViewPrivate;

/**
 * @brief CEF与Qt的整合类
 * @author Alex.peng
 */
class QCEFVIEW_EXPORT QCefView : public QWidget {
    Q_OBJECT
public:

    /**
     * @brief 创建QCefView
     * @param
     *  url                 默认打开的页面URL
     *  webId               指定页面唯一标识
     *  parentId            父对象
     * @author Alex.peng
     */
    QCefView(const QString url, const QString& webId, QWidget* parent = nullptr, int width = 0, int height = 0);

    QCefView(QWidget* parent = nullptr);

    ~QCefView();

    /**
     * @brief 获取CEF创建的窗口句柄
     * @author Alex.peng
     */
    WId getCefWinId();

    /**
     * @brief 加载指定URL
     * @param
     *  url                 要加载的URL
     * @author Alex.peng
     */
    void browserLoadUrl(const QString& url);

    /**
     * @brief 是否可以后退
     * @author Alex.peng
     */
    bool browserCanGoBack();

    /**
     * @brief 是否可以前进
     * @author Alex.peng
     */
    bool browserCanGoForward();

    /**
     * @brief 后退
     * @author Alex.peng
     */
    void browserGoBack();

    /**
     * @brief 前进
     * @author Alex.peng
     */
    void browserGoForward();

    /**
     * @brief 页面是否正在加载
     * @author Alex.peng
     */
    bool browserIsLoading();

    /**
     * @brief 重新加载页面
     * @author Alex.peng
     */
    void browserReload();

    /**
     * @brief 停止加载页面
     * @author Alex.peng
     */
    void browserStopLoad();

    /**
     * @brief 获取当前加载的页面URL
     * @author Alex.peng
     */
    QString getUrl();

    /// <summary>
    ///
    /// </summary>
    int getBrowserId();

    /// <summary>
    ///
    /// </summary>
    QString getWebId();

    /// <summary>
    ///
    /// </summary>
    /// <param name="javaScriptCode"></param>
    /// <param name="scriptUrl"></param>
    /// <param name="startLine"></param>
    void execJavaScript(const QString& javaScriptCode, const QString& scriptUrl = "", int startLine = 0);

    /**
     * @brief 设置鼠标事件钩子
     * @author Alex.peng
     */
    bool setInputEventHook();

    /**
     * @brief 获取页面缩放因子
     * @author Alex.peng
     */
    double getZoomLevel();

    /**
     * @brief 设置页面缩放因子
     * @author Alex.peng
     */
    void setZoomLevel(double level);


protected:

    virtual void resizeEvent(QResizeEvent *event) override;

    void mouseMoveEvent(QMouseEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;

    void mouseReleaseEvent(QMouseEvent *event) override;

signals:
    /// <summary>
    ///
    /// </summary>
    /// <param name="isLoading"></param>
    /// <param name="canGoBack"></param>
    /// <param name="canGoForward"></param>
    void loadingStateChanged(bool isLoading, bool canGoBack, bool canGoForward);

    /// <summary>
    ///
    /// </summary>
    void loadStart();

    /// <summary>
    ///
    /// </summary>
    /// <param name="httpStatusCode"></param>
    void loadEnd(int httpStatusCode);

    /// <summary>
    ///
    /// </summary>
    /// <param name="errorCode"></param>
    /// <param name="failedUrl"></param>
    void loadError(int errorCode, const QString& failedUrl);

signals:
    void sig_allBrowserClosed();

    void sig_notifyFullScreenModeChanged(bool fullscreen);

private:
    class QCefViewPrivate* pPrivate_ = nullptr;
};

#endif // QCEFVIEW_H
