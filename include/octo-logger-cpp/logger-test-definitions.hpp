#ifndef LOGGER_TEST_DEFINITIONS_HPP_
#define LOGGER_TEST_DEFINITIONS_HPP_

// Note: If you use any of these definitions remember to ***add the source file to the tests' CMake target***.
#ifdef UNIT_TESTS

#define TESTS_VIRTUAL virtual

#define TESTS_MOCK_CLASS(class_name)                                                                                   \
  public:                                                                                                              \
    class class_name##Mock;

#else

#define TESTS_VIRTUAL

#define TESTS_MOCK_CLASS(class_name)

#endif

#endif // LOGGER_TEST_DEFINITIONS_HPP_
