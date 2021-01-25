project(${QCEF_OVERLAY_DLL})

include(GenerateExportHeader)
set_property(GLOBAL PROPERTY AUTOGEN_SOURCE_GROUP "Generated Files")
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
find_package(Qt5 COMPONENTS Core REQUIRED)

file(GLOB_RECURSE _HEADER_FILES
	"*.h"
)
source_group("Header Files" FILES ${_HEADER_FILES})

file(GLOB_RECURSE _SOURCE_FILES
	"*.cpp"
	"*.cc"
)
source_group("Source Files" FILES ${_SOURCE_FILES})


add_library(${PROJECT_NAME} SHARED
	${_SOURCE_FILES}
	${_HEADER_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER QCefWebEngine)
