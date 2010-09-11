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

TRANSLATIONS = translations/aptosid-manual-reader_de.ts

target.path = /usr/bin
icontarget.path = /usr/share/icons/hicolor/32x32/apps/
icontarget.files = src/aptosid-manual-reader.png
iconstarget.path = /usr/share/aptosid-manual-reader
iconstarget.files = icons
shortcuttarget.path = /usr/share/applications/aptosid
shortcuttarget.files = src/aptosid-manual-reader.desktop
csstarget.path = /usr/share/aptosid-manual-reader/css/
csstarget.files = src/content.css
langtarget.path = /usr/share/aptosid-manual-reader/translations/
langtarget.files = translations/aptosid-manual-reader_de.qm

INSTALLS += target icontarget iconstarget shortcuttarget csstarget langtarget