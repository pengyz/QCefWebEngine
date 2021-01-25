project(${QCEF_BROWSER_EXE})

set_property(GLOBAL PROPERTY AUTOGEN_SOURCE_GROUP "Generated Files")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
find_package(Qt5 COMPONENTS Core REQUIRED)

include_directories(
    ${CMAKE_SOURCE_DIR}/src/QCefProto
)

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

add_executable(${PROJECT_NAME} WIN32
    ${_SRC_FILES}
    ${_RES_FILES}
    ${PROJECT_NAME}.manifest
)
target_compile_definitions(${PROJECT_NAME} PRIVATE
	UNICODE
	_UNICODE
)

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER ${QCEF_FOLDER_NAME})

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
)
