#include "QCefCoreManagerBaseImpl.h"
#include "QCefCoreBrowserBase.h"
#include "CCefSetting.h"
#include "CefViewBrowserApp/QCefViewBrowserApp.h"
#include "CefViewBrowserApp/QCefBrowserHandlerBase.h"
#include "public/CefCoreBrowser.h"
#include "QCefProtocol.h"

#include <QMetaType>


class QCefCoreBrowserBase;
Q_DECLARE_METATYPE(CefRefPtr<CefBrowser>);

QCefCoreManagerBaseImpl::QCefCoreManagerBaseImpl(QObject* parent /*= nullptr*/)
{
    qRegisterMetaType<CefRefPtr<CefBrowser>>();
}

bool QCefCoreManagerBaseImpl::createBrowser(const QString& webId, const QString& url, const QString& shmName,
    HWND hParentWnd, QMap<QString, QString> extraData)
{
    // Set window info
    CefWindowInfo window_info;
    RECT rc = { 0 };
    if (_bWindowless) {
        window_info.SetAsWindowless(hParentWnd);
    } else {
        window_info.SetAsChild(hParentWnd, rc);
    }
    CefRefPtr<CefDictionaryValue> extra_info = CefDictionaryValue::Create();
    extra_info->SetString("webId", webId.toStdWString());
    if (!shmName.isEmpty())
        extra_info->SetString("shmName", shmName.toStdWString());
    for (const auto& key : extraData.keys()) {
        extra_info->SetString(key.toStdWString(), extraData[key].toStdWString());
    }
    CefBrowserSettings browserSettings;
    browserSettings.minimum_font_size = 12; //模拟Chrome，最小字号限制为12
    browserSettings.plugins = STATE_ENABLED/*STATE_DISABLED*/; // disable all plugins
    if (!CefBrowserHost::CreateBrowser(
        window_info,        // window info
        _browserHandler,    // handler
        url.toStdString(),  // url
        browserSettings,   // settings
        extra_info,
        CefRequestContext::GetGlobalContext())) {
        return false;
    }
    //将webId压入queue，方便后续快速添加Browser
    QMutexLocker lck(&_webIdQueueMutex);
    _webIdQueue.push_back({ webId, shmName, extraData });
    return true;
}

void QCefCoreManagerBaseImpl::init(bool bWindowless, bool bForceEnableDevTool)
{
    _bWindowless = bWindowless;
    initCef(bForceEnableDevTool);
    //初始化BrowserHandler
    _browserHandler = doInitCefBrowserHandler();
    connect(_browserHandler.get(), &QCefBrowserHandlerBase::sig_addBrowser, this, &QCefCoreManagerBaseImpl::onAddBrowser, Qt::DirectConnection);
    connect(_browserHandler.get(), &QCefBrowserHandlerBase::sig_removeBrowser, this, &QCefCoreManagerBaseImpl::onRemoveBrowser, Qt::DirectConnection);
    connect(_browserHandler.get(), &QCefBrowserHandlerBase::sig_closingBrowser, this, &QCefCoreManagerBaseImpl::onClosingBrowser, Qt::DirectConnection);
    connect(_browserHandler.get(), &QCefBrowserHandlerBase::sig_notifyBrowserEarly, this, &QCefCoreManagerBaseImpl::onAddBrowserEarly, Qt::DirectConnection);
    connect(_browserHandler.get(), &QCefBrowserHandlerBase::sig_notifyFullScreenModeChanged, this, &QCefCoreManagerBaseImpl::sig_notifyFullScreenModeChanged, Qt::QueuedConnection);
    connect(_browserHandler.get(), &QCefBrowserHandlerBase::sig_notifyDevToolBrowser, this, &QCefCoreManagerBaseImpl::onAddDevToolBrowser, Qt::DirectConnection);
    connect(_browserHandler.get(), &QCefBrowserHandlerBase::sig_removeDevToolBrowser, this, &QCefCoreManagerBaseImpl::onRemoveDevToolBrowser, Qt::DirectConnection);

    connect(_browserHandler.get(), &QCefBrowserHandlerBase::sig_loadingStateChanged, this, &QCefCoreManagerBaseImpl::sig_loadingStateChanged, Qt::QueuedConnection);
    connect(_browserHandler.get(), &QCefBrowserHandlerBase::sig_loadStart, this, &QCefCoreManagerBaseImpl::sig_loadStart, Qt::QueuedConnection);
    connect(_browserHandler.get(), &QCefBrowserHandlerBase::sig_loadEnd, this, &QCefCoreManagerBaseImpl::sig_loadEnd, Qt::QueuedConnection);
    connect(_browserHandler.get(), &QCefBrowserHandlerBase::sig_loadError, this, &QCefCoreManagerBaseImpl::sig_loadError, Qt::QueuedConnection);
}

void QCefCoreManagerBaseImpl::deInit()
{
    // This is not the last time release
    if (--_nBrowserRefCount > 0)
        return;

    // Destroy the application
    _app = nullptr;

    // Shutdown CEF
    CefShutdown();
}

QSharedPointer<QCefCoreBrowserBase> QCefCoreManagerBaseImpl::getCoreBrowser(int browserId)
{
    QMutexLocker lck(&_browserMapMutex);
    auto pos = std::find_if(_browserMap.begin(), _browserMap.end(), [browserId](const QSharedPointer<QCefCoreBrowserBase>& browserInfo) {
        return browserId == browserInfo->getBrowserId();
    });
    if (pos != _browserMap.end()) {
        return *pos;
    }
    return QSharedPointer<QCefCoreBrowserBase>(nullptr);
}

QSharedPointer<QCefCoreBrowserBase> QCefCoreManagerBaseImpl::getCoreBrowser(const QString& webId)
{
    QMutexLocker lck(&_browserMapMutex);
    if (_browserMap.contains(webId))
        return _browserMap[webId];
    return QSharedPointer<QCefCoreBrowserBase>(nullptr);
}

void QCefCoreManagerBaseImpl::closeAllBrowsers(bool bForce)
{
    QMutexLocker lck(&_browserMapMutex);
    if (_browserMap.isEmpty()) {
        emit sig_allBrowserClosed();
        return;
    }

    for (auto& browsers : _browserMap) {
        closeBrowser(browsers, bForce);
    }
}

bool QCefCoreManagerBaseImpl::initCef(bool bForceEnableDevTool)
{
    // This is not the first time initialization
    if (++_nBrowserRefCount > 1)
        return true;

    // Enable High-DPI support on Windows 7 or newer.
    CefEnableHighDPISupport();

    // This is the first time initialization
    CCefSetting::initializeInstance();

    CefString(&_cef_settings.browser_subprocess_path) = CCefSetting::browser_sub_process_path;
    CefString(&_cef_settings.resources_dir_path) = CCefSetting::resource_directory_path;
    CefString(&_cef_settings.locales_dir_path) = CCefSetting::locales_directory_path;
    CefString(&_cef_settings.user_agent) = CCefSetting::user_agent;
    CefString(&_cef_settings.cache_path) = CCefSetting::cache_path;
    CefString(&_cef_settings.user_data_path) = CCefSetting::user_data_path;
    CefString(&_cef_settings.locale) = CCefSetting::locale;
    CefString(&_cef_settings.accept_language_list) = CCefSetting::accept_language_list;
    CefString(&_cef_settings.log_file) = CCefSetting::log_file_name;

    _cef_settings.persist_session_cookies = CCefSetting::persist_session_cookies;
    _cef_settings.persist_user_preferences = CCefSetting::persist_user_preferences;
    //debug settings, it can be forcely enabled
    CCefSetting::debug_enabled = bForceEnableDevTool || CCefSetting::debug_enabled;
    if (CCefSetting::debug_enabled) {
        _cef_settings.remote_debugging_port = CCefSetting::remote_debugging_port;
        _cef_settings.log_severity = LOGSEVERITY_DEFAULT;
    } else {
        _cef_settings.log_severity = LOGSEVERITY_DISABLE;
    }
    _cef_settings.background_color = CCefSetting::background_color;
    _cef_settings.no_sandbox = true;
    _cef_settings.pack_loading_disabled = false;
    _cef_settings.multi_threaded_message_loop = true;
    _cef_settings.windowless_rendering_enabled = _bWindowless;

    if (!modifyCefConfig(_cef_settings)) {
        //TRACEE("modifyCefConfig failed !");
        return false;
    }

    _app = new QCefViewBrowserApp();

    HINSTANCE hInstance = ::GetModuleHandle(nullptr);
    CefMainArgs main_args(hInstance);

    // Initialize CEF.
    void* sandboxInfo = nullptr;
    if (!CefInitialize(main_args, _cef_settings, _app, sandboxInfo)) {
        //TRACEE("CefInitialize failed !");
        return false;
    }
    return true;
}

