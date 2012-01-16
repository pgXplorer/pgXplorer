/*
  LICENSE AND COPYRIGHT INFORMATION - Please read carefully.

  Copyright (c) 2011, davyjones <davyjones@github>

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

#include <QDateTime>
#include <QMainWindow>
#include <QStatusBar>
#include <QMenuBar>
#include <QApplication>
#include <QPainter>
#include <QDebug>
#include "database.h"
#include "schema.h"
#include "table.h"
#include "tableview.h"
#include "queryview.h"
#include "schemaLink.h"
#include "tableLink.h"
#include "functionlink.h"
#include "connectionproperties.h"
#include "mainWin.h"
#include "pgxconsole.h"
#include "pgxeditor.h"
#include "licensedialog.h"

bool MainWin::document_unsaved = false;

Canvas::Canvas(
        QGraphicsScene& s, QWidget *parent,
        const char *name, Qt::WindowFlags f) :
    QGraphicsView(&s, parent)
{
    setMouseTracking(false);
    setAcceptDrops(true);
    zoom = false;
    zoom_cursor = QCursor(QPixmap(qApp->applicationDirPath().append("/icons/zoom.png")));
    setOptimizationFlags(QGraphicsView::DontSavePainterState);
    setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    setObjectName(name);
    setWindowFlags(f);
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
}

void Canvas::clear()
{
    scene()->clear();
}

void Canvas::wheelEvent(QWheelEvent *wheelEvent)
{
    wheelEvent->ignore();
}

void Canvas::mouseMoveEvent(QMouseEvent *event)
{
    if((event->buttons() == Qt::RightButton) && (zoom == false))
        zoom = true;
    QGraphicsView::mouseMoveEvent(event);
}

void Canvas::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::RightButton && (zoom == true)) {
        fitInView(scene()->selectionArea().boundingRect(), Qt::KeepAspectRatio);
        scene()->setSelectionArea(QPainterPath());
    }
    QGraphicsView::mouseReleaseEvent(event);
}

void Canvas::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void Canvas::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}

void Canvas::dropEvent(QDropEvent *event)
{
    event->acceptProposedAction();
    foreach(QUrl file, event->mimeData()->urls())
        mainwin->newPgxplorer(file.toLocalFile());
}

MainWin::MainWin(QWidget *parent, const QString arg1, Qt::WindowFlags f)
{
    setAttribute(Qt::WA_DeleteOnClose);
    readSettings();
    createActions();
    createMenus();

    setAcceptDrops(true);

    if(language_setting.compare("japanese") == 0)
        setLanguageJapanese();
    else if(language_setting.compare("french") == 0)
        setLanguageFrench();
    else
        setLanguageDefault();

    toolbar = new QToolBar;
    toolbar->setIconSize(QSize(36,36));
    toolbar->setObjectName("mainwin");
    toolbar->setMovable(false);
    toolbar->addAction(new_file_action);
    toolbar->addAction(open_file_action);
    toolbar->addAction(save_file_action);
    toolbar->addAction(save_file_as_action);
    toolbar->addSeparator();
    toolbar->addAction(connection_properties_action);
    toolbar->addAction(console_action);
    toolbar->addAction(editor_action);
    toolbar->addAction(search_action);
    toolbar->addAction(fullscreen_action);
    //toolbar->addSeparator();
    //toolbar->addAction(treeview_action);
    //toolbar->addAction(columnview_action);
    //toolbar->addSeparator();
    toolbar->addSeparator();
    toolbar->addAction(tableview_action);
    toolbar->addAction(viewview_action);
    toolbar->addAction(functionview_action);
    toolbar->addSeparator();

    addToolBar(toolbar);

    canvas = new Canvas(scene, this);
    canvas->setSceneRect(QRectF());

    QShortcut *shortcut_search = new QShortcut(QKeySequence::Find, this);
    connect(shortcut_search, SIGNAL(activated()), this, SLOT(search()));
    QShortcut *shortcut_restore_win = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(shortcut_restore_win, SIGNAL(activated()), this, SLOT(restore()));

    connect(canvas, SIGNAL(search()), this, SLOT(search()));

    setCentralWidget(canvas);

    printer = 0;

    search_box = new QLineEdit(canvas);
    search_box->hide();

    QPalette palette;
    palette.setColor(search_box->backgroundRole(), Qt::yellow);
    search_box->setPalette(palette);

    completer = new QCompleter;
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    search_box->setCompleter(completer);

    QSettings settings("pgXplorer","pgXplorer");
    bool is_maximized = settings.value("mainwin_maximized", false).toBool();
    if(is_maximized) {
       showMaximized();
    }

    newFile();
    if(!arg1.isEmpty())
        open(arg1);
}

void MainWin::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        file_menu->setTitle(MainWin::tr("&File"));
        tool_menu->setTitle(MainWin::tr("&Tools"));
        display_menu->setTitle(MainWin::tr("&Display"));
        language_menu->setTitle(MainWin::tr("&Language"));
        help_menu->setTitle(MainWin::tr("&Help"));

        new_file_action->setText(MainWin::tr("&New"));
        open_file_action->setText(MainWin::tr("&Open..."));
        save_file_action->setText(MainWin::tr("&Save"));
        save_file_as_action->setText(MainWin::tr("Save &As..."));
        exit_action->setText(MainWin::tr("E&xit"));
        connection_properties_action->setText(MainWin::tr("Connection properties"));
        console_action->setText(MainWin::tr("SQL Console"));
        editor_action->setText(MainWin::tr("SQL Editor"));
        search_action->setText(MainWin::tr("Search for items"));
        fullscreen_action->setText(MainWin::tr("Fullscreen"));
        treeview_action->setText(MainWin::tr(""));
        columnview_action->setText(MainWin::tr(""));
        tableview_action->setText(MainWin::tr("Tables"));
        viewview_action->setText(MainWin::tr("Views"));
        functionview_action->setText(MainWin::tr("Functions"));
        english_action->setText(MainWin::tr("English"));
        japanese_action->setText(MainWin::tr("Japanese"));
        french_action->setText(MainWin::tr("French"));
        help_action->setText(MainWin::tr("&Help"));
        about_action->setText(MainWin::tr("&License key/About"));

        emit languageChanged(event);
    }
}

void MainWin::readSettings()
{
    QSettings settings("pgXplorer","pgXplorer");
    QPoint pos = settings.value("mainwin_pos", QPoint(100, 100)).toPoint();
    QSize size = settings.value("mainwin_size", QSize(1024, 768)).toSize();
    resize(size);
    move(pos);

    language_setting = settings.value("language", "english").toString();
    display_setting = settings.value("display", "table").toString();
}

void MainWin::writeSettings()
{
    QSettings settings("pgXplorer","pgXplorer");
    if(isMaximized()) {
        settings.setValue("mainwin_maximized", true);
        showNormal();
    }
    else
        settings.setValue("mainwin_maximized", false);

    settings.setValue("mainwin_pos", pos());
    settings.setValue("mainwin_size", size());
    settings.setValue("language", language_setting);
    settings.setValue("display", display_setting);
}

void MainWin::openFile()
{
    if(document_unsaved)
    {
        QMessageBox msgBox;
        msgBox.setText("The document has been modified.");
        msgBox.setInformativeText("Do you want to save your changes?");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);
        int ret = msgBox.exec();

        switch (ret) {
            case QMessageBox::Save:
                saveFileAs();
                delete database;
                break;
            case QMessageBox::Discard:
                delete database;
                break;
            case QMessageBox::Cancel:
                return;
                break;
            default:
                break;
        }
    }

    QString file_name = QFileDialog::getOpenFileName(this, MainWin::tr("Open File"),
                                                     "C:/",
                                                     MainWin::tr("Database (*.pgx);;All files (*)"));
    if(file_name.isEmpty())
        return;
    open(file_name);
}

void MainWin::open(QString file_name)
{
    database_file.setFileName(file_name);
    if (!database_file.open(QIODevice::ReadWrite))
             return;
    QDataStream database_file_stream(&database_file);
    QByteArray magic_from_file;
    QByteArray magic;
    magic.resize(27);
    magic[0] = 0xef;
    magic[1] = 0xbd;
    magic[2] = 0x84;
    magic[3] = 0xef;
    magic[4] = 0xbd;
    magic[5] = 0x81;
    magic[6] = 0xef;
    magic[7] = 0xbd;
    magic[8] = 0x96;
    magic[9] = 0xef;
    magic[10] = 0xbd;
    magic[11] = 0x99;
    magic[12] = 0xef;
    magic[13] = 0xbd;
    magic[14] = 0x8a;
    magic[15] = 0xef;
    magic[16] = 0xbd;
    magic[17] = 0x8f;
    magic[18] = 0xef;
    magic[19] = 0xbd;
    magic[20] = 0x8e;
    magic[21] = 0xef;
    magic[22] = 0xbd;
    magic[23] = 0x85;
    magic[24] = 0xef;
    magic[25] = 0xbd;
    magic[26] = 0x93;
    database_file_stream >> magic_from_file;
    if(magic != magic_from_file) {
        QMessageBox *fileErr = new QMessageBox("pgXplorer", MainWin::tr("Not a database file."),
                                               QMessageBox::Critical, 1, 0, 0, this, FALSE );
        fileErr->setButtonText(1, "Close");
        fileErr->show();
        return;
    }
    qint32 version;
    database_file_stream >> version;
    if(version > 100) {
        QMessageBox *fileVerErr = new QMessageBox("pgXplorer", MainWin::tr("Database file version not supported."),
                                               QMessageBox::Critical, 1, 0, 0, this, FALSE );
        fileVerErr->setButtonText(1, "Close");
        fileVerErr->show();
        return;
    }

    QString stream_data, host, dbname, username, password;
    qint32 port;

    database_file_stream >> host;
    database_file_stream >> port;
    database_file_stream >> dbname;
    database_file_stream >> username;
    database_file_stream >> password;

    database_file_stream >> stream_data;
    if(!stream_data.compare("database") == 0)
        return;
    QString database_name;
    database_file_stream >> database_name;
    QPointF database_position;
    database_file_stream >> database_position;

    if(newDatabase(host, port, dbname, username, password))
        database->setName(database_name);
    //database->setPos(database_position);
    //database->setPos(scene.width()/4, scene.height()/4);

    bool database_collapsed;
    database_file_stream >> database_collapsed;
    if(!database_collapsed)
    {
        database->setDatabaseCollapsed(false);
        /*
        database_file_stream >> stream_data;
        if(!stream_data.compare("schema list") == 0)
        {
            while(stream_data.compare("end schema list"))
            {

                database_file_stream >> stream_data;
            }
        }
        */
    }
    //scene.setSceneRect(QRectF());
    this->setWindowTitle("pgXplorer - " + file_name);
    database_file.close();
}

