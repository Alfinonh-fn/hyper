# Global resource file to embed static data into code, available as static lib 'resources'. File names needs to be unique in the /resources directory and below.
# All files are available with their file name by calling ":/FILENAME". Probably you need to call Q_INIT_RESOURCE("resources") once
#
# Define the current source locations
SET(CURRENT_SOURCE_DIR ${CMAKE_SOURCE_DIR}/resources)

# catch all files
FILE ( GLOB Hyperhdr_RESFILES "${CURRENT_SOURCE_DIR}/icons/*" "${CURRENT_SOURCE_DIR}/ssl/*" )

# fill resources.qrc with RESFILES
FOREACH( f ${Hyperhdr_RESFILES} )
	get_filename_component(fname ${f} NAME)
	SET(HYPERHDR_RES "${HYPERHDR_RES}\n\t\t<file alias=\"${fname}\">${f}</file>")
ENDFOREACH()

# prep file
CONFIGURE_FILE(${CURRENT_SOURCE_DIR}/resources.qrc.in ${CMAKE_BINARY_DIR}/resources.qrc )
SET(Hyperhdr_RES ${CMAKE_BINARY_DIR}/resources.qrc)

add_library(resources
	${Hyperhdr_RES}
)
