TEMPLATE = app
TARGET = mtpd
DEPENDPATH += .
INCLUDEPATH += . ../mts
LIBS += -L../mts -lmeegomtp

QT -= gui
CONFIG += link_pkgconfig

SOURCES += service.cpp

#install
target.path += /usr/bin/
target.files = mtpd
INSTALLS += target

#clean
QMAKE_CLEAN += $(TARGET)
