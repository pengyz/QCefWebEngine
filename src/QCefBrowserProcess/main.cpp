#ifdef _WIN32
#pragma region windows_headers
#include <tchar.h>
#include <windows.h>
#include <tlhelp32.h>
#pragma endregion windows_headers
#endif

#include <thread>

#pragma region cef_headers
#include <include/cef_app.h>
#pragma endregion cef_headers

#pragma region project_heasers
#include "QCefViewRenderApp.h"
#pragma endregion project_heasers

HANDLE GetParentProcess()
{
    HANDLE Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    PROCESSENTRY32 ProcessEntry = {};
    ProcessEntry.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(Snapshot, &ProcessEntry)) {
        DWORD CurrentProcessId = GetCurrentProcessId();

        do {
            if (ProcessEntry.th32ProcessID == CurrentProcessId)
                break;
        } while (Process32Next(Snapshot, &ProcessEntry));
    }

    CloseHandle(Snapshot);

    return OpenProcess(SYNCHRONIZE, FALSE, ProcessEntry.th32ParentProcessID);
}

//入口函数
int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);
    CefEnableHighDPISupport();

    //等待父进程结束后退出自身，解决Browser进程崩溃后Render进程残留问题
    HANDLE ParentProcess = GetParentProcess();
    std::thread([ParentProcess]()
    {
        WaitForSingleObject(ParentProcess, INFINITE);
        ExitProcess(0);
    }).detach();

    CefRefPtr<QCefViewRenderApp> app(new QCefViewRenderApp);
    CefMainArgs main_args(hInstance);
    void* sandboxInfo = nullptr;
    return CefExecuteProcess(main_args, app, sandboxInfo);
}