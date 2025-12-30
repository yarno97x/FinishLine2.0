# ---- GoogleTest ----
find_package(GTest CONFIG REQUIRED)
find_package(Threads REQUIRED)

# ---- Collect test sources ----
file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/unit/*.cpp"
)

# ---- Tests executable ----
add_executable(FinishLineTests ${TEST_SOURCES})

target_link_libraries(FinishLineTests PRIVATE
    finishline_core
    GTest::gtest
    GTest::gtest_main
    Threads::Threads
)

target_include_directories(FinishLineTests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/helper"
)

# ---- Test discovery ----
include(GoogleTest)
gtest_discover_tests(FinishLineTests)
