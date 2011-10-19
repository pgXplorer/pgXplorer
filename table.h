#ifndef TABLE_H
#define TABLE_H

#include <QMouseEvent>
#include <QSqlTableModel>
#include <QtGui>
#include "database.h"
#include "tableLink.h"

#define TABLE_WIDTH 80
#define TABLE_HEIGHT 20

class Table : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

private:
    Database* db;
    QSqlQueryModel* model;
    Schema* schPar;
    QString name;
    bool status;
    bool collapsed;
    QList<TableLink *> edgeList;
    QPointF newPos;
protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);
public:
    enum { Type = 10102 };
    int type() const
    {
        return Type;
    }
    Table(Database* db, Schema* parent, QString tblName);
    ~Table(){};
    bool advance();
    void addEdge(TableLink *edge);
    QList<TableLink *> edges() const;
    QRectF boundingRect() const
    {
        return QRectF(-40,-20,85,30);
    }
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
               QWidget* widget)
    {
        if(this->isSelected())
            painter->setPen(QColor(100,50,50));
        else
            painter->setPen(QColor(0,0,0,0));
        if(!this->getCollapsed()) {
            painter->setBrush(QColor(220,220,220));
            painter->drawRect(-40,-10,TABLE_WIDTH,TABLE_HEIGHT);
            painter->setBrush(QColor(200,200,200));
            QPointF right[4] = {
                 QPointF(+40,-10),
                 QPointF(+40,+10),
                 QPointF(+45,0),
                 QPointF(+45,-20)
             };
            painter->drawPolygon(right, 4);
            painter->setBrush(QColor(210,210,210));
            QPointF top[4] = {
                 QPointF(-30,-20),
                 QPointF(-40,-10),
                 QPointF(+40,-10),
                 QPointF(+45,-20)
             };
            painter->drawPolygon(top, 4);
            painter->setPen(Qt::darkGray);
            QPointF textPos(-6*this->name.left(6).length(),+5);
            painter->drawText(textPos, this->name.length()>6?this->name.left(6)+
                              "..":this->name);
        }
        else {
            painter->setBrush(QColor(200,150,150));
            painter->drawRect(-40,-10,TABLE_WIDTH,TABLE_HEIGHT);
            painter->setBrush(QColor(230,200,200));
            QPointF right[4] = {
                QPointF(+40,-10),
                QPointF(+40,+10),
                QPointF(+45,0),
                QPointF(+45,-20)
             };
            painter->drawPolygon(right, 4);
            painter->setBrush(QColor(250,220,220));
            QPointF top[4] = {
                QPointF(-30,-20),
                QPointF(-40,-10),
                QPointF(+40,-10),
                QPointF(+45,-20)
             };
            painter->drawPolygon(top, 4);
            painter->setPen(QColor(100,50,50));
            QPointF textPos(-6*this->name.left(6).length(),+5);
            painter->drawText(textPos, this->name.length()>6?this->name.left(6)+
                              "..":this->name);
        }
        painter->setRenderHint(QPainter::Antialiasing, true);
    }
    void showTable(Table*){};
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
    void setParent(Schema* sch)
    {
        this->schPar = sch;
    }
    Schema* getParent()
    {
        return this->schPar;
    }
    QSqlQueryModel* getModel()
    {
        return this->model;
    }
    void setModel(QSqlQueryModel* model)
    {
        this->model = model;
    }
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *);
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *);
    void showView2(Table*);

private slots:
    void showView(Database*, Schema*, Table*);
    void hideView(Database*, Schema*, Table*);

Q_SIGNALS:
    void expand(Database*, Schema*, Table*);
    void collapse(Database*, Schema*, Table*);
};

#endif // TABLE_H
