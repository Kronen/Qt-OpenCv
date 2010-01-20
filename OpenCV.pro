# -------------------------------------------------
# Project created by QtCreator 2009-12-15T23:07:40
# -------------------------------------------------
TARGET = OpenCV
TEMPLATE = app

# OpenCv Configuration
INCLUDEPATH += "C:\OpenCV2.0\include\opencv"
LIBS += "C:\OpenCV2.0\lib\libcv200.dll.a"
LIBS += "C:\OpenCV2.0\lib\libcxcore200.dll.a"
LIBS += "C:\OpenCV2.0\lib\libhighgui200.dll.a"

# LIBS += "C:\OpenCV2.0\lib\libcvaux200.dll.a"
SOURCES += main.cpp \
    camerawindow.cpp \
    opencvwidget.cpp
HEADERS += camerawindow.h \
    opencvwidget.h
RESOURCES += resources.qrc
