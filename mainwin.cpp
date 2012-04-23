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

#include <QDateTime>
#include <QMainWindow>
#include <QStatusBar>
#include <QMenuBar>
#include <QApplication>
#include <QPainter>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QDebug>
#include "database.h"
#include "schema.h"
#include "table.h"
#include "tableview.h"
#include "designview.h"
#include "viewview.h"
#include "queryview.h"
#include "schemalink.h"
#include "tablelink.h"
#include "functionlink.h"
#include "connectionproperties.h"
#include "mainwin.h"
#include "pgxconsole.h"
#include "pgxeditor.h"
#include "licensedialog.h"
#include "help.h"

bool MainWin::session_unsaved = false;

GraphicsView::GraphicsView(
        QGraphicsScene& s, QWidget *parent,
        const char *name, Qt::WindowFlags f) :
    QGraphicsView(&s, parent)
{
    setMouseTracking(false);
    setAcceptDrops(true);
    zoom = false;
    zoom_cursor = QCursor(QPixmap(":/icons/zoom.png"));
    setOptimizationFlags(QGraphicsView::DontSavePainterState);
    setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    setObjectName(name);
    setWindowFlags(f);
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
}

void GraphicsView::clear()
{
    scene()->clear();
}

void GraphicsView::wheelEvent(QWheelEvent *wheelEvent)
{
    wheelEvent->ignore();
}

void GraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    if((event->buttons() == Qt::RightButton) && (zoom == false))
        zoom = true;
    QGraphicsView::mouseMoveEvent(event);
}

void GraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::RightButton && (zoom == true)) {
        fitInView(scene()->selectionArea().boundingRect(), Qt::KeepAspectRatio);
        scene()->setSelectionArea(QPainterPath());
    }
    QGraphicsView::mouseReleaseEvent(event);
}

void GraphicsView::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void GraphicsView::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}

void GraphicsView::dropEvent(QDropEvent *event)
{
    event->acceptProposedAction();
    foreach(QUrl file, event->mimeData()->urls())
        mainwin->newPgxplorer(file.toLocalFile());
}

MainWin::MainWin(QWidget *parent, const QString arg1, Qt::WindowFlags f)
{
    //checkForUpdates();

    setAttribute(Qt::WA_DeleteOnClose);
    readSettings();
    createActions();
    createMenus();

    help = new Help(qApp->applicationDirPath());

    setAcceptDrops(true);

    if(language() == MainWin::Japanese)
        setLanguageJapanese();
    else if(language() == MainWin::French)
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

    graphics_view = new GraphicsView(scene, this);
    graphics_view->setSceneRect(QRectF());

    QShortcut *shortcut_search = new QShortcut(QKeySequence::Find, this);
    connect(shortcut_search, SIGNAL(activated()), this, SLOT(search()));
    QShortcut *shortcut_restore_win = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(shortcut_restore_win, SIGNAL(activated()), this, SLOT(restore()));
    QShortcut *shortcut_default_view = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_0), this);
    connect(shortcut_default_view, SIGNAL(activated()), this, SLOT(noZoom()));
    QShortcut *shortcut_zoom_in = new QShortcut(QKeySequence::ZoomIn, this);
    connect(shortcut_zoom_in, SIGNAL(activated()), this, SLOT(zoomIn()));
    QShortcut *shortcut_zoom_out = new QShortcut(QKeySequence::ZoomOut, this);
    connect(shortcut_zoom_out, SIGNAL(activated()), this, SLOT(zoomOut()));

    connect(graphics_view, SIGNAL(search()), this, SLOT(search()));

    setCentralWidget(graphics_view);

    printer = 0;

    search_box = new QLineEdit(graphics_view);
    search_box->hide();

    QPalette palette;
    palette.setColor(search_box->backgroundRole(), Qt::yellow);
    search_box->setPalette(palette);

    completer = new QCompleter;
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    search_box->setCompleter(completer);

    QSettings settings("pgXplorer","pgXplorer");
    bool is_maximized = settings.value("mainwin_maximized", false).toBool();
    if(is_maximized)
       showMaximized();

    newFile();

    if(!arg1.isEmpty())
        open(arg1);
}

void MainWin::checkForUpdates()
{
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    connect(manager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(replyFinished(QNetworkReply*)));

    QNetworkRequest request;
    request.setUrl(QUrl("http://pgxplorer.com/"));
    request.setRawHeader("User-Agent", "pgXplorer updater 1.0");

    QNetworkReply *reply = manager->post(request, "");
    connect(reply, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(slotError(QNetworkReply::NetworkError)));
    connect(reply, SIGNAL(sslErrors(QList<QSslError>)),
            this, SLOT(slotSslErrors(QList<QSslError>)));
}

void MainWin::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        file_menu->setTitle(MainWin::tr("&File"));
        tool_menu->setTitle(MainWin::tr("&Tools"));
        windows_menu->setTitle(MainWin::tr("&Windows"));
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
        english_action->setText(MainWin::tr("English (default)"));
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

    lang = static_cast<MainWin::Language> (settings.value("language", MainWin::English).toInt());
    disp_mode = static_cast<MainWin::DisplayMode> (settings.value("display", MainWin::Tables).toInt());
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
    settings.setValue("language", (int) lang);
    settings.setValue("display", (int) disp_mode);
}

