plasma_install_package(activitypager org.kde.plasma.activitypager)

add_subdirectory(kicker)
add_subdirectory(kickoff)
add_subdirectory(trash)

add_subdirectory(taskmanager)
add_subdirectory(window-list)
plasma_install_package(icontasks org.kde.plasma.icontasks)
plasma_install_package(margins-separator org.kde.plasma.marginsseparator)

if(PlasmaActivities_FOUND)
   add_subdirectory(pager)
   add_subdirectory(showActivityManager)
endif()

add_subdirectory(minimizeall)
add_subdirectory(showdesktop)
add_subdirectory(kimpanel)
plasma_install_package(keyboardlayout org.kde.plasma.keyboardlayout)
