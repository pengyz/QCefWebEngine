#ifndef QCEFSETTINGS_H
#define QCEFSETTINGS_H
#pragma once

#pragma region qt_headers
#include <QtCore/qglobal.h>
#include <QString>
#include <QColor>
#pragma endregion qt_headers
#include "qcefcore_export.h"

/**
 * @brief CEF设置相关接口
 * @author Alex.peng
 */
namespace QCefSetting {
    /**
     * @brief 设置浏览器进程路径
     * @author Alex.peng
     */
    void QCEFCORE_EXPORT setBrowserSubProcessPath(const QString& path);

    /**
     * @brief 获取浏览器进程路径
     * @author Alex.peng
     */
    const QCEFCORE_EXPORT QString browserSubProcessPath();

    /**
     * @brief 设置资源路径
     * @author Alex.peng
     */
    void QCEFCORE_EXPORT setResourceDirectoryPath(const QString& path);

    /**
     * @brief 获取资源路径
     * @author Alex.peng
     */
    const QCEFCORE_EXPORT QString resourceDirectoryPath();

    /**
     * @brief 设置本地化资源路径
     * @author Alex.peng
     */
    void QCEFCORE_EXPORT setLocalesDirectoryPath(const QString& path);

    /**
     * @brief 获取本地化资源路径
     * @author Alex.peng
     */
    const QCEFCORE_EXPORT QString localesDirectoryPath();

    /**
     * @brief 设置User-Agent
     * @author Alex.peng
     */
    void QCEFCORE_EXPORT setUserAgent(const QString& agent);

    /**
     * @brief 获取User-Agent
     * @author Alex.peng
     */
    const QCEFCORE_EXPORT QString userAgent();

    /**
     * @brief 设置缓存路径
     * @author Alex.peng
     */
    void QCEFCORE_EXPORT setCachePath(const QString& path);

    /**
     * @brief 获取缓存路径
     * @author Alex.peng
     */
    const QCEFCORE_EXPORT QString cachePath();

    /**
     * @brief 设置用户数据路径
     * @author Alex.peng
     */
    void QCEFCORE_EXPORT setUserDataPath(const QString& path);

    /**
     * @brief 获取用数据路径
     * @author Alex.peng
     */
    const QCEFCORE_EXPORT QString userDataPath();

    /**
     * @brief 设置是否启用持久化cookie
     * @author Alex.peng
     */
    void QCEFCORE_EXPORT setPersistSessionCookies(bool enabled);

    /**
     * @brief 获取是否使用持久化cookie
     * @author Alex.peng
     */
    const QCEFCORE_EXPORT bool persistSessionCookies();

    /**
     * @brief 设置是否启用持久化用户设置
     * @author Alex.peng
     */
    void QCEFCORE_EXPORT setPersistUserPreferences(bool enabled);

    /**
     * @brief 获取是否启用用户设置状态
     * @author Alex.peng
     */
    const QCEFCORE_EXPORT bool persistUserPreferences();

    /**
     * @brief 设置Locale
     * @author Alex.peng
     */
    void QCEFCORE_EXPORT setLocale(const QString& locale);

    /**
     * @brief 获取Locale
     * @author Alex.peng
     */
    const QCEFCORE_EXPORT QString locale();

    /**
     * @brief 设置远程调试端口
     * @author Alex.peng
     */
    void QCEFCORE_EXPORT setRemoteDebuggingPort(int port);

    /**
     * @brief 获取远程调试端口
     * @author Alex.peng
     */
    const QCEFCORE_EXPORT int remoteDebuggingPort();

    /**
     * @brief 获取调试控制台宽度
     * @author Alex.peng
     */
    const QCEFCORE_EXPORT int devToolWidth();

    /**
     * @brief 获取调试控制台高度
     * @author Alex.peng
     */
    const QCEFCORE_EXPORT int devToolHeight();

    /**
     * @brief 设置背景色
     * @author Alex.peng
     */
    void QCEFCORE_EXPORT setBackgroundColor(const QColor& color);

    /**
     * @brief 获取背景色
     * @author Alex.peng
     */
    const QCEFCORE_EXPORT QColor backgroundColor();

    /**
     * @brief 设置accept-language-list
     * @author Alex.peng
     */
    void QCEFCORE_EXPORT setAcceptLanguageList(const QString& languages);

    /**
     * @brief 获取accept-language-list
     * @author Alex.peng
     */
    const QCEFCORE_EXPORT QString acceptLanguageList();

    /**
     * @brief 设置日志等级
     * @author Alex.peng
     */
    void QCEFCORE_EXPORT setLogLevel(int logLevel);

    /**
     * @brief 获取日志等级
     * @author Alex.peng
     */
    int QCEFCORE_EXPORT logLevel();

};

#endif
