cmake_minimum_required(VERSION 3.10)

project(FinishLine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- Headers ----
set(PROJECT_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/headers")

# ---- Collect sources ----
file(GLOB_RECURSE CORE_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
list(FILTER CORE_SOURCES EXCLUDE REGEX ".*/main\\.cpp$")

# ---- Core library (shared by app + tests) ----
add_library(finishline_core ${CORE_SOURCES})
target_include_directories(finishline_core PUBLIC "${PROJECT_INCLUDE_DIR}")

# ---- Dependencies ----
find_package(CURL REQUIRED)
find_package(LibXml2 REQUIRED)

target_link_libraries(finishline_core PUBLIC
    CURL::libcurl
    LibXml2::LibXml2
)

# ---- App executable ----
add_executable(FinishLine src/main.cpp)
target_link_libraries(FinishLine PRIVATE finishline_core)

# ---- Tests ----
enable_testing()
add_subdirectory(tests)
