TEMPLATE	= app
TARGET		= pgXplorer

CONFIG		+= qt warn_on

HEADERS		= \
    tableLink.h \
    schemaLink.h \
    database.h \
    schema.h \
    table.h \
    mainWin.h \
    pgxconsole.h \
    queryview.h \
    tableview.h \
    querymodel.h \
    connectionproperties.h \
    pgxeditor.h \
    function.h \
    functionlink.h \
    view.h \
    highlighter.h \
    licensedialog.h
SOURCES		= main.cpp \
    tableLink.cpp \
    schemaLink.cpp \
    database.cpp \
    schema.cpp \
    table.cpp \
    mainWin.cpp \
    pgxconsole.cpp \
    queryview.cpp \
    tableview.cpp \
    querymodel.cpp \
    connectionproperties.cpp \
    pgxeditor.cpp \
    function.cpp \
    functionlink.cpp \
    view.cpp \
    highlighter.cpp \
    licensedialog.cpp
QT += sql translator

RESOURCES += pgXplorer.qrc

TRANSLATIONS = pgXplorer_ja.ts \
               pgXplorer_fr.ts

RC_FILE = pgXplorer.rc

CODECFORTR = UTF-8

# install
target.path = C:/QtSDK/Examples/4.7/graphicsview/pgXplorer
sources.files = $$SOURCES $$HEADERS $$RESOURCES pgXplorer.pro
sources.path = C:/QtSDK/Examples/4.7/graphicsview/pgXplorer
INSTALLS += target sources

OTHER_FILES += \
    pgXplorer.rc


