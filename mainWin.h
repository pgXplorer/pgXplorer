/*
  LICENSE AND COPYRIGHT INFORMATION - Please read carefully.

  Copyright (c) 2011, davyjones <davyjones@github.com>

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

class ConnectionProperties;
class SchemaLink;
class TableLink;
class FunctionLink;
class Database;
class Schema;
class Table;
class View;
class TableView;
class PgxConsole;
class PgxEditor;
class Function;
class QueryView;

class Canvas : public QGraphicsView {
    Q_OBJECT

public:
    Canvas(QGraphicsScene&, QWidget *parent=0,
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

protected:
    void wheelEvent(QWheelEvent *event);
    //void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

signals:
    void status(const QString&);
    void clicked();
    void search();

private:
    bool zoom;
    QCursor zoom_cursor;
    QRubberBand *rubberBand;
    QPoint origin;
};

class MainWin : public QMainWindow {
    Q_OBJECT

public:
    static bool document_unsaved;
    QStringList table_completer_list;
    QStringList view_completer_list;
    QStringList function_completer_list;
    MainWin(QWidget *parent=0, const QString name=QString(""), Qt::WindowFlags f=0);
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
    void clearPgxconsoleList();
    void clearPgxeditorList();
    void clearQueryViewList();
    void clearChildWindows();

public slots:
    void about();
    void document_changed();
    void tableViewClosed(TableView *);
    void queryViewClosed(QueryView *);
    void pgxconsoleClosed(PgxConsole *);
    void pgxeditorClosed(PgxEditor *);
    void newDatabase();
    bool newDatabase(QString, qint32, QString, QString, QString);

protected:
     void resizeEvent(QResizeEvent *event);
     void changeEvent(QEvent*);
     void contextMenuEvent(QContextMenuEvent*);
     void closeEvent(QCloseEvent*);

private slots:
    void newView();
    void clear();
    void openFile();
    void open(QString);
    void saveFile();
    void save(QString);
    void saveFileAs();
    //int saveState(QString);
    void quitApp();
    void newFile();
    void newProcess();
    void openDatabaseProperties();
    void showPgxconsole();
    void showPgxeditor();
    void showFunctionEditor(Schema *, Function*);
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
    void showTables(Schema*);
    void showAllTables();
    void hideTables(Schema*);
    void hideAllTables();
    void showViews(Schema*);
    void showAllViews();
    void hideViews(Schema*);
    void hideAllViews();
    void showFunctions(Schema*);
    void showAllFunctions();
    void hideFunctions(Schema*);
    void hideAllFunctions();
    void hideOtherTables(Schema*);
    void showTableView(Database *, Schema *, Table*);
    void clearTableView(Database *, Schema *, Table*);
    void dropTableView(Database *, Schema *, Table*);
    void showViewView(Database *, Schema *, View*);
    void dropViewView(Database *, Schema *, View*);
    void showQueryView(Database *, QString);
    void fitView();
    void zoomIn(const QPointF);
    void zoomOut(const QPointF);
    void noZoom();

Q_SIGNALS:
    void clicked();
    void closing();
    void showColumnView();

private:
    QTranslator translator;

    QFile database_file;

    QGraphicsScene scene;
    Canvas *canvas;
    QLineEdit *search_box;
    QCompleter *completer;
    QPrinter *printer;
    QString language_setting;

    Database *database;
    QList<TableView*> table_view_list;
    QList<TableView*> view_view_list;
    QList<QueryView*> query_view_list;
    QList<PgxConsole*> pgxconsole_list;
    QList<PgxEditor*> pgxeditor_list;

    QMenu *file_menu;
    QMenu *tool_menu;
    QMenu *view_menu;
    QMenu *help_menu;
    QMenu *display_menu;
    QMenu *language_menu;

    QToolBar *toolbar;

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
