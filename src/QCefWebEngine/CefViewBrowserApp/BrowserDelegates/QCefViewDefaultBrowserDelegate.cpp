#include "QCefViewDefaultBrowserDelegate.h"
#include "QCefProtocol.h"
#include "public/QCefSetting.h"
#include "client_switches.h"

#include <QString>
#include <QDebug>

namespace QCefViewDefaultBrowserDelegate {
    void
        CreateBrowserDelegate(QCefViewBrowserApp::BrowserDelegateSet& delegates)
    {
        delegates.insert(new BrowserDelegate());
    }

    BrowserDelegate::BrowserDelegate() {}

    void BrowserDelegate::OnContextInitialized(CefRefPtr<QCefViewBrowserApp> app)
    {
    }

    void BrowserDelegate::OnBeforeChildProcessLaunch(CefRefPtr<QCefViewBrowserApp> app, CefRefPtr<CefCommandLine> command_line)
    {
        //loglevel
        auto logLevel = QString::number(QCefSetting::logLevel()).toStdWString();
        command_line->AppendSwitchWithValue(QCEF_LOGLEVEL_OPTION_NAME, logLevel);
        //parentId, used by js binding shared memory
        QString parentId = QString::number(GetCurrentProcessId());
        command_line->AppendSwitchWithValue(QCEF_PARENT_ID_NAME, parentId.toStdWString());
    }
}