SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Tools/src/ImageConv/ImageConv.cpp
)

SET(EXTRA_INCLUDE_DIRS ${EXTRA_INCLUDE_DIRS}
	${KLAYGE_PROJECT_DIR}/Tools/Include)

SET(EXTRA_LINKED_DIRS ${EXTRA_LINKED_DIRS}
	${KLAYGE_TOOLS_LIB_OUTPUT_DIR})

SET(EXTRA_LINKED_LIBRARIES ${EXTRA_LINKED_LIBRARIES}
	debug ToolCommon${KLAYGE_OUTPUT_SUFFIX}${CMAKE_DEBUG_POSTFIX} optimized ToolCommon${KLAYGE_OUTPUT_SUFFIX}
	${KLAYGE_FILESYSTEM_LIBRARY})

SETUP_TOOL(ImageConv)
