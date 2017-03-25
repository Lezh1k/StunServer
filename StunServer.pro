TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += src/main.c \
    src/stun.c

HEADERS += \
    include/stun.h \
    include/commons.h

INCLUDEPATH += include
