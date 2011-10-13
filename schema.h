#ifndef SCHEMA_H
#define SCHEMA_H

#include <QMouseEvent>
#include <QSqlDatabase>
#include <QtGui>
#include "schemaLink.h"
#include "tableLink.h"

#define SCHEMA_WIDTH 70
#define SCHEMA_HEIGHT 40
#define A_RADIUS (SCHEMA_WIDTH*5)
#define B_RADIUS (SCHEMA_HEIGHT*6)

class Schema : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

private:
    QList<QString> tblList;
    Database* dbPar;
    QString name;
    bool status;
    bool collapsed;
    QList<SchemaLink *> edgeList;
    QList<TableLink *> linkList;
    QPointF newPos;
protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);
public:
    enum { Type = 10101 };
    int type() const
    {
        return Type;
    }
    Schema(Database* parent, QString schName);
    ~Schema(){};
    bool advance();
    void addEdge(SchemaLink *edge);
    void addEdge(TableLink *edge);
    QList<SchemaLink *> dblink() const;
    QList<TableLink *> tablelinks() const;
    QRectF boundingRect() const
    {
        return QRectF(-35, -30, 80, 50);
    }
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
               QWidget* widget)
    {
        if(this->isSelected())
            painter->setPen(QColor(100,100,100));
        else
            painter->setPen(QColor(0,0,0,0));
        if(this->getCollapsed()) {
            painter->setBrush(QColor(200,200,200));
            painter->drawRect(-35,-20,SCHEMA_WIDTH,SCHEMA_HEIGHT);
            painter->setBrush(QColor(190,190,190));
            QPointF right[4] = {
                 QPointF(+35,-20),
                 QPointF(+35,+20),
                 QPointF(+40,+10),
                 QPointF(+40,-30)
             };
            painter->drawPolygon(right, 4);
            painter->setBrush(QColor(220,220,220));
            QPointF top[4] = {
                 QPointF(-25,-30),
                 QPointF(-35,-20),
                 QPointF(+35,-20),
                 QPointF(+40,-30)
             };
            painter->drawPolygon(top, 4);
            painter->setPen(Qt::darkGray);
            QPointF textPos(-3*this->name.left(8).length(),+5);
            painter->drawText(textPos, this->name.left(8));
        }
        else {
            painter->setBrush(QColor(150,150,200));
            painter->drawRect(-35,-20,SCHEMA_WIDTH,SCHEMA_HEIGHT);
            painter->setBrush(QColor(120,120,150));
            QPointF right[4] = {
                 QPointF(+35,-20),
                 QPointF(+35,+20),
                 QPointF(+40,+10),
                 QPointF(+40,-30)
             };
            painter->drawPolygon(right, 4);
            painter->setBrush(QColor(200,200,250));
            QPointF top[4] = {
                 QPointF(-25,-30),
                 QPointF(-35,-20),
                 QPointF(+35,-20),
                 QPointF(+40,-30)
             };
            painter->drawPolygon(top, 4);
            painter->setPen(QColor(50,50,100));
            QPointF textPos(-3*this->name.left(8).length(),+5);
            painter->drawText(textPos, this->name.left(8));
        }
        painter->setRenderHint(QPainter::Antialiasing, true);
    }
    QString getName()
    {
        return this->name;
    }
    void setName(QString name)
    {
        this->name = name;
    }
    bool getCollapsed()
    {
        return this->collapsed;
    }
    void setCollapsed(bool collapsed)
    {
        this->collapsed = collapsed;
    }
    bool getStatus()
    {
        return this->status;
    }
    void setStatus(bool status)
    {
        this->status = status;
    }
    void setRt(int x, int y, qreal radius, qreal dtheta)
    {
        this->setX(x + radius*sin(dtheta));
        this->setY(y + radius*cos(dtheta));
    }
    Database* getParent()
    {
        return this->dbPar;
    }
    void setParent(Database* db)
    {
        this->dbPar = db;
    }
    QList<QString> getTblList()
    {
        return this->tblList;
    }
    void setTblList(QList<QString> tblList)
    {
        this->tblList = tblList;
    }
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *);
    virtual void contextMenuEvent ( QGraphicsSceneContextMenuEvent *);

Q_SIGNALS:
    void expand(Schema*);
    void collapse(Schema*);
};

#endif // SCHEMA_H