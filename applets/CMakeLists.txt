add_subdirectory(kickoff)
plasma_install_package(trash org.kde.plasma.trash)

if(NOT WIN32)
   # tasks and windowlist depend on libs/taskmanager
   add_subdirectory(taskmanager)
   plasma_install_package(window-list org.kde.plasma.windowlist)

if(KF5Activities_FOUND)
   add_subdirectory(pager)
   add_subdirectory(showActivityManager)
endif()
endif()
