# wersja Qt: 5.11.1 MSVC2015

QT       += core gui widgets serialport

CONFIG += Console

TARGET = MKMX_TestApp
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    engine/interface.cpp \
    utils/ringbuffer.cpp \
    engine/engine.cpp \
    utils/debugtools.cpp \
    utils/crctools.cpp

HEADERS  += mainwindow.h \
    engine/interface.h \
    utils/ringbuffer.h \
    engine/engine.h \
    version.h \
    utils/debugtools.h \
    utils/crctools.h

FORMS    += mainwindow.ui

RESOURCES   +=  MKMX_TestApp.qrc

RC_FILE = MKMX_TestApp.rc

CONFIG(release, debug|release) {
    DESTDIR = $$PWD/_installer/_installer_sources

    COPY_FILES += $$TARGET.exe

    win32 {
        DESTDIR_WIN = $${DESTDIR}
        DESTDIR_WIN ~= s,/,\\,g
        PWD_WIN = $${PWD}
        PWD_WIN ~= s,/,\\,g
        for(FILE, COPY_FILES) {
            QMAKE_POST_LINK += $$quote(cmd /c copy /y $${PWD_WIN}\\$${FILE} $${DESTDIR_WIN}$$escape_expand(\\n\\t))
        }
    }
}
