TEMPLATE	= app
TARGET		= pgXplorer

CONFIG		+= qt warn_on

VER_MAJ = 1
VER_MIN = 0
VER_PAT = 0
VERSION = 1.0.0

#DEFINES *= QT_USE_QSTRINGBUILDER

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
    reportwindow.h \
    pgxplorerapplication.h \
    statusview.h \
    comboheader.h \
    tablequerythread.h \
    simplequerythread.h

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
    reportwindow.cpp \
    pgxplorerapplication.cpp \
    statusview.cpp \
    comboheader.cpp \
    tablequerythread.cpp \
    simplequerythread.cpp

QT += core gui sql widgets network

QMAKE_CXXFLAGS += -std=c++11

#SUBDIRS = threads models gui graphicalitems views

win32:INCLUDEPATH += "C:/tmp3/postgresql-9.2.4/src/interfaces/libpq" "C:/tmp3/postgresql-9.2.4/src/include"
unix:INCLUDEPATH += "/home/nimbus/libharu/include" "/usr/include"

win32:LIBS += C:/tmp5/libhpdf-2.3.0RC2/src/.libs/libhpdf.a -lQt5Concurrent C:/tmp3/postgresql-9.2.4/src/interfaces/libpq/libpq.a
unix:LIBS += "/home/nimbus/libharu/src/libhpdfs.a" -lQt5Concurrent -lz -lpq

MOC_DIR = mocs
OBJECTS_DIR = objs
RESOURCES += pgXplorer.qrc

TRANSLATIONS = pgXplorer_ja.ts \
               pgXplorer_fr.ts \
               qt_ja.ts

RC_FILE = pgXplorer.rc

ICON = database.icns

CODECFORTR = UTF-8

# install
#target.path = C:/QtSDK/Examples/4.7/graphicsview/pgXplorer
#sources.files = $$SOURCES $$HEADERS $$RESOURCES pgXplorer.pro
#sources.path = C:/QtSDK/Examples/4.7/graphicsview/pgXplorer
#INSTALLS += target sources

OTHER_FILES += \
    pgXplorer.rc
