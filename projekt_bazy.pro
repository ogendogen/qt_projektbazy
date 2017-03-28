#-------------------------------------------------
#
# Project created by QtCreator 2016-05-10T20:22:32
#
#-------------------------------------------------

QT       += core gui
QT       += sql
CONFIG   += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = projekt_bazy
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    auth.cpp \
    users.cpp \
    books.cpp \
    lendbook.cpp \
    bookback.cpp \
    readers.cpp

HEADERS  += mainwindow.h \
    auth.h \
    users.h \
    books.h \
    lendbook.h \
    bookback.h \
    readers.h

FORMS    += mainwindow.ui \
    auth.ui \
    users.ui \
    books.ui \
    lendbook.ui \
    bookback.ui \
    readers.ui
