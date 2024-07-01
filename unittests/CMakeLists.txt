FIND_PACKAGE(Catch2 REQUIRED CONFIG)
FIND_PACKAGE(trompeloeil REQUIRED CONFIG)

SET(UT_EXECUTABLE ${PROJECT_NAME}-tests)

# UT definition
ADD_EXECUTABLE(${UT_EXECUTABLE}
    ${PROJECT_SOURCE_DIR}/src/logger.cpp
    ${PROJECT_SOURCE_DIR}/src/manager-config.cpp
    ${PROJECT_SOURCE_DIR}/src/manager.cpp
    src/log-tests.cpp
    src/logger-tests.cpp
    src/logging-tests.cpp
    $<$<BOOL:${WITH_AWS}>:${PROJECT_SOURCE_DIR}/src/aws/cloudwatch-sink.cpp>
    $<$<BOOL:${WITH_AWS}>:src/cloudwatch-sink-tests.cpp>
    src/test.cpp
)

ADD_DEPENDENCIES(${UT_EXECUTABLE} ${PROJECT_NAME})

# Properties
SET_TARGET_PROPERTIES(${UT_EXECUTABLE} PROPERTIES CXX_STANDARD 17 POSITION_INDEPENDENT_CODE ON)

TARGET_COMPILE_DEFINITIONS(${UT_EXECUTABLE}
    PRIVATE
        UNIT_TESTS
)

TARGET_INCLUDE_DIRECTORIES(${UT_EXECUTABLE}
    PUBLIC
    # Unit tests includes
    ${CMAKE_CURRENT_LIST_DIR}/include

    # Project includes
    ${PROJECT_SOURCE_DIR}/include
)

TARGET_LINK_LIBRARIES(${UT_EXECUTABLE}
    ${PROJECT_NAME}

    # 3rdParties
    Catch2::Catch2
    trompeloeil::trompeloeil
)

# Discover tests
INCLUDE(CTest)
INCLUDE(Catch)
CATCH_DISCOVER_TESTS(${UT_EXECUTABLE})
