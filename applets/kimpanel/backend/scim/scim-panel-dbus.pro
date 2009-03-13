QT += dbus
TEMPLATE = app
CONFIG += no_keywords
DEFINES += SCIM_ICON_DIR=\\\"/usr/share/scim/icons/\\\"
SOURCES += main.cpp
INCLUDEPATH += /usr/include/scim-1.0/
LIBS += -lscim-1.0 
TARGET = scim-panel-dbus
