#-------------------------------------------------
#
# Project created by QtCreator 2022-11-14T15:34:28
#
#-------------------------------------------------

QT       += core gui network sql charts multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = smart_farm
TEMPLATE = app

RC_ICONS = logo.ico
VERSION = 1.0.0
#message($$QMAKESPEC)

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS HAVE_CONFIG_H

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    qrcode/bitstream.c \
    qrcode/mask.c \
    qrcode/mmask.c \
    qrcode/mqrspec.c \
    qrcode/qrencode.c \
    qrcode/qrinput.c \
    qrcode/qrspec.c \
    qrcode/rscode.c \
    qrcode/split.c \
    management.cpp \
    login.cpp \
    calendar/lunarcalendarinfo.cpp \
    calendar/lunarcalendaritem.cpp \
    calendar/lunarcalendarwidget.cpp \
    calendar/qlineeditcalendar.cpp \
    piechart.cpp \
    flatui.cpp \
    maskwidget.cpp \
    dialog_seach.cpp \
    dialog_delete.cpp \
    barchar.cpp \
    common.cpp

HEADERS += \
        mainwindow.h \
    qrcode/bitstream.h \
    qrcode/config.h \
    qrcode/mask.h \
    qrcode/mmask.h \
    qrcode/mqrspec.h \
    qrcode/qrencode.h \
    qrcode/qrencode_inner.h \
    qrcode/qrinput.h \
    qrcode/qrspec.h \
    qrcode/rscode.h \
    qrcode/split.h \
    management.h \
    login.h \
    calendar/lunarcalendarinfo.h \
    calendar/lunarcalendaritem.h \
    calendar/lunarcalendarwidget.h \
    calendar/qlineeditcalendar.h \
    piechart.h \
    flatui.h \
    maskwidget.h \
    dialog_seach.h \
    dialog_delete.h \
    barchar.h \
    common.h

FORMS += \
        mainwindow.ui \
    management.ui \
    dialog_seach.ui \
    dialog_delete.ui

RESOURCES += \
    image/res.qrc \
    sound/sound.qrc

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/lib/ -lQt5Qmqtt
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/lib/ -lQt5Qmqttd

INCLUDEPATH += $$PWD/src/mqtt
DEPENDPATH += $$PWD/src/mqtt