void MainWin::saveFile()
{
    //if(document_unsaved)
    {
        if(database_file.fileName().isEmpty())
        {
            saveFileAs();
        }
        else
        {
            save(QString());
        }
    }
    database_file.close();
    document_unsaved = false;
}

void MainWin::saveFileAs()
{
    QString file_name = QFileDialog::getSaveFileName(this, MainWin::tr("Open File"),
                                                     "C:/",
                                                     MainWin::tr("Database (*.pgx);;All files (*)"));
    if(file_name.isEmpty())
        return;
    save(file_name);
    this->setWindowTitle("pgXplorer - " + file_name);
}

void MainWin::save(QString file_name)
{
    if(database_file.isOpen())
        database_file.close();
    database_file.setFileName(file_name.isEmpty()?database_file.fileName():file_name);
    if (!database_file.open(QIODevice::ReadWrite))
             return;
    QSqlDatabase database_connection = QSqlDatabase::database(QString("base").append(QString::number(database->dbViewObjectId)));
    QDataStream database_file_stream(&database_file);

    // Set magic number = 0x616479766F6A656E0073
    QByteArray magic;
    magic.resize(27);
    magic[0] = 0xef;
    magic[1] = 0xbd;
    magic[2] = 0x84;
    magic[3] = 0xef;
    magic[4] = 0xbd;
    magic[5] = 0x81;
    magic[6] = 0xef;
    magic[7] = 0xbd;
    magic[8] = 0x96;
    magic[9] = 0xef;
    magic[10] = 0xbd;
    magic[11] = 0x99;
    magic[12] = 0xef;
    magic[13] = 0xbd;
    magic[14] = 0x8a;
    magic[15] = 0xef;
    magic[16] = 0xbd;
    magic[17] = 0x8f;
    magic[18] = 0xef;
    magic[19] = 0xbd;
    magic[20] = 0x8e;
    magic[21] = 0xef;
    magic[22] = 0xbd;
    magic[23] = 0x85;
    magic[24] = 0xef;
    magic[25] = 0xbd;
    magic[26] = 0x93;
    database_file_stream << magic;

    // Set version number as 100
    database_file_stream << (qint32)100;

    // Set Qt compatible version(s)
    database_file_stream.setVersion(QDataStream::Qt_4_7);

    // Copy the relevant data
    database_file_stream << database_connection.hostName();
    database_file_stream << (qint32)database_connection.port();
    database_file_stream << database_connection.databaseName();
    database_file_stream << database_connection.userName();
    database_file_stream << database_connection.password();

    database_file_stream << QString("database");
    database_file_stream << database->getName();
    database_file_stream << database->pos();

    if(database->getDatabaseCollapsed())
    {
        database_file_stream << true;
    }
    else
    {
        database_file_stream << false;
        /*
        database_file_stream << QString("schema list");
        foreach(Schema *schema, db->getSchemaList())
        {
            database_file_stream << schema->getName();
            if(!schema->getSchemaCollapsed())
            {
                database_file_stream << true;
                database_file_stream << QString("table list");
                foreach(Table *table, schema->getTableList())
                {
                    database_file_stream << table;
                }
                database_file_stream << QString("end table list");
            }
            else
            {
                database_file_stream << false;
            }
        }
        database_file_stream << QString("end schema list");
        */
    }
}

