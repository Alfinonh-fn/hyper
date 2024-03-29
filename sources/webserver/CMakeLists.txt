# Define the current source locations
set(CURRENT_HEADER_DIR ${CMAKE_SOURCE_DIR}/include/webserver)
set(CURRENT_SOURCE_DIR ${CMAKE_SOURCE_DIR}/sources/webserver)

FILE ( GLOB WebConfig_SOURCES "${CURRENT_HEADER_DIR}/*.h"  "${CURRENT_SOURCE_DIR}/*.h"  "${CURRENT_SOURCE_DIR}/*.cpp" )
FILE ( GLOB_RECURSE webFiles RELATIVE ${CMAKE_BINARY_DIR}  ${CMAKE_SOURCE_DIR}/assets/webconfig/* )

FOREACH( f ${webFiles} )
    STRING ( REPLACE "../assets/webconfig/" "" fname ${f})
    SET(HYPERHDR_WEBCONFIG_RES "${HYPERHDR_WEBCONFIG_RES}\n\t\t<file alias=\"/webconfig/${fname}\">${f}</file>")
ENDFOREACH()
CONFIGURE_FILE(${CURRENT_SOURCE_DIR}/WebConfig.qrc.in ${CMAKE_BINARY_DIR}/WebConfig.qrc )
SET(WebConfig_RESOURCES ${CMAKE_BINARY_DIR}/WebConfig.qrc)

add_library(webserver
	${WebConfig_SOURCES}
	${WebConfig_RESOURCES}
)

target_link_libraries(webserver
	hyperhdr-base
	hyperhdr-utils
	hyperhdr-api
	Qt${Qt_VERSION}::Network
)
