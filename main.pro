TARGET = rclone-ctl-gui
DESTDIR = bin/
OBJECTS_DIR = obj/
INCLUDEPATH = inc/
HEADERS = inc/*
SOURCES = src/*
QT += widgets

DEFINES += PROGRAM=\\\"rclone-ctl-gui\\\"

CONFIG += lrelease embed_translations
RESOURCES += res/res.qrc



