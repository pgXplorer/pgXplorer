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

QString MainWin::hostS = "127.0.0.1";
int MainWin::portS = 5432;
QString MainWin::dbnameS = "";
QString MainWin::userS = "postgres";
QString MainWin::passwordS = "";

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
    menubar = menuBar();
    fileMenu = new QMenu( menubar );
    fileMenu->insertItem("&New", this, SLOT(newFile()), Qt::CTRL+Qt::Key_N);

    fileMenu->insertItem("&Open", this, SLOT(openFile()), Qt::CTRL+Qt::Key_O);
    fileMenu->insertItem("&Save", this, SLOT(saveFile()), Qt::CTRL+Qt::Key_S);
    fileMenu->insertItem("&Save As...", this, SLOT(saveFileAs()), Qt::CTRL+Qt::SHIFT+Qt::Key_S);
    fileMenu->addSeparator();
    fileMenu->insertItem("E&xit", qApp, SLOT(quit()), Qt::CTRL+Qt::Key_Q);
    menubar->insertItem("&File", fileMenu);
    toolMenu = new QMenu( menubar );
    toolMenu->insertItem("SQL Console", this, SLOT(console()), Qt::ALT+Qt::Key_C);
    toolMenu->insertItem("Fullscreen", this, SLOT(fullscreen()), Qt::Key_F11);
    menubar->insertItem("&Tools", toolMenu);
    /*viewMenu = new QMenu( menubar );
    viewMenu->insertItem("&Enlarge", this, SLOT(enlarge()), Qt::SHIFT+Qt::CTRL+Qt::Key_Plus);
    viewMenu->insertItem("Shr&ink", this, SLOT(shrink()), Qt::SHIFT+Qt::CTRL+Qt::Key_Minus);
    viewMenu->insertSeparator();
    viewMenu->insertItem("&Zoom in", this, SLOT(zoomIn()), Qt::CTRL+Qt::Key_Plus);
    viewMenu->insertItem("Zoom &out", this, SLOT(zoomOut()), Qt::CTRL+Qt::Key_Minus);
    viewMenu->insertItem("Translate left", this, SLOT(moveL()), Qt::CTRL+Qt::Key_Left);
    viewMenu->insertItem("Translate right", this, SLOT(moveR()), Qt::CTRL+Qt::Key_Right);
    viewMenu->insertItem("Translate up", this, SLOT(moveU()), Qt::CTRL+Qt::Key_Up);
    viewMenu->insertItem("Translate down", this, SLOT(moveD()), Qt::CTRL+Qt::Key_Down);
    viewMenu->insertItem("&Default", this, SLOT(default1()), Qt::CTRL+Qt::Key_Home);
    menubar->insertItem("&View", viewMenu);*/
    menubar->addSeparator();
    helpMenu = new QMenu( menubar );
    helpMenu->insertItem("&About", this, SLOT(help()), Qt::Key_F1);
    helpMenu->setItemChecked(dbf_id, TRUE);
    menubar->insertItem("&Help", helpMenu);

    QShortcut* shortcut_restore_win = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(shortcut_restore_win, SIGNAL(activated()), this, SLOT(restore()));

    setCentralWidget(edt);
    readSettings();
    prn = 0;
}

void MainWin::closeEvent(QCloseEvent *event)
{
    {
        QSqlDatabase sqldb = QSqlDatabase::database("base");
        sqldb.close();
    }
    QSqlDatabase::removeDatabase("base");
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

void MainWin::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                     "C:/",
                                                     tr("Database (*.pgx)"));
    //QString fileName = QFileDialog::getOpenFileName(this);
    if(fileName.isEmpty())
        return;
    clear();
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
    Database* db = new Database(this);
    db->setPos(scn.width()/2, scn.height()/2);
    scn.addItem(db);

    fileStrm >> hostS;
    fileStrm >> portS;
    fileStrm >> dbnameS;
    fileStrm >> userS;
    fileStrm >> passwordS;
    db->setConnProps(hostS, portS, dbnameS, userS, passwordS);

    this->setWindowTitle("pgXplorer - " + fileName);
}

void MainWin::saveFile()
{

}

void MainWin::saveFileAs()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Open File"),
                                                     "C:/",
                                                     tr("Database (*.pgx)"));
    if(fileName.isEmpty())
        return;
    save(fileName);
    saveState(fileName);
}

int MainWin::saveState(QString fileName)
{

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

void MainWin::newFile()
{
    // Add a database object to canvas
    Database* db = new Database(this);
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
    PgxConsole *pgxconsole = new PgxConsole(0);
    pgxconsole->insertPlainText("");
    pgxconsole->setDbPros(MainWin::hostS, MainWin::portS, MainWin::dbnameS, MainWin::userS, MainWin::passwordS);
    pgxconsole->show();
}

void MainWin::fullscreen()
{
    this->showFullScreen();
    menubar->hide();
}

void MainWin::restore()
{
    this->showNormal();
    menubar->show();
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
            QObject::connect(sch, SIGNAL(expand(Database*, Schema*)), this, SLOT(addTable(Database*, Schema*)));
            QObject::connect(sch, SIGNAL(collapse(Database*, Schema*)), this, SLOT(delTable(Database*, Schema*)));
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

void MainWin::addTable(Database* db, Schema* sch)
{
    // If table list not populated, populate it.
    // Afterwards, display the table list.
    if(sch->childItems().count() == 0) {
        int tblListSiz = sch->getTblList().size();
        for (int i = 0; i < tblListSiz; ++i) {
            Table* tbl = new Table(db, sch, sch->getTblList().at(i));
            TableLink* lnk = new TableLink(sch, tbl);
            lnk->setParentItem(sch);
            lnk->setFlag(QGraphicsItem::ItemStacksBehindParent);
            QObject::connect(tbl, SIGNAL(expand(Database*, Schema*, Table*)), tbl, SLOT(showView(Database*, Schema*, Table*)));
            QObject::connect(tbl, SIGNAL(collapse(Database*, Schema*, Table*)), tbl, SLOT(hideView(Database*, Schema*, Table*)));
        }
    }
    else {
        QList<QGraphicsItem*> L = sch->childItems();
        int LSiz = L.size();
        for (int i = 0; i < LSiz; ++i)
            L.at(i)->setVisible(true);
    }
}

void MainWin::delTable(Database* db, Schema* sch)
{
    // Remove a schema from the database.
    QList<QGraphicsItem*> L = sch->childItems();
    for (int i = 0; i < L.size(); ++i)
        L.at(i)->setVisible(false);
        //Table* t = qgraphicsitem_cast<Table *>(L.at(i));
}

void LaunchTable::showTbl(Database* db, Schema* sch, Table *tbl)
{
    // Construct proper tablename name for data
    // retrieval as well as the titlebar.
    QString tblName = sch->getName();
    tblName.append(".");
    tblName.append(tbl->getName());

    TableView* tview = new TableView(db, tblName, tblName, Qt::WA_DeleteOnClose);
    //tview->setParent();
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
