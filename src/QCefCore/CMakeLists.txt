cmake_minimum_required(VERSION 3.4.1)
project(${QCEF_CORE_DLL})
INCLUDE(GenerateExportHeader)


set_property(GLOBAL PROPERTY AUTOGEN_SOURCE_GROUP "Generated Files")
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
find_package(Qt5 COMPONENTS Core GUI Widgets Sql REQUIRED)

SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${QCEF_VIEW_SDK_LIB_OUT})
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${QCEF_VIEW_SDK_LIB_OUT})

include_directories(
  # ${CMAKE_SOURCE_DIR}/third_party/npl-shm/
  ${CMAKE_SOURCE_DIR}
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

QT5_WRAP_CPP(MOC_SOURCES ${WRAP_MOC_SOURCES})

if (OS_WINDOWS)
  file(GLOB_RECURSE _RES_FILES
    "*.rc"
  )
  source_group("Resource Files" ${_RES_FILES})
endif()


add_library(${PROJECT_NAME} SHARED 
  ${_SRC_FILES}
  ${_HEADER_FILES}
  ${_RES_FILES}
  ${WRAP_MOC_SOURCES}
)
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
    ${CEF_DLL_WRAPPER}
    ${CEF_LIB_FILE}
    # NglBase
    Opengl32
	# npl-shm
)
GENERATE_EXPORT_HEADER(${PROJECT_NAME})
file(GLOB _EXPORT_HEADERS_${PROJECT_NAME} "public/*")

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory ${QCEF_VIEW_SDK_INC_OUT}
  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${_EXPORT_HEADERS_${PROJECT_NAME}} ${QCEF_VIEW_SDK_INC_OUT}
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/include
  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_BINARY_DIR}/qcefcore_export.h ${CMAKE_SOURCE_DIR}/include
)