MainWin::~MainWin()
{
    //delete printer;
}

void MainWin::resizeEvent(QResizeEvent *event)
{
    if(search_box->isVisible())
    {
        adjustSearchBoxPosition();
    }
}

void MainWin::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void MainWin::dropEvent(QDropEvent *event)
{
    event->acceptProposedAction();
    foreach(QUrl file, event->mimeData()->urls())
        newPgxplorer(file.toLocalFile());
}

void MainWin::wheelEvent(QWheelEvent *wheelEvent)
{
    // Capture wheel events to zoom-in or
    // zoom-out the canvas.
    wheelEvent->accept();
    // zoom out
    if (wheelEvent->delta()>0)
        zoomOut(mapFromGlobal(wheelEvent->globalPos()));
    // zoom in
    else
        zoomIn(mapFromGlobal(wheelEvent->globalPos()));

    if(search_box->isVisible())
    {
        adjustSearchBoxPosition();
    }
}

void MainWin::contextMenuEvent(QContextMenuEvent *event)
{
    if(canvas->isZoom()) {
        canvas->setZoom(false);
        return;
    }
    QMenu menu;
    menu.addAction(MainWin::tr("Default view"));
    menu.addAction(MainWin::tr("Zoom in"));
    menu.addAction(MainWin::tr("Zoom out"));
    menu.addAction(MainWin::tr("Fit view"));
    QAction *a = menu.exec(event->globalPos());
    if(a && QString::compare(a->text(),MainWin::tr("Default view"))==0) {
        noZoom();
    }
    else if(a && QString::compare(a->text(),MainWin::tr("Zoom in"))==0) {
        zoomIn(event->globalPos());
    }
    else if(a && QString::compare(a->text(),MainWin::tr("Zoom out"))==0) {
        zoomOut(event->globalPos());
    }
    else if(a && QString::compare(a->text(),MainWin::tr("Fit view"))==0) {
        fitView();
    }
}

void MainWin::closeEvent(QCloseEvent *event)
{
    //clearChildWindows();
    //QSqlDatabase::removeDatabase(QString("base").append(QString::number(database->getId())));
    writeSettings();
    quitApp();
    //QMainWindow::closeEvent(event);
}

void MainWin::clearSchemas()
{
    foreach(Schema *schema, database->getSchemaList()) {
        foreach(Table *table, schema->getTableList())
            delete table;
        schema->getTableList().clear();
        delete schema;
    }
    database->getSchemaList().clear();
}

void MainWin::clearTableViewList()
{
    foreach(TableView *table_view, table_view_list)
        table_view->close();
    table_view_list.clear(); 
}

void MainWin::clearViewViewList()
{
    foreach(TableView *view_view, view_view_list)
        view_view->close();
    view_view_list.clear();
}

void MainWin::clearQueryViewList()
{
    foreach(QueryView *query_view, query_view_list)
        query_view->close();
    query_view_list.clear();
}

void MainWin::clearPgxconsoleList()
{
    foreach(PgxConsole *pgxconsole, pgxconsole_list)
        pgxconsole->closeMain();
    pgxconsole_list.clear();
}

void MainWin::clearPgxeditorList()
{
    foreach(PgxEditor *pgxeditor, pgxeditor_list)
        pgxeditor->closeMain();
    pgxeditor_list.clear();
}

void MainWin::newView()
{
    MainWin *mainwin = new MainWin(0, QString(""), Qt::Widget);
    mainwin->show();
}

void MainWin::clear()
{
    canvas->clear();
}

void MainWin::about()
{
    LicenseDialog *license_dalog = new LicenseDialog(this);
    license_dalog->exec();
}

void MainWin::fitView()
{
    canvas->fitInView(scene.sceneRect(), Qt::KeepAspectRatio);
}

void MainWin::document_changed()
{
    document_unsaved = true;
}

void MainWin::tableViewClosed(TableView *table_view)
{
    table_view_list.removeOne(table_view);
}

void MainWin::viewViewClosed(TableView *table_view)
{
    view_view_list.removeOne(table_view);
}

void MainWin::queryViewClosed(QueryView *query_view)
{
    query_view_list.removeOne(query_view);
}

void MainWin::pgxconsoleClosed(PgxConsole *pgxconsole)
{
    pgxconsole_list.removeOne(pgxconsole);
}

void MainWin::pgxeditorClosed(PgxEditor *pgxeditor)
{
    pgxeditor_list.removeOne(pgxeditor);
}

void MainWin::noZoom()
{
    canvas->resetTransform();
    canvas->centerOn(database);
}

void MainWin::zoomIn(const QPointF centre)
{
    //canvas->centerOn((centre.toPoint()));
    canvas->scale( 1.1, 1.1 );
}

void MainWin::zoomOut(const QPointF centre)
{
    //canvas->centerOn((centre.toPoint()));
    canvas->scale( 0.9, 0.9 );
}

void MainWin::newFile()
{
    if(document_unsaved)
    {
        QMessageBox msgBox;
        msgBox.setText("The document has been modified.");
        msgBox.setInformativeText("Do you want to save your changes?");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);
        int ret = msgBox.exec();

        switch (ret) {
            case QMessageBox::Save:
                saveFileAs();
                delete database;
                break;
            case QMessageBox::Discard:
                delete database;
                break;
            case QMessageBox::Cancel:
                return;
                break;
            default:
                break;
        }
    }
    newDatabase();
}

void MainWin::newPgxplorer()
{
    newPgxplorer(QString());
}

void MainWin::newPgxplorer(QString file)
{
    QProcess *new_pgxplorer = new QProcess;

#ifdef Q_WS_WIN
    new_pgxplorer->start("pgXplorer.exe", QStringList() << file);
#endif

#ifdef Q_WS_X11
    new_pgxplorer->start("./pgXplorer", QStringList() << file);
#endif
}

void MainWin::newDatabase()
{
    // Add a database object to canvas
    database = new Database(this, 0);
    scene.addItem(database);
}

