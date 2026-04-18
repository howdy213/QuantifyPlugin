QT += core gui qml
QT += widgets

TEMPLATE = lib
CONFIG += plugin

CONFIG += c++2b
TARGET = QuantifyPlugin
LIBS += -luser32
LIBS += -lOle32
win32 {
    CONFIG(debug, debug|release) {
        DESTDIR  = ../../src/debug/plugins/howdy213
    } else {
        DESTDIR  = ../../src/release/plugins/howdy213
    }
}
win32:CONFIG(release, debug|release): LIBS += -L../../src/release -lwecore
else:win32:CONFIG(debug, debug|release): LIBS += -L../../src/debug/ -lwecore
include(../../deps/WECore/WECore.pri)
include(../../deps/Shared.pri)
INCLUDEPATH +=\
    ../../deps/ \
    "C:\Program Files\OpenSSL-Win64\include"

LIBS += -L"C:\Program Files\OpenSSL-Win64\lib\VC\x64\MD"
LIBS += -llibcrypto
SOURCES += \
    classrecord.cpp \
    classrule.cpp \
    encryptor.cpp \
    grouprecord.cpp \
    jsrule.cpp \
    linenumbertextedit.cpp \
    logger.cpp \
    nativerule.cpp \
    quantify.cpp \
    quantifydialog.cpp \
    quantifydisplayviewdialog.cpp \
    quantifydisplaywindow.cpp \
    quantifyeditwindow.cpp \
    quantifyhelpdialog.cpp \
    quantifyplugin.cpp \
    quantifysettingwindow.cpp \
    rulebase.cpp \
    studentrecord.cpp \
    studentrecordviewer.cpp \
    virtualkeyboard.cpp

HEADERS += \
    classrecord.h \
    classrule.h \
    encryptor.h \
    grouprecord.h \
    jsrule.h \
    linenumbertextedit.h \
    logger.h \
    nativerule.h \
    quantify.h \
    quantifydialog.h \
    quantifydisplayviewdialog.h \
    quantifydisplaywindow.h \
    quantifyeditwindow.h \
    quantifyhelpdialog.h \
    quantifyplugin.h \
    quantifysettingwindow.h \
    rulebase.h \
    studentrecord.h \
    studentrecordviewer.h \
    virtualkeyboard.h

DISTFILES += QuantifyPlugin.json

# Default rules for deployment.
unix {
    target.path = $$[QT_INSTALL_PLUGINS]/generic
}
!isEmpty(target.path): INSTALLS += target

FORMS += \
    quantifydialog.ui \
    quantifydisplayviewdialog.ui \
    quantifydisplaywindow.ui \
    quantifyeditwindow.ui \
    quantifyhelpdialog.ui \
    quantifysettingwindow.ui

RESOURCES += \
    resources.qrc
