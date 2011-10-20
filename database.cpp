#include "database.h"
#include "properties.h"

Database::Database(MainWin* mainwin)
{
    setStatus(false);
    setCollapsed(true);
    setFlag(QGraphicsItem::ItemIsMovable, false);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setToolTip(getName());
    setFlag(ItemSendsGeometryChanges);
    setCacheMode(DeviceCoordinateCache);
    setZValue(-1);
    connect(this, SIGNAL(setMainWinTitle(QString)), mainwin, SLOT(setWindowTitle(QString)));
    connect(this, SIGNAL(expand(Database*)), mainwin, SLOT(addSchema(Database*)));
    connect(this, SIGNAL(collapse(Database*)), mainwin, SLOT(delSchema(Database*)));
}

void Database::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if(!getStatus()) {
        setHost("127.0.0.1");
        setPort("5432");
        setUser("postgres");
        setPassword("");
        PropDialog *props = new PropDialog(this);
        props->exec();
        return;
    }
    if(getStatus() && getCollapsed()) {
        setCollapsed(false);
        emit expand(this);
    }
    else if(getStatus() && !getCollapsed()) {
        setCollapsed(true);
        emit collapse(this);
    }
    update();
}

void Database::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QMenu menu;
    //menu.setBackgroundColor(QColor(205,205,205));
    if(getCollapsed())
        menu.addAction("Expand");
    else
        menu.addAction("Collapse");
    menu.addAction(QApplication::translate("Database", "Remove", 0, QApplication::UnicodeUTF8));
    menu.addSeparator();
    menu.addAction(QApplication::translate("Database", "Properties", 0, QApplication::UnicodeUTF8));
    QAction *a = menu.exec(event->screenPos());
    if(a && QString::compare(a->text(),"Remove")==0) {
        if(QSqlDatabase::connectionNames().length()>0)
            QSqlDatabase::removeDatabase("base");
        emit setMainWinTitle("pgXplorer");
        delDatabase(this);
    }
    else if(a && QString::compare(a->text(),"Expand")==0) {
        setCollapsed(false);
        expand(this);
        update();
    }
    else if(a && QString::compare(a->text(),"Collapse")==0) {
        setCollapsed(true);
        collapse(this);
        update();
    }
    else if(a && QString::compare(a->text(),"Properties")==0) {
        PropDialog *props = new PropDialog(this);
        props->setWindowFlags(Qt::FramelessWindowHint);
        props->exec();
    }
}

void Database::delDatabase(Database *db)
{
    delete db;
}

bool Database::populate()
{
    QList<QString> schlist;
    setSchList(schlist);
    {
        QSqlQuery* query = new QSqlQuery(db);
        query->exec("SELECT DISTINCT nspname FROM pg_namespace WHERE \
                   nspname NOT LIKE 'pg_%' AND nspname<>'information_schema' \
                   ORDER BY nspname;");
        while (query->next())
             schlist << query->value(0).toString();
    }
    setSchList(schlist);
    setStatus(true);
    return true;
}

bool Database::setConnProps(const QString srv, const qint32 port, const QString datab, const QString user, const QString pass)
{
    if(QSqlDatabase::connectionNames().length()>0)
        QSqlDatabase::removeDatabase("base");

    QStringList drvs(db.drivers());
    if(!drvs.contains("QPSQL")) {
        QMessageBox::critical(0, qApp->tr("Database error"),
            qApp->tr("Unable to establish a database connection.\n"
                     "No PostgreSQL support.\n"), QMessageBox::Cancel);
        return false;
    }
    db = QSqlDatabase::addDatabase("QPSQL", "base");
    db.setHostName(srv);
    db.setPort(port);
    db.setDatabaseName(datab);
    db.setUserName(user);
    db.setPassword(pass);
    setName(db.databaseName());
    if (!db.open()) {
        QMessageBox::critical(0, qApp->tr("Database error"),
            qApp->tr("Couldn't connect to database.\n"
                     "Check connection parameters.\n"), QMessageBox::Cancel);
        return false;
    }
    if (!populate()) {
        QMessageBox::critical(0, qApp->tr("Database error"),
            qApp->tr("Unable to populate with schema(s).\n"
                     "Contact database administrator.\n"), QMessageBox::Cancel);
        return false;
    }
    update();
    return true;
}

bool Database::advance()
{
    if (newPos == pos())
        return false;
    setPos(newPos);
    return true;
}

QVariant Database::itemChange(GraphicsItemChange change, const QVariant &value)
{
    switch (change) {
    case ItemPositionHasChanged:
        foreach (SchemaLink *edge, edgeList)
            edge->adjust();
        break;
    default:
        break;
    }

    return QGraphicsItem::itemChange(change, value);
}

void Database::addEdge(SchemaLink *edge)
{
    edgeList << edge;
    edge->adjust();
}