bool MainWin::newDatabase(QString host, qint32 port, QString dbname, QString username, QString password)
{
    //First save the database identity and then clear
    //the scene.
    int database_id = database->getId();
    clearChildWindows();
    scene.clear();

    // Add a database object to canvas
    database = new Database(this, database_id);
    scene.addItem(database);
    if(database->setConnectionProperties(host, port, dbname, username, password) == false)
        return false;
    database->setToolTip(QString("Host: ") + host + "\nPort: " + QString::number(port) + "\nUser: " + username);
    if(tableview_action->isChecked())
        showAllTables();
    else if(viewview_action->isChecked())
        showAllViews();
    else if(functionview_action->isChecked())
        showAllFunctions();
    canvas->translate(0,-37.5);
    canvas->centerOn(database);
    canvas->setDragMode(QGraphicsView::RubberBandDrag);
    canvas->setMainWin(this);
    console_action->setEnabled(true);
    editor_action->setEnabled(true);
    return true;
}

void  MainWin::clearChildWindows()
{
    clearTableViewList();
    clearViewViewList();
    clearPgxconsoleList();
    clearPgxeditorList();
    clearQueryViewList();
    clearSchemas();
}

void MainWin::quitApp()
{
    if(document_unsaved)
    {
        QMessageBox msgBox;
        msgBox.setText("The document has been modified.");
        msgBox.setInformativeText("Do you want to save your changes?");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);
        int ret = msgBox.exec();

        switch (ret) {
            case QMessageBox::Save:
                saveFileAs();
                delete database;
                break;
            case QMessageBox::Discard:
                delete database;
                break;
            case QMessageBox::Cancel:
                return;
                break;
            default:
                break;
        }
    }

    foreach(QString database_connection_name, QSqlDatabase::connectionNames()) {
        QSqlDatabase database_connection = QSqlDatabase::database(database_connection_name);
        database_connection.close();
    }

    writeSettings();
    qApp->quit();
}

void MainWin::openDatabaseProperties()
{
    database->showPropertyDialog();
}

void MainWin::showPgxconsole()
{
    // Pseudo SQL console.
    // Goal: Accept all possible Postgresql related
    // DDL and DML commands and produce output
    // accordingly.
    PgxConsole *pgxconsole = new PgxConsole(database);
    pgxconsole_list.append(pgxconsole);
    QObject::connect(pgxconsole, SIGNAL(showQueryView(Database *, QString)), this, SLOT(showQueryView(Database*,QString)));
    QObject::connect(pgxconsole, SIGNAL(pgxconsoleClosing(PgxConsole*)), this, SLOT(pgxconsoleClosed(PgxConsole*)));
    QObject::connect(pgxconsole, SIGNAL(newPgxconsole()), this, SLOT(showPgxconsole()));
    QObject::connect(this, SIGNAL(languageChanged(QEvent*)), pgxconsole, SLOT(languageChanged(QEvent*)));

    QSettings settings("pgXplorer","pgXplorer");
    QPoint pos = settings.value("pgxconsole_pos", QPoint(100, 100)).toPoint();
    QSize size = settings.value("pgxconsole_size", QSize(640, 480)).toSize();

    pgxconsole->setResizePos(size, pos);
}

void MainWin::toggleFullscreen()
{
    if(!isFullScreen()) {
        showFullScreen();
        menuBar()->hide();
        toolbar->hide();
    }
    else {
        showNormal();
        menuBar()->show();
        toolbar->show();
    }
}

void MainWin::showTreeview()
{
    //if(treeview_action->isChecked())
}

void MainWin::search()
{
    if(search_action->isChecked() || search_box->isHidden()) {
        search_action->setChecked(true);
        delete completer;
        if(tableview_action->isChecked())
            completer = new QCompleter(table_completer_list, this);
        else if(viewview_action->isChecked())
            completer = new QCompleter(view_completer_list, this);
        else if(functionview_action->isChecked())
            completer = new QCompleter(function_completer_list, this);
        else
            completer = new QCompleter;
        search_box->setCompleter(completer);
        if(isFullScreen()) {
            search_box->setGeometry(this->width()-202,0,200,search_box->height());
            //search_box->setGeometry(0,0,200,search_box->height());
        }
        else
            adjustSearchBoxPosition();
        search_box->setFocus();
        search_box->show();
    }
    else {
        search_box->clear();
        search_box->hide();
    }
}

void MainWin::setLanguageDefault()
{
    translator.load(qApp->applicationDirPath().append("/pgXplorer_ja"));
    qApp->removeTranslator(&translator);
    japanese_action->setIconVisibleInMenu(false);
    translator.load(qApp->applicationDirPath().append("/pgXplorer_fr"));
    qApp->removeTranslator(&translator);
    french_action->setIconVisibleInMenu(false);
    english_action->setIconVisibleInMenu(true);
    language_setting = "english";
}

void MainWin::setLanguageJapanese()
{
    translator.load(qApp->applicationDirPath().append("/pgXplorer_ja"));
    qApp->installTranslator(&translator);
    japanese_action->setIconVisibleInMenu(true);
    french_action->setIconVisibleInMenu(false);
    english_action->setIconVisibleInMenu(false);
    language_setting = "japanese";
}

void MainWin::setLanguageFrench()
{
    translator.load(qApp->applicationDirPath().append("/pgXplorer_fr"));
    qApp->installTranslator(&translator);
    japanese_action->setIconVisibleInMenu(false);
    french_action->setIconVisibleInMenu(true);
    english_action->setIconVisibleInMenu(false);
    language_setting = "french";
}

void MainWin::restore()
{
    if(search_action->isChecked()) {
        search_box->clear();
        search_action->setChecked(false);
        search_box->setVisible(false);
    }
    else if(this->isFullScreen())
        toggleFullscreen();
}

void MainWin::showSchemas()
{
    QList<Schema*> schema_list = database->getSchemaList();
    foreach (Schema *schema, schema_list)
        schema->show();
    scene.setSceneRect(QRectF());
}

void MainWin::explodeAndShowSchemas()
{
    QList<Schema*> schema_children_list = database->getSchemaList();
    foreach (Schema *schema, schema_children_list)
    {
        showTables(schema);
        schema->setSchemaCollapsed(false);
        schema->resetTables();
        schema->update();
    }
    showSchemas();
    scene.setSceneRect(QRectF());
}

void MainWin::explodeAndShowSchemasVertically()
{
    QList<Schema*> schema_children_list = database->getSchemaList();
    foreach (Schema *schema, schema_children_list)
    {
        showTables(schema);
        schema->setSchemaCollapsed(false);
        schema->resetTablesVertically();
        schema->update();
    }
    showSchemas();
    scene.setSceneRect(QRectF());
}

void MainWin::hideSchemas()
{
    QList<Schema*> schema_list = database->getSchemaList();
    foreach (Schema *schema, schema_list)
        schema->hide();
}

