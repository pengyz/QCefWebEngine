#pragma once;
#include <qt_windows.h>
#include <map>
#include <QString>
#include <QVariantList>
#include <QSize>
#include <QPoint>
#include <QMultiMap>
#include <QSharedPointer>
#include <stdint.h>
#include <mutex>
#include <condition_variable>
#include <QVector>
#include <QRect>
#include "qcefcore_export.h"

struct QOverlayImageInfo;

struct CEFPaintData {
    std::atomic<int> needPaint = false;         //是否需要绘制
    const void* buffer = nullptr;               //缓冲区指针
    int width = 0;                              //宽度
    int height = 0;                             //高度
    QVector<QRect> dirtyRects;                  //脏矩形
    std::mutex paintMutex;                      //互斥量
    std::condition_variable cv;                 //条件变量
};

class CefCoreBrowser {
    //getter and setter
public:
    /**
     * @brief 浏览器类型
     * @author Alex.peng
     */
    enum BrowserType {
        e_NormalBrowser,
        e_OverlayBrowser,
        e_HtmlSurfaceBrowser,
    };

    virtual const QString& getTitle() const = 0;
    virtual void setTitle(const QString& title) = 0;
    virtual const QPoint& getMousePoint() const = 0;
    virtual void setMousePoint(const QPoint& point) = 0;
    virtual const QSize& getSize() const = 0;
    virtual void setSize(const QSize& rect) = 0;
    virtual float getDeviceScaleFactor() const = 0;
    virtual void setDeviceScaleFactor(float scaleFactor) = 0;
    virtual bool isMouseDown() const = 0;
    virtual void setMouseDown(bool bDown) = 0;
    virtual double getScrollX() const = 0;
    virtual void setScrollX(double scrollX) = 0;
    virtual double getScrollY() const = 0;
    virtual void setScrollY(double scrollY) = 0;
    virtual int getBrowserId() const = 0;
    virtual const QString& getWebId() const = 0;
    virtual void setWebId(const QString& webId) = 0;
    virtual HWND getBrowserWnd() const = 0;
    virtual QOverlayImageInfo* getImageInfo() const = 0;
    virtual const std::multimap<QString, QString>& getHeaders() = 0;
    virtual void addHeader(const QString& key, const QString& value) = 0;
    virtual bool isVisible() = 0;
    virtual void setClosing(bool bClosing) = 0;
    virtual bool isClosing() = 0;


    //对象锁
    virtual bool tryLock() = 0;
    virtual void lock() = 0;
    virtual void unlock() = 0;
    //获取额外信息
    virtual const QString getExtraData(const QString& key) = 0;
    virtual BrowserType browserType() = 0;

    //操作
public:
    virtual bool browserLoadUrl(const QString& url) = 0;
    virtual bool browserPostUrl(const QString& url, const QByteArray& postBytes) = 0;
    virtual bool browserCanGoBack() = 0;
    virtual bool browserCanGoForward() = 0;
    virtual bool browserGoBack() = 0;
    virtual bool browserGoForward() = 0;
    virtual bool browserIsLoading() = 0;
    virtual bool browserReload() = 0;
    virtual bool browserStopLoad() = 0;
    virtual QString browserGetUrl() = 0;
    virtual bool notifyMoveOrResizeStarted() = 0;
    virtual bool invokeJavaScriptCallback(qint64 frameId, const QString& jsCallbackSignature, QVariantList params) = 0;
    virtual bool clearJavaScriptCallback(qint64 frameId, const QString& jsCallbackSignature) = 0;
    virtual bool execJavaScript(const QString& javaScriptCode, const QString& scriptUrl, int startLine) = 0;
    virtual bool closeBrowser(bool bForce) = 0;
    virtual bool resizeBrowser() = 0;
    virtual bool resizeBrowser(int width, int height) = 0;
    virtual bool getLinkAtPosition(int x, int y) = 0;
    virtual bool copy() = 0;
    virtual bool past() = 0;
    virtual bool cut() = 0;
    virtual bool undo() = 0;
    virtual bool redo() = 0;
    virtual bool selectAll() = 0;
    virtual bool browserSetFocus(bool hasFocus) = 0;
    virtual bool find(const QString& pchSearchStr, bool bCurrentlyInFind, bool bReverse) = 0;
    virtual bool stopFinding() = 0;
    virtual bool viewSource() = 0;
    virtual bool setScaleFactor(float scaleFactor) = 0;
    virtual bool updateHorizontalScroll(double delta) = 0;
    virtual bool updateVerticalScroll(double delta) = 0;
    virtual bool setBrowserVisible(bool bVisible) = 0;
    virtual double getZoomLevel() = 0;
    virtual bool setZoomLevel(double level) = 0;
    //保存paint数据
    virtual void waitPaint(const void* buffer, int width, int height, const QVector<QRect>& dirtyRects) = 0;
    virtual CEFPaintData& paintData() = 0;


#pragma region(mouse event)
    virtual bool browserMouseEvent(unsigned int message, int x, int y, unsigned int flags) = 0;
    virtual bool browserMouseMove(int x, int y) = 0;
    virtual bool browserMouseUp(int type) = 0;
    virtual bool browserMouseDown(int type) = 0;
    virtual bool browserMouseDoubleClick(int type) = 0;
    virtual bool browserMouseWheel(int delta) = 0;
#pragma endregion

#pragma region(key event)
    virtual bool browserKeyDown(unsigned int virtualKeyCode, unsigned int flags, bool systemKey) = 0;
    virtual bool browserKeyUp(unsigned int virtualKeyCode, unsigned int flags, bool systemKey) = 0;
    virtual bool browserKeyPress(unsigned int charCode, unsigned int flags, bool systemKey) = 0;
#pragma endregion

};

#define LockCoreBrowser(browser)    QCefCoreBrowserLocker lck##__LINE__ (browser, __FILE__, __FUNCTION__, __LINE__)

class QCEFCORE_EXPORT QCefCoreBrowserLocker {
public:
    QCefCoreBrowserLocker(const QSharedPointer<CefCoreBrowser>& browser, const char* file, const char* function, int line);

    ~QCefCoreBrowserLocker();

    QCefCoreBrowserLocker(const QCefCoreBrowserLocker& locker) = delete;

private:
    QSharedPointer<CefCoreBrowser> pBrowserPtr;
    QString _file;
    QString _function;
    int _line = -1;
};
