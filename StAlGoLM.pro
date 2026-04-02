QT -= gui

QT += network
QT += sql

CONFIG += c++11 console
CONFIG -= app_bundle

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    main.cpp \
    mytcpserver.cpp \
    server_functions.cpp \
    databasemanager.cpp \
    task1.cpp \
    task2.cpp \
    task3.cpp \
    task4.cpp

HEADERS += \
    mytcpserver.h \
    server_functions.h \
    databasemanager.h \
    task1.h \
    task2.h \
    task3.h \
    task4.h

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
