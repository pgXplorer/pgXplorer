/*
  LICENSE AND COPYRIGHT INFORMATION - Please read carefully.

  Copyright (c) 2011-2012, davyjones <davyjones@github>

  Permission to use, copy, modify, and/or distribute this software for any
  purpose with or without fee is hereby granted, provided that the above
  copyright notice and this permission notice appear in all copies.

  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#ifndef PGXPLORER
#define PGXPLORER

#include <QMouseEvent>
#include <QtGui>

/*!
  @mainpage PgXplorer
  @section intro_sec Introduction
  pgXplorer is a freely downloadable, BSD licensed
  administration interface for PostgreSQL family of databases.
  Features include a table viewer and the Pgx SQL console (PgxConsole).
 */

class MainWin;
class ConnectionProperties;
class SchemaLink;
class TableLink;
class FunctionLink;
class Database;
class Schema;
class Table;
class View;
class TableView;
class DesignView;
class ViewView;
class PgxConsole;
class PgxEditor;
class Function;
class QueryView;
class Help;

class GraphicsView : public QGraphicsView {
    Q_OBJECT

public:
    GraphicsView(QGraphicsScene&, QWidget *parent=0,
                 const char *name=0, Qt::WindowFlags f=0);
    void clear();
    bool isZoom()
    {
        return zoom;
    }
    void setZoom(bool zoom)
    {
        this->zoom = zoom;
    }
    void setMainWin(MainWin *mainwin)
    {
        this->mainwin = mainwin;
    }

protected:
    void wheelEvent(QWheelEvent *event);
    //void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void dragEnterEvent(QDragEnterEvent *);
    void dragMoveEvent(QDragMoveEvent *);
    void dropEvent(QDropEvent *);

signals:
    void status(const QString&);
    void clicked();
    void search();

private:
    bool zoom;
    MainWin *mainwin;
    QCursor zoom_cursor;
    QPoint origin;
};

class GraphicsTextItem : public QGraphicsTextItem
{
    Q_OBJECT

private:
    QString schema_name; //if applicable

protected:
     void keyPressEvent(QKeyEvent *event)
     {
         switch(event->key()) {
         case Qt::Key_Return:
         case Qt::Key_Enter:
             event->accept();
             emit enterPressed(schema_name, this);
             break;
         default:
             QGraphicsTextItem::keyPressEvent(event);
         }
     }

     void focusOutEvent(QFocusEvent *event)
     {
         if(event->reason() == Qt::PopupFocusReason)
             QGraphicsTextItem::focusOutEvent(event);
         else
            emit enterPressed(schema_name, this);
     }

public:
     void setSchemaName(QString schema_name)
     {
         this->schema_name = schema_name;
     }

signals:
     void enterPressed(QString, GraphicsTextItem*);
};

class MainWin : public QMainWindow {
    Q_OBJECT
    Q_PROPERTY(Language language READ language WRITE setLanguage)
    Q_ENUMS(Language)
    Q_PROPERTY(DisplayMode display_mode READ displayMode WRITE setDisplayMode)
    Q_ENUMS(DisplayMode)

public:
    static bool session_unsaved;

    enum Language {English, Japanese, French};
    void setLanguage(Language language);
    Language language() const;

    enum DisplayMode {Tables, Views, Functions};
    void setDisplayMode(DisplayMode display_mode);
    DisplayMode displayMode() const;

    QStringList table_completer_list;
    QStringList view_completer_list;
    QStringList function_completer_list;
    MainWin(QWidget *parent=0, const QString arg1=QLatin1String(""), Qt::WindowFlags f=0);
    ~MainWin();
    QGraphicsScene& getScn()
    {
        return scene;
    }

    QLineEdit *getSearchBox()
    {
        return search_box;
    }

    bool isColumnView()
    {
        return columnview_action->isChecked();
    }
    bool isTableView()
    {
        return tableview_action->isChecked();
    }
    bool isViewView()
    {
        return viewview_action->isChecked();
    }
    bool isFunctionView()
    {
        return functionview_action->isChecked();
    }

