# 
# The main config file for QCefWebEngine
# NOTE:
#   Usually there is no need to change this file.
#   Change the config.cmake instead.
#
cmake_minimum_required(VERSION 3.12.5)
project(QCefWebEngine)

# Set SDK directories
set(CEF_SDK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third/cef_binary")
set(QT_SDK_DIR $ENV{QT_DIR})

# Determine the platform.
if("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
  set(OS_MACOSX 1)
  set(OS_POSIX 1)
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
  set(OS_LINUX 1)
  set(OS_POSIX 1)
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
  set(OS_WINDOWS 1)
endif()

# Determine the project architecture.
if(NOT DEFINED PROJECT_ARCH)
  if(CMAKE_SIZEOF_VOID_P MATCHES 8)
    set(PROJECT_ARCH "x86_64")
  else()
    set(PROJECT_ARCH "x86")
  endif()

  if(OS_MACOSX)
    # PROJECT_ARCH should be specified on Mac OS X.
    message(WARNING "No PROJECT_ARCH value specified, using ${PROJECT_ARCH}")
  endif()
endif()
# 设置vs属性，强制utf8编码 release版本生成pdb
if(OS_WINDOWS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /source-charset:utf-8")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
endif()

# Set common configurations
###############################################################
# Use solution folder
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_CXX_STANDARD 17)
set(CXX_STANDARD_REQUIRED)
set(CMAKE_CONFIGURATION_TYPES Debug Release)
set(QCEF_VIEW_SDK_OUT ${CMAKE_SOURCE_DIR}/bin/${PROJECT_ARCH})
set(QCEF_VIEW_SDK_BIN_OUT ${CMAKE_SOURCE_DIR}/bin/${PROJECT_ARCH})
set(QCEF_VIEW_SDK_LIB_OUT ${CMAKE_SOURCE_DIR}/bin/${PROJECT_ARCH})
set(QCEF_VIEW_SDK_INC_OUT ${CMAKE_SOURCE_DIR}/include)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${QCEF_VIEW_SDK_BIN_OUT})
###############################################################

# Append the QT dir to CMAKE_PREFIX_PATH
###############################################################
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${QT_SDK_DIR})
###############################################################


# Set CEF root dir and append it to CMAKE_MODULE_PATH
###############################################################
set(CEF_ROOT "${CEF_SDK_DIR}")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CEF_ROOT}/cmake")
###############################################################


# Config the CEF
###############################################################

# Set the runtime library type
set(CEF_RUNTIME_LIBRARY_FLAG "/MD" CACHE STRING "Use dynamic runtime")

# Disable the SANDBOX
set(USE_SANDBOX OFF CACHE BOOL "Disable the Sandbox")

# Find the CEF package
find_package(CEF REQUIRED)

# Store the libcef.lib path and cef dll wrapper target name
# NOTE: we set this just for simplify the following referring.
set(CEF_LIB_FILE "${CEF_BINARY_DIR}/libcef.lib")
set(CEF_DLL_WRAPPER "libcef_dll_wrapper")

# Add build target 
add_subdirectory(${CEF_LIBCEF_DLL_WRAPPER_PATH} ${CEF_LIBCEF_DLL_WRAPPER_PATH})
PRINT_CEF_CONFIG()
###############################################################

# Add all targets
###############################################################

include_directories(
  QCefProto
  ${CEF_ROOT}
)

set(QCEF_WEBENGINE_DLL "QCefWebEngine")
set(QCEF_BROWSER_DLL "QCefBrowser")
set(QCEF_VIEW_DLL "QCefView")
set(QCEF_OVERLAY_DLL "QCefOffscreen")
set(QCEF_BROWSER_EXE "QCefBrowserProcess")

set(QCEF_FOLDER_NAME "QCefWebEngine")

add_subdirectory(src/QCefWebEngine)
add_subdirectory(src/QCefView)
add_subdirectory(src/QCefOffscreen)
add_subdirectory(src/QCefBrowserProcess)
add_subdirectory(src/QCefProto)


###test
add_subdirectory(test/App)

###############################################################

# set folder for libcef_dll_wrapper
set_target_properties(libcef_dll_wrapper PROPERTIES FOLDER third_party)
