#-------------------------------------------------
#
# Project created by QtCreator 2009-02-25T12:59:34
#
#-------------------------------------------------

QT       += dbus
#QT       -= gui

INCLUDEPATH += /usr/include/glib-2.0
INCLUDEPATH += /usr/include/gtk-2.0
INCLUDEPATH += /usr/include/scim-1.0

LIBS += -lscim-1.0

TARGET = scim-panel-dbus
#DESTDIR = /usr/lib/scim-1.0/

CONFIG   += console
CONFIG   += no_keywords
#CONFIG   += release
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp
