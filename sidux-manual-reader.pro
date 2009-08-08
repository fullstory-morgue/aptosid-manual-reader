QT      +=  webkit
HEADERS += src/smr.h 
SOURCES += src/main.cpp src/smr.cpp
OBJECTS_DIR = build
MOC_DIR = build
UI_DIR = build
DESTDIR = bin
TEMPLATE = app
CONFIG += qt
FORMS += src/smr.ui

target.path = /usr/bin
icontarget.path = /usr/share/icons/hicolor/32x32/apps/
icontarget.files = src/sidux-manual-reader.png
iconstarget.path = /usr/share/sidux-manual-reader
iconstarget.files = icons
shortcuttarget.path = /usr/share/applications/sidux
shortcuttarget.files = src/sidux-manual-reader.desktop
csstarget.path = /usr/share/sidux-manual-reader/css/
csstarget.files = src/content.css

INSTALLS += target icontarget iconstarget shortcuttarget csstarget