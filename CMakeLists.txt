CMAKE_MINIMUM_REQUIRED(VERSION 3.16)

PROJECT(octo-logger-cpp)

# Add CMake options to path
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/)
INCLUDE(ConfigOptions)

SET (JSON_ENABLED OFF)
IF (WITH_AWS OR WITH_JSON_FORMATTING)
    SET (JSON_ENABLED ON)
ENDIF()

FIND_PACKAGE(fmt REQUIRED CONFIG)
IF (JSON_ENABLED)
    FIND_PACKAGE(nlohmann_json REQUIRED CONFIG COMPONENTS)
ENDIF()
IF (WITH_AWS)
    FIND_PACKAGE(AWSSDK REQUIRED CONFIG COMPONENTS logs)
ENDIF()

# Library definition
ADD_LIBRARY(octo-logger-cpp STATIC
    src/channel.cpp
    src/context-info.cpp
    src/log.cpp
    src/logger.cpp
    src/fork-safe-mutex.cpp
    src/manager-config.cpp
    src/manager.cpp
    src/sink-config.cpp
    src/sink-factory.cpp
    src/sink.cpp
    src/sinks/console-sink.cpp
    src/sinks/file-sink.cpp
    $<$<NOT:$<PLATFORM_ID:Windows>>:src/sinks/syslog-sink.cpp>
    $<$<BOOL:${WITH_AWS}>:src/aws/cloudwatch-sink.cpp>
    $<$<BOOL:${WITH_AWS}>:src/aws/aws-log-system.cpp>
)

# Alias
ADD_LIBRARY(core::octo-logger-cpp ALIAS octo-logger-cpp)

# Properties
SET_TARGET_PROPERTIES(octo-logger-cpp PROPERTIES CXX_STANDARD 17 POSITION_INDEPENDENT_CODE ON)

TARGET_COMPILE_OPTIONS(octo-logger-cpp
    PRIVATE
        $<$<NOT:$<PLATFORM_ID:Windows>>:-Werror=return-type>
        $<$<NOT:$<PLATFORM_ID:Windows>>:-Werror=switch>
)

TARGET_INCLUDE_DIRECTORIES(octo-logger-cpp
    PUBLIC
        # Logger includes
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

TARGET_COMPILE_DEFINITIONS(octo-logger-cpp
    PUBLIC
        $<$<BOOL:${WITH_AWS}>:OCTO_LOGGER_WITH_AWS>
        $<$<BOOL:${JSON_ENABLED}>:OCTO_LOGGER_WITH_JSON_FORMATTING>
)

TARGET_LINK_LIBRARIES(octo-logger-cpp
    fmt::fmt
    $<$<BOOL:${JSON_ENABLED}>:nlohmann_json::nlohmann_json>
    $<$<BOOL:${WITH_AWS}>:AWS::aws-sdk-cpp-logs>
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
IF (NOT DISABLE_TESTS AND NOT WIN32)
    ENABLE_TESTING()
    ADD_SUBDIRECTORY(unittests)
ENDIF ()