void MainWin::showTables(Schema *schema)
{
    hideViews(schema);
    hideFunctions(schema);
    foreach (Table *table, schema->getTableList())
        table->show();
    schema->setSchemaCollapsed(false);
    schema->update();
}

void MainWin::showAllTables()
{
    display_setting = "table";
    delete completer;
    completer = new QCompleter(table_completer_list, this);
    search_box->setCompleter(completer);
    QList<Schema*> schema_list = database->getSchemaList();
    foreach (Schema *schema, schema_list)
        showTables(schema);
    scene.setSceneRect(QRectF());
}

void MainWin::hideTables(Schema *schema)
{
    foreach (Table *table, schema->getTableList())
        table->hide();
    schema->setSchemaCollapsed(true);
    schema->update();
}

void MainWin::hideAllTables()
{
    QList<Schema*> schema_list = database->getSchemaList();
    foreach (Schema *schema, schema_list)
        hideTables(schema);
}

void MainWin::showViews(Schema *schema)
{
    hideTables(schema);
    hideFunctions(schema);
    foreach (View *view, schema->getViewList())
        view->show();
    schema->setSchemaCollapsed(false);
    schema->update();
    scene.setSceneRect(QRectF());
}

void MainWin::showAllViews()
{
    display_setting = "view";
    delete completer;
    completer = new QCompleter(view_completer_list, this);
    search_box->setCompleter(completer);
    QList<Schema*> schema_list = database->getSchemaList();
    foreach (Schema *schema, schema_list)
        showViews(schema);
    scene.setSceneRect(QRectF());
}

void MainWin::hideViews(Schema *schema)
{
    foreach (View *view, schema->getViewList())
        view->hide();
    schema->setSchemaCollapsed(true);
    schema->update();
}

void MainWin::hideAllViews()
{
    QList<Schema*> schema_list = database->getSchemaList();
    foreach (Schema *schema, schema_list)
        hideViews(schema);
}

void MainWin::showFunctions(Schema *schema)
{
    hideTables(schema);
    hideViews(schema);
    foreach (Function *func, schema->getFunctionList())
        func->show();
    schema->setSchemaCollapsed(false);
    schema->update();
}

void MainWin::showAllFunctions()
{
    display_setting = "function";
    delete completer;
    completer = new QCompleter(function_completer_list, this);
    search_box->setCompleter(completer);
    QList<Schema*> schema_list = database->getSchemaList();
    foreach (Schema *schema, schema_list)
        showFunctions(schema);
    scene.setSceneRect(QRectF());
}

void MainWin::hideFunctions(Schema *schema)
{
    foreach (Function *func, schema->getFunctionList())
        func->hide();
    schema->setSchemaCollapsed(true);
    schema->update();
}

void MainWin::hideAllFunctions()
{
    QList<Schema*> schema_list = database->getSchemaList();
    foreach (Schema *schema, schema_list)
        hideFunctions(schema);
}

void MainWin::hideOtherTables(Schema *schema)
{
    QList<Schema*> schema_list = database->getSchemaList();
    foreach (Schema *ohter_schema, schema_list)
        if(ohter_schema != schema)
            hideTables(ohter_schema);
}

