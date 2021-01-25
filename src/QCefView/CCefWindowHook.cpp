#include "CCefWindowHook.h"

#include <QMutexLocker>

QMutex CCefWindowHook::g_hwndDataMutex;
QMap<HWND, CCefWindowHook*> CCefWindowHook::g_hwndDataMap;


CCefWindowHook::CCefWindowHook(QObject * parent)
    : QObject(parent)
    , m_pOldWindowProc(NULL)
    , m_hHookWnd(NULL)
    , m_hBrowser(NULL)
    , m_hTarget(NULL)
{}

CCefWindowHook::~CCefWindowHook()
{
    clear();
}

bool CCefWindowHook::installWinProcHook(HWND hBrowser, HWND hTarget, const QString& windowClassName)
{
    m_hBrowser = hBrowser;
    m_hTarget = hTarget;
    //安装消息拦截器，用来处理窗口拖动
    m_hHookWnd = GetWindowHandle(hBrowser, ::GetCurrentProcessId(), ::GetCurrentThreadId(), windowClassName.toStdString().c_str());
    if (!m_hHookWnd)
        return false;
    QMutexLocker lck(&g_hwndDataMutex);
    //already hooked
    if (g_hwndDataMap[m_hHookWnd] == this)
        return true;
    g_hwndDataMap.insert(m_hHookWnd, this);
    ::SetTimer(m_hHookWnd, 0, 0, [](HWND m_hHookWnd, UINT message, UINT_PTR idTimer, DWORD dwTime) {
        QMutexLocker lck(&g_hwndDataMutex);
        if (g_hwndDataMap.contains(m_hHookWnd)) {
            auto pImp = reinterpret_cast<CCefWindowHook*>(g_hwndDataMap[m_hHookWnd]);
            pImp->m_pOldWindowProc = SetWindowLongPtr(m_hHookWnd, GWLP_WNDPROC, (LONG_PTR)CCefWindowHook::MessageWindowProc);
        }
        KillTimer(m_hHookWnd, idTimer);
    });
    return true;
}

void CCefWindowHook::clear()
{
    if (m_hHookWnd) {
        SetWindowLongPtr((HWND)m_hHookWnd, GWLP_WNDPROC, m_pOldWindowProc);
        QMutexLocker lck(&g_hwndDataMutex);
        if (g_hwndDataMap.contains(m_hHookWnd))
            g_hwndDataMap.remove(m_hHookWnd);
        m_hHookWnd = nullptr;
    }
}

LRESULT CALLBACK CCefWindowHook::MessageWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CCefWindowHook* pThis;
    {
        QMutexLocker lck(&g_hwndDataMutex);
        if (!g_hwndDataMap.contains(hwnd))
            return 0;
        pThis = reinterpret_cast<CCefWindowHook*>(g_hwndDataMap[hwnd]);
    }
    if (!pThis)
        return 0;
    BOOL bHandle = FALSE;
    LRESULT lr = pThis->MessageProc(uMsg, wParam, lParam, bHandle);
    if (bHandle) return lr;
    return CallWindowProc((WNDPROC)(pThis->m_pOldWindowProc), hwnd, uMsg, wParam, lParam);
}

LRESULT CCefWindowHook::MessageProc(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandle)
{
    switch (uMsg) {
    case WM_LBUTTONDOWN: {
        ::SendMessage(m_hTarget, uMsg, wParam, lParam);
    }break;
    case WM_LBUTTONUP: {
        ::SendMessage(m_hTarget, uMsg, wParam, lParam);
    }break;
    case WM_MOUSEMOVE: {
        ::SendMessage(m_hTarget, uMsg, wParam, lParam);
    }break;
    case WM_KEYDOWN: {
        ::SendMessage(m_hTarget, uMsg, wParam, lParam);
    }break;
    case WM_KEYUP: {
        ::SendMessage(m_hTarget, uMsg, wParam, lParam);
    }break;
    }
    return 0;
}



BOOL EnumChildWindows_(HWND hWnd, __in WNDENUMPROC lpEnumFunc, __in LPARAM lParam);
BOOL CALLBACK EnumProcForGetWindowHandle(HWND hWnd, LPARAM lParam);

HWND GetWindowHandle(HWND hParentWnd, DWORD dwProcessId, DWORD dwThreadId, const char* szWindowClassName)
{
    GetWindowHandleData data; data.dwProcessId = dwProcessId;
    data.dwThreadId = dwThreadId;
    data.strWindowName = QString::fromLocal8Bit(szWindowClassName);
    EnumChildWindows_(hParentWnd, EnumProcForGetWindowHandle, (LPARAM)(&data));
    return data.hWindow;
}

BOOL EnumChildWindows_(HWND hWnd, __in WNDENUMPROC lpEnumFunc, __in LPARAM lParam)
{
    HWND hChildWnd = GetWindow(hWnd, GW_CHILD);
    if (NULL == hChildWnd) return FALSE;

    do {
        if (NULL != lpEnumFunc) {
            if (!lpEnumFunc(hChildWnd, lParam)) {
                return FALSE;
            }
        }

        EnumChildWindows_(hChildWnd, lpEnumFunc, lParam);
        hChildWnd = GetWindow(hChildWnd, GW_HWNDNEXT);
        Sleep(0);
    } while (NULL != hChildWnd);
    return TRUE;
}

BOOL CALLBACK EnumProcForGetWindowHandle(HWND hWnd, LPARAM lParam)
{
    GetWindowHandleData* pData = (GetWindowHandleData*)(lParam);
    if (NULL == pData) return TRUE;

    //判断是否是目标进程的窗口
    DWORD dwWndPid = 0, dwWndThreadId = 0;
    dwWndThreadId = GetWindowThreadProcessId(hWnd, &dwWndPid);
    if (0 != pData->dwProcessId && pData->dwProcessId != dwWndPid) return TRUE;

    char szClassName[1024] = { 0 };
    GetClassNameA(hWnd, szClassName, 1024);
    if (0 == _stricmp(szClassName, pData->strWindowName.toStdString().c_str())) {
        //if (pData->dwThreadId != 0 && dwWndThreadId == pData->dwThreadId) {
        pData->hWindow = hWnd;
        return FALSE;
        //}
    }
    return TRUE;
}