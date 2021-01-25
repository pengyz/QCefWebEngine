#include <QCoreApplication>
#include <QDir>
#include <QCefProtocol.h>

#include "CCefSetting.h"
#include "include/storage.h"


CefString CCefSetting::browser_sub_process_path;

CefString CCefSetting::resource_directory_path;

CefString CCefSetting::locales_directory_path;

CefString CCefSetting::user_agent;

CefString CCefSetting::cache_path;

CefString CCefSetting::user_data_path;

int CCefSetting::persist_session_cookies;

int CCefSetting::persist_user_preferences;

CefString CCefSetting::locale;

int CCefSetting::remote_debugging_port;

cef_color_t CCefSetting::background_color;

CefString CCefSetting::accept_language_list;

int CCefSetting::debug_enabled = false;

int CCefSetting::devToolWidth = 800;

int CCefSetting::devToolHeight = 600;

CefString CCefSetting::log_file_name;



void CCefSetting::initializeInstance()
{
    static CCefSetting s_instance;
}

CCefSetting::CCefSetting()
{
    QString strAppData = QDir::toNativeSeparators(QDir::cleanPath(QString("%1/BaoGames/").arg(QString::fromLocal8Bit(qgetenv("AppData")))));
    QDir ExeDir = QDir::current();
    QDir appDataDir(strAppData);
    QString strCacheDir, strBackgroundColor, strBrowserHelperName, strLogFileName;
    if (!QLocalStorage::get()->HasLoaded("browser")) {
        QString strBrowserConfigFilePath = QDir::current().absoluteFilePath("Config/browser.json");
        QLocalStorage::get()->Load(QLocalStorage::eStorage_Json, strBrowserConfigFilePath);
    }
    QLocalStorage::get()->GetString("browser", "cacheDir", strCacheDir);
    QLocalStorage::get()->GetString("browser", "backgroundColor", strBackgroundColor);
    QLocalStorage::get()->GetString("browser", "browserHelper", strBrowserHelperName);
    QLocalStorage::get()->GetString("browser", "logFileName", strLogFileName);
    QLocalStorage::get()->GetInteger("browser.devConfig", "enable", debug_enabled);
    QLocalStorage::get()->GetInteger("browser.devConfig", "port", remote_debugging_port);
    QLocalStorage::get()->GetInteger("browser.devConfig", "width", devToolWidth);
    QLocalStorage::get()->GetInteger("browser.devConfig", "height", devToolHeight);


    QString strExePath = ExeDir.filePath(strBrowserHelperName.isEmpty() ? RENDER_PROCESS_NAME : strBrowserHelperName);
    browser_sub_process_path.FromWString(QDir::toNativeSeparators(strExePath).toStdWString());

    QString strResPath = ExeDir.filePath(RESOURCE_DIRECTORY_NAME);
    resource_directory_path.FromWString(QDir::toNativeSeparators(strResPath).toStdWString());

    QDir ResPath(strResPath);
    locales_directory_path.FromWString(QDir::toNativeSeparators(ResPath.filePath(LOCALES_DIRECTORY_NAME)).toStdWString());

    accept_language_list.FromString("zh-CN");

    cache_path.FromWString(QDir::toNativeSeparators(appDataDir.filePath(strCacheDir.isEmpty() ? CACHE_DIRECTORY_NAME : strCacheDir)).toStdWString());

    user_agent.FromString(QCEF_USER_AGENT);

    if (!strLogFileName.isEmpty())
        log_file_name.FromWString(QDir::toNativeSeparators(appDataDir.filePath(strLogFileName)).toStdWString());

    if (!strBackgroundColor.isEmpty())
        background_color = strBackgroundColor.toUInt(nullptr, 16);
}