    void clearTableViewList();
    void clearDesignViewList();
    void clearViewViewList();
    void clearPgxconsoleList();
    void clearPgxeditorList();
    void clearQueryViewList();
    void newSchema();
    void clearSchemas();
    void clearChildWindows();
    void checkForUpdates();

public slots:
    void about();
    void showHelp();
    void document_changed();
    void tableViewClosed(TableView *);
    void designViewClosed(DesignView *);
    void viewViewClosed(ViewView *);
    void queryViewClosed(QueryView *);
    void pgxconsoleClosed(PgxConsole *);
    void pgxeditorClosed(PgxEditor *);
    void newDatabase();
    bool newDatabase(QString, qint32, QString, QString, QString);
    void newPgxplorer(QString);
    void createSchema(QString, QString);
    void createTable(QString, GraphicsTextItem*);
    void showAllTables();
    void showAllViews();
    void showAllFunctions();
    void populateWindowMenu();

protected:
     void resizeEvent(QResizeEvent *);
     void changeEvent(QEvent*);
     void dragEnterEvent(QDragEnterEvent *);
     void dropEvent(QDropEvent *);
     void contextMenuEvent(QContextMenuEvent *);
     void closeEvent(QCloseEvent *);

private slots:
    void clear();
    void openFile();
    void open(QString);
    void saveFile();
    void save(QString);
    bool saveFileAs();
    bool quitApp();
    bool newFile();
    void newPgxplorer();
    void openDatabaseProperties();
    void showPgxconsole();
    void showPgxeditor();
    void showPgxeditor(QString);
    void showPgxeditor(QString, QString);
    void showFunctionEditor(Schema *, Function *);
    void showViewEditor(Schema *, View *);
    void toggleFullscreen();
    void showTreeview();
    void search();
    void setLanguageDefault();
    void setLanguageJapanese();
    void setLanguageFrench();
    void restore();
    void showSchemas();
    void explodeAndShowSchemas();
    void explodeAndShowSchemasVertically();
    void hideSchemas();
    void showTables(Schema *);
    void hideTables(Schema *);
    void hideAllTables();
    void showViews(Schema *);
    void hideViews(Schema *);
    void hideAllViews();
    void showFunctions(Schema *);
    void newTable(Schema *);
    void newFunction(Schema *);
    void hideFunctions(Schema *);
    void hideAllFunctions();
    void hideOtherTables(Schema *);
    void showTableView(Database *, Schema *, Table *);
    void showDesignView(Database *, Schema *, Table *);
    void clearTableView(Database *, Schema *, Table *);
    void dropTable(Database *, Schema *, Table *);
    void showViewView(Database *, Schema *, View *);
    void dropView(Database *, Schema *, View *);
    void dropFunction(Database *, Schema *, Function *);
    void showQueryView(Database *, QString);
    void fitView();
    void zoomIn();
    void zoomOut();
    void zoomIn(const QPointF);
    void zoomOut(const QPointF);
    void noZoom();

signals:
    void languageChanged(QEvent *);
    void clicked();
    void closing();
    void showColumnView();

private:
    QTranslator translator;
    QTranslator qt_translator;

    Language lang;
    DisplayMode disp_mode;

    QFile database_file;

    QGraphicsScene scene;
    GraphicsView *graphics_view;
    QLineEdit *search_box;
    QCompleter *completer;
    QPrinter *printer;

    Database *database;
    QList<TableView *> table_view_list;
    QList<DesignView *> design_view_list;
    QList<ViewView *> view_view_list;
    QList<QueryView *> query_view_list;
    QList<PgxConsole *> pgxconsole_list;
    QList<PgxEditor *> pgxeditor_list;
    QList<QAction*> windows;

    QMenu *file_menu;
    QMenu *tool_menu;
    QMenu *windows_menu;
    QMenu *view_menu;
    QMenu *help_menu;
    QMenu *display_menu;
    QMenu *language_menu;

    QToolBar *toolbar;
    Help *help;

    QAction *new_file_action;
    QAction *open_file_action;
    QAction *save_file_action;
    QAction *save_file_as_action;
    QAction *exit_action;
    QAction *connection_properties_action;
    QAction *console_action;
    QAction *editor_action;
    QAction *search_action;
    QAction *fullscreen_action;
    QActionGroup *layout_view_actiongroup;
    QAction *treeview_action;
    QAction *columnview_action;
    QActionGroup *content_view_actiongroup;
    QAction *tableview_action;
    QAction *viewview_action;
    QAction *functionview_action;
    QAction *english_action;
    QAction *japanese_action;
    QAction *french_action;
    QAction *help_action;
    QAction *about_action;

    void createActions();
    void createMenus();
    void readSettings();
    void writeSettings();
    void adjustSearchBoxPosition();

    void wheelEvent(QWheelEvent *);
};

#endif
