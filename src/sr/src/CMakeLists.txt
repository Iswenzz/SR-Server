add_library(SR STATIC)

add_subdirectory(Audio)
add_subdirectory(Commands)
add_subdirectory(Game)
add_subdirectory(System)
add_subdirectory(Utils)

# Build
target_sources(SR PRIVATE
	API.cpp
	API.hpp
	Application.cpp
	Application.hpp
	Base.hpp
	Common.hpp
	Macros.hpp)

target_precompile_headers(SR PUBLIC Base.hpp Common.hpp)
target_link_libraries(SR PUBLIC
	CoD4DM1::CoD4DM1
	glm::glm
	libsamplerate::libsamplerate
	minimp3::minimp3
	speex::speex)

target_include_directories(SR PUBLIC
	.
	../..
	../../version
	../../../plugins)

# Tree
get_target_property(SOURCES SR SOURCES)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

# Install
install(TARGETS SR
	LIBRARY DESTINATION "${CMAKE_SOURCE_DIR}/../../lib"
	ARCHIVE DESTINATION "${CMAKE_SOURCE_DIR}/../../lib")

install(DIRECTORY ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib/
	DESTINATION "${CMAKE_SOURCE_DIR}/../../lib"
	FILES_MATCHING PATTERN "*.a"
	PATTERN "pkgconfig" EXCLUDE)
