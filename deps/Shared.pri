########################################
# Shared.pri
########################################

QT += core gui

# TODO: Define your C++ version. c++14, c++17, etc.
CONFIG += c++17

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# For DLL visibility
#DEFINES += QXlsx_SHAREDLIB QXlsx_EXPORTS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0



########################################
# source code
include(QXlsx/QXlsx.pri)
INCLUDEPATH += $$PWD
HEADERS += \
    $$PWD/sidebar.h \

SOURCES += \
    $$PWD/sidebar.cpp \


########################################
# custom setting for compiler & system

