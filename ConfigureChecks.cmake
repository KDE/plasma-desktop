set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH} )

execute_process(COMMAND ${PKG_CONFIG_EXECUTABLE} --variable=xkb_base xkeyboard-config OUTPUT_VARIABLE XKBDIR OUTPUT_STRIP_TRAILING_WHITESPACE)
if (NOT EXISTS "${XKBDIR}")
    message(FATAL_ERROR "Couldn't find XKB location: .${XKBDIR}.")
endif()
set(KWIN_BIN "kwin_x11" CACHE STRING "Name of the KWin binary")

check_function_exists(getpassphrase HAVE_GETPASSPHRASE)
check_function_exists(vsyslog HAVE_VSYSLOG)
check_function_exists(statvfs HAVE_STATVFS)

check_include_files(limits.h HAVE_LIMITS_H)
check_include_files(sys/time.h HAVE_SYS_TIME_H)     # ksmserver, ksplashml, sftp
check_include_files(stdint.h HAVE_STDINT_H)         # kcontrol/kfontinst
check_include_files("sys/stat.h;sys/vfs.h" HAVE_SYS_VFS_H) # statvfs for plasma/solid
check_include_files("sys/stat.h;sys/statvfs.h" HAVE_SYS_STATVFS_H) # statvfs for plasma/solid
check_include_files(sys/param.h HAVE_SYS_PARAM_H)
check_include_files("sys/param.h;sys/mount.h" HAVE_SYS_MOUNT_H)
check_include_files("sys/types.h;sys/statfs.h" HAVE_SYS_STATFS_H)
check_include_files(unistd.h HAVE_UNISTD_H)
check_include_files(malloc.h HAVE_MALLOC_H)
check_function_exists(statfs HAVE_STATFS)
macro_bool_to_01(OPENGL_FOUND HAVE_OPENGL) # kwin
macro_bool_to_01(X11_XShm_FOUND HAVE_XSHM) # kwin, ksplash
macro_bool_to_01(X11_XTest_FOUND HAVE_XTEST) # khotkeys, kxkb, kdm
macro_bool_to_01(X11_Xcomposite_FOUND HAVE_XCOMPOSITE) # kicker, kwin
macro_bool_to_01(X11_Xcursor_FOUND HAVE_XCURSOR) # many uses
macro_bool_to_01(X11_Xdamage_FOUND HAVE_XDAMAGE) # kwin
macro_bool_to_01(X11_Xfixes_FOUND HAVE_XFIXES) # klipper, kicker, kwin
macro_bool_to_01(X11_Xinerama_FOUND HAVE_XINERAMA)
macro_bool_to_01(X11_Xrandr_FOUND HAVE_XRANDR) # kwin
macro_bool_to_01(X11_Xrender_FOUND HAVE_XRENDER) # kcontrol/style, kicker
macro_bool_to_01(X11_xf86misc_FOUND HAVE_XF86MISC) # kdesktop and kcontrol/lock
macro_bool_to_01(X11_dpms_FOUND HAVE_DPMS) # kdesktop
macro_bool_to_01(X11_XSync_FOUND HAVE_XSYNC) # kwin

set(CMAKE_EXTRA_INCLUDE_FILES sys/socket.h)

check_function_exists(getpeereid  HAVE_GETPEEREID) # kdesu
check_function_exists(setpriority  HAVE_SETPRIORITY) # kscreenlocker 

set(CMAKE_REQUIRED_INCLUDES ${X11_Xrandr_INCLUDE_PATH}/Xrandr.h)
set(CMAKE_REQUIRED_LIBRARIES ${X11_Xrandr_LIB})
check_function_exists(XRRGetScreenSizeRange XRANDR_1_2_FOUND)
macro_bool_to_01(XRANDR_1_2_FOUND HAS_RANDR_1_2)
check_function_exists(XRRGetScreenResourcesCurrent XRANDR_1_3_FOUND)
macro_bool_to_01(XRANDR_1_3_FOUND HAS_RANDR_1_3)
