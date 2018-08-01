QT += core
QT -= gui

CONFIG += c++11

TARGET = opengl_test
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

LIBS += -lGLEW -lglfw -lGL -lSOIL -lassimp

SOURCES += main.cpp

HEADERS += \
    shader.h \
    camera.h \
    mesh.h \
    arrays.h

DEFINES += QT_DEPRECATED_WARNINGS

DISTFILES += \
    shader.vert \
    shader.frag \
    lampshader.frag \
    simpleshader.vert \
    scalingshader.vert \
    singlecolorshader.frag \
    rgbashader.frag \
    postshader.frag \
    postshader.vert \
    skyboxshader.frag \
    skyboxshader.vert
