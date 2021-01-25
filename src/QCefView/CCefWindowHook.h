#pragma once;
#include <QObject>
#include <qt_windows.h>
#include <QMutex>
#include <QMap>

/**
 * @brief 用于遍历窗口的记录结构，被GetWindowHandle函数所使用
 * @author Alex.peng
 */
struct GetWindowHandleData {
    DWORD	dwProcessId;			// [IN]
    DWORD	dwThreadId;				// [IN]
    QString	strWindowName;			// [IN]
    HWND	hWindow;				// [OUT]
public:
    GetWindowHandleData() :dwProcessId(0), dwThreadId(0), hWindow(NULL) { ; }
};

//遍历获取指定窗口类名的窗口
HWND GetWindowHandle(HWND hParentWnd, DWORD dwProcessId, DWORD dwThreadId, const char* szWindowClassName);

///
/// \brief 钩子类，用来处理CEF的窗口透传，实现在Web页面指定区域拖拽移动窗口效果
///
/// \author Alex.peng
///
class CCefWindowHook : public QObject {
public:
    CCefWindowHook(QObject* parent = nullptr);
    ~CCefWindowHook() override;

    ///
    /// \brief 安装钩子过滤函数
    ///
    /// \author Albert.xu
    /// \Param 要被Hooke的浏览器窗口句柄
    /// \Param 接受消息的目标窗口句柄
    /// \Param 目标窗口类名，用于配合hBrowser查找窗口
    bool installWinProcHook(HWND hBrowser, HWND hTarget, const QString& windowClassName);
    void clear();

protected:
    static LRESULT CALLBACK MessageWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT MessageProc(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandle);

private:
    LONG_PTR m_pOldWindowProc;                          //保存旧的WndProc
    HWND m_hHookWnd;                                    //要hook的窗口句柄
    HWND m_hBrowser;                                    //浏览器窗口句柄
    HWND m_hTarget;                                     //接收消息的窗口句柄
    static QMutex g_hwndDataMutex;
    static QMap<HWND, CCefWindowHook*> g_hwndDataMap;
};