project(Browser)

include(GenerateExportHeader)
set_property(GLOBAL PROPERTY AUTOGEN_SOURCE_GROUP "Generated Files")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
find_package(Qt5 COMPONENTS Core Widgets REQUIRED)

file(GLOB_RECURSE _HEADER_FILES
	"*.h"
)
source_group("Header Files" FILES ${_HEADER_FILES})

file(GLOB_RECURSE _SOURCE_FILES
	"*.cpp"
	"*.cc"
)
source_group("Source Files" FILES ${_SOURCE_FILES})


add_executable(${PROJECT_NAME} WIN32
	${_SOURCE_FILES}
	${_HEADER_FILES}
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
	UNICODE
	_UNICODE
)

target_link_libraries(${PROJECT_NAME}
)

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER test)
