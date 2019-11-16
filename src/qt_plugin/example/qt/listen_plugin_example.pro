TARGET = listen_plugin_example
TEMPLATE = app
QT += core widgets gui
CONFIG += c++14
DEFINES += QT_DEPRECATED_WARNINGS

LIBPATH = $$PWD/../../lib
INCLUDEPATH += $$PWD/../../include
LIBS += -L$$LIBPATH/ -llisten

SOURCES += main.cpp \
           listener.cpp
HEADERS += listener.h
FORMS += listener.ui