void QCefCoreManagerBaseImpl::onAddBrowser(const QString& webId, const CefRefPtr<CefBrowser>& browser,
    const QString& shmName, const QMap<QString, QString> extraData)
{
    QMutexLocker lck(&_browserMapMutex);
    int browserId = browser->GetIdentifier();
    if (_browserMap.contains(webId)) {
        //already added
        if (browser->IsSame(_browserMap[webId]->_browser))
            return;
        _browserMap.remove(webId);
        //TRACEE("webId: %s invalid, try readd it!", qPrintable(webId));
    }
    // MessageBoxA(NULL, "QCefCoreManagerBaseImpl::onAddBrowser", "", MB_OK);
    QString strShmNameReal = shmName + QString::number(browserId);

    auto pCoreBrowser = doCreateCoreBrowser(browser, webId, strShmNameReal, extraData, this);
    if (!pCoreBrowser) {
        //TRACEE("webId: %s create failed!", qPrintable(webId));
        return;
    }
    _browserMap[webId] = pCoreBrowser;
    if (pCoreBrowser->getBrowserWnd())
        emit sig_notifyBrowserWindowIntergrate(webId, browserId, (qint32)pCoreBrowser->getBrowserWnd());

    emit sig_browserAddedNotify(webId, browserId);
}

void QCefCoreManagerBaseImpl::onRemoveBrowser(int browserId)
{
    QMutexLocker lck(&_browserMapMutex);
    bool bBrowserMapEmpty = _browserMap.isEmpty();

    //check browser
    auto pos = std::find_if(_browserMap.begin(), _browserMap.end(), [=](const QSharedPointer<QCefCoreBrowserBase>& browserInfo) {
        if (browserInfo->getBrowserId() == browserId)
            return true;
        return false;
    });
    if (pos != _browserMap.end()) {
        QString webId = pos.key();
        _browserMap.erase(pos);
        //TRACED("browser id: %d webId: %s removed browser !", browserId, qPrintable(webId));
        emit sig_removeBrowserNotify(webId, browserId);
        if (_browserMap.isEmpty())
            bBrowserMapEmpty = true;
    }

    //can we finally close?
    if (bBrowserMapEmpty && _devToolsMap.isEmpty())
        emit sig_allBrowserClosed();
}

void QCefCoreManagerBaseImpl::onClosingBrowser(int browserId)
{
    //标记当前browser为关闭中状态
    QMutexLocker lck(&_browserMapMutex);
    //check browser
    auto pos = std::find_if(_browserMap.begin(), _browserMap.end(), [=](const QSharedPointer<QCefCoreBrowserBase>& browserInfo) {
        if (browserInfo->getBrowserId() == browserId)
            return true;
        return false;
    });
    if (pos != _browserMap.end()) {
        QString webId = pos.key();
        pos.value()->setClosing(true); //设置关闭标志位
    }
}


void QCefCoreManagerBaseImpl::onAddDevToolBrowser(const CefRefPtr<CefBrowser>& browser)
{
    //保存devTool的浏览器对象
    QMutexLocker lck(&_browserMapMutex);
    if (_devToolsMap.contains(browser->GetIdentifier())) {
        _devToolsMap.remove(browser->GetIdentifier());
    }
    _devToolsMap[browser->GetIdentifier()] = browser;
}

void QCefCoreManagerBaseImpl::onRemoveDevToolBrowser(int browserId)
{
    //移除devTool的浏览器对象
    QMutexLocker lck(&_browserMapMutex);
    if (!_devToolsMap.contains(browserId))
        return;
    _devToolsMap.remove(browserId);
    if (_devToolsMap.isEmpty() && _browserMap.isEmpty())
        emit sig_allBrowserClosed();
}

void QCefCoreManagerBaseImpl::onAddBrowserEarly(const CefRefPtr<CefBrowser>& browser)
{
    QString strWebId, strShmName;
    QMap<QString, QString> extraData;
    {
        QMutexLocker lck1(&_webIdQueueMutex);
        if (_webIdQueue.size()) {
            std::tie(strWebId, strShmName, extraData) = _webIdQueue.front();
            _webIdQueue.pop_front();
        }
    }
    if (strWebId.isEmpty())
        return;
    onAddBrowser(strWebId, browser, strShmName, extraData);
}

bool QCefCoreManagerBaseImpl::closeBrowser(QSharedPointer<QCefCoreBrowserBase> browser, bool bForce)
{
    if (!browser) {
        //TRACED("browser not found !");
        return false;
    }
    if (browser->_browser->IsLoading()) {
        browser->_browser->StopLoad();
    }
    browser->_browser->GetHost()->CloseBrowser(bForce);
    return true;
}