QT += core
QT -= gui

CONFIG += c++11

TARGET = opengl_test
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

LIBS += -lGLEW -lglfw -lGL -lSOIL

SOURCES += main.cpp

DEFINES += QT_DEPRECATED_WARNINGS

DISTFILES += \
    shader.vert \
    shader.frag \
    lampshader.vert \
    lampshader.frag

HEADERS += \
    shader.h \
    camera.h
