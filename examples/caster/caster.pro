TARGET = caster
TEMPLATE = app
CONFIG += c++17 console

LIBPATH = $$PWD/../../lib
INCLUDEPATH += $$PWD/../../include
LIBS += -L$$LIBPATH/ -lcast

SOURCES += main.cpp