void MainWin::createActions()
{
    new_file_action = new QAction(QIcon(qApp->applicationDirPath().append("/icons/new.png")), MainWin::tr("&New"), this);
    new_file_action->setShortcuts(QKeySequence::New);
    new_file_action->setStatusTip(MainWin::tr("Create a new file"));
    connect(new_file_action, SIGNAL(triggered()), this, SLOT(newPgxplorer()));

    open_file_action = new QAction(QIcon(qApp->applicationDirPath().append("/icons/open.png")), MainWin::tr("&Open..."), this);
    open_file_action->setShortcuts(QKeySequence::Open);
    open_file_action->setStatusTip(MainWin::tr("Open an existing file"));
    connect(open_file_action, SIGNAL(triggered()), this, SLOT(openFile()));

    save_file_action = new QAction(QIcon(qApp->applicationDirPath().append("/icons/save.png")), MainWin::tr("&Save"), this);
    save_file_action->setShortcuts(QKeySequence::Save);
    save_file_action->setStatusTip(MainWin::tr("Save the document to disk"));
    connect(save_file_action, SIGNAL(triggered()), this, SLOT(saveFile()));

    save_file_as_action = new QAction(QIcon(qApp->applicationDirPath().append("/icons/save_as.png")), MainWin::tr("Save &As..."), this);
    save_file_as_action->setShortcuts(QKeySequence::SaveAs);
    save_file_as_action->setStatusTip(MainWin::tr("Save the document under a new name"));
    connect(save_file_as_action, SIGNAL(triggered()), this, SLOT(saveFileAs()));

    exit_action = new QAction(MainWin::tr("E&xit"), this);
    exit_action->setShortcuts(QKeySequence::Quit);
    exit_action->setStatusTip(MainWin::tr("Exit the application"));
    connect(exit_action, SIGNAL(triggered()), this, SLOT(quitApp()));

    connection_properties_action = new QAction(QIcon(qApp->applicationDirPath().append("/icons/properties.png")), MainWin::tr("Connection properties"), this);
    connection_properties_action->setShortcuts(QKeySequence::SaveAs);
    connection_properties_action->setStatusTip(MainWin::tr("Set connection properties"));
    connect(connection_properties_action, SIGNAL(triggered()), this, SLOT(openDatabaseProperties()));

    console_action = new QAction(QIcon(qApp->applicationDirPath().append("/icons/console.png")), MainWin::tr("SQL Console"), this);
    console_action->setShortcuts(QKeySequence::SaveAs);
    console_action->setEnabled(false);
    console_action->setStatusTip(MainWin::tr("Launch SQL console"));
    connect(console_action, SIGNAL(triggered()), this, SLOT(showPgxconsole()));

    editor_action = new QAction(QIcon(qApp->applicationDirPath().append("/icons/editor.png")), MainWin::tr("SQL Editor"), this);
    editor_action->setShortcuts(QKeySequence::SaveAs);
    editor_action->setEnabled(false);
    editor_action->setStatusTip(MainWin::tr("Launch SQL editor"));
    connect(editor_action, SIGNAL(triggered()), this, SLOT(showPgxeditor()));

    search_action = new QAction(QIcon(qApp->applicationDirPath().append("/icons/search.png")), MainWin::tr("Search for items"), this);
    //search_action->setShortcuts(QKeySequence::Find);
    search_action->setStatusTip(MainWin::tr("Highlight items that match"));
    search_action->setCheckable(true);
    connect(search_action, SIGNAL(triggered()), this, SLOT(search()));

    fullscreen_action = new QAction(QIcon(qApp->applicationDirPath().append("/icons/fullscreen.png")), MainWin::tr("Fullscreen"), this);
    fullscreen_action->setShortcut(QKeySequence(Qt::Key_F11));
    fullscreen_action->setStatusTip(MainWin::tr("Occupy full desktop"));
    connect(fullscreen_action, SIGNAL(triggered()), this, SLOT(toggleFullscreen()));

    treeview_action = new QAction(QIcon(qApp->applicationDirPath().append("/icons/treeview.png")), MainWin::tr("Treeview"), this);
    treeview_action->setShortcut(QKeySequence(Qt::Key_T));
    treeview_action->setStatusTip(MainWin::tr("Show database contents in tree view"));
    treeview_action->setCheckable(true);
    connect(treeview_action, SIGNAL(triggered()), this, SLOT(showTreeview()));

    columnview_action = new QAction(QIcon(qApp->applicationDirPath().append("/icons/columnview.png")), MainWin::tr("Column view"), this);
    columnview_action->setShortcut(QKeySequence(Qt::Key_T));
    columnview_action->setStatusTip(MainWin::tr("Show database contents in column view"));
    columnview_action->setCheckable(true);
    connect(columnview_action, SIGNAL(triggered()), this, SIGNAL(showColumnView()));

    layout_view_actiongroup = new QActionGroup(this);
    layout_view_actiongroup->addAction(treeview_action);
    layout_view_actiongroup->addAction(columnview_action);
    columnview_action->setChecked(true);

    tableview_action = new QAction(QIcon(qApp->applicationDirPath().append("/icons/table.png")), MainWin::tr("Tables"), this);
    tableview_action->setShortcut(QKeySequence(Qt::Key_T));
    tableview_action->setStatusTip(MainWin::tr("Show database tables"));
    tableview_action->setCheckable(true);
    connect(tableview_action, SIGNAL(triggered()), this, SLOT(showAllTables()));

    viewview_action = new QAction(QIcon(qApp->applicationDirPath().append("/icons/view2.png")), MainWin::tr("Views"), this);
    viewview_action->setShortcut(QKeySequence(Qt::Key_V));
    viewview_action->setStatusTip(MainWin::tr("Show database views"));
    viewview_action->setCheckable(true);
    connect(viewview_action, SIGNAL(triggered()), this, SLOT(showAllViews()));

    functionview_action = new QAction(QIcon(qApp->applicationDirPath().append("/icons/function.png")), MainWin::tr("Functions"), this);
    functionview_action->setShortcut(QKeySequence(Qt::Key_F));
    functionview_action->setStatusTip(MainWin::tr("Show database functions"));
    functionview_action->setCheckable(true);
    connect(functionview_action, SIGNAL(triggered()), this, SLOT(showAllFunctions()));

    content_view_actiongroup = new QActionGroup(this);
    content_view_actiongroup->addAction(tableview_action);
    content_view_actiongroup->addAction(viewview_action);
    content_view_actiongroup->addAction(functionview_action);
    if(display_setting.compare("view") == 0)
        viewview_action->setChecked(true);
    else if(display_setting.compare("function") == 0)
        functionview_action->setChecked(true);
    else
        tableview_action->setChecked(true);

    english_action = new QAction(QIcon(qApp->applicationDirPath().append("/icons/ok.png")), MainWin::tr("English (default)"), this);
    connect(english_action, SIGNAL(triggered()), this, SLOT(setLanguageDefault()));
    if(language_setting.compare("english"))
        english_action->setIconVisibleInMenu(false);

    japanese_action = new QAction(QIcon(qApp->applicationDirPath().append("/icons/ok.png")), MainWin::tr("Japanese"), this);
    connect(japanese_action, SIGNAL(triggered()), this, SLOT(setLanguageJapanese()));
    if(language_setting.compare("japanese"))
        japanese_action->setIconVisibleInMenu(false);

    french_action = new QAction(QIcon(qApp->applicationDirPath().append("/icons/ok.png")), MainWin::tr("French"), this);
    connect(french_action, SIGNAL(triggered()), this, SLOT(setLanguageFrench()));
    if(language_setting.compare("french"))
        french_action->setIconVisibleInMenu(false);

    help_action = new QAction(QIcon(qApp->applicationDirPath().append("/icons/help.png")), MainWin::tr("&Help"), this);
    help_action->setShortcuts(QKeySequence::HelpContents);
    help_action->setStatusTip(MainWin::tr("Help contents"));
    //connect(help_action, SIGNAL(triggered()), this, SLOT(help()));

    about_action = new QAction(MainWin::tr("&License key/About"), this);
    about_action->setStatusTip(MainWin::tr("License key and other information"));
    connect(about_action, SIGNAL(triggered()), this, SLOT(about()));
}

void MainWin::createMenus()
{
    file_menu = menuBar()->addMenu(MainWin::tr("&File"));
    file_menu->addAction(new_file_action);
    file_menu->addAction(open_file_action);
    file_menu->addAction(save_file_action);
    file_menu->addAction(save_file_as_action);
    file_menu->addSeparator();
    file_menu->addAction(exit_action);

    tool_menu = menuBar()->addMenu(MainWin::tr("&Tools"));
    tool_menu->addAction(connection_properties_action);
    tool_menu->addAction(console_action);
    tool_menu->addAction(editor_action);
    tool_menu->addAction(search_action);
    tool_menu->addAction(fullscreen_action);

    display_menu = tool_menu->addMenu(MainWin::tr("&Display"));
    display_menu->addAction(tableview_action);
    display_menu->addAction(viewview_action);
    display_menu->addAction(functionview_action);

    language_menu = tool_menu->addMenu(MainWin::tr("&Language"));
    language_menu->addAction(english_action);
    language_menu->addAction(japanese_action);
    //language_menu->addAction(french_action);

    menuBar()->addSeparator();

    help_menu = menuBar()->addMenu(MainWin::tr("&Help"));
    help_menu->addAction(help_action);
    //help_menu->addAction(about_action);
}

void MainWin::showTableView(Database *database, Schema *schema, Table *table)
{
    table->setColumnList();
    QSettings settings("pgXplorer","pgXplorer");
    QString table_name = schema->getName();
    table_name.append(".\"");
    table_name.append(table->getName());
    table_name.append("\"");
    TableView *table_view = new TableView(database, table_name, table_name, table->getColumnList(), false, Qt::WA_DeleteOnClose);
    table_view_list.append(table_view);
    QObject::connect(table_view, SIGNAL(tableViewClosing(TableView*)), this, SLOT(tableViewClosed(TableView*)));
    QObject::connect(this, SIGNAL(languageChanged(QEvent*)), table_view, SLOT(languageChanged(QEvent*)));

    QPoint pos = settings.value("tableview_pos", QPoint(100, 100)).toPoint();
    QSize size = settings.value("tableview_size", QSize(1024, 768)).toSize();
    table_view->resize(size);
    table_view->move(pos);
    table_view->show();
    bool is_maximized = settings.value("tableview_maximized", false).toBool();
    if(is_maximized) {
       showMaximized();
    }
}

