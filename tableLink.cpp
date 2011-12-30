/*
  LICENSE AND COPYRIGHT INFORMATION - Please read carefully.

  Copyright (c) 2011, davyjones <davyjones@github>

  Permission to use, copy, modify, and/or distribute this software for any
  purpose with or without fee is hereby granted, provided that the above
  copyright notice and this permission notice appear in all copies.

  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include <QPainter>
#include "tableLink.h"
#include "schema.h"
#include "table.h"
#include "mainWin.h"
#include <math.h>

TableLink::TableLink(Schema *source_node, Table *destination_node)
{
    schema = source_node;
    table = destination_node;
    //schema->addEdge(this);
    //table->addEdge(this);
    setParentItem(source_node);
    setFlag(QGraphicsItem::ItemStacksBehindParent);
    setZValue(-100);
    adjust();
}

void TableLink::adjust()
{
    QLineF line(0, 0, table->x(), table->y());
    prepareGeometryChange();
    schema_point = line.p1();
    table_point = line.p2();
}

QRectF TableLink::boundingRect() const
{
    return QRectF(schema_point, QSizeF(table_point.x() - schema_point.x(),
                                      table_point.y() - schema_point.y()))
        .normalized().adjusted(-.5, -.5, .5, .5);
}

void TableLink::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    QLineF line(schema_point, table_point);
    if (qFuzzyCompare(line.length(), qreal(0.)))
        return;
    painter->setPen(QPen(QColor(200,200,250), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter->drawLine(line);
}
