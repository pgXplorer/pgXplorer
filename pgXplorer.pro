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
    tableproperties.h
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
    tableproperties.cpp

QT += sql translator network

RESOURCES += pgXplorer.qrc

TRANSLATIONS = pgXplorer_ja.ts \
               pgXplorer_fr.ts \
               qt_ja.ts

RC_FILE = pgXplorer.rc

ICON = database.icns

CODECFORTR = UTF-8

# install
target.path = C:/QtSDK/Examples/4.7/graphicsview/pgXplorer
sources.files = $$SOURCES $$HEADERS $$RESOURCES pgXplorer.pro
sources.path = C:/QtSDK/Examples/4.7/graphicsview/pgXplorer
INSTALLS += target sources

OTHER_FILES += \
    pgXplorer.rc \
    icons/zoom_icon.png \
    icons/zoom.png \
    icons/wrap.png \
    icons/view-refresh.svgz \
    icons/view2.png \
    icons/truncate.png \
    icons/treeview.png \
    icons/tableview.svgz \
    icons/table.png \
    icons/selected_execute.png \
    icons/search.svgz \
    icons/search.png \
    icons/save-as.svgz \
    icons/save_as.png \
    icons/save.svgz \
    icons/save.png \
    icons/replace_previous.png \
    icons/replace_next.png \
    icons/removecolumn.png \
    icons/refresh.png \
    icons/properties.svgz \
    icons/properties.png \
    icons/previous.png \
    icons/paste.png \
    icons/open.svgz \
    icons/open.png \
    icons/ok.png \
    icons/next.png \
    icons/new.svgz \
    icons/new.png \
    icons/key.png \
    icons/help.png \
    icons/function.png \
    icons/fullscreen.svgz \
    icons/fullscreen.png \
    icons/find_previous.png \
    icons/find_next.png \
    icons/find.png \
    icons/filter.png \
    icons/execute.png \
    icons/exclude.png \
    icons/edit-table-delete-row.svgz \
    icons/editor.png \
    icons/descending.png \
    icons/delete_rows.png \
    icons/database.png \
    icons/cut.png \
    icons/copy_sql.svgz \
    icons/copy_sql.png \
    icons/copy.svgz \
    icons/copy.png \
    icons/console.svgz \
    icons/console.png \
    icons/columnview.png \
    icons/clear.png \
    icons/buy.png \
    icons/backwards.png \
    icons/ascending.png \
    icons/copy_with_headers.png
