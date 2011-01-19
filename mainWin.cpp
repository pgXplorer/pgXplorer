#include <QDateTime>
#include <QMainWindow>
#include <QStatusBar>
#include <QMenuBar>
#include <QApplication>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QDebug>
#include "database.h"
#include "schema.h"
#include "table.h"
#include "tableview.h"
#include "queryview.h"
#include "schemaLink.h"
#include "tableLink.h"
#include "properties.h"
#include "mainWin.h"
#include "libpq-fe.h"
#include "pgxconsole.h"

FigureEditor::FigureEditor(
        QGraphicsScene& s, QWidget* parent,
        const char* name, Qt::WindowFlags f) :
    QGraphicsView(&s,parent)
{
    setObjectName(name);
    setWindowFlags(f);
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
}

void FigureEditor::clear()
{
    scene()->clear();
}

void FigureEditor::wheelEvent(QWheelEvent *wheelEvent)
{
    wheelEvent->ignore();
}

MainWin::MainWin(QGraphicsScene& c, QWidget* parent, const char* name, Qt::WindowFlags f) :
    QMainWindow(parent,name,f), scn(c)
{
    edt = new FigureEditor(scn, this);
    QMenuBar* menubar = menuBar();
    QMenu* file = new QMenu( menubar );
    file->insertItem("&New db", this, SLOT(newDb()), Qt::CTRL+Qt::Key_N);
    file->insertItem("&Open db", this, SLOT(openDbFile()), Qt::CTRL+Qt::Key_O);
    file->insertItem("&Save db", this, SLOT(saveDbFile()), Qt::CTRL+Qt::Key_S);
    file->addSeparator();
    file->insertItem("E&xit", qApp, SLOT(quit()), Qt::CTRL+Qt::Key_Q);
    menubar->insertItem("&File", file);
    QMenu* edit = new QMenu( menubar );
    edit->insertItem("SQL Console", this, SLOT(console()), Qt::ALT+Qt::Key_C);
    menubar->insertItem("&Tools", edit);
    QMenu* view = new QMenu( menubar );
    view->insertItem("&Enlarge", this, SLOT(enlarge()), Qt::SHIFT+Qt::CTRL+Qt::Key_Plus);
    view->insertItem("Shr&ink", this, SLOT(shrink()), Qt::SHIFT+Qt::CTRL+Qt::Key_Minus);
    view->insertSeparator();
    view->insertItem("&Zoom in", this, SLOT(zoomIn()), Qt::CTRL+Qt::Key_Plus);
    view->insertItem("Zoom &out", this, SLOT(zoomOut()), Qt::CTRL+Qt::Key_Minus);
    view->insertItem("Translate left", this, SLOT(moveL()), Qt::CTRL+Qt::Key_Left);
    view->insertItem("Translate right", this, SLOT(moveR()), Qt::CTRL+Qt::Key_Right);
    view->insertItem("Translate up", this, SLOT(moveU()), Qt::CTRL+Qt::Key_Up);
    view->insertItem("Translate down", this, SLOT(moveD()), Qt::CTRL+Qt::Key_Down);
    view->insertItem("&Default", this, SLOT(default1()), Qt::CTRL+Qt::Key_Home);
    menubar->insertItem("&View", view);
    menubar->addSeparator();
    QMenu* help = new QMenu( menubar );
    help->insertItem("&About", this, SLOT(help()), Qt::Key_F1);
    help->setItemChecked(dbf_id, TRUE);
    menubar->insertItem("&Help",help);
    setCentralWidget(edt);
    readSettings();
    prn = 0;
}

void MainWin::closeEvent(QCloseEvent *event)
{
    writeSettings();
    qApp->quit();
}

void MainWin::readSettings()
{
    QSettings settings("pgXplorer", "pgXplorer");
    QPoint pos = settings.value("pos", QPoint(100, 100)).toPoint();
    QSize size = settings.value("size", QSize(1024, 768)).toSize();
    resize(size);
    move(pos);
}

void MainWin::writeSettings()
{
    this->getScn().children();
    QSettings settings("pgXplorer", "pgXplorer");
    settings.setValue("pos", pos());
    settings.setValue("size", size());
}

void MainWin::openDbFile()
{
    clear();
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                     "C:/",
                                                     tr("Database (*.pgx)"));
    //QString fileName = QFileDialog::getOpenFileName(this);
    if(fileName.isEmpty())
        return;
    open(fileName);
}