void MainWin::openFile()
{
    if(session_unsaved)
    {
        QMessageBox msgBox(this);
        msgBox.setText(tr("The document has been modified."));
        msgBox.setInformativeText(tr("Do you want to save your changes?"));
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);
        int ret = msgBox.exec();

        switch (ret) {
            case QMessageBox::Save:
                if(saveFileAs() == false)
                    return;
                break;
            case QMessageBox::Discard:
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
    if(version > 010) {
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
    if(!database_collapsed) {
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
    setWindowTitle("pgXplorer - " + file_name);
    database_file.close();
    save_file_action->setEnabled(false);
    session_unsaved = false;
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
}

bool MainWin::saveFileAs()
{
    QString file_name = QFileDialog::getSaveFileName(this, MainWin::tr("Open File"),
                                                     "C:/",
                                                     MainWin::tr("Database (*.pgx);;All files (*)"));
    if(file_name.isEmpty())
        return false;
    save(file_name);
    setWindowTitle("pgXplorer - " + file_name);
    return true;
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

    // Set version number as 010
    database_file_stream << (qint32) 010;

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

    if(database->getDatabaseCollapsed()) {
        database_file_stream << true;
    }
    else {
        database_file_stream << false;
        /*
        database_file_stream << QLatin1String("schema list");
        foreach(Schema *schema, db->getSchemaList())
        {
            database_file_stream << schema->getName();
            if(!schema->getSchemaCollapsed())
            {
                database_file_stream << true;
                database_file_stream << QLatin1String("table list");
                foreach(Table *table, schema->getTableList())
                {
                    database_file_stream << table;
                }
                database_file_stream << QLatin1String("end table list");
            }
            else
            {
                database_file_stream << false;
            }
        }
        database_file_stream << QLatin1String("end schema list");
        */
    }
    save_file_action->setEnabled(false);
    session_unsaved = false;
}

MainWin::~MainWin()
{
    //delete printer;
}

void MainWin::resizeEvent(QResizeEvent *)
{
    if(search_box->isVisible())
        adjustSearchBoxPosition();
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
    wheelEvent->accept();

    //Capture Control key pressed with mouse wheel
    //events to zoom-in or zoom-out the canvas.
    if(wheelEvent->modifiers() == Qt::ControlModifier) {
        //zoom out
        if (wheelEvent->delta()>0)
            zoomOut(mapFromGlobal(wheelEvent->globalPos()));
        //zoom in
        else
            zoomIn(mapFromGlobal(wheelEvent->globalPos()));

        //Adjust search box position if scrollbars disappear.
        if(search_box->isVisible())
            adjustSearchBoxPosition();
    }
    //Shift key pressed mouse wheel should scroll the canvas left-right.
    else if(wheelEvent->modifiers() == Qt::ShiftModifier) {
        int value = graphics_view->horizontalScrollBar()->value();
        graphics_view->horizontalScrollBar()->setValue(value - (wheelEvent->delta() >> 1));
    }
    //Normal mouse wheel should scroll the canvas up-down.
    else if(wheelEvent->modifiers() == Qt::NoModifier) {
        int value = graphics_view->verticalScrollBar()->value();
        graphics_view->verticalScrollBar()->setValue(value - (wheelEvent->delta() >> 1));
    }
}

void MainWin::contextMenuEvent(QContextMenuEvent *event)
{
    if(graphics_view->isZoom()) {
        graphics_view->setZoom(false);
        return;
    }
    QMenu menu;
    menu.addAction(MainWin::tr("Default view"));
    menu.addAction(MainWin::tr("Zoom in"));
    menu.addAction(MainWin::tr("Zoom out"));
    menu.addAction(MainWin::tr("Fit view"));
    menu.addSeparator();
    if(database->getDatabaseStatus()) {
        menu.addAction(tr("Refresh"));
        //menu.addAction(tr("New schema..."));
    }
    QAction *a = menu.exec(event->globalPos());
    if(a && QString::compare(a->text(),MainWin::tr("Default view"))==0)
        noZoom();
    else if(a && QString::compare(a->text(),MainWin::tr("Zoom in"))==0)
        zoomIn(event->globalPos());
    else if(a && QString::compare(a->text(),MainWin::tr("Zoom out"))==0)
        zoomOut(event->globalPos());
    else if(a && QString::compare(a->text(),MainWin::tr("Fit view"))==0)
        fitView();
    else if(a && QString::compare(a->text(),MainWin::tr("Refresh"))==0)
        database->populateDatabase();
    else if(a && QString::compare(a->text(),MainWin::tr("New schema..."))==0)
        newSchema();
}

void MainWin::closeEvent(QCloseEvent *event)
{
    //QSqlDatabase::removeDatabase(QLatin1String("base").append(QString::number(database->getId())));
    writeSettings();
    if(quitApp() == false)
        event->ignore();
}

void MainWin::newSchema()
{
    Schema *new_schema = new Schema(this, database, QLatin1String(""), database->getSchemaCount(), database->getSchemaCount()+1);

    GraphicsTextItem *new_schema_name = new GraphicsTextItem;

    QTimeLine *timer = new QTimeLine(300);
    timer->setFrameRange(5, 10);

    QGraphicsItemAnimation *animation = new QGraphicsItemAnimation;
    animation->setItem(new_schema);
    animation->setTimeLine(timer);

    for (int i = 5; i < 10; ++i)
        animation->setScaleAt(i/10.0, (i+1)/10.0, (i+1)/10.0);

    new_schema_name->setPlainText(tr("New schema"));
    new_schema_name->setParentItem(new_schema);
    new_schema_name->setPos(-new_schema->boundingRect().width()/2+5,-10);
    new_schema_name->setTextInteractionFlags(Qt::TextEditorInteraction);
    new_schema_name->setFocus(Qt::TabFocusReason);
    QTextCursor tc = new_schema_name->textCursor();
    tc.select(QTextCursor::Document);
    new_schema_name->setTextCursor(tc);
    connect(new_schema_name, SIGNAL(enterPressed(QString,QString)), this, SLOT(createSchema(QString,QString)));

    database->appendSchemaList(new_schema);
    //database->setSchemaCount(database->getSchemaCount()+1);
    new_schema->resetPos();
    graphics_view->ensureVisible(new_schema);

    timer->start();
}

void MainWin::createSchema(QString, QString new_schema_name)
{
    {
        QSqlDatabase database_connection = QSqlDatabase::addDatabase("QPSQL", QString("create schema ").append(new_schema_name));
        database_connection.setHostName(database->getHost());
        database_connection.setPort(database->getPort().toInt());
        database_connection.setDatabaseName(database->getName());
        database_connection.setUserName(database->getUser());
        database_connection.setPassword(database->getPassword());
        if (!database_connection.open()) {
            QMessageBox::critical(this, MainWin::tr("Database error"),
                MainWin::tr("Unable to establish a database connection.\n"
                            "No PostgreSQL support.\n"), QMessageBox::Cancel);
            return;
        }
        QSqlQuery query(QString("CREATE SCHEMA ").append("\"" + new_schema_name + "\""), database_connection);
        query.exec();
        if(query.lastError().isValid()) {
            //QMessageBox::critical(0, MainWin::tr("Database error"),
            //query.lastError().databaseText(), QMessageBox::Close);
        }
    }
    QSqlDatabase::removeDatabase(QString("create schema ").append(new_schema_name));
    database->populateDatabase();
}

void MainWin::createTable(QString schema_name, GraphicsTextItem *text_item)
{
    if(text_item->toPlainText().isEmpty()) {
        text_item->setFocus();
        return;
    }
    foreach(Schema *schema, database->getSchemaList()) {
        if(schema->getName().compare(schema_name) == 0)
            foreach(Table *table, schema->getTableList())
                if(table->getName().compare(text_item->toPlainText()) == 0) {
                    int response = QMessageBox::critical(this, MainWin::tr("Database error"),
                                    MainWin::tr("A table with this name \"<html><b>%1</b></html>\" already exists in schema \"<html><b>%2</b></html>\".\n"
                                    "Please choose another name or discard.\n").arg(text_item->toPlainText(), schema_name), QMessageBox::Retry | QMessageBox::Discard);
                    if(response == QMessageBox::Discard)
                        database->populateDatabase();
                    else
                        text_item->setFocus();
                    return;
                }
    }

    {
        QSqlDatabase database_connection = QSqlDatabase::addDatabase("QPSQL", QString("create table ").append(schema_name).append(text_item->toPlainText()));
        database_connection.setHostName(database->getHost());
        database_connection.setPort(database->getPort().toInt());
        database_connection.setDatabaseName(database->getName());
        database_connection.setUserName(database->getUser());
        database_connection.setPassword(database->getPassword());
        if (!database_connection.open()) {
            QMessageBox::critical(this, MainWin::tr("Database error"),
                MainWin::tr("Unable to establish a database connection.\n"
                            "No PostgreSQL support.\n"), QMessageBox::Cancel);
            return;
        }
        QSqlQuery query(QString("CREATE TABLE ").append("\"" + schema_name + "\".").append("\"" + text_item->toPlainText() + "\"()"),
                        database_connection);
        query.exec();
        if(query.lastError().isValid()) {
            //QMessageBox::critical(0, MainWin::tr("Database error"),
            //query.lastError().databaseText(), QMessageBox::Close);
        }
    }
    QSqlDatabase::removeDatabase(QString("create table ").append(schema_name).append(text_item->toPlainText()));
    database->populateDatabase();
}

void MainWin::clearSchemas()
{
    foreach(Schema *schema, database->getSchemaList()) {
        foreach(Table *table, schema->getTableList())
            delete table;
        schema->getTableList().clear();
        foreach(View *view, schema->getViewList())
            delete view;
        schema->getViewList().clear();
        foreach(Function *function, schema->getFunctionList())
            delete function;
        schema->getFunctionList().clear();
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

void MainWin::clearDesignViewList()
{
    foreach(DesignView *design_view, design_view_list)
        design_view->close();
    design_view_list.clear();
}

void MainWin::clearViewViewList()
{
    foreach(ViewView *view_view, view_view_list)
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

void MainWin::clear()
{
    graphics_view->clear();
}

void MainWin::about()
{
    LicenseDialog *license_dalog = new LicenseDialog(this);
    license_dalog->exec();
}

void MainWin::showHelp()
{
    QString help_url = QLatin1String("file:///");
    help_url.append(qApp->applicationDirPath());
    help_url.append("/documentation/pgXplorer_help.pdf");
    QDesktopServices::openUrl(QUrl(help_url));
}

void MainWin::fitView()
{
    graphics_view->fitInView(scene.sceneRect(), Qt::KeepAspectRatio);
}

void MainWin::document_changed()
{
    session_unsaved = true;
}

void MainWin::tableViewClosed(TableView *table_view)
{
    table_view_list.removeOne(table_view);
}

void MainWin::designViewClosed(DesignView *design_view)
{
    design_view_list.removeOne(design_view);
}

void MainWin::viewViewClosed(ViewView *view_view)
{
    view_view_list.removeOne(view_view);
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
    graphics_view->resetTransform();
    graphics_view->centerOn(database);
}

void MainWin::zoomIn()
{
    //graphics_view->centerOn((centre.toPoint()));
    graphics_view->scale( 1.1, 1.1 );
}

void MainWin::zoomOut()
{
    //graphics_view->centerOn((centre.toPoint()));
    graphics_view->scale( 0.9, 0.9 );
}

void MainWin::zoomIn(const QPointF centre)
{
    //graphics_view->centerOn((centre.toPoint()));
    graphics_view->scale( 1.1, 1.1 );
}

void MainWin::zoomOut(const QPointF centre)
{
    //graphics_view->centerOn((centre.toPoint()));
    graphics_view->scale( 0.9, 0.9 );
}

bool MainWin::newFile()
{
    if(session_unsaved) {
        QMessageBox msgBox(this);
        msgBox.setText(tr("The database session has been modified."));
        msgBox.setInformativeText(tr("Do you want to save your changes?"));
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);
        int ret = msgBox.exec();

        switch (ret) {
            case QMessageBox::Save:
                if(saveFileAs() == false)
                    return false;
                delete database;
                break;
            case QMessageBox::Discard:
                delete database;
                break;
            case QMessageBox::Cancel:
                return false;
                break;
            default:
                break;
        }
    }
    newDatabase();
    return true;
}

void MainWin::newPgxplorer()
{
    newPgxplorer(QString());
}

void MainWin::newPgxplorer(QString file)
{
    QProcess *new_pgxplorer = new QProcess;
    new_pgxplorer->start(qApp->applicationFilePath(), QStringList() << file);
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

    //Add a database object to canvas
    database = new Database(this, database_id);
    scene.addItem(database);
    if(database->setConnectionProperties(host, port, dbname, username, password) == false)
        return false;
    database->setToolTip(tr("Host") + ": " + host + "\n" +
                         tr("Port") + ": " + QString::number(port) + "\n" +
                         tr("User") + ": " + username);
    if(tableview_action->isChecked())
        showAllTables();
    else if(viewview_action->isChecked())
        showAllViews();
    else if(functionview_action->isChecked())
        showAllFunctions();
    graphics_view->translate(0,-37.5);
    graphics_view->centerOn(database);
    graphics_view->setDragMode(QGraphicsView::RubberBandDrag);
    graphics_view->setMainWin(this);
    console_action->setEnabled(true);
    editor_action->setEnabled(true);
    save_file_action->setEnabled(true);
    save_file_as_action->setEnabled(true);
    session_unsaved = true;
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

bool MainWin::quitApp()
{
    if(session_unsaved) {
        QMessageBox msgBox(this);
        msgBox.setText(tr("The database session has been modified."));
        msgBox.setInformativeText(tr("Do you want to save your changes?"));
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);
        int ret = msgBox.exec();

        switch (ret) {
            case QMessageBox::Save:
                if(saveFileAs() == false)
                    return false;
                delete database;
                break;
            case QMessageBox::Discard:
                delete database;
                break;
            case QMessageBox::Cancel:
                return false;
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
    return true;
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
    /*foreach(Schema *schema, database->getSchemaList()) {
        foreach (Table *table, schema->getTableList())
            pgxconsole->appendCompleterList(table->getName());
        foreach (View *view, schema->getViewList())
            pgxconsole->appendCompleterList(view->getName());
        foreach (Function *function, schema->getFunctionList())
            pgxconsole->appendCompleterList(function->getName());
    }*/

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

MainWin::Language MainWin::language() const
{
    return lang;
}

void MainWin::setLanguage(Language language)
{
    lang = language;
}

MainWin::DisplayMode MainWin::displayMode() const
{
    return disp_mode;
}

void MainWin::setDisplayMode(DisplayMode display_mode)
{
    disp_mode = display_mode;
}

void MainWin::setLanguageDefault()
{
    qt_translator.load(qApp->applicationDirPath().append("/qt_ja"));
    translator.load(qApp->applicationDirPath().append("/pgXplorer_ja"));
    qApp->removeTranslator(&translator);
    qApp->removeTranslator(&qt_translator);
    japanese_action->setIconVisibleInMenu(false);

    qt_translator.load(qApp->applicationDirPath().append("/qt_fr"));
    translator.load(qApp->applicationDirPath().append("/pgXplorer_fr"));
    qApp->removeTranslator(&translator);
    qApp->removeTranslator(&qt_translator);
    french_action->setIconVisibleInMenu(false);
    english_action->setIconVisibleInMenu(true);
    setLanguage(MainWin::English);
}

void MainWin::setLanguageJapanese()
{
    qt_translator.load(qApp->applicationDirPath().append("/qt_ja"));
    translator.load(qApp->applicationDirPath().append("/pgXplorer_ja"));
    qApp->installTranslator(&translator);
    qApp->installTranslator(&qt_translator);
    japanese_action->setIconVisibleInMenu(true);
    french_action->setIconVisibleInMenu(false);
    english_action->setIconVisibleInMenu(false);
    setLanguage(MainWin::Japanese);
}

void MainWin::setLanguageFrench()
{
    qt_translator.load(qApp->applicationDirPath().append("/qt_fr"));
    translator.load(qApp->applicationDirPath().append("/pgXplorer_fr"));
    qApp->installTranslator(&translator);
    qApp->installTranslator(&qt_translator);
    japanese_action->setIconVisibleInMenu(false);
    french_action->setIconVisibleInMenu(true);
    english_action->setIconVisibleInMenu(false);
    setLanguage(MainWin::French);
}

void MainWin::restore()
{
    if(search_action->isChecked()) {
        search_box->clear();
        search_action->setChecked(false);
        search_box->setVisible(false);
    }
    else if(isFullScreen())
        toggleFullscreen();
    else
        database->populateDatabase();
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
    setDisplayMode(MainWin::Tables);
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
    setDisplayMode(MainWin::Views);
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
    setDisplayMode(MainWin::Functions);
    delete completer;
    completer = new QCompleter(function_completer_list, this);
    search_box->setCompleter(completer);
    QList<Schema*> schema_list = database->getSchemaList();
    foreach (Schema *schema, schema_list)
        showFunctions(schema);
    scene.setSceneRect(QRectF());
}

void MainWin::populateWindowMenu()
{
    windows_menu->clear();
    windows.clear();
    foreach(TableView *table_view, table_view_list) {
        QAction* window = new QAction(QIcon(":/icons/table.png"), table_view->tableName(), this);
        connect(window, SIGNAL(triggered()), table_view, SLOT(bringOnTop()));
        windows.append(window);
    }
    foreach(ViewView *view_view, view_view_list) {
        QAction* window = new QAction(QIcon(":/icons/view2.png"), view_view->viewName(), this);
        connect(window, SIGNAL(triggered()), view_view, SLOT(bringOnTop()));
        windows.append(window);
    }
    foreach(QueryView *query_view, query_view_list) {
        QAction* window = new QAction(query_view->query().left(10).append(QLatin1String("...")), this);
        connect(window, SIGNAL(triggered()), query_view, SLOT(bringOnTop()));
        windows.append(window);
    }
    foreach(DesignView *design_view, design_view_list) {
        QAction* window = new QAction(QIcon(":/icons/design.png"), design_view->tableName(), this);
        connect(window, SIGNAL(triggered()), design_view, SLOT(bringOnTop()));
        windows.append(window);
    }
    foreach(PgxEditor *pgxeditor, pgxeditor_list) {
        QString window_name;
        QAction* window;
        if(pgxeditor->editorName().isEmpty()) {
            window_name = pgxeditor->toPlainText().left(10).append(QLatin1String("..."));
            window = new QAction(QIcon(":/icons/editor.png"), window_name, this);
        }
        else {
            window_name = pgxeditor->editorName();
            window = new QAction(QIcon(":/icons/function.png"), window_name, this);
        }
        connect(window, SIGNAL(triggered()), pgxeditor->mainWin(), SLOT(bringOnTop()));
        windows.append(window);
    }
    foreach(PgxConsole *pgxconsole, pgxconsole_list) {
        QAction* window = new QAction(QIcon(":/icons/console.png"), pgxconsole->toPlainText().left(10).append(QLatin1String("...")), this);
        connect(window, SIGNAL(triggered()), pgxconsole->mainWin(), SLOT(bringOnTop()));
        windows.append(window);
    }
    if(windows.isEmpty())
        windows_menu->addAction(tr(""));
    windows_menu->addActions(windows);
}

void MainWin::hideFunctions(Schema *schema)
{
    foreach (Function *func, schema->getFunctionList())
        func->hide();
    schema->setSchemaCollapsed(true);
    schema->update();
}

void MainWin::newTable(Schema *schema)
{
    Table *new_table = new Table(database, schema, QLatin1String(""), schema->getTableCount(), QColor(100,50,50));
    GraphicsTextItem *new_table_name = new GraphicsTextItem;

    QTimeLine *timer = new QTimeLine(300);
    timer->setFrameRange(5, 10);

    QGraphicsItemAnimation *animation = new QGraphicsItemAnimation;
    animation->setItem(new_table);
    animation->setTimeLine(timer);

    for (int i = 5; i < 10; ++i)
        animation->setScaleAt(i/10.0, (i+1)/10.0, (i+1)/10.0);

    new_table_name->setPlainText(tr("New table"));
    new_table_name->setParentItem(new_table);
    new_table_name->setTextInteractionFlags(Qt::TextEditorInteraction);
    new_table_name->setFocus(Qt::TabFocusReason);
    new_table_name->setSchemaName(schema->getName());
    new_table_name->setPos(-new_table->boundingRect().width()/2+5,-10);
    QTextCursor tc = new_table_name->textCursor();
    tc.select(QTextCursor::Document);
    new_table_name->setTextCursor(tc);
    schema->appendTableList(new_table);
    schema->setTableCount(schema->getTableCount()+1);
    new_table->verticalPosition2();
    connect(new_table_name, SIGNAL(enterPressed(QString, GraphicsTextItem*)), this, SLOT(createTable(QString, GraphicsTextItem*)));
    graphics_view->ensureVisible(new_table);
    timer->start();
}

void MainWin::newFunction(Schema *schema)
{
    showPgxeditor(QLatin1String(" "), QString());
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
    new_file_action = new QAction(QIcon(":/icons/new.png"), MainWin::tr("&New"), this);
    new_file_action->setShortcuts(QKeySequence::New);
    new_file_action->setStatusTip(MainWin::tr("Create a new file"));
    connect(new_file_action, SIGNAL(triggered()), this, SLOT(newPgxplorer()));

    open_file_action = new QAction(QIcon(":/icons/open.png"), MainWin::tr("&Open..."), this);
    open_file_action->setShortcuts(QKeySequence::Open);
    open_file_action->setStatusTip(MainWin::tr("Open an existing file"));
    connect(open_file_action, SIGNAL(triggered()), this, SLOT(openFile()));

    save_file_action = new QAction(QIcon(":/icons/save.png"), MainWin::tr("&Save"), this);
    save_file_action->setShortcuts(QKeySequence::Save);
    save_file_action->setStatusTip(MainWin::tr("Save the database session to disk"));
    save_file_action->setEnabled(false);
    connect(save_file_action, SIGNAL(triggered()), this, SLOT(saveFile()));

    save_file_as_action = new QAction(QIcon(":/icons/save_as.png"), MainWin::tr("Save &As..."), this);
    save_file_as_action->setShortcuts(QKeySequence::SaveAs);
    save_file_as_action->setStatusTip(MainWin::tr("Save the database session under a new name"));
    save_file_as_action->setEnabled(false);
    connect(save_file_as_action, SIGNAL(triggered()), this, SLOT(saveFileAs()));

    exit_action = new QAction(MainWin::tr("E&xit"), this);
    exit_action->setShortcuts(QKeySequence::Quit);
    exit_action->setStatusTip(MainWin::tr("Exit the application"));
    connect(exit_action, SIGNAL(triggered()), this, SLOT(quitApp()));

    connection_properties_action = new QAction(QIcon(":/icons/properties.png"), MainWin::tr("Connection properties"), this);
    connection_properties_action->setShortcut(QKeySequence(Qt::Key_P));
    connection_properties_action->setStatusTip(MainWin::tr("Set connection properties"));
    connect(connection_properties_action, SIGNAL(triggered()), this, SLOT(openDatabaseProperties()));

    console_action = new QAction(QIcon(":/icons/console.png"), MainWin::tr("SQL Console"), this);
    console_action->setShortcut(QKeySequence(Qt::Key_C));
    console_action->setEnabled(false);
    console_action->setStatusTip(MainWin::tr("Launch SQL console"));
    connect(console_action, SIGNAL(triggered()), this, SLOT(showPgxconsole()));

    editor_action = new QAction(QIcon(":/icons/editor.png"), MainWin::tr("SQL Editor"), this);
    editor_action->setShortcut(QKeySequence(Qt::Key_E));
    editor_action->setEnabled(false);
    editor_action->setStatusTip(MainWin::tr("Launch SQL editor"));
    connect(editor_action, SIGNAL(triggered()), this, SLOT(showPgxeditor()));

    search_action = new QAction(QIcon(":/icons/search.png"), MainWin::tr("Search for items"), this);
    //search_action->setShortcuts(QKeySequence::Find);
    search_action->setStatusTip(MainWin::tr("Highlight items that match"));
    search_action->setCheckable(true);
    connect(search_action, SIGNAL(triggered()), this, SLOT(search()));

    fullscreen_action = new QAction(QIcon(":/icons/fullscreen.png"), MainWin::tr("Fullscreen"), this);
    fullscreen_action->setShortcut(QKeySequence(Qt::Key_F11));
    fullscreen_action->setStatusTip(MainWin::tr("Occupy full desktop"));
    connect(fullscreen_action, SIGNAL(triggered()), this, SLOT(toggleFullscreen()));

    treeview_action = new QAction(QIcon(":/icons/treeview.png"), MainWin::tr("Treeview"), this);
    treeview_action->setShortcut(QKeySequence(Qt::Key_T));
    treeview_action->setStatusTip(MainWin::tr("Show database contents in tree view"));
    treeview_action->setCheckable(true);
    connect(treeview_action, SIGNAL(triggered()), this, SLOT(showTreeview()));

    columnview_action = new QAction(QIcon(":/icons/columnview.png"), MainWin::tr("Column view"), this);
    columnview_action->setShortcut(QKeySequence(Qt::Key_T));
    columnview_action->setStatusTip(MainWin::tr("Show database contents in column view"));
    columnview_action->setCheckable(true);
    connect(columnview_action, SIGNAL(triggered()), this, SIGNAL(showColumnView()));

    layout_view_actiongroup = new QActionGroup(this);
    layout_view_actiongroup->addAction(treeview_action);
    layout_view_actiongroup->addAction(columnview_action);
    columnview_action->setChecked(true);

    tableview_action = new QAction(QIcon(":/icons/table.png"), MainWin::tr("Tables"), this);
    tableview_action->setShortcut(QKeySequence(Qt::Key_T));
    tableview_action->setStatusTip(MainWin::tr("Show database tables"));
    tableview_action->setCheckable(true);
    connect(tableview_action, SIGNAL(triggered()), this, SLOT(showAllTables()));

    viewview_action = new QAction(QIcon(":/icons/view2.png"), MainWin::tr("Views"), this);
    viewview_action->setShortcut(QKeySequence(Qt::Key_V));
    viewview_action->setStatusTip(MainWin::tr("Show database views"));
    viewview_action->setCheckable(true);
    connect(viewview_action, SIGNAL(triggered()), this, SLOT(showAllViews()));

    functionview_action = new QAction(QIcon(":/icons/function.png"), MainWin::tr("Functions"), this);
    functionview_action->setShortcut(QKeySequence(Qt::Key_F));
    functionview_action->setStatusTip(MainWin::tr("Show database functions"));
    functionview_action->setCheckable(true);
    connect(functionview_action, SIGNAL(triggered()), this, SLOT(showAllFunctions()));

    content_view_actiongroup = new QActionGroup(this);
    content_view_actiongroup->addAction(tableview_action);
    content_view_actiongroup->addAction(viewview_action);
    content_view_actiongroup->addAction(functionview_action);

    if(displayMode() == MainWin::Views)
        viewview_action->setChecked(true);
    else if(displayMode() == MainWin::Functions)
        functionview_action->setChecked(true);
    else
        tableview_action->setChecked(true);

    english_action = new QAction(QIcon(":/icons/ok.png"), MainWin::tr("English (default)"), this);
    connect(english_action, SIGNAL(triggered()), this, SLOT(setLanguageDefault()));
    if(language() != MainWin::English)
        english_action->setIconVisibleInMenu(false);

    japanese_action = new QAction(QIcon(":/icons/ok.png"), MainWin::tr("Japanese"), this);
    connect(japanese_action, SIGNAL(triggered()), this, SLOT(setLanguageJapanese()));
    if(language() != MainWin::Japanese)
        japanese_action->setIconVisibleInMenu(false);

    french_action = new QAction(QIcon(":/icons/ok.png"), MainWin::tr("French"), this);
    connect(french_action, SIGNAL(triggered()), this, SLOT(setLanguageFrench()));
    if(language() != MainWin::French)
        french_action->setIconVisibleInMenu(false);

    help_action = new QAction(QIcon(":/icons/help.png"), MainWin::tr("&Help"), this);
    help_action->setShortcuts(QKeySequence::HelpContents);
    help_action->setStatusTip(MainWin::tr("Help contents"));
    connect(help_action, SIGNAL(triggered()), this, SLOT(showHelp()));

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

    windows_menu = menuBar()->addMenu(MainWin::tr("&Windows"));
    connect(windows_menu, SIGNAL(aboutToShow()), this, SLOT(populateWindowMenu()));

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
    //table->setColumnData();
    table->copyPrimaryKey();
    QSettings settings("pgXplorer","pgXplorer");
    QString table_name = schema->getName();
    table_name.append(".\"");
    table_name.append(table->getName());
    table_name.append("\"");
    TableView *table_view = new TableView(database, table_name, table_name, table->getColumnList(), table->getPrimaryKey(), table->getColumnTypes(), table->getColumnLengths(), false, Qt::WA_DeleteOnClose);
    table_view_list.append(table_view);
    QObject::connect(table_view, SIGNAL(tableViewClosing(TableView*)), this, SLOT(tableViewClosed(TableView*)));
    QObject::connect(this, SIGNAL(languageChanged(QEvent*)), table_view, SLOT(languageChanged(QEvent*)));

    QPoint pos = settings.value("tableview_pos", QPoint(100, 100)).toPoint();
    QSize size = settings.value("tableview_size", QSize(1024, 768)).toSize();
    table_view->resize(size);
    table_view->move(pos);
    table_view->show();
    bool is_maximized = settings.value("tableview_maximized", false).toBool();
    if(is_maximized)
       table_view->showMaximized();
}

void MainWin::showDesignView(Database *database, Schema *schema, Table *table)
{
    //table->setColumnData();
    table->copyPrimaryKey();
    QSettings settings("pgXplorer","pgXplorer");
    QString table_name = schema->getName();
    table_name.append(".\"");
    table_name.append(table->getName());
    table_name.append("\"");
    foreach(DesignView *design_view, design_view_list)
        if(design_view->tableName().compare(table_name) == 0) {
            design_view->bringOnTop();
            return;
        }

    DesignView *design_view = new DesignView(database, table, table_name, table_name, table->getColumnList(), table->getPrimaryKey(), table->getColumnTypes(), table->getColumnLengths(), table->getColumnNulls(), false, Qt::WA_DeleteOnClose);
    design_view_list.append(design_view);
    QObject::connect(design_view, SIGNAL(designViewClosing(DesignView*)), this, SLOT(designViewClosed(DesignView*)));
    QObject::connect(this, SIGNAL(languageChanged(QEvent*)), design_view, SLOT(languageChanged(QEvent*)));

    QPoint pos = settings.value("designview_pos", QPoint(100, 100)).toPoint();
    QSize size = settings.value("designview_size", QSize(1024, 768)).toSize();
    design_view->resize(size);
    design_view->move(pos);
    design_view->show();
    bool is_maximized = settings.value("designview_maximized", false).toBool();
    if(is_maximized)
       design_view->showMaximized();
}

void MainWin::showViewView(Database *database, Schema *schema, View *view)
{
    view->setColumnData();
    QString view_name = schema->getName();
    view_name.append(".\"");
    view_name.append(view->getName());
    view_name.append("\"");
    ViewView *view_view = new ViewView(database, view_name, view_name, view->getColumnList(), view->getColumnTypes(), true, Qt::WA_DeleteOnClose);
    view_view_list.append(view_view);
    QObject::connect(view_view, SIGNAL(viewViewClosing(ViewView*)), this, SLOT(viewViewClosed(ViewView*)));
    QObject::connect(this, SIGNAL(languageChanged(QEvent*)), view_view, SLOT(languageChanged(QEvent*)));

    QSettings settings("pgXplorer", "pgXplorer");
    QPoint pos = settings.value("viewview_pos", QPoint(100, 100)).toPoint();
    QSize size = settings.value("viewview_size", QSize(1024, 768)).toSize();
    view_view->resize(size);
    view_view->move(pos);
    view_view->show();
    bool is_maximized = settings.value("viewview_maximized", false).toBool();
    if(is_maximized)
       view_view->showMaximized();
}

void MainWin::clearTableView(Database *database, Schema *schema, Table *table)
{
    QString table_name = schema->getName();
    table_name.append(".\"");
    table_name.append(table->getName());
    table_name.append("\"");
    QString message(MainWin::tr("Clear contents of table <html><b>%1</b></html>?\nThis action will destroy all data in this table and cannot be undone.\n"
                       "Do you want to continue?").arg(table_name));
    int ret = QMessageBox::warning(this, MainWin::tr("pgXplorer"), message,
                                    QMessageBox::Ok | QMessageBox::Cancel);
    if(ret == QMessageBox::Cancel)
        return;

    {
        QSqlDatabase database_connection = QSqlDatabase::addDatabase("QPSQL", QString("clear ").append(table_name));
        database_connection.setHostName(database->getHost());
        database_connection.setPort(database->getPort().toInt());
        database_connection.setDatabaseName(database->getName());
        database_connection.setUserName(database->getUser());
        database_connection.setPassword(database->getPassword());
        if (!database_connection.open()) {
            QMessageBox::critical(this, MainWin::tr("Database error"),
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
    QString table_name = schema->getName();
    table_name.append(".\"");
    table_name.append(table->getName());
    table_name.append("\"");
    QString message(MainWin::tr("Delete table <html><b>%1</b></html>?\nThis action will destroy this table and all its data and cannot be undone.\n"
                                "Do you want to continue?").arg(table_name));
    int ret = QMessageBox::warning(this, MainWin::tr("pgXplorer"), message,
                                    QMessageBox::Ok | QMessageBox::Cancel);
    if(ret == QMessageBox::Cancel)
        return;

    {
        QSqlDatabase database_connection = QSqlDatabase::addDatabase("QPSQL", "drop " + table_name);
        database_connection.setHostName(database->getHost());
        database_connection.setPort(database->getPort().toInt());
        database_connection.setDatabaseName(database->getName());
        database_connection.setUserName(database->getUser());
        database_connection.setPassword(database->getPassword());
        if (!database_connection.open()) {
            QMessageBox::critical(this, MainWin::tr("Database error"),
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
    QSqlDatabase::removeDatabase("DROP " + table_name);
    schema->resetTablesVertically2();
}

void MainWin::dropView(Database *database, Schema *schema, View *view)
{
    QString view_name = schema->getName();
    view_name.append(".\"");
    view_name.append(view->getName());
    view_name.append("\"");
    QString message(MainWin::tr("Delete view <html><b>%1</b></html>?\nThis action will destroy this view and all its data and cannot be undone.\n"
                                "Do you want to continue?").arg(view_name));
    int ret = QMessageBox::warning(this, MainWin::tr("pgXplorer"), message,
                                    QMessageBox::Ok | QMessageBox::Cancel);
    if(ret == QMessageBox::Cancel)
        return;

    {
        QSqlDatabase database_connection = QSqlDatabase::addDatabase("QPSQL", "drop " + view_name);
        database_connection.setHostName(database->getHost());
        database_connection.setPort(database->getPort().toInt());
        database_connection.setDatabaseName(database->getName());
        database_connection.setUserName(database->getUser());
        database_connection.setPassword(database->getPassword());
        if (!database_connection.open()) {
            QMessageBox::critical(this, MainWin::tr("Database error"),
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
    QString function_name = schema->getName();
    function_name.append(".\"");
    function_name.append(function->getName());
    function_name.append("\"");
    QString message(MainWin::tr("Delete function <html><b>%1</b></html>?\nThis action will destroy this function and cannot be undone.\n"
                                "Do you want to continue?").arg(function_name));
    int ret = QMessageBox::warning(this, MainWin::tr("pgXplorer"), message,
                                    QMessageBox::Ok | QMessageBox::Cancel);
    if(ret == QMessageBox::Cancel)
        return;

    {
        QSqlDatabase database_connection = QSqlDatabase::addDatabase("QPSQL", "drop " + function_name);
        database_connection.setHostName(database->getHost());
        database_connection.setPort(database->getPort().toInt());
        database_connection.setDatabaseName(database->getName());
        database_connection.setUserName(database->getUser());
        database_connection.setPassword(database->getPassword());
        if (!database_connection.open()) {
            QMessageBox::critical(this, MainWin::tr("Database error"),
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
    QString full_function_name = schema->getName();
    full_function_name.append(".\"");
    QString function_name = function->getName();
    full_function_name.append(function_name);
    full_function_name.append("\"");
    QString function_args = function->getArgs();
    QString function_arg_types = function->getArgTypes();
    PgxEditor *pgxeditor = new PgxEditor(database, full_function_name, function_args);

    pgxeditor_list.append(pgxeditor);

    QSettings settings("pgXplorer","pgXplorer");
    QObject::connect(pgxeditor, SIGNAL(showQueryView(Database *, QString)), this, SLOT(showQueryView(Database*, QString)));
    QObject::connect(pgxeditor, SIGNAL(pgxeditorClosing(PgxEditor*)), this, SLOT(pgxeditorClosed(PgxEditor*)));
    QObject::connect(pgxeditor, SIGNAL(newPgxeditor()), this, SLOT(showPgxeditor()));
    QObject::connect(pgxeditor, SIGNAL(newPgxeditor(QString)), this, SLOT(showPgxeditor(QString)));
    QObject::connect(this, SIGNAL(languageChanged(QEvent*)), pgxeditor, SLOT(languageChanged(QEvent*)));

    QString function_definition;
    {
        QSqlDatabase database_connection = QSqlDatabase::addDatabase("QPSQL", "function definition " + function_name);
        database_connection.setHostName(database->getHost());
        database_connection.setPort(database->getPort().toInt());
        database_connection.setDatabaseName(database->getName());
        database_connection.setUserName(database->getUser());
        database_connection.setPassword(database->getPassword());
        if (!database_connection.open()) {
            QMessageBox::critical(this, MainWin::tr("Database error"),
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
    pgxeditor->setTitle(full_function_name);
    pgxeditor->setText(function_definition, false);
    pgxeditor->moveCursor(QTextCursor::Start);
    pgxeditor->ensureCursorVisible();
    pgxeditor->ensurePolished();
    pgxeditor->hightlightFirstBlock();

    QPoint pos = settings.value("pgxeditor_pos", QPoint(100, 100)).toPoint();
    QSize size = settings.value("pgxeditor_size", QSize(640, 480)).toSize();

    pgxeditor->setResizePos(size, pos);
}

void MainWin::showViewEditor(Schema *schema, View *view)
{
    QString view_name = view->getName();
    PgxEditor *pgxeditor = new PgxEditor(database, "", "");
    pgxeditor_list.append(pgxeditor);

    QSettings settings("pgXplorer","pgXplorer");
    QObject::connect(pgxeditor, SIGNAL(showQueryView(Database *, QString)), this, SLOT(showQueryView(Database*, QString)));
    QObject::connect(pgxeditor, SIGNAL(pgxeditorClosing(PgxEditor*)), this, SLOT(pgxeditorClosed(PgxEditor*)));
    QObject::connect(pgxeditor, SIGNAL(newPgxeditor()), this, SLOT(showPgxeditor()));
    QObject::connect(pgxeditor, SIGNAL(newPgxeditor(QString)), this, SLOT(showPgxeditor(QString)));
    QObject::connect(this, SIGNAL(languageChanged(QEvent*)), pgxeditor, SLOT(languageChanged(QEvent*)));

    QString view_definition;
    {
        QSqlDatabase database_connection = QSqlDatabase::addDatabase("QPSQL", "view definition " + view_name);
        database_connection.setHostName(database->getHost());
        database_connection.setPort(database->getPort().toInt());
        database_connection.setDatabaseName(database->getName());
        database_connection.setUserName(database->getUser());
        database_connection.setPassword(database->getPassword());
        if (!database_connection.open()) {
            QMessageBox::critical(this, MainWin::tr("Database error"),
                MainWin::tr("Unable to establish a database connection.\n"
                            "No PostgreSQL support.\n"), QMessageBox::Cancel);
            return;
        }
        QSqlQueryModel temp_query_model;
        temp_query_model.setQuery(QString("SELECT definition \
                                          FROM pg_views v WHERE viewname = '")
                                          + view_name
                                          + QString("' AND schemaname = '" + schema->getName()
                                          + "' " ), database_connection);
        view_definition = temp_query_model.data(temp_query_model.index(0,0)).toString();
    }
    QSqlDatabase::removeDatabase("view definition " + view_name);
    pgxeditor->setTitle(schema->getName().append(".").append(view_name));
    pgxeditor->setText(view_definition, false);
    pgxeditor->moveCursor(QTextCursor::Start);
    pgxeditor->ensureCursorVisible();
    pgxeditor->ensurePolished();

    QPoint pos = settings.value("pgxeditor_pos", QPoint(100, 100)).toPoint();
    QSize size = settings.value("pgxeditor_size", QSize(640, 480)).toSize();

    pgxeditor->setResizePos(size, pos);
}

void MainWin::showPgxeditor()
{
    PgxEditor *pgxeditor = new PgxEditor(database, "", "");

    pgxeditor_list.append(pgxeditor);

    QSettings settings("pgXplorer","pgXplorer");
    QObject::connect(pgxeditor, SIGNAL(showQueryView(Database *, QString)), this, SLOT(showQueryView(Database*, QString)));
    QObject::connect(pgxeditor, SIGNAL(pgxeditorClosing(PgxEditor*)), this, SLOT(pgxeditorClosed(PgxEditor*)));
    QObject::connect(pgxeditor, SIGNAL(newPgxeditor()), this, SLOT(showPgxeditor()));
    QObject::connect(pgxeditor, SIGNAL(newPgxeditor(QString)), this, SLOT(showPgxeditor(QString)));
    QObject::connect(this, SIGNAL(languageChanged(QEvent*)), pgxeditor, SLOT(languageChanged(QEvent*)));

    pgxeditor->moveCursor(QTextCursor::Start);
    pgxeditor->ensureCursorVisible();
    pgxeditor->ensurePolished();

    QPoint pos = settings.value("pgxeditor_pos", QPoint(100, 100)).toPoint();
    QSize size = settings.value("pgxeditor_size", QSize(640, 480)).toSize();

    pgxeditor->setResizePos(size, pos);
}

void MainWin::showPgxeditor(QString query)
{
    PgxEditor *pgxeditor = new PgxEditor(database, "", "");
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

void MainWin::showPgxeditor(QString function_name, QString query)
{
    PgxEditor *pgxeditor = new PgxEditor(database, function_name, "");
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
    if(graphics_view->verticalScrollBar()->isVisible())
       search_box->setGeometry(this->width()-graphics_view->verticalScrollBar()->width()-202,0,200,search_box->height());
    else
        search_box->setGeometry(this->width()-202,0,200,search_box->height());
}
