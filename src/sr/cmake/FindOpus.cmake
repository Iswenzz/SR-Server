find_path(OPUS_INCLUDE_DIR "opus/opus.h")
find_library(OPUS_LIB opus)

set(OPUS_LIBS
	${OPUS_LIB})

set(OPUS_INCLUDE_DIRS
	${OPUS_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Opus DEFAULT_MSG OPUS_LIBS OPUS_INCLUDE_DIRS)
mark_as_advanced(OPUS_LIBS OPUS_INCLUDE_DIRS)

if(NOT TARGET Opus::opus)
	add_library(Opus::opus INTERFACE IMPORTED)
	set_property(TARGET Opus::opus PROPERTY INTERFACE_LINK_LIBRARIES "${OPUS_LIBS}")
	set_property(TARGET Opus::opus PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${OPUS_INCLUDE_DIRS}")
endif()
