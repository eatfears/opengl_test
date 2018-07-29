QT += core
QT -= gui

CONFIG += c++11

TARGET = opengl_test
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

LIBS += -lGLEW -lglfw -lGL

SOURCES += main.cpp \
    shader.cpp

DEFINES += QT_DEPRECATED_WARNINGS

DISTFILES += \
    shader.vert \
    shader.frag

HEADERS += \
    shader.h
