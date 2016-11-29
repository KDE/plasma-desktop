find_package(KDED ${KF5_MIN_VERSION} CONFIG REQUIRED)

add_definitions(-DTRANSLATION_DOMAIN=\"kcm_touchpad\")

set(CMAKE_MODULE_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules
    ${CMAKE_MODULE_PATH}
)
set(CMAKE_INCLUDE_CURRENT_DIR true)
set(TOUCHPAD_KCM_VERSION ${PROJECT_VERSION})

add_subdirectory(src)
add_subdirectory(icon)
