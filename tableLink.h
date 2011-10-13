#ifndef TABLELINK_H
#define TABLELINK_H

#include <QGraphicsItem>

class Schema;
class Table;

class TableLink : public QGraphicsItem
{
public:
    TableLink(Schema *sourceNode, Table *destNode);
    Schema *sourceNode() const;
    Table *destNode() const;
    void adjust();
    enum { Type = UserType + 3 };
    int type() const { return Type; }

protected:
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
    Schema *source;
    Table *dest;
    QPointF sourcePoint;
    QPointF destPoint;
};

#endif // TABLELINK_H