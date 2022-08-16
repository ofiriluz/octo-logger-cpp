CMAKE_MINIMUM_REQUIRED(VERSION 3.16)

PROJECT(octo-logger-cpp)

# Add CMake options to path
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/)
INCLUDE(ConfigOptions)
INCLUDE(LoadConanBuildInfo)
LOAD_CONAN_BUILD_INFO()

# Library definition
ADD_LIBRARY(octo-logger-cpp STATIC
    src/channel.cpp
    src/log.cpp
    src/logger.cpp
    src/manager-config.cpp
    src/manager.cpp
    src/sink-config.cpp
    src/sink-factory.cpp
    src/sink.cpp
    src/sinks/console-sink.cpp
    src/sinks/file-sink.cpp
    $<$<PLATFORM_ID:Linux>:src/sinks/syslog-sink.cpp>
)

# Properties
SET_TARGET_PROPERTIES(octo-logger-cpp PROPERTIES CXX_STANDARD 17 POSITION_INDEPENDENT_CODE ON)

TARGET_COMPILE_OPTIONS(octo-logger-cpp
    PRIVATE
        -Werror=return-type
        -Werror=switch
)

TARGET_INCLUDE_DIRECTORIES(octo-logger-cpp
    PUBLIC
        # Logger includes
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

TARGET_LINK_LIBRARIES(octo-logger-cpp
    CONAN_PKG::fmt
)

# Installation of the logger
INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION include
)

INSTALL(TARGETS octo-logger-cpp
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# Examples
IF(NOT DISABLE_EXAMPLES)
    ADD_SUBDIRECTORY(examples)
ENDIF()

# Unittests
IF (NOT DISABLE_TESTS)
    ENABLE_TESTING()
    ADD_SUBDIRECTORY(unittests)
ENDIF ()
