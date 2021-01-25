cmake_minimum_required(VERSION 3.4.1)
project(${QCEF_BROWSER_DLL})

set_property(GLOBAL PROPERTY AUTOGEN_SOURCE_GROUP "Generated Files")
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
find_package(Qt5 COMPONENTS Core REQUIRED)

set(CMAKE_CXX_STANDARD 11)
set(CXX_STANDARD_REQUIRED)

file(GLOB_RECURSE _SRC_FILES
  "*.cpp"
  "*.h"
)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX Source FILES ${_SRC_FILES})

if (OS_WINDOWS)
  file(GLOB_RECURSE _RES_FILES
    "*.ico"
    "*.rc"
  ) 
  source_group("Resources" ${_RES_FILES})
endif()

add_library(${PROJECT_NAME} SHARED
    ${_SRC_FILES}
    ${_RES_FILES}
)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER QCefWebEngine)

if (OS_WINDOWS)
  target_compile_definitions(${PROJECT_NAME} PRIVATE
    UNICODE
    _UNICODE
  )
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE
  ${CEF_DLL_WRAPPER}
  ${CEF_LIB_FILE}
  Qt5::Core
  NglBase
)