void MainWin::showViewView(Database *database, Schema *schema, View *view)
{
    QString view_name = schema->getName();
    view_name.append(".\"");
    view_name.append(view->getName());
    view_name.append("\"");
    TableView *view_view = new TableView(database, view_name, view_name, view->getColumnList(), true, Qt::WA_DeleteOnClose);
    view_view_list.append(view_view);
    QObject::connect(view_view, SIGNAL(tableViewClosing(TableView*)), this, SLOT(tableViewClosed(TableView*)));
    QObject::connect(this, SIGNAL(languageChanged(QEvent*)), view_view, SLOT(languageChanged(QEvent*)));

    QSettings settings("pgXplorer","pgXplorer");
    QPoint pos = settings.value("tableview_pos", QPoint(100, 100)).toPoint();
    QSize size = settings.value("tableview_size", QSize(1024, 768)).toSize();
    view_view->resize(size);
    view_view->move(pos);
    view_view->show();
}

void MainWin::clearTableView(Database *database, Schema *schema, Table *table)
{
    int ret = QMessageBox::warning(this, MainWin::tr("pgXplorer"),
                                    MainWin::tr("This action will destroy all data in this table and cannot be undone.\n"
                                       "Do you want to continue?"),
                                    QMessageBox::Ok | QMessageBox::Cancel);
    if(ret == QMessageBox::Cancel)
        return;
    QString table_name = schema->getName();
    table_name.append(".\"");
    table_name.append(table->getName());
    table_name.append("\"");

    {
        QSqlDatabase database_connection = QSqlDatabase::addDatabase("QPSQL", QString("clear ").append(table_name));
        database_connection.setHostName(database->getHost());
        database_connection.setPort(database->getPort().toInt());
        database_connection.setDatabaseName(database->getName());
        database_connection.setUserName(database->getUser());
        database_connection.setPassword(database->getPassword());
        if (!database_connection.open()) {
            QMessageBox::critical(0, MainWin::tr("Database error"),
                MainWin::tr("Unable to establish a database connection.\n"
                         "No PostgreSQL support.\n"), QMessageBox::Cancel);
            return;
        }
        QSqlQuery query(QString("TRUNCATE TABLE ").append(table_name), database_connection);
        query.exec();
        //if(query.lastError().isValid())
        //    QMessageBox::critical(0, MainWin::tr("Database error"),
        //    query.lastError().databaseText(), QMessageBox::Close);
    }
    QSqlDatabase::removeDatabase(QString("clear ").append(table_name));
}

void MainWin::dropTable(Database *database, Schema *schema, Table *table)
{
    int ret = QMessageBox::warning(this, MainWin::tr("pgXplorer"),
                                    MainWin::tr("This action will destroy this table and all its data and cannot be undone.\n"
                                       "Do you want to continue?"),
                                    QMessageBox::Ok | QMessageBox::Cancel);
    if(ret == QMessageBox::Cancel)
        return;
    QString table_name = schema->getName();
    table_name.append(".\"");
    table_name.append(table->getName());
    table_name.append("\"");

    {
        QSqlDatabase database_connection = QSqlDatabase::addDatabase("QPSQL", "drop " + table_name);
        database_connection.setHostName(database->getHost());
        database_connection.setPort(database->getPort().toInt());
        database_connection.setDatabaseName(database->getName());
        database_connection.setUserName(database->getUser());
        database_connection.setPassword(database->getPassword());
        if (!database_connection.open()) {
            QMessageBox::critical(0, MainWin::tr("Database error"),
                MainWin::tr("Unable to establish a database connection.\n"
                         "No PostgreSQL support.\n"), QMessageBox::Close);
            return;
        }
        QSqlQuery query(QString("DROP TABLE ").append(table_name), database_connection);
        query.exec();
        //if(query.lastError().isValid())
        //    QMessageBox::critical(0, MainWin::tr("Database error"),
        //    query.lastError().databaseText(), QMessageBox::Close);
    }
    QSqlDatabase::removeDatabase("drop " + table_name);
    schema->resetTablesVertically2();
}

void MainWin::dropView(Database *database, Schema *schema, View *view)
{
    int ret = QMessageBox::warning(this, MainWin::tr("pgXplorer"),
                                    MainWin::tr("This action will destroy this view and all its data and cannot be undone.\n"
                                       "Do you want to continue?"),
                                    QMessageBox::Ok | QMessageBox::Cancel);
    if(ret == QMessageBox::Cancel)
        return;
    QString view_name = schema->getName();
    view_name.append(".\"");
    view_name.append(view->getName());
    view_name.append("\"");

    {
        QSqlDatabase database_connection = QSqlDatabase::addDatabase("QPSQL", "drop " + view_name);
        database_connection.setHostName(database->getHost());
        database_connection.setPort(database->getPort().toInt());
        database_connection.setDatabaseName(database->getName());
        database_connection.setUserName(database->getUser());
        database_connection.setPassword(database->getPassword());
        if (!database_connection.open()) {
            QMessageBox::critical(0, MainWin::tr("Database error"),
                MainWin::tr("Unable to establish a database connection.\n"
                         "No PostgreSQL support.\n"), QMessageBox::Close);
            return;
        }
        QSqlQuery query(QString("DROP VIEW ").append(view_name), database_connection);
        query.exec();
        //if(query.lastError().isValid())
        //    QMessageBox::critical(0, MainWin::tr("Database error"),
        //    query.lastError().databaseText(), QMessageBox::Close);
    }
    QSqlDatabase::removeDatabase("drop " + view_name);
    schema->resetViewsVertically2();
}

void MainWin::dropFunction(Database *database, Schema *schema, Function *function)
{
    int ret = QMessageBox::warning(this, MainWin::tr("pgXplorer"),
                                    MainWin::tr("This action will destroy this function and cannot be undone.\n"
                                       "Do you want to continue?"),
                                    QMessageBox::Ok | QMessageBox::Cancel);
    if(ret == QMessageBox::Cancel)
        return;
    QString function_name = schema->getName();
    function_name.append(".\"");
    function_name.append(function->getName());
    function_name.append("\"");

    {
        QSqlDatabase database_connection = QSqlDatabase::addDatabase("QPSQL", "drop " + function_name);
        database_connection.setHostName(database->getHost());
        database_connection.setPort(database->getPort().toInt());
        database_connection.setDatabaseName(database->getName());
        database_connection.setUserName(database->getUser());
        database_connection.setPassword(database->getPassword());
        if (!database_connection.open()) {
            QMessageBox::critical(0, MainWin::tr("Database error"),
                MainWin::tr("Unable to establish a database connection.\n"
                         "No PostgreSQL support.\n"), QMessageBox::Close);
            return;
        }
        QSqlQuery query(QString("DROP FUNCTION ").append(function_name).append(function->getArgTypesToText()), database_connection);
        query.exec();
        //if(query.lastError().isValid())
        //    QMessageBox::critical(0, MainWin::tr("Database error"),
        //    query.lastError().databaseText(), QMessageBox::Close);
    }
    QSqlDatabase::removeDatabase("drop " + function_name);
    schema->resetFunctionsVertically2();
}

