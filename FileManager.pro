#-------------------------------------------------
#
# Project created by QtCreator 2014-12-24T12:45:26
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = FileManager
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    aboutdlg.cpp \
    inputdialog.cpp \
    editwindow.cpp \
    atrdialog.cpp \
    keydialog.cpp \
    senddialog.cpp \
    logindialog.cpp \
    popup.cpp \
    pull_file.cpp

HEADERS  += mainwindow.h \
    aboutdlg.h \
    CFileNode.h \
    inputdialog.h \
    editwindow.h \
    atrdialog.h \
    keydialog.h \
    senddialog.h \
    smtp.h \
    logindialog.h \
    popup.h \
    pull_file.h

FORMS    += mainwindow.ui \
    aboutdlg.ui \
    inputdialog.ui \
    atrdialog.ui \
    editwindow.ui \
    atrdialog.ui \
    keydialog.ui \
    senddialog.ui \
    logindialog.ui \
    popup.ui \
    pull_file.ui

RESOURCES += \
    menu.qrc

LIBS += -L/usr/lib/x86_64-linux-gnu/ -lssh

OTHER_FILES +=



