#ifndef QCEFCOMMON_H
#define QCEFCOMMON_H
#if defined(_MSVC_) && _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


/**
 * @brief 自定义scheme
 * @author Alex.peng
 */
#define QCEF_SCHEMA "qcef"

/**
 * @brief JS绑定的根对象名
 * @author Alex.peng
 */
#define QCEF_OBJECT_NAME "nts"

/**
 * @brief 执行JS调用的消息名
 * @author Alex.peng
 */
#define QCEF_INVOKENGLMETHOD "invokeNglMethod"

/**
 * @brief 执行回调函数的消息名
 * @author Alex.peng
 */
#define QCEF_INVOKENGLCALLBACK "invokeNglCallback"

/**
 * @brief 读取属性值
 * @author Alex.peng
 */
#define QCEF_RETRIEVEPROPERTY   "retrieveProperty"

/**
 * @brief 设置属性值
 * @author Alex.peng
 */
#define QCEF_SETPROPERTY        "setProperty"


/**
 * @brief 清理回调函数的消息名
 * @author Alex.peng
 */
#define QCEF_CLEARNGLCALLBACKS "clearNglCallbacks"

/**
 * @brief 通知浏览器创建消息名
 * @author Alex.peng
 */
#define QCEF_NOTIFYBROWSERCREATED   "notifyBrowserCreated"

/**
 * @brief 浏览器进程文件名
 * @author Alex.peng
 */
#define RENDER_PROCESS_NAME "Browser.exe"


/**
 * @brief 资源文件夹
 * @author Alex.peng
 */
#define RESOURCE_DIRECTORY_NAME "resources"

/**
 * @brief 本地化资源文件夹
 * @author Alex.peng
 */
#define LOCALES_DIRECTORY_NAME "locales"

/**
 * @brief 缓存文件夹
 * @author Alex.peng
 */
#define CACHE_DIRECTORY_NAME "cache2"

/**
 * @brief User-Agent
 * @author Alex.peng
 */
#define QCEF_USER_AGENT "NTSEngine/1.0 Mozilla/5.0 (Windows 986.2) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/72.0.3626 Safari/537.36"

/**
 * @brief JS绑定所需的共享内存名
 * @author Alex.peng
 */
#define QCEF_JSBRIDGE_NAME "__NGL_JSBINDING_SHARED_%1__"

/**
 * @brief render进程启动参数选项名：日志等级
 * @author Alex.peng
 */
#define QCEF_LOGLEVEL_OPTION_NAME    "log-level"

/**
 * @brief render进程启动参数选项名：父进程Id
 * @author Alex.peng
 */
#define QCEF_PARENT_ID_NAME           "qcef-parent-id"


//HtmlSurface添加的消息名
#define QCEF_GET_LINK_AT_POSITION       "getLinkAtPosition"
#define QCEF_SET_HORIZONTAL_SCROLL      "setHorizontalScroll"
#define QCEF_SET_VERTICAL_SCROLL        "setVerticalScroll"
//返回值管道缓冲区长度
#define QCEF_RET_PIPE_BUFFER_LEN        2048

//HtmlSurface传递的参数
#define QCEF_SURFACE_TYPE_NAME          "surfaceType"
#define QCEF_SURFACE_TYPE_VALUE_OVERLAY "Overlay"
#define QCEF_SURFACE_TYPE_VALUE_SDK     "SDK"

#endif
