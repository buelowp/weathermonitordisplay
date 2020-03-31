TEMPLATE = app
TARGET = weatherdisplay
CONFIG += gui debug core
QT += network widgets qmqtt
OBJECTS_DIR = .obj
MOC_DIR = .moc
SOURCES = src/main.cpp src/primarydisplay.cpp
HEADERS = src/primarydisplay.h

