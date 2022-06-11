/* KDE's configuration directory */
#define KDE_CONFDIR "${KDE_INSTALL_CONFDIR}"

/* xkb resources directory */
#cmakedefine XKBDIR "${XKBDIR}"

/* Number of bits in a file offset, on hosts where this is settable. */
#define _FILE_OFFSET_BITS 64

#cmakedefine01 HAVE_PACKAGEKIT

#define WORKSPACE_VERSION_STRING "${PROJECT_VERSION}"