void MainWin::open(QString fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
             return;
    QDataStream fileStrm(&file);
    quint32 magic;
    fileStrm >> magic;
    if(magic != 0xA0B0C0D0) {
        QMessageBox* fileErr = new QMessageBox("pgXplorer", "Not a database file.",
                                               QMessageBox::Critical, 1, 0, 0, this, 0, FALSE );
        fileErr->setButtonText(1, "Close");
        fileErr->show();
        return;
    }
    qint32 version;
    fileStrm >> version;
    if(version > 100) {
        QMessageBox* fileVerErr = new QMessageBox("pgXplorer", "Database file version not supported.",
                                               QMessageBox::Critical, 1, 0, 0, this, 0, FALSE );
        fileVerErr->setButtonText(1, "Close");
        fileVerErr->show();
        return;
    }
    Database* db = new Database();
    db->setPos(scn.width()/2, scn.height()/2);
    scn.addItem(db);
    QObject::connect(db, SIGNAL(expand(Database*)), this, SLOT(addSchema(Database*)));
    QObject::connect(db, SIGNAL(collapse(Database*)), this, SLOT(delSchema(Database*)));
    QString srv;
    fileStrm >> srv;
    quint32 port;
    fileStrm >> port;
    QString datab;
    fileStrm >> datab;
    QString user;
    fileStrm >> user;
    QString pass;
    fileStrm >> pass;
    db->setConnProps(srv, port, datab, user, pass);
}

void MainWin::saveDbFile()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Open File"),
                                                     "C:/",
                                                     tr("Database (*.pgx)"));
    if(fileName.isEmpty())
        return;
    save(fileName);
}

void MainWin::save(QString fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly))
             return;
    QSqlDatabase db = QSqlDatabase::database(QSqlDatabase::connectionNames().at(0));
    QDataStream fileStrm(&file);

    // Set magic number = 0xA0B0C0D0
    fileStrm << (quint32)0xA0B0C0D0;

    // Set version number as 100
    fileStrm << (qint32)100;

    // Set Qt compatible version(s)
    fileStrm.setVersion(QDataStream::Qt_4_7);

    // Copy the relevant data
    fileStrm << db.hostName();
    fileStrm << (qint32)db.port();
    fileStrm << db.databaseName();
    fileStrm << db.userName();
    fileStrm << db.password();
}

MainWin::~MainWin()
{
    delete prn;
}

void MainWin::wheelEvent(QWheelEvent *wheelEvent)
{
    // Capture wheel events to zoom-in or
    // zoom-out the canvas.
    wheelEvent->accept();
    // zoom out
    if (wheelEvent->delta()>0)
        zoomOut();
    // zoom in
    else
        zoomIn();
}

void MainWin::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    // BEGIN TODO 3
    // Rubber band selection of canvas's graphic objects
    origin.setX(event->pos().x());
    origin.setY(event->pos().y());
    if(!rubberBand)
        rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
    rubberBand->setGeometry(QRect(origin, QSize()));
    rubberBand->show();
    emit clicked();
    // END TODO 3
}

void MainWin::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    // BEGIN TODO 3
    rubberBand->setGeometry(QRect(origin, event->pos().toPoint()).normalized());
    // END TODO 3
}

void MainWin::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    // BEGIN TODO 3
    rubberBand->hide();
    // END TODO 3
}

void MainWin::newView()
{
    MainWin *mainwin = new MainWin(scn, 0, 0, Qt::WDestructiveClose);
    mainwin->show();
}

void MainWin::clear()
{
    /*! Wipe the canvas clean of all objects.
     */
    edt->clear();
}

void MainWin::help()
{
    // Help message box
    static QMessageBox* about = new QMessageBox( "pgXplorer",
            "<h3>The PostgreSQL explorer</h3>"
            "<ul>"
                "<li> explore pg."
            "</ul>", QMessageBox::Information, 1, 0, 0, this, 0, FALSE );
    about->setButtonText( 1, "Close" );
    about->show();
}

void MainWin::enlarge()
{
    scn.setSceneRect(0, 0, scn.width()*4/3, scn.height()*4/3);
}

void MainWin::shrink()
{
    scn.setSceneRect(0, 0, qMax(scn.width()*3/4, qreal(1.0)), qMax(scn.height()*3/4, qreal(1.0)));
}

void MainWin::zoomIn()
{
    edt->scale( 1.1, 1.1 );
}

void MainWin::zoomOut()
{
    edt->scale( 0.9, 0.9 );
}

void MainWin::default1()
{
    edt->scale( 1, 1 );
}

void MainWin::moveL()
{
    edt->translate( -16, 0 );
}

void MainWin::moveR()
{
    edt->translate( +16, 0 );
}

void MainWin::moveU()
{
    edt->translate( 0, -16 );
}

void MainWin::moveD()
{
    edt->translate( 0, +16 );
}

