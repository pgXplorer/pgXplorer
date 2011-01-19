TEMPLATE	= app
TARGET		= pgXplorer

CONFIG		+= qt warn_on

HEADERS		= \
    tableLink.h \
    schemaLink.h \
    properties.h \
    database.h \
    schema.h \
    table.h \
    mainWin.h \
    queryview.h \
    tableview.h \
    pgxconsole.h \
    explainview.h
SOURCES		= main.cpp \
    tableLink.cpp \
    schemaLink.cpp \
    properties.cpp \
    database.cpp \
    schema.cpp \
    table.cpp \
    mainWin.cpp \
    queryview.cpp \
    tableview.cpp \
    pgxconsole.cpp \
    explainview.cpp
QT += sql translator

RESOURCES += portedcanvas.qrc

TRANSLATIONS = pgXplorer_ja.ts \
               pgXplorer_fr.ts

# CODECFORTR = UTF-8

# install
target.path = $$[QT_INSTALL_EXAMPLES]/graphicsview/portedcanvas
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS portedcanvas.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/graphicsview/portedcanvas
INSTALLS += target sources

OTHER_FILES +=
