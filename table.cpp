#include "mainWin.h"
#include "database.h"
#include "table.h"
#include "schema.h"

Table::Table(Database* db, Schema* sch, QString tblName)
{
    this->db = db;
    int xs = sch->x();
    int ys = sch->y();
    int siz = sch->getTblList().size();
    int i = sch->getTblList().indexOf(tblName)-(siz/2);
    int radius = 8 * siz;
    if(radius < 100) radius = 100;
    qreal dtheta = 2*M_PI*i/siz;
    this->setParent(sch);
    this->setParentItem(sch);
    //this->setPos(i*radius, (-xs/ys)*i*radius+((radius+sqrt(xs*xs+ys*ys)/(cos(atan(xs/ys))))));
    if(siz/2 == 0)
        this->setPos(radius, radius*sin(dtheta));
        //this->setPos(radius*cos(atan(ys/xs))-radius*sin(dtheta)*sin(atan(ys/xs)),
        //             radius*sin(atan(ys/xs))+radius*sin(dtheta)*cos(atan(ys/xs)));
    else
        this->setPos(radius, radius*sin(dtheta));
        //this->setPos(radius*cos(atan(ys/xs))-radius*cos(dtheta)*sin(atan(ys/xs)),
        //             radius*sin(atan(ys/xs))+radius*cos(dtheta)*cos(atan(ys/xs)));
    this->setName(tblName);
    this->setStatus(false);
    this->setCollapsed(true);
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setToolTip(this->getName());
    setFlag(ItemSendsGeometryChanges);
    setCacheMode(DeviceCoordinateCache);
    setZValue(-1);
}

void Table::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QMenu menu;
    menu.setStyleSheet("QMenu { font-size:12px; width: 100px; color:white; left: 20px; background-color:qlineargradient(x1:0, y1:0, x2:0, y2:1, stop: 0 #cccccc, stop: 1 #333333);}");
    menu.addAction(tr("View"));
    menu.addSeparator();
    menu.addAction(tr("Delete"));
    menu.addAction("Action 1");
    QAction *a = menu.exec(event->screenPos());
    if(a && QString::compare(a->text(),"Delete") == 0)
    {
        //this;
    }
}

void Table::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *)
{
    /*
    if(this->getCollapsed())
    {
        this->setCollapsed(false);
        this->view->show();;
    }
    else
    {
        this->setCollapsed(true);
        this->view->raise();
    }*/
    /*
    if(this->getCollapsed())
    {
        this->setCollapsed(false);
        emit expand(this);
    }
    else
    {
        this->setCollapsed(true);
        emit collapse(this);
    }*/
    emit expand(this->db, this->getParent(), this);
    update();
}

void Table::showView(Database* db, Schema* sch, Table *tbl)
{
    LaunchTable* lt = new LaunchTable;
    lt->start();
    lt->showTbl(db, sch, tbl);
}

void Table::hideView(Database*, Schema*, Table*)
{

}

bool Table::advance()
{
    if (newPos == pos())
        return false;
    setPos(newPos);
    return true;
}

QVariant Table::itemChange(GraphicsItemChange change, const QVariant &value)
{
    switch (change) {
    case ItemPositionHasChanged:
        foreach (TableLink *edge, edgeList)
            edge->adjust();
        break;
    default:
        break;
    };
    return QGraphicsItem::itemChange(change, value);
}

void Table::addEdge(TableLink *edge)
{
    edgeList << edge;
    edge->adjust();
}

QList<TableLink *> Table::edges() const
{
    return edgeList;
}
