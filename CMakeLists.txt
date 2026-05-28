cmake_minimum_required(VERSION 3.16)

project("3D Controller Overlay" VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

set(PROJECT_STRING "${PROJECT_NAME} ${PROJECT_VERSION}")
set(PROJECT_SHORTNAME "3dco")

add_subdirectory(src)

configure_file(src/config.h.in config.h @ONLY)

target_include_directories(${PROJECT_SHORTNAME} PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/res"
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    ${Stb_INCLUDE_DIR}
)

if(WIN32)
    set(PROJECT_COMPANY "ceski")
    set(PROJECT_COPYRIGHT "Copyright (C) 2026")
    set(PROJECT_FILE_VERSION "${PROJECT_VERSION_MAJOR},${PROJECT_VERSION_MINOR},${PROJECT_VERSION_PATCH},0")
    configure_file(res/version.rc.in version.rc)
    target_sources(${PROJECT_SHORTNAME} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/version.rc")
endif()

find_package(glad CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(SDL3 CONFIG REQUIRED)
find_package(Stb REQUIRED)

target_link_libraries(${PROJECT_SHORTNAME} PRIVATE
    glad::glad
    glfw
    glm::glm
    imgui::imgui
    SDL3::SDL3
)

if(WIN32)
    if(MSVC)
        target_link_options(${PROJECT_SHORTNAME} PRIVATE "$<IF:$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>,/SUBSYSTEM:CONSOLE,/SUBSYSTEM:WINDOWS>")
    else()
        target_link_options(${PROJECT_SHORTNAME} PRIVATE "$<IF:$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>,-mconsole,-mwindows>")
    endif()
endif()

include(CheckIPOSupported)
check_ipo_supported(RESULT result OUTPUT output)
if(result)
    set_target_properties(${PROJECT_SHORTNAME} PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
    message(WARNING "IPO is not supported: ${output}")
endif()
