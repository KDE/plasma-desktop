find_package(LibAttica 0.1.4)
set_package_properties(LibAttica PROPERTIES DESCRIPTION "A library to access Open Collaboration Service providers"
                       URL "https://projects.kde.org/attica"
                       TYPE REQUIRED
                       PURPOSE "Attica is needed for the Get Hot New Stuff support"
                      )

include_directories(${LIBATTICA_INCLUDE_DIR})

add_subdirectory(kdeplugin)
add_subdirectory(kcm)

