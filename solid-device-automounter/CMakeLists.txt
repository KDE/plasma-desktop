project(device_automounter)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib)

set(device_automounter_lib_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/lib/AutomounterSettings.cpp)

add_subdirectory(kcm)
add_subdirectory(kded)
