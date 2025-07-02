# Executable definition
ADD_EXECUTABLE(basic-logger
    src/basic_logger.cpp
)

ADD_EXECUTABLE(sink-options
        src/sink_options.cpp
)

# Properties
SET_TARGET_PROPERTIES(basic-logger sink-options
        PROPERTIES
        CXX_STANDARD 17
        POSITION_INDEPENDENT_CODE ON
)

TARGET_LINK_LIBRARIES(basic-logger
    octo-logger-cpp

    # Static 3rd parties
    $<$<PLATFORM_ID:Linux>:pthread>
)

TARGET_LINK_LIBRARIES(sink-options
    octo-logger-cpp

    # Static 3rd parties
    $<$<PLATFORM_ID:Linux>:pthread>
)

# Installation of the example
INSTALL(TARGETS basic-logger sink-options
    RUNTIME DESTINATION examples
)
