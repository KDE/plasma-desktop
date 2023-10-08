# SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

find_package(PlasmaWaylandProtocols 1.6 REQUIRED)
find_package(Threads REQUIRED)

set(inputsynthlib_SRCS
    inputsynth.cpp
)

add_library(inputsynth SHARED ${inputsynthlib_SRCS})
set_property(TARGET inputsynth PROPERTY POSITION_INDEPENDENT_CODE ON) # Required for ctypes
generate_export_header(inputsynth)

qt6_generate_wayland_protocol_client_sources(inputsynth FILES ${PLASMA_WAYLAND_PROTOCOLS_DIR}/fake-input.xml)

target_link_libraries(inputsynth
    PRIVATE
        Threads::Threads
        Qt::Core
        Qt::Gui # QGuiApplication
        Qt::WaylandClient # Fake input protocol
        Wayland::Client
)

# Do not really install the library
if(DEFINED ENV{KDECI_BUILD})
install(TARGETS inputsynth ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
else()
install(TARGETS inputsynth DESTINATION ${CMAKE_BINARY_DIR})
endif()
