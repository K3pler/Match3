cmake_minimum_required(VERSION 3.14)
project(Project)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

include(FetchContent)

# SFML
set(SFML_BUILD_AUDIO OFF)
set(SFML_BUILD_NETWORK OFF)

FetchContent_Declare(
        sfml
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 2.6.1
)
FetchContent_MakeAvailable(sfml)

# SQLite3
add_executable(Project main.cpp sqlite3.c)


target_link_libraries(Project PRIVATE
        sfml-graphics
        sfml-window
        sfml-system
)

# Копирование шрифта
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/arial.otf)
    add_custom_command(TARGET Project POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/arial.otf
            $<TARGET_FILE_DIR:Project>
    )
endif()

# Копирование DLL SFML
if(WIN32)
    add_custom_command(TARGET Project POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "$<TARGET_RUNTIME_DLLS:Project>"
    $<TARGET_FILE_DIR:Project>
    COMMAND_EXPAND_LISTS
    )
endif ()