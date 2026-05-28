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

string(TIMESTAMP PROJECT_DATE "%Y-%m-%d" UTC)
set(CPACK_PACKAGE_NAME ${PROJECT_SHORTNAME})
set(CPACK_PACKAGE_VENDOR ${PROJECT_COMPANY})
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_RESOURCE_FILE_LICENSE ${PROJECT_SOURCE_DIR}/LICENSE)
# Remove before release
set(CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME})
if(WIN32)
    set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
    set(CPACK_GENERATOR ZIP)
    if(CMAKE_VERSION VERSION_GREATER_EQUAL 4.3)
        set(CPACK_COMPRESSION_LEVEL 9)
    endif()
    set(CPACK_BIN_DIR ".")
    install(TARGETS ${PROJECT_SHORTNAME} RUNTIME DESTINATION .)
    install(FILES LICENSE DESTINATION . RENAME license.txt)
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/models" DESTINATION .)
else()
    file(GENERATE
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/appimage_generator.cmake"
        CONTENT [[
            find_program(LINUXDEPLOY_EXECUTABLE
                NAMES linuxdeploy linuxdeploy-x86_64.AppImage
                PATHS ${CPACK_PACKAGE_DIRECTORY}/linuxdeploy)

            if(NOT LINUXDEPLOY_EXECUTABLE)
                set(LINUXDEPLOY_EXECUTABLE
                    ${CPACK_PACKAGE_DIRECTORY}/linuxdeploy/linuxdeploy)
                file(DOWNLOAD
                    "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage"
                    ${LINUXDEPLOY_EXECUTABLE}
                    INACTIVITY_TIMEOUT 10)
                execute_process(COMMAND chmod +x ${LINUXDEPLOY_EXECUTABLE} COMMAND_ECHO STDOUT)
            endif()

            execute_process(COMMAND
                    ${CMAKE_COMMAND} -E env LDAI_OUTPUT=${CPACK_PACKAGE_FILE_NAME}.AppImage
                    LINUXDEPLOY_OUTPUT_VERSION=${CPACK_PACKAGE_VERSION} NO_STRIP=1
                    ${LINUXDEPLOY_EXECUTABLE} --appimage-extract-and-run
                    --appdir=${CPACK_TEMPORARY_DIRECTORY}
                    --executable=${CPACK_TEMPORARY_DIRECTORY}/${CPACK_PACKAGING_INSTALL_PREFIX}/${CPACK_BIN_DIR}/3dco
                    --desktop-file=${CPACK_TEMPORARY_DIRECTORY}/${CPACK_PACKAGING_INSTALL_PREFIX}/share/applications/3dco.desktop
                    --icon-file=${CPACK_TEMPORARY_DIRECTORY}/${CPACK_PACKAGING_INSTALL_PREFIX}/share/icons/hicolor/128x128/apps/3dco.png
                    --output=appimage)
        ]]
    )
    set(CPACK_GENERATOR External)
    set(CPACK_EXTERNAL_ENABLE_STAGING ON)
    set(CPACK_EXTERNAL_PACKAGE_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/appimage_generator.cmake")
    set(CPACK_PACKAGING_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")
    include(GNUInstallDirs)
    set(CPACK_BIN_DIR "${CMAKE_INSTALL_BINDIR}")
    install(TARGETS ${PROJECT_SHORTNAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    install(FILES "LICENSE" DESTINATION "share/doc/${PROJECT_SHORTNAME}")
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/models" DESTINATION "share/${PROJECT_SHORTNAME}")
    install(FILES res/3dco.desktop DESTINATION share/applications)
    install(FILES res/3dco.png DESTINATION share/icons/hicolor/128x128/apps)
endif()
include(CPack)
