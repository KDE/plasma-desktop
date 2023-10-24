# SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

find_package(Python 3.11 COMPONENTS Interpreter Development)
if (NOT Python_Development_FOUND)
    return()
endif()
find_package(PlasmaWaylandProtocols 1.6 REQUIRED)
find_package(Threads REQUIRED)

add_library(inputsynth SHARED inputsynth.cpp)
set_target_properties(inputsynth PROPERTIES
    PREFIX ""
    OUTPUT_NAME "inputsynth_plasma_desktop"
    POSITION_INDEPENDENT_CODE ON
)

qt6_generate_wayland_protocol_client_sources(inputsynth FILES ${PLASMA_WAYLAND_PROTOCOLS_DIR}/fake-input.xml)

target_link_libraries(inputsynth
    PRIVATE
        Threads::Threads
        Qt::Core
        Qt::Gui # QGuiApplication
        Qt::WaylandClient # Fake input protocol
        Wayland::Client
        Python::Python
)

if(DEFINED ENV{KDECI_BUILD})
    set(SETUPTOOLS_INSTALL_FLAG --user)
else()
    set(SETUPTOOLS_INSTALL_FLAG --prefix ${CMAKE_INSTALL_PREFIX})
endif()
add_custom_command(TARGET inputsynth
    POST_BUILD
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/setup.py install ${SETUPTOOLS_INSTALL_FLAG}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
