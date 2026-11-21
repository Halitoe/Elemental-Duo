cmake_minimum_required(VERSION 3.24)
project(ElementalDuo VERSION 1.0 LANGUAGES CXX)

include(CTest)
add_library(elemental-logic src/Game.cpp src/Levels.cpp)
target_include_directories(elemental-logic PUBLIC src)
target_compile_features(elemental-logic PUBLIC cxx_std_17)
if(MSVC)
    add_compile_options(/utf-8)
    target_compile_options(elemental-logic PRIVATE /utf-8)
endif()

option(ELEMENTAL_BUILD_GAME "Build the SFML game window" ON)
if(ELEMENTAL_BUILD_GAME)
    include(FetchContent)
    set(BUILD_SHARED_LIBS OFF)
    set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)
    set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(SFML_BUILD_TEST_SUITE OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(SFML
        URL https://codeload.github.com/SFML/SFML/tar.gz/refs/tags/3.0.2
        URL_HASH SHA256=0034e05f95509e5d3fb81b1625713e06da7b068f210288ce3fd67106f8f46995
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(SFML)
    add_executable(ElementalDuo src/main.cpp src/Presentation.cpp)
    target_link_libraries(ElementalDuo PRIVATE elemental-logic SFML::Graphics SFML::Audio)
    add_custom_command(TARGET ElementalDuo POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_directory
                "${PROJECT_SOURCE_DIR}/assets" "$<TARGET_FILE_DIR:ElementalDuo>/assets"
        VERBATIM
    )
    target_compile_definitions(ElementalDuo PRIVATE ELEMENTAL_ASSET_PATH="${PROJECT_SOURCE_DIR}/assets")
endif()

if(BUILD_TESTING)
    add_executable(ElementalTests tests/Playthrough.cpp)
    target_link_libraries(ElementalTests PRIVATE elemental-logic)
    add_test(NAME Gameplay COMMAND ElementalTests)
endif()
