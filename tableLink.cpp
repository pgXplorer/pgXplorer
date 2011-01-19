#include <QPainter>
#include "tableLink.h"
#include "schema.h"
#include "table.h"
#include "mainWin.h"
#include <math.h>

TableLink::TableLink(Schema *sourceNode, Table *destNode)
{
    source = sourceNode;
    dest = destNode;
    source->addEdge(this);
    dest->addEdge(this);
    this->setZValue(-100);
    adjust();
}

Schema *TableLink::sourceNode() const
{
    return source;
}

Table *TableLink::destNode() const
{
    return dest;
}

void TableLink::adjust()
{
    if (!source || !dest)
        return;
    QLineF line(0, 0, dest->x(), dest->y());
    //qreal length = line.length();
    prepareGeometryChange();
    sourcePoint = line.p1();
    destPoint = line.p2();
}

QRectF TableLink::boundingRect() const
{
    if (!source || !dest)
        return QRectF();
    qreal penWidth = 1;
    qreal extra = (penWidth)/2.0;
    return QRectF(sourcePoint, QSizeF(destPoint.x() - sourcePoint.x(),
                                      destPoint.y() - sourcePoint.y()))
        .normalized().adjusted(-extra, -extra, extra, extra);
}

void TableLink::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    if (!source || !dest)
        return;
    QLineF line(sourcePoint, destPoint);
    if (qFuzzyCompare(line.length(), qreal(0.)))
        return;
    // Draw the line itself
    painter->setPen(QPen(QColor(200,200,250), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter->drawLine(line);
}
