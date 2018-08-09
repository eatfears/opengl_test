QT += core
QT -= gui

CONFIG += c++11

TARGET = opengl_test
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

LIBS += -lGLEW -lglfw -lGL -lSOIL -lassimp


SOURCES += main.cpp \
    imgui/imgui.cpp \
    imgui/imgui_draw.cpp \
    imgui/imgui_demo.cpp \
    gui/imgui_impl_opengl3.cpp \
    gui/imgui_impl_glfw.cpp \
    gui/gui.cpp

HEADERS += \
    shader.h \
    camera.h \
    mesh.h \
    arrays.h \
    imgui/*.h \
    gui/imgui_impl_opengl3.h \
    gui/imgui_impl_glfw.h \
    gui/gui.h

DEFINES += QT_DEPRECATED_WARNINGS

DISTFILES += \
    shaders/shader.vert \
    shaders/shader.frag \
    shaders/lampshader.frag \
    shaders/simpleshader.vert \
    shaders/scalingshader.vert \
    shaders/singlecolorshader.frag \
    shaders/rgbashader.frag \
    shaders/postshader.frag \
    shaders/postshader.vert \
    shaders/skyboxshader.frag \
    shaders/skyboxshader.vert \
    shaders/homes.geom
