#include "public/QCefCookie.h"

#include <include/cef_cookie.h>
#include <include/cef_task.h>

class Visitor :public CefCookieVisitor {
    friend class CCefCookie;
public:
    Visitor(const QString& u, PFNGetCookieCallback callback);
public:
    virtual bool Visit(const CefCookie& ck, int count, int total, bool& deleteCookie) override;
private:
    QStringList	_domains;
    QStringList	_paths;
    QStringList	_values;
    QStringList	_names;
    QString _url;
    PFNGetCookieCallback _callback;
private:
    IMPLEMENT_REFCOUNTING(Visitor);
};

class CCookieTask :public CefTask {
private:
    QString		m_wstrUrl;
    CefCookie	m_cookie;
public:
    CCookieTask(const QString& url, const CefCookie& ck);
    ~CCookieTask() = default;
public:
    virtual void Execute() override;
private:
    IMPLEMENT_REFCOUNTING(CCookieTask);
};

Visitor::Visitor(const QString& u, PFNGetCookieCallback callback)
    :_url(u), _callback(callback)
{
}

bool Visitor::Visit(const CefCookie& ck, int count, int total, bool& deleteCookie)
{
    //TRACED("count[%d] total[%d]", count, total);
    QString domain = QString::fromWCharArray(ck.domain.str);
    QString path = QString::fromWCharArray(ck.path.str);
    QString name = QString::fromWCharArray(ck.name.str);
    QString value = QString::fromWCharArray(ck.value.str);
    _domains.push_back(domain);
    _paths.push_back(path);
    _names.push_back(name);
    _values.push_back(value);
    deleteCookie = false;
    if (count + 1 == total) {
        QString strCookie = CCefCookie::buildCookieString(_url, _domains, _paths, _names, _values);
        if (_callback)
            _callback(strCookie);
    }
    return true;
}

//////////////////////////////////////////////////
CCookieTask::CCookieTask(const QString& url, const CefCookie& ck) :m_wstrUrl(url), m_cookie(ck)
{
}

void CCookieTask::Execute()
{
    //TRACET();
    CefRefPtr<CefCookieManager> manager = CefCookieManager::GetGlobalManager(NULL);
    //TRACED("this[%08x] manager[%08x]", this, manager.get());
    bool ret = manager->SetCookie(m_wstrUrl.toStdWString().c_str(), m_cookie, NULL);
    //TRACED("%d", ret);
}

//////////////////////////////////////////////////////
void CCefCookie::getCookie(const QString& url, PFNGetCookieCallback cb)
{
    //TRACET();
    CefRefPtr<CefCookieManager> manager = CefCookieManager::GetGlobalManager(NULL);
    CefRefPtr<CefCookieVisitor> visitor = new Visitor(url, cb);
    manager->VisitAllCookies(visitor);
}

bool CCefCookie::setCookie(const QString& url, const QString& cookieData)
{
    //TRACET();
    //RASSERT(NULL != url && NULL != cookieData, false);
    //TRACED(L"url[%s]", url.toStdString().c_str());
    QStringList domains, paths, names, values;
    parseCookieString(cookieData, domains, paths, names, values);
    size_t size = domains.size();
    CefRefPtr<CefCookieManager> manager = CefCookieManager::GetGlobalManager(NULL);
    //TRACED("manager[%08x]", manager.get());
    //RASSERT(NULL != manager, false);

    for (size_t i = 0; i < size; ++i) {
        //TRACED(L"domain[%s] path[%s] name[%s] value[%s]", qPrintable(domains[i]), qPrintable(paths[i]), qPrintable(names[i]), qPrintable(values[i]));
        setCookieItem(url, domains[i], paths[i], names[i], values[i]);
    }
    return true;
}

bool CCefCookie::setCookieItem(const QString& url, const QString& domain, const QString& path, const QString& name, const QString& value)
{
    //TRACET();
    CefCookie ck;
    std::wstring wstrDomain = domain.toLower().toStdWString();
    if (wstrDomain == L"localhost" || wstrDomain == L".localhost")			// 对于localhost需要特殊处理
    {
        wstrDomain = L"";
    }
    std::wstring wPath = path.toStdWString();
    std::wstring wValue = value.toStdWString();
    std::wstring wName = name.toStdWString();

    cef_string_set(wstrDomain.c_str(), wstrDomain.size(), &ck.domain, true);
    cef_string_set(wPath.c_str(), wPath.size(), &ck.path, true);
    cef_string_set(wValue.c_str(), wValue.size(), &ck.value, true);
    cef_string_set(wName.c_str(), wName.size(), &ck.name, true);

    CefPostTask(TID_IO, new CCookieTask(url, ck));
    return true;
}

void CCefCookie::parseCookieString(const QString& cookieData, QStringList& domains, QStringList& paths, QStringList& names, QStringList& values)
{
    //TRACET();
    //TRACED("data[%s]", qPrintable(cookieData));
    QStringList cookies = cookieData.split("\t");
    size_t size = cookies.size();
    domains.clear(); domains.reserve(size);
    paths.clear(); paths.reserve(size);
    names.clear(); names.reserve(size);
    values.clear(); values.reserve(size);
    for (size_t i = 0; i < size; ++i) {
        //TRACED(L"%s", qPrintable(cookies[i]));
        QStringList items = cookies[i].split(";");
        QString wstrValue, wstrDomain, wstrPath, wstrName;
        for (int k = 0; k < items.size(); ++k) {
            QStringList ts = items[k].split("=");
            if (ts.size() < 1) continue;
            ts[0] = ts[0].trimmed();
            QString wstr;
            if (ts.size() >= 2) {
                ts[1] = ts[1].trimmed();
                wstr = ts[1];
            }
            QString strKey = ts[0].toLower();
            if (strKey == "domain") wstrDomain = wstr;
            else if (strKey == "path") wstrPath = wstr;
            else wstrName = ts[0], wstrValue = wstr;
        }
        //TRACED(L"value[%s],domain[%s],path[%s]", qPrintable(wstrValue), qPrintable(wstrDomain), qPrintable(wstrPath));
        names.push_back(wstrName);
        values.push_back(wstrValue);
        domains.push_back(wstrDomain);
        paths.push_back(wstrPath);
    }
}

QString CCefCookie::buildCookieString(const QString& url, const QStringList& domains, const QStringList& paths, const QStringList& names, const QStringList& values)
{
    //TRACET();
    size_t size = domains.size();
    //RASSERT(size == paths.size() && size == names.size() && size == values.size(), "");
    QString dest = url;
    if (dest.startsWith("http://"))
        dest = dest.mid(7);
    else if (dest.startsWith("https://"))
        dest = dest.mid(8);
    //TRACED(L"dest[%s]", qPrintable(dest));
    QStringList cookies; cookies.reserve(size);
    for (size_t i = 0; i < size; ++i) {
        QString cookie = QString("domain=%1;path=%2;%3=%4;").arg(domains[i]).arg(paths[i]).arg(names[i]).arg(values[i]);
        //TRACED(L"count[%d] cookie[%s]", i, qPrintable(cookie));
        if (!dest.isEmpty() && dest != domains[i])
            continue;
        cookies.push_back(cookie);
    }
    QString data = cookies.join("\t");
    //TRACED(L"data[%s]", qPrintable(data));
    return data;
}