void MainWin::newDb()
{
    // Add a database object to canvas
    Database* db = new Database();
    db->setPos(scn.width()/2, scn.height()/2);
    scn.addItem(db);
    db->setHost("127.0.0.1");
    db->setPort("5432");
    db->setUser("postgres");
    db->setPassword("");
    PropDialog *props = new PropDialog(db);
    props->setGeometry((width())/2, (height())/2, 256, 288);
    props->exec();
    QObject::connect(db, SIGNAL(expand(Database*)), this, SLOT(addSchema(Database*)));
    QObject::connect(db, SIGNAL(collapse(Database*)), this, SLOT(delSchema(Database*)));
}

void MainWin::console()
{
    // Pseudo SQL console.
    // Goal: Accept all possible Postgresql related
    // DDL and DML commands and produce output
    // accordingly.
    PgxConsole *pgxconsole = new PgxConsole();
    pgxconsole->insertPlainText("");
    pgxconsole->show();
}

void MainWin::addSchema(Database* db)
{
    // If schema list not populated, populate it.
    // Afterwards, display the schema list.
    if(db->childItems().count() == 0) {
        int schListSiz = db->getSchList().size();
        for (int i = 0; i < schListSiz; ++i) {
            Schema* sch = new Schema(db, db->getSchList().at(i));
            SchemaLink* lnk = new SchemaLink(db, sch);
            lnk->setParentItem(db);
            lnk->setFlag(QGraphicsItem::ItemStacksBehindParent);
            QObject::connect(sch, SIGNAL(expand(Schema*)), this, SLOT(addTable(Schema*)));
            QObject::connect(sch, SIGNAL(collapse(Schema*)), this, SLOT(delTable(Schema*)));
        }
    }
    else {
        QList<QGraphicsItem*> L = db->childItems();
        int LSiz = L.size();
        for (int i = 0; i < LSiz; ++i)
            L.at(i)->setVisible(true);
    }
}

void MainWin::delSchema(Database* db)
{
    // Remove a schema from the database.
    QList<QGraphicsItem*> L = db->childItems();
    int LSiz = L.size();
    for (int i = 0; i < LSiz; ++i)
        L.at(i)->setVisible(false);
}

void MainWin::addTable(Schema* sch)
{
    // If table list not populated, populate it.
    // Afterwards, display the table list.
    if(sch->childItems().count() == 0) {
        int tblListSiz = sch->getTblList().size();
        for (int i = 0; i < tblListSiz; ++i) {
            Table* tbl = new Table(sch, sch->getTblList().at(i));
            TableLink* lnk = new TableLink(sch, tbl);
            lnk->setParentItem(sch);
            lnk->setFlag(QGraphicsItem::ItemStacksBehindParent);
            QObject::connect(tbl, SIGNAL(expand(Schema*,Table*)), tbl, SLOT(showView(Schema*,Table*)));
            QObject::connect(tbl, SIGNAL(collapse(Schema*,Table*)), tbl, SLOT(hideView(Schema*,Table*)));
        }
    }
    else {
        QList<QGraphicsItem*> L = sch->childItems();
        int LSiz = L.size();
        for (int i = 0; i < LSiz; ++i)
            L.at(i)->setVisible(true);
    }
}

void MainWin::delTable(Schema* sch)
{
    // Remove a schema from the database.
    QList<QGraphicsItem*> L = sch->childItems();
    for (int i = 0; i < L.size(); ++i)
        L.at(i)->setVisible(false);
        //Table* t = qgraphicsitem_cast<Table *>(L.at(i));
}

void LaunchTable::showTbl(Schema* sch, Table *tbl)
{
    //QSqlQueryModel* model = new QSqlQueryModel;
    // Construct proper tablename name for data
    // retrieval as well as the titlebar.
    QString tblName = sch->getName();
    tblName.append(".");
    tblName.append(tbl->getName());
    //QTime t;
    //t.start();
    //model->setQuery("SELECT * FROM " + tblName + " LIMIT 200000 OFFSET 0;");
    //model->setEditStrategy(QSqlQueryModel::OnManualSubmit);
    //model->select();
    //model->rowCount();
    //tbl->setModel(model);
    TableView* tview = new TableView(tblName, tblName, Qt::WA_DeleteOnClose);
    tview->show();
}

void LaunchTable::runQry(QString query)
{
    // Check if query is pure whitespace
    if(query.simplified().isEmpty())
        return;
    QSqlQueryModel* model = new QSqlQueryModel;
    model->setQuery(query);
    if(model->lastError().isValid()) {
        QMessageBox::critical(0, qApp->tr("Query error"),
            model->lastError().text(), QMessageBox::Cancel);
        return;
    }
    QueryView* view = new QueryView(0, model, query, 0, 0, 0, Qt::WA_DeleteOnClose);
    exec();
}
