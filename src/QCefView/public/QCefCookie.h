#pragma once;
#include "QCefSetting.h"
#include <QStringList>
#include <QUrl>
#include <functional>
#include "qcefview_export.h"

typedef void(*PFNGetCookieCallback)(const QString& strCookie);

class QCEFVIEW_EXPORT CCefCookie {
public:
    /**
     * @brief 获取cookie
     * @author Alex.peng
     */
    static void getCookie(const QString& url, PFNGetCookieCallback cb);
    /**
     * @brief 设置cookie
     * @author Alex.peng
     */
    static bool setCookie(const QString& url, const QString& cookieData);
    /**
     * @brief 设置cookie项
     * @author Alex.peng
     */
    static bool setCookieItem(const QString& url, const QString& domain, const QString& path, const QString& name, const QString& value);
    /**
     * @brief 构建cookie字符串
     * @author Alex.peng
     */
    static QString buildCookieString(const QString& url, const QStringList& domains, const QStringList& paths, const QStringList& names, const QStringList& values);
private:
    static void parseCookieString(const QString& cookieData, QStringList& domains, QStringList& paths, QStringList& names, QStringList& values);
};