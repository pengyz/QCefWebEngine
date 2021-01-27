#pragma once; 
#include <include/cef_browser.h>
#include <QString>
#include <QObject>
#include <QSize>
#include <QMutex>
#include <QSharedPointer>
#include <QMap>
#include <QVector>
#include <functional>
#include "public/CefCoreBrowser.h"
#include "qcefwebengine_export.h"


class QCEFWEBENGINE_EXPORT QCefCoreBrowserBase
    : public QObject
    , public CefCoreBrowser {
    friend class QCefCoreManagerImplOverlay;
    friend class QCefCoreManagerBaseImpl;
public:
    QCefCoreBrowserBase(const CefRefPtr<CefBrowser>& browser, const QString& webId, const QMap<QString, QString> extraData, QObject* parent = nullptr);

    //getter and setter
public:

    virtual const QString& getTitle() const override;
    virtual void setTitle(const QString& title) override;
    virtual const QPoint& getMousePoint() const override;
    virtual void setMousePoint(const QPoint& point) override;
    virtual const QSize& getSize() const override;
    virtual void setSize(const QSize& rect) override;
    virtual float getDeviceScaleFactor() const override;
    virtual void setDeviceScaleFactor(float scaleFactor) override;
    virtual bool isMouseDown() const override;
    virtual void setMouseDown(bool bDown) override;
    virtual double getScrollX() const override;
    virtual void setScrollX(double scrollX) override;
    virtual double getScrollY() const override;
    virtual void setScrollY(double scrollY) override;
    virtual int getBrowserId() const override;
    virtual const QString& getWebId() const override;
    virtual void setWebId(const QString& webId);
    virtual HWND getBrowserWnd() const override;
    virtual QOverlayImageInfo* getImageInfo() const override = 0;
    virtual void addHeader(const QString& key, const QString& value) override;
    virtual bool isVisible() override;
    virtual void setClosing(bool bClosing) override;
    virtual bool isClosing() override;

    virtual const std::multimap<QString, QString>& getHeaders() override;
    virtual bool tryLock() override;
    virtual void lock() override;
    virtual void unlock() override;
    virtual const QString getExtraData(const QString& key) override;
    virtual CefCoreBrowser::BrowserType browserType() override;

    //操作
public:
    virtual bool browserLoadUrl(const QString& url) override;
    virtual bool browserPostUrl(const QString& url, const QByteArray& postBytes) override;
    virtual bool browserCanGoBack() override;
    virtual bool browserCanGoForward() override;
    virtual bool browserGoBack() override;
    virtual bool browserGoForward() override;
    virtual bool browserIsLoading() override;
    virtual bool browserReload() override;
    virtual bool browserStopLoad() override;
    virtual QString browserGetUrl() override;
    virtual bool notifyMoveOrResizeStarted() override;
    virtual bool invokeJavaScriptCallback(qint64 frameId, const QString& jsCallbackSignature, QVariantList params) override;
    virtual bool clearJavaScriptCallback(qint64 frameId, const QString& jsCallbackSignature) override;
    virtual bool execJavaScript(const QString& javaScriptCode, const QString& scriptUrl, int startLine) override;
    virtual bool closeBrowser(bool bForce) override;
    virtual bool resizeBrowser() override;
    virtual bool resizeBrowser(int width, int height) override;
    virtual bool getLinkAtPosition(int x, int y) override;
    virtual bool copy() override;
    virtual bool past() override;
    virtual bool cut() override;
    virtual bool undo() override;
    virtual bool redo() override;
    virtual bool selectAll() override;
    virtual bool browserSetFocus(bool hasFocus) override;
    virtual bool find(const QString& pchSearchStr, bool bCurrentlyInFind, bool bReverse) override;
    virtual bool stopFinding() override;
    virtual bool viewSource() override;
    virtual bool setScaleFactor(float scaleFactor) override;
    virtual bool updateHorizontalScroll(double delta) override;
    virtual bool updateVerticalScroll(double delta) override;
    virtual bool setBrowserVisible(bool bHidden) override;
    virtual void waitPaint(const void* buffer, int width, int height, const QVector<QRect>& dirtyRects) override;
    virtual CEFPaintData& paintData() override;
    virtual double getZoomLevel() override;
    virtual bool setZoomLevel(double level) override;

#pragma region(mouse event)
    virtual bool browserMouseEvent(unsigned int message, int x, int y, unsigned int flags) override;
    virtual bool browserMouseMove(int x, int y) override;
    virtual bool browserMouseUp(int type) override;
    virtual bool browserMouseDown(int type) override;
    virtual bool browserMouseDoubleClick(int type) override;
    virtual bool browserMouseWheel(int delta) override;
#pragma endregion

#pragma region(key event)
    virtual bool browserKeyDown(unsigned int virtualKeyCode, unsigned int flags, bool systemKey) override;
    virtual bool browserKeyUp(unsigned int virtualKeyCode, unsigned int flags, bool systemKey) override;
    virtual bool browserKeyPress(unsigned int charCode, unsigned int flags, bool systemKey) override;

#pragma endregion

protected:
    //渲染数据
    CEFPaintData _paintData;


private:
    QString _webId;
    CefRefPtr<CefBrowser> _browser;
    QSize _size{ 1, 1 };
    QString _title;
    QPoint _mousePoint;
    std::multimap<QString, QString> headers;
    bool _mouseDown = false;
    float _deviceScaleFactor{ 1.0 };
    double _scrollX{ 0.0 };
    double _scrollY{ 0.0 };
    int _findId = 0;
    QMap<QString, QString> _extraData; //创建浏览器时传递的额外信息
    bool _visible = true;
    QMutex _objMutex;
    bool _isClosing = false;            //是否正在关闭
};