void MainWin::showQueryView(Database *database, QString command)
{
    QueryView *query_view = new QueryView(database, command);
    query_view_list.append(query_view);
    QObject::connect(query_view, SIGNAL(queryViewClosing(QueryView*)), this, SLOT(queryViewClosed(QueryView*)));

    QSettings settings("pgXplorer","pgXplorer");
    QPoint pos = settings.value("queryview_pos", QPoint(100, 100)).toPoint();
    QSize size = settings.value("queryview_size", QSize(1024, 768)).toSize();
    query_view->resize(size);
    query_view->move(pos);
    query_view->show();
}

void MainWin::showFunctionEditor(Schema *schema, Function *function)
{
    QString function_name = function->getName();
    QString function_args = function->getArgs();
    QString function_arg_types = function->getArgTypes();
    PgxEditor *pgxeditor = new PgxEditor(database, function_name);
    pgxeditor_list.append(pgxeditor);

    QSettings settings("pgXplorer","pgXplorer");
    QObject::connect(pgxeditor, SIGNAL(showQueryView(Database *, QString)), this, SLOT(showQueryView(Database*, QString)));
    QObject::connect(pgxeditor, SIGNAL(pgxeditorClosing(PgxEditor*)), this, SLOT(pgxeditorClosed(PgxEditor*)));
    QObject::connect(pgxeditor, SIGNAL(newPgxeditor()), this, SLOT(showPgxeditor()));
    QObject::connect(pgxeditor, SIGNAL(newPgxeditor(QString)), this, SLOT(showPgxeditor(QString)));

    QString function_definition;
    {
        QSqlDatabase database_connection = QSqlDatabase::addDatabase("QPSQL", "function definition " + function_name);
        database_connection.setHostName(database->getHost());
        database_connection.setPort(database->getPort().toInt());
        database_connection.setDatabaseName(database->getName());
        database_connection.setUserName(database->getUser());
        database_connection.setPassword(database->getPassword());
        if (!database_connection.open()) {
            QMessageBox::critical(0, MainWin::tr("Database error"),
                MainWin::tr("Unable to establish a database connection.\n"
                         "No PostgreSQL support.\n"), QMessageBox::Cancel);
            return;
        }
        QSqlQueryModel temp_query_model;
        temp_query_model.setQuery(QString("SELECT pg_get_functiondef(p.oid) \
                                          FROM pg_proc p JOIN pg_namespace n \
                                          ON pronamespace = n.oid WHERE proname = '")
                                          + function_name +
                                          QString("' AND nspname = '" + schema->getName() + "' "
                                          "AND proargtypes='" + function_arg_types + "'"), database_connection);

        function_definition = temp_query_model.data(temp_query_model.index(0,0)).toString();
    }
    QSqlDatabase::removeDatabase("function definition " + function_name);
    pgxeditor->setTitle(schema->getName().append(".").append(function_name));
    pgxeditor->setText(function_definition, false);
    pgxeditor->moveCursor(QTextCursor::Start);
    pgxeditor->ensureCursorVisible();
    pgxeditor->ensurePolished();

    QPoint pos = settings.value("pgxeditor_pos", QPoint(100, 100)).toPoint();
    QSize size = settings.value("pgxeditor_size", QSize(640, 480)).toSize();

    pgxeditor->setResizePos(size, pos);
}

void MainWin::showPgxeditor()
{
    PgxEditor *pgxeditor = new PgxEditor(database, "");
    pgxeditor_list.append(pgxeditor);

    QSettings settings("pgXplorer","pgXplorer");
    QObject::connect(pgxeditor, SIGNAL(showQueryView(Database *, QString)), this, SLOT(showQueryView(Database*, QString)));
    QObject::connect(pgxeditor, SIGNAL(pgxeditorClosing(PgxEditor*)), this, SLOT(pgxeditorClosed(PgxEditor*)));
    QObject::connect(pgxeditor, SIGNAL(newPgxeditor()), this, SLOT(showPgxeditor()));
    QObject::connect(pgxeditor, SIGNAL(newPgxeditor(QString)), this, SLOT(showPgxeditor(QString)));

    pgxeditor->moveCursor(QTextCursor::Start);
    pgxeditor->ensureCursorVisible();
    pgxeditor->ensurePolished();

    QPoint pos = settings.value("pgxeditor_pos", QPoint(100, 100)).toPoint();
    QSize size = settings.value("pgxeditor_size", QSize(640, 480)).toSize();

    pgxeditor->setResizePos(size, pos);
}

void MainWin::showPgxeditor(QString query)
{
    PgxEditor *pgxeditor = new PgxEditor(database, "");
    pgxeditor_list.append(pgxeditor);

    QSettings settings("pgXplorer","pgXplorer");
    QObject::connect(pgxeditor, SIGNAL(showQueryView(Database *, QString)), this, SLOT(showQueryView(Database*, QString)));
    QObject::connect(pgxeditor, SIGNAL(pgxeditorClosing(PgxEditor*)), this, SLOT(pgxeditorClosed(PgxEditor*)));
    QObject::connect(pgxeditor, SIGNAL(newPgxeditor()), this, SLOT(showPgxeditor()));
    QObject::connect(pgxeditor, SIGNAL(newPgxeditor(QString)), this, SLOT(showPgxeditor(QString)));

    pgxeditor->ensureCursorVisible();
    pgxeditor->ensurePolished();

    QPoint pos = settings.value("pgxeditor_pos", QPoint(100, 100)).toPoint();
    QSize size = settings.value("pgxeditor_size", QSize(640, 480)).toSize();

    pgxeditor->setResizePos(size, pos);
    pgxeditor->setText(query, false);
    pgxeditor->moveCursor(QTextCursor::End);
}

void MainWin::adjustSearchBoxPosition()
{
    if(canvas->verticalScrollBar()->isVisible())
       search_box->setGeometry(this->width()-canvas->verticalScrollBar()->width()-202,0,200,search_box->height());
    else
        search_box->setGeometry(this->width()-202,0,200,search_box->height());
}
