# Define the current source locations
SET(CURRENT_HEADER_DIR ${CMAKE_SOURCE_DIR}/include/grabber)
SET(CURRENT_SOURCE_DIR ${CMAKE_SOURCE_DIR}/sources/grabber/MF)

FILE ( GLOB WMF_SOURCES "${CURRENT_HEADER_DIR}/MF*.h"  "${CURRENT_SOURCE_DIR}/*.h"  "${CURRENT_SOURCE_DIR}/*.cpp" )

add_library(MF-grabber ${WMF_SOURCES} )

target_link_libraries(MF-grabber
	hyperhdr-base
	${QT_LIBRARIES}
)

target_include_directories(MF-grabber PUBLIC ${TURBOJPEG_INCLUDE_DIRS})
target_link_libraries(MF-grabber ${TURBOJPEG_LINK_LIBRARIES})
