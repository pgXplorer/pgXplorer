#include "schema.h"
#include "database.h"

Schema::Schema(Database* db, QString schName)
{
    int aradius = A_RADIUS;
    int bradius = B_RADIUS;
    qreal dtheta;
    int siz = db->getSchList().size();
    int i = db->getSchList().indexOf(schName);
    dtheta = -2*M_PI*i/siz - M_PI_2;
    setParent(db);
    setParentItem(db);
    setPos(aradius*sin(dtheta), bradius*cos(dtheta));
    setName(schName);
    setStatus(false);
    setCollapsed(true);
    QList<QString> tbllist;
    QSqlQuery query;
    //query.prepare("SELECT tablename FROM pg_tables WHERE schemaname=?");
    //query.addBindValue(schName);
    //query.exec();
    query.exec("SELECT tablename FROM pg_tables WHERE schemaname='" + schName + "'");
    while (query.next())
         tbllist << query.value(0).toString();
    setTblList(tbllist);
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setToolTip(this->getName());
    setFlag(ItemSendsGeometryChanges);
    setCacheMode(DeviceCoordinateCache);
    setZValue(-1);
}

void Schema::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *)
{
    if(getCollapsed()) {
        this->setCollapsed(false);
        emit expand(this);
    }
    else {
        this->setCollapsed(true);
        emit collapse(this);
    }
    update();
}

void Schema::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QMenu menu;
    //menu.setBackgroundColor(QColor(205,205,205));
    menu.addAction("Expand");
    menu.addAction("Delete");
    QAction *a = menu.exec(event->screenPos());
    if(a && QString::compare(a->text(),"Delete") == 0) {
        //this;
    }
}

bool Schema::advance()
{
    if (newPos == pos())
        return false;
    setPos(newPos);
    return true;
}

QVariant Schema::itemChange(GraphicsItemChange change, const QVariant &value)
{
    switch (change) {
    case ItemPositionHasChanged:
        foreach (SchemaLink *edge, edgeList)
            edge->adjust();
        break;
    default:
        break;
    };

    return QGraphicsItem::itemChange(change, value);
}

void Schema::addEdge(SchemaLink *edge)
{
    edgeList << edge;
    edge->adjust();
}

void Schema::addEdge(TableLink *edge)
{
    linkList << edge;
    edge->adjust();
}

QList<SchemaLink *> Schema::dblink() const
{
    return edgeList;
}

QList<TableLink *> Schema::tablelinks() const
{
    return linkList;
}