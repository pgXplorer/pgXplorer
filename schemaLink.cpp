#include <QPainter>
#include "schemaLink.h"
#include "database.h"
#include "schema.h"
#include "mainWin.h"
#include <math.h>

SchemaLink::SchemaLink(Database *sourceNode, Schema *destNode)
{
    source = sourceNode;
    dest = destNode;
    source->addEdge(this);
    dest->addEdge(this);
    this->setZValue(-100);
    adjust();
}

Database *SchemaLink::sourceNode() const
{
    return source;
}

Schema *SchemaLink::destNode() const
{
    return dest;
}

void SchemaLink::adjust()
{
    if (!source || !dest)
        return;
    QLineF line(0, 0, dest->x(), dest->y());
    //qreal length = line.length();
    prepareGeometryChange();
    sourcePoint = line.p1();
    destPoint = line.p2();
}

QRectF SchemaLink::boundingRect() const
{
    if (!source || !dest)
        return QRectF();
    qreal penWidth = 1;
    qreal extra = (penWidth)/2.0;
    return QRectF(sourcePoint, QSizeF(destPoint.x() - sourcePoint.x(),
                                      destPoint.y() - sourcePoint.y()))
        .normalized().adjusted(-extra, -extra, extra, extra);
}

void SchemaLink::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    if (!source || !dest)
        return;
    QLineF line(sourcePoint, destPoint);
    if (qFuzzyCompare(line.length(), qreal(0.)))
        return;
    // Draw the line itself
    painter->setPen(QPen(QColor(200,200,200), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter->drawLine(line);
}
