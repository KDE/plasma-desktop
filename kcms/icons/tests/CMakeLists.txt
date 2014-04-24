set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. )

set(testicons_SRCS testicons.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../icons.cpp )

add_executable(testicons ${testicons_SRCS})
ecm_mark_as_test(testicons)

target_link_libraries(testicons
    Qt5::Widgets
    KF5::KCMUtils
    KF5::I18n
    KF5::KDELibs4Support
    ${X11_LIBRARIES}
)
