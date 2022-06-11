/* KDE's configuration directory */
#define KDE_CONFDIR "${KDE_INSTALL_CONFDIR}"

/* xkb resources directory */
#cmakedefine XKBDIR "${XKBDIR}"

/* Number of bits in a file offset, on hosts where this is settable. */
#define _FILE_OFFSET_BITS 64

/* Define to 1 if you have packagekit available. */
#cmakedefine HAVE_PACKAGEKIT 1

/*
    On HP-UX, the declaration of vsnprintf() is needed every time !
*/

#define WORKSPACE_VERSION_STRING "${PROJECT_VERSION}"
