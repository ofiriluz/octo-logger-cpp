# Executable definition
ADD_EXECUTABLE(basic-logger
    src/basic_logger.cpp
)

# Properties
SET_TARGET_PROPERTIES(basic-logger PROPERTIES CXX_STANDARD 17 POSITION_INDEPENDENT_CODE ON)

TARGET_LINK_LIBRARIES(basic-logger
    octo-logger-cpp

    # Static 3rd parties
    $<$<PLATFORM_ID:Linux>:pthread>
)

# Installation of the example
INSTALL(TARGETS basic-logger
    RUNTIME DESTINATION examples
)
