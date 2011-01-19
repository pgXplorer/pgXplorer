#ifndef SCHEMALINK_H
#define SCHEMALINK_H

#include <QGraphicsItem>

class Database;
class Schema;

class SchemaLink : public QGraphicsItem
{
public:
    SchemaLink(Database *sourceNode, Schema *destNode);
    Database *sourceNode() const;
    Schema *destNode() const;
    void adjust();
    enum { Type = UserType + 2 };
    int type() const { return Type; }
    
protected:
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    
private:
    Database *source;
    Schema *dest;

    QPointF sourcePoint;
    QPointF destPoint;
};

#endif // SCHEMALINK_H
