cmake_minimum_required(VERSION 3.10)
project(Project)

set(CMAKE_CXX_STANDARD 17)

# Указываем путь к вручную установленной SFML
set(SFML_DIR "D:/SFML-2.6.1/lib/cmake/SFML")

find_package(SFML 2.6 CONFIG REQUIRED COMPONENTS graphics window system)
find_package(unofficial-sqlite3 CONFIG REQUIRED)

add_executable(Project main.cpp)

target_link_libraries(Project PRIVATE
        sfml-graphics
        sfml-window
        sfml-system
        unofficial::sqlite3::sqlite3
)


# Путь к MinGW из CLion
set(MINGW_BIN_DIR "C:/Program Files/JetBrains/CLion 2026.1/bin/mingw/bin")

# Копируем DLL MinGW
file(GLOB MINGW_DLLS "${MINGW_BIN_DIR}/libstdc++-6.dll" "${MINGW_BIN_DIR}/libgcc_s_seh-1.dll")
foreach(DLL ${MINGW_DLLS})
    add_custom_command(TARGET Project POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${DLL}" $<TARGET_FILE_DIR:Project>)
endforeach()

# Копируем DLL-файлы SFML

set(SFML_BIN_DIR "D:/SFML-2.6.1/bin")
file(GLOB SFML_DLLS "${SFML_BIN_DIR}/*.dll")
foreach(DLL ${SFML_DLLS})
    add_custom_command(TARGET Project POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${DLL}" $<TARGET_FILE_DIR:Project>)
endforeach()
