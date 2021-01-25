cmake_minimum_required(VERSION 3.4.1)
project(${QCEF_VIEW_DLL})
INCLUDE(GenerateExportHeader)


set_property(GLOBAL PROPERTY AUTOGEN_SOURCE_GROUP "Generated Files")
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
find_package(Qt5 COMPONENTS Core GUI Widgets Sql REQUIRED)

SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${QCEF_VIEW_SDK_LIB_OUT})
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${QCEF_VIEW_SDK_LIB_OUT})

include_directories(
  ${CMAKE_SOURCE_DIR}/
  "${CMAKE_CURRENT_SOURCE_DIR}/Generated Files"
   ${CMAKE_SOURCE_DIR}/third_party/npl-shm/
)

file(GLOB_RECURSE _SRC_FILES
  "*.cpp"
  "*.cc"
)
source_group("Source Files" FILES ${_HEADER_FILES})

file(GLOB_RECURSE _HEADER_FILES
  "*.h"
)
source_group("Header Files" FILES ${_HEADER_FILES})

if (OS_WINDOWS)
  file(GLOB _RES_FILES
    "*.rc"
  )
  source_group(Resource ${_RES_FILES})
endif()

add_library(${PROJECT_NAME} SHARED 
  ${_SRC_FILES}
  ${_HEADER_FILES}
  ${_RES_FILES}
)
GENERATE_EXPORT_HEADER(${PROJECT_NAME})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER QCefWebEngine)


if (OS_WINDOWS)
  target_compile_definitions(${PROJECT_NAME} PRIVATE
    UNICODE
    _UNICODE
    QCEFVIEW_LIB
  )
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Core
    Qt5::Gui 
    Qt5::Widgets
    NglBase
    QCefCore
    ${CEF_DLL_WRAPPER}
    ${CEF_LIB_FILE}
)

# add_dependencies(${PROJECT_NAME} NglBrowser)

file(GLOB _EXPORT_HEADERS_${PROJECT_NAME} "public/*")

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CEF_RESOURCE_DIR} $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources
  COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources/icudtl.dat $<TARGET_FILE_DIR:${PROJECT_NAME}>
  COMMAND ${CMAKE_COMMAND} -E remove $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources/icudtl.dat
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CEF_BINARY_DIR} $<TARGET_FILE_DIR:${PROJECT_NAME}>
  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${_EXPORT_HEADERS_${PROJECT_NAME}} ${QCEF_VIEW_SDK_INC_OUT}
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/include/
  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_BINARY_DIR}/qcefview_export.h ${CMAKE_SOURCE_DIR}/include
)