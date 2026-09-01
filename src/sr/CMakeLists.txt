cmake_minimum_required(VERSION 4.2)
project(SR VERSION 21.3.0 LANGUAGES CXX)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-m32")

# CMake
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(ClangTools)

# CPack
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
set(CPACK_GENERATOR 7Z)
include(CPack)

# Dependencies
find_package(CoD4DM1 CONFIG REQUIRED)
find_package(GLM REQUIRED)
find_package(Libsamplerate REQUIRED)
find_package(Minimp3 REQUIRED)
find_package(speex CONFIG REQUIRED)

# Build
add_subdirectory(src)
