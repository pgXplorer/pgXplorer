TEMPLATE	= app
TARGET		= pgXplorer

CONFIG		+= qt warn_on

VER_MAJ = 0
VER_MIN = 1
VER_PAT = 0
VERSION = 0.1.0

HEADERS		= \
    database.h \
    table.h \
    pgxconsole.h \
    queryview.h \
    querymodel.h \
    connectionproperties.h \
    pgxeditor.h \
    function.h \
    functionlink.h \
    view.h \
    highlighter.h \
    licensedialog.h \
    tableview.h \
    tablelink.h \
    schema.h \
    mainwin.h \
    schemalink.h \
    tablemodel.h \
    viewview.h \
    help.h \
    designview.h \
    checkboxdelegate.h \
    tableproperties.h \
    graphwindow.h \
    pgxplorerapplication.h
SOURCES		= main.cpp \
    database.cpp \
    table.cpp \
    pgxconsole.cpp \
    queryview.cpp \
    querymodel.cpp \
    connectionproperties.cpp \
    pgxeditor.cpp \
    function.cpp \
    functionlink.cpp \
    view.cpp \
    highlighter.cpp \
    licensedialog.cpp \
    mainwin.cpp \
    schemalink.cpp \
    schema.cpp \
    tableview.cpp \
    tablelink.cpp \
    tablemodel.cpp \
    viewview.cpp \
    help.cpp \
    designview.cpp \
    checkboxdelegate.cpp \
    tableproperties.cpp \
    graphwindow.cpp \
    pgxplorerapplication.cpp

QT += sql translator network

RESOURCES += pgXplorer.qrc

TRANSLATIONS = pgXplorer_ja.ts \
               pgXplorer_fr.ts \
               qt_ja.ts

RC_FILE = pgXplorer.rc

ICON = database.icns

CODECFORTR = UTF-8

desktop.path = /usr/share/applications
desktop.files += pgXplorer.desktop

# install
target.path = C:/QtSDK/Examples/4.7/graphicsview/pgXplorer
sources.files = $$SOURCES $$HEADERS $$RESOURCES pgXplorer.pro
sources.path = C:/QtSDK/Examples/4.7/graphicsview/pgXplorer
INSTALLS += target sources desktop

OTHER_FILES += \
    pgXplorer.